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
		public NativeRenderer _renderer;
		public MainMenuView _mainMenu;

		static MainWindow()
		{
			WindowStateProperty.Changed.AddClassHandler<MainWindow>((x, e) => x.OnWindowStateChanged());
			IsActiveProperty.Changed.AddClassHandler<MainWindow>((x, e) => x.OnActiveChanged());
		}

		public MainWindow()
		{
			DataContext = new MainWindowViewModel();

			EmuApi.InitDll();

			Directory.CreateDirectory(ConfigManager.HomeFolder);
			Directory.SetCurrentDirectory(ConfigManager.HomeFolder);

			InitializeComponent();

			_shortcutHandler = new ShortcutHandler(this);

			AddHandler(DragDrop.DropEvent, OnDrop);

			//Allows us to catch LeftAlt/RightAlt key presses
			AddHandler(InputElement.KeyDownEvent, OnPreviewKeyDown, RoutingStrategies.Tunnel, true);
			AddHandler(InputElement.KeyUpEvent, OnPreviewKeyUp, RoutingStrategies.Tunnel, true);

			_renderer = this.FindControl<NativeRenderer>("Renderer");
			_mainMenu = this.FindControl<MainMenuView>("MainMenu");
			_mouseManager = new MouseManager(this, _renderer, _mainMenu);
			ConfigManager.Config.MainWindow.LoadWindowSettings(this);

#if DEBUG
			this.AttachDevTools();
#endif
		}

		protected override void ArrangeCore(Rect finalRect)
		{
			//TODO why is this needed to make resizing the window by setting ClientSize work?
			base.ArrangeCore(new Rect(ClientSize));
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_timerBackgroundFlag.Stop();
			ConfigManager.Config.MainWindow.SaveWindowSettings(this);
		}

		protected override void OnClosed(EventArgs e)
		{
			base.OnClosed(e);
			ConfigManager.Config.Save();
			EmuApi.Release();
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

			_timerBackgroundFlag.Interval = TimeSpan.FromMilliseconds(200);
			_timerBackgroundFlag.Tick += timerUpdateBackgroundFlag;
			_timerBackgroundFlag.Start();

			Task.Run(() => {
				//Load all styles after 15ms to let the UI refresh once with the startup styles
				System.Threading.Thread.Sleep(15);
				Dispatcher.UIThread.Post(() => {
					StyleHelper.ApplyTheme(ConfigManager.Config.Preferences.Theme);
				});
			});
			
			Task.Run(() => {
				EmuApi.InitializeEmu(ConfigManager.HomeFolder, PlatformImpl.Handle.Handle, _renderer.Handle, false, false, false);
				_baseScreenSize = EmuApi.GetBaseScreenSize();
				_listener = new NotificationListener();
				_listener.OnNotification += OnNotification;

				_model.Init(this);
				ConfigManager.Config.InitializeDefaults();
				ConfigManager.Config.ApplyConfig();

				ProcessCommandLineArgs(Program.CommandLineArgs);

				ConfigManager.Config.Preferences.UpdateFileAssociations();
				SingleInstance.Instance.ArgumentsReceived += Instance_ArgumentsReceived;

				Dispatcher.UIThread.Post(() => {
					//Load the debugger window styles once everything else is done
					StyleHelper.LoadDebuggerStyles();
				});
			});
		}

		private void Instance_ArgumentsReceived(object? sender, ArgumentsReceivedEventArgs e)
		{
			ProcessCommandLineArgs(e.Args);
		}

		private bool ProcessCommandLineArgs(string[] args)
		{
			foreach(string arg in args) {
				if(File.Exists(arg)) {
					LoadRomHelper.LoadFile(arg);
					return true;
				}
			}
			return false;
		}

		private void OnNotification(NotificationEventArgs e)
		{
			DebugWindowManager.ProcessNotification(e);

			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					RomInfo romInfo = EmuApi.GetRomInfo();
					Dispatcher.UIThread.Post(() => {
						_model.RecentGames.Visible = false;
						_model.RomInfo = romInfo;
					});
					break;

				case ConsoleNotificationType.DebuggerResumed:
				case ConsoleNotificationType.GameResumed:
					Dispatcher.UIThread.Post(() => {
						_model.RecentGames.Visible = false;
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
						double dpiScale = LayoutHelper.GetLayoutScale(this);
						double scale = ClientSize.Width * dpiScale / _baseScreenSize.Width;
						SetScale(scale);
						_baseScreenSize = EmuApi.GetBaseScreenSize();
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
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public void SetScale(double scale)
		{
			double dpiScale = LayoutHelper.GetLayoutScale(this);
			scale /= dpiScale;

			FrameInfo screenSize = EmuApi.GetBaseScreenSize();
			if(WindowState == WindowState.Normal) {
				_renderer.Width = double.NaN;
				_renderer.Height = double.NaN;

				double aspectRatio = EmuApi.GetAspectRatio();
				ClientSize = new Size(screenSize.Width * scale, screenSize.Width * scale / aspectRatio + _mainMenu.Bounds.Height);
				ResizeRenderer();
			} else if(WindowState == WindowState.Maximized || WindowState == WindowState.FullScreen) {
				_renderer.Width = screenSize.Width * scale;
				_renderer.Height = screenSize.Height * scale;
			}
		}

		private void OnWindowStateChanged()
		{
			if(WindowState == WindowState.Normal) {
				_renderer.Width = double.NaN;
				_renderer.Height = double.NaN;
				ResizeRenderer();
			}

			_mainMenu.IsVisible = WindowState != WindowState.FullScreen;
		}

		public void ToggleFullscreen()
		{
			if(WindowState == WindowState.FullScreen) {
				WindowState = WindowState.Normal;
			} else {
				WindowState = WindowState.FullScreen;
			}
		}

		private void OnPreviewKeyDown(object? sender, KeyEventArgs e)
		{
			InputApi.SetKeyState((int)e.Key, true);
			if(e.Key == Key.Tab || e.Key == Key.F10) {
				//Prevent menu/window from handling these keys to avoid issue with custom shortcuts
				e.Handled = true;
			}
		}

		private void OnPreviewKeyUp(object? sender, KeyEventArgs e)
		{
			InputApi.SetKeyState((int)e.Key, false);
		}

		private void OnActiveChanged()
		{
			ConfigApi.SetEmulationFlag(EmulationFlags.InBackground, !IsActive);
			InputApi.ResetKeyState();
		}

		private bool _needResume = false;
		private void timerUpdateBackgroundFlag(object? sender, EventArgs e)
		{
			Window? activeWindow = ApplicationHelper.GetActiveWindow();

			PreferencesConfig cfg = ConfigManager.Config.Preferences;

			bool needPause = activeWindow == null && cfg.PauseWhenInBackground;
			if(activeWindow != null) {
				bool isConfigWindow = (activeWindow != this);
				needPause |= cfg.PauseWhenInMenusAndConfig && !isConfigWindow && _mainMenu.MainMenu.IsOpen; //in main menu
				needPause |= cfg.PauseWhenInMenusAndConfig && isConfigWindow; //in a window that's neither the main window nor a debug tool
			}

			if(needPause) {
				if(!EmuApi.IsPaused()) {
					_needResume = true;
					EmuApi.Pause();
				}
			} else if(_needResume) {
				EmuApi.Resume();
				_needResume = false;
			}
		}
	}
}
