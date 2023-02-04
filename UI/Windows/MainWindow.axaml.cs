using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.ViewModels;
using Mesen.Config;
using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.IO;
using System.Linq;
using Mesen.Utilities;
using Mesen.Interop;
using Mesen.Views;
using Avalonia.Layout;
using Mesen.Debugger.Utilities;
using System.ComponentModel;
using System.Threading;
using Mesen.Debugger.Windows;
using Avalonia.Input.Platform;
using System.Collections.Generic;
using Mesen.Controls;

namespace Mesen.Windows
{
	public class MainWindow : Window
	{
		private DispatcherTimer _timerBackgroundFlag = new DispatcherTimer();
		private MainWindowViewModel _model = null!;

		private NotificationListener? _listener = null;
		private ShortcutHandler _shortcutHandler;

		private FrameInfo _baseScreenSize;
		private MouseManager _mouseManager;
		private NativeRenderer _renderer;
		private SoftwareRendererView _softwareRenderer;
		private ContentControl _audioPlayer;
		private MainMenuView _mainMenu;
		private CommandLineHelper? _cmdLine;

		private bool _testModeEnabled;
		private bool _needResume = false;

		private Size _originalSize;
		private PixelPoint _originalPos;
		private WindowState _prevWindowState;

		//Used to suppress key-repeat keyup events on Linux
		private Dictionary<Key, IDisposable> _pendingKeyUpEvents = new();
		private bool _isLinux = false;

		static MainWindow()
		{
			WindowStateProperty.Changed.AddClassHandler<MainWindow>((x, e) => x.OnWindowStateChanged());
			IsActiveProperty.Changed.AddClassHandler<MainWindow>((x, e) => x.OnActiveChanged());
		}

		public MainWindow()
		{
			_testModeEnabled = System.Diagnostics.Debugger.IsAttached;
			_isLinux = RuntimeInformation.IsOSPlatform(OSPlatform.Linux);

			DataContext = new MainWindowViewModel();
			InitGlobalShortcuts();

			EmuApi.InitDll();

			Directory.CreateDirectory(ConfigManager.HomeFolder);
			Directory.SetCurrentDirectory(ConfigManager.HomeFolder);

			InitializeComponent();

			_shortcutHandler = new ShortcutHandler(this);

			AddHandler(DragDrop.DropEvent, OnDrop);

			//Allows us to catch LeftAlt/RightAlt key presses
			AddHandler(InputElement.KeyDownEvent, OnPreviewKeyDown, RoutingStrategies.Tunnel, true);
			AddHandler(InputElement.KeyUpEvent, OnPreviewKeyUp, RoutingStrategies.Tunnel, true);

			_renderer = this.GetControl<NativeRenderer>("Renderer");
			_softwareRenderer = this.GetControl<SoftwareRendererView>("SoftwareRenderer");
			_audioPlayer = this.GetControl<ContentControl>("AudioPlayer");
			_mainMenu = this.GetControl<MainMenuView>("MainMenu");
			_mouseManager = new MouseManager(this, _renderer, _mainMenu);
			ConfigManager.Config.MainWindow.LoadWindowSettings(this);

#if DEBUG
			this.AttachDevTools();
#endif
		}

		private static void InitGlobalShortcuts()
		{
			PlatformHotkeyConfiguration hotkeyConfig = AvaloniaLocator.Current.GetRequiredService<PlatformHotkeyConfiguration>();
			List<KeyGesture> gestures = hotkeyConfig.OpenContextMenu;
			for(int i = gestures.Count - 1; i >= 0; i--) {
				if(gestures[i].Key == Key.F10 && gestures[i].KeyModifiers == KeyModifiers.Shift) {
					//Disable Shift-F10 shortcut to open context menu - interferes with default shortcut for step back
					gestures.RemoveAt(i);
				}
			}
			hotkeyConfig.Copy.Add(new KeyGesture(Key.Insert, KeyModifiers.Control));
			hotkeyConfig.Paste.Add(new KeyGesture(Key.Insert, KeyModifiers.Shift));
			hotkeyConfig.Cut.Add(new KeyGesture(Key.Delete, KeyModifiers.Shift));
		}

		protected override void ArrangeCore(Rect finalRect)
		{
			//TODOv2 why is this needed to make resizing the window by setting ClientSize work?
			base.ArrangeCore(new Rect(ClientSize));
		}

		private bool _needCloseValidation = true;
		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			if(_needCloseValidation) {
				e.Cancel = true;
				ValidateExit();
			} else {
				//Close all other windows first
				DebugWindowManager.CloseAllWindows();
				foreach(Window wnd in ApplicationHelper.GetOpenedWindows()) {
					if(wnd != this) {
						wnd.Close();
					}
				}

				if(ApplicationHelper.GetOpenedWindows().Count > 1) {
					e.Cancel = true;
					return;
				}

				_timerBackgroundFlag.Stop();
				EmuApi.Stop();
				ConfigManager.Config.MainWindow.SaveWindowSettings(this);
				ConfigManager.Config.Save();
			}
		}

		private async void ValidateExit()
		{
			if(!ConfigManager.Config.Preferences.ConfirmExitResetPower || await MesenMsgBox.Show(null, "ConfirmExit", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes) {
				_needCloseValidation = false;
				Close();
			}
		}

		protected override void OnClosed(EventArgs e)
		{
			base.OnClosed(e);
			_mouseManager.Dispose();
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is MainWindowViewModel model) {
				_model = model;
			}
		}

		private void ResizeRenderer()
		{
			_renderer.InvalidateMeasure();
			_renderer.InvalidateArrange();
		}

		private void OnDrop(object? sender, DragEventArgs e)
		{
			string? filename = e.Data.GetFileNames()?.FirstOrDefault();
			if(filename != null) {
				if(File.Exists(filename)) {
					LoadRomHelper.LoadFile(filename);
					Activate();
				} else {
					EmuApi.DisplayMessage("Error", "File not found: " + filename);
				}
			}
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			ConfigManager.Config.Preferences.ApplyFontOptions();
			ConfigManager.Config.Debug.Fonts.ApplyConfig();

			_timerBackgroundFlag.Interval = TimeSpan.FromMilliseconds(200);
			_timerBackgroundFlag.Tick += timerUpdateBackgroundFlag;
			_timerBackgroundFlag.Start();

			Task.Run(() => {
				//Load all styles after 15ms to let the UI refresh once with the startup styles
				System.Threading.Thread.Sleep(15);
				Dispatcher.UIThread.Post(() => {
					StyleHelper.ApplyTheme(ConfigManager.ActiveTheme);
				});
			});
			
			Task.Run(() => {
				CommandLineHelper cmdLine = new CommandLineHelper(Program.CommandLineArgs, true);
				_cmdLine = cmdLine;

				EmuApi.InitializeEmu(ConfigManager.HomeFolder, PlatformImpl?.Handle.Handle ?? IntPtr.Zero, _renderer.Handle, cmdLine.NoAudio, cmdLine.NoVideo, cmdLine.NoInput);

				ConfigManager.Config.RemoveObsoleteConfig();
				
				//InitializeDefaults must be after InitializeEmu, otherwise keybindings will be empty
				ConfigManager.Config.InitializeDefaults();

				_baseScreenSize = EmuApi.GetBaseScreenSize();
				_listener = new NotificationListener();
				_listener.OnNotification += OnNotification;

				_model.Init(this);

				ConfigManager.Config.ApplyConfig();

				if(ConfigManager.Config.Preferences.OverrideGameFolder && Directory.Exists(ConfigManager.Config.Preferences.GameFolder)) {
					EmuApi.AddKnownGameFolder(ConfigManager.Config.Preferences.GameFolder);
				}
				foreach(RecentItem recentItem in ConfigManager.Config.RecentFiles.Items) {
					EmuApi.AddKnownGameFolder(recentItem.RomFile.Folder);
				}

				ConfigManager.Config.Preferences.UpdateFileAssociations();
				SingleInstance.Instance.ArgumentsReceived += Instance_ArgumentsReceived;

				Dispatcher.UIThread.Post(() => {
					cmdLine.LoadFiles();
					cmdLine.OnAfterInit(this);

					//Load the debugger window styles once everything else is done
					StyleHelper.LoadDebuggerStyles();

					if(ConfigManager.Config.Preferences.AutomaticallyCheckForUpdates) {
						_model.MainMenu.CheckForUpdate(this, true);
					}
				});
			});
		}

		private void Instance_ArgumentsReceived(object? sender, ArgumentsReceivedEventArgs e)
		{
			Dispatcher.UIThread.Post(() => {
				CommandLineHelper cmdLine = new(e.Args, false);
				ConfigManager.Config.ApplyConfig();
				cmdLine.LoadFiles();
			});
		}

		private void OnNotification(NotificationEventArgs e)
		{
			DebugWindowManager.ProcessNotification(e);

			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					CheatCodes.ApplyCheats();
					RomInfo romInfo = EmuApi.GetRomInfo();
					Dispatcher.UIThread.Post(() => {
						_model.RomInfo = romInfo;
					});
					GameConfig.LoadGameConfig(romInfo).ApplyConfig();

					GameLoadedEventParams evtParams = Marshal.PtrToStructure<GameLoadedEventParams>(e.Parameter);
					if(!evtParams.IsPowerCycle) {
						Dispatcher.UIThread.Post(() => {
							_model.RecentGames.Visible = false;

							DispatcherTimer.RunOnce(() => {
								if(_cmdLine != null) {
									_cmdLine?.ProcessPostLoadCommandSwitches(this);
									_cmdLine = null;
								}

								if(WindowState == WindowState.FullScreen || WindowState == WindowState.Maximized) {
									//Force resize of renderer when loading a game while in fullscreen
									//Prevents some issues when fullscreen was turned on before loading a game, etc.
									_renderer.Width = double.NaN;
									_renderer.Height = double.NaN;
									ResizeRenderer();
								}
							}, TimeSpan.FromMilliseconds(50));
						});
					}

					Dispatcher.UIThread.Post(() => {
						ApplicationHelper.GetExistingWindow<HdPackBuilderWindow>()?.Close();
					});
					break;

				case ConsoleNotificationType.DebuggerResumed:
				case ConsoleNotificationType.GameResumed:
					Dispatcher.UIThread.Post(() => {
						_model.RecentGames.Visible = false;
					});
					break;

				case ConsoleNotificationType.RequestConfigChange:
					Dispatcher.UIThread.Post(() => {
						UpdateInputConfiguration();
					});
					break;

				case ConsoleNotificationType.EmulationStopped:
					Dispatcher.UIThread.Post(() => {
						_model.RomInfo = new RomInfo();
						_model.RecentGames.Init(GameScreenMode.RecentGames);
					});
					break;

				case ConsoleNotificationType.ResolutionChanged:
					Dispatcher.UIThread.Post(() => {
						ProcessResolutionChange();
					});
					break;

				case ConsoleNotificationType.ExecuteShortcut:
					ExecuteShortcutParams p = Marshal.PtrToStructure<ExecuteShortcutParams>(e.Parameter);
					Dispatcher.UIThread.Post(() => {
						_shortcutHandler.ExecuteShortcut(p.Shortcut);
					});
					break;

				case ConsoleNotificationType.MissingFirmware:
					MissingFirmwareMessage msg = Marshal.PtrToStructure<MissingFirmwareMessage>(e.Parameter);
					TaskCompletionSource tcs = new TaskCompletionSource();
					Dispatcher.UIThread.Post(async () => {
						await FirmwareHelper.RequestFirmwareFile(msg);
						tcs.SetResult();
					});
					tcs.Task.Wait();
					break;

				case ConsoleNotificationType.BeforeGameLoad:
					Dispatcher.UIThread.Post(() => {
						ApplicationHelper.GetExistingWindow<HdPackBuilderWindow>()?.Close();
					});
					break;

				case ConsoleNotificationType.RefreshSoftwareRenderer:
					SoftwareRendererFrame frame = Marshal.PtrToStructure<SoftwareRendererFrame>(e.Parameter);
					_softwareRenderer.UpdateSoftwareRenderer(frame);
					break;
			}
		}

		private static void UpdateInputConfiguration()
		{
			//Used to update input devices when the core requests changes (NES-only for now)
			ConfigManager.Config.Nes.UpdateInputFromCoreConfig();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void ProcessResolutionChange()
		{
			double dpiScale = LayoutHelper.GetLayoutScale(this);
			double xScale = ClientSize.Width * dpiScale / _baseScreenSize.Width;
			double yScale = ClientSize.Height * dpiScale / _baseScreenSize.Height;
			SetScale(Math.Min(Math.Round(xScale), Math.Round(yScale)));
			_baseScreenSize = EmuApi.GetBaseScreenSize();
		}

		public void SetScale(double scale)
		{
			//TODOv2 - Calling this twice seems to fix what might be an issue in Avalonia?
			//On the first call, when DPI > 100%, sometimes the "finalSize" received by
			//ArrangeOverride in NativeRenderer does not match what was given here
			InternalSetScale(scale);
			InternalSetScale(scale);
		}

		private void InternalSetScale(double scale)
		{
			double dpiScale = LayoutHelper.GetLayoutScale(this);
			scale /= dpiScale;

			FrameInfo screenSize = EmuApi.GetBaseScreenSize();
			if(WindowState == WindowState.Normal) {
				_renderer.Width = double.NaN;
				_renderer.Height = double.NaN;

				double aspectRatio = EmuApi.GetAspectRatio();

				//When menu is set to auto-hide, don't count its height when calculating the window's final size
				double menuHeight = ConfigManager.Config.Preferences.AutoHideMenu ? 0 : _mainMenu.Bounds.Height;

				double width = Math.Max(MinWidth, Math.Round(screenSize.Height * scale * aspectRatio));
				double height = Math.Max(MinHeight, Math.Round(screenSize.Height * scale));
				ClientSize = new Size(width, height + menuHeight + _audioPlayer.Bounds.Height);
				_model.SoftwareRenderer.Width = width;
				_model.SoftwareRenderer.Height = height;
				ResizeRenderer();
			} else if(WindowState == WindowState.Maximized || WindowState == WindowState.FullScreen) {
				_renderer.Width = Math.Round(screenSize.Width * scale);
				_renderer.Height = Math.Round(screenSize.Height * scale);
				_model.SoftwareRenderer.Width = screenSize.Width * scale;
				_model.SoftwareRenderer.Height = screenSize.Height * scale;
			}
		}

		private void OnWindowStateChanged()
		{
			_renderer.Width = double.NaN;
			_renderer.Height = double.NaN;
			ResizeRenderer();
	
			UpdateSoftwareRendererSize();
		}

		private void UpdateSoftwareRendererSize()
		{
			if(_model.SoftwareRenderer.FrameSurface != null) {
				FrameInfo screenSize = EmuApi.GetBaseScreenSize();
				bool isFullscreen = WindowState == WindowState.FullScreen;
				double menuHeight = (isFullscreen || ConfigManager.Config.Preferences.AutoHideMenu) ? 0 : _mainMenu.Bounds.Height;
				double height = Height - menuHeight - _audioPlayer.Bounds.Height;
				double scale = height / screenSize.Height;
				if(ConfigManager.Config.Video.FullscreenForceIntegerScale && (WindowState == WindowState.Maximized || isFullscreen)) {
					scale = Math.Floor(scale);
				}
				SetScale(scale);
			}
		}

		public void ToggleFullscreen()
		{
			if(WindowState == WindowState.FullScreen) {
				WindowState = _prevWindowState;
				if(ConfigManager.Config.Video.UseExclusiveFullscreen) {
					EmuApi.SetExclusiveFullscreenMode(false, _renderer.Handle);
				}
				RestoreOriginalWindowSize();
			} else {
				_originalSize = ClientSize;
				_originalPos = Position;
				if(ConfigManager.Config.Video.UseExclusiveFullscreen) {
					if(!EmuApi.IsRunning()) {
						//Prevent entering fullscreen mode until a game is loaded
						return;
					}
					EmuApi.SetExclusiveFullscreenMode(true, PlatformImpl?.Handle.Handle ?? IntPtr.Zero);
				}
				_prevWindowState = WindowState;
				WindowState = WindowState.FullScreen;
			}
		}

		private void RestoreOriginalWindowSize()
		{
			Task.Run(() => {
				Thread.Sleep(30);
				Dispatcher.UIThread.Post(() => {
					if(WindowState == WindowState.Normal) {
						ClientSize = _originalSize;
						Position = _originalPos;
					}
				});
				Thread.Sleep(100);
				Dispatcher.UIThread.Post(() => {
					if(WindowState == WindowState.Normal) {
						ClientSize = _originalSize;
						Position = _originalPos;
					}
				});
			});
		}

		protected override void OnLostFocus(RoutedEventArgs e)
		{
			base.OnLostFocus(e);
			if(WindowState == WindowState.FullScreen && ConfigManager.Config.Video.UseExclusiveFullscreen) {
				ToggleFullscreen();
			}
		}

		private bool ProcessTestModeShortcuts(Key key)
		{
			if(key == Key.F1) {
				if(TestApi.RomTestRecording()) {
					TestApi.RomTestStop();
				} else {
					RomTestHelper.RecordTest();
				}
				return true;
			} else if(key == Key.F2) {
				RomTestHelper.RunTest();
				return true;
			} else if(key == Key.F3) {
				RomTestHelper.RunAllTests();
				return true;
			}
			return false;
		}

		private void OnPreviewKeyDown(object? sender, KeyEventArgs e)
		{
			if(_testModeEnabled && e.KeyModifiers == KeyModifiers.Alt && ProcessTestModeShortcuts(e.Key)) {
				return;
			}

			if(e.Key != Key.None) {
				if(_isLinux && _pendingKeyUpEvents.TryGetValue(e.Key, out IDisposable? cancelTimer)) {
					//Cancel any pending key up event
					cancelTimer.Dispose();
					_pendingKeyUpEvents.Remove(e.Key);
				}

				InputApi.SetKeyState((UInt16)e.Key, true);
			}

			if(e.Key == Key.Tab || e.Key == Key.F10) {
				//Prevent menu/window from handling these keys to avoid issue with custom shortcuts
				e.Handled = true;
			}
		}

		private void OnPreviewKeyUp(object? sender, KeyEventArgs e)
		{
			if(e.Key != Key.None) {
				if(_isLinux) {
					//Process keyup events after 1ms on Linux to prevent key repeat from triggering key up/down repeatedly
					IDisposable cancelTimer = DispatcherTimer.RunOnce(() => InputApi.SetKeyState((UInt16)e.Key, false), TimeSpan.FromMilliseconds(1), DispatcherPriority.MaxValue);
					_pendingKeyUpEvents[e.Key] = cancelTimer;
				} else {
					InputApi.SetKeyState((UInt16)e.Key, false);
				}
			}
		}

		private void OnActiveChanged()
		{
			ConfigApi.SetEmulationFlag(EmulationFlags.InBackground, !IsActive);
			InputApi.ResetKeyState();
		}

		private void timerUpdateBackgroundFlag(object? sender, EventArgs e)
		{
			Window? activeWindow = ApplicationHelper.GetActiveWindow();

			PreferencesConfig cfg = ConfigManager.Config.Preferences;

			bool needPause = activeWindow == null && cfg.PauseWhenInBackground;
			if(activeWindow != null) {
				bool isConfigWindow = (activeWindow != this) && !DebugWindowManager.IsDebugWindow(activeWindow);
				needPause |= cfg.PauseWhenInMenusAndConfig && !isConfigWindow && _mainMenu.MainMenu.IsOpen; //in main menu
				needPause |= cfg.PauseWhenInMenusAndConfig && isConfigWindow; //in a window that's neither the main window nor a debug tool
			}

			if(needPause) {
				if(!EmuApi.IsPaused()) {
					_needResume = true;

					DebuggerWindow? wnd = DebugWindowManager.GetDebugWindow<DebuggerWindow>(x => x.CpuType == _model.RomInfo.ConsoleType.GetMainCpuType());
					if(wnd != null) {
						//If the debugger window for the main cpu is opened, suppress the "bring to front on break" behavior
						wnd.SuppressBringToFront();
					}

					EmuApi.Pause();
				}
			} else if(_needResume) {
				EmuApi.Resume();
				_needResume = false;
			}
		}
	}
}
