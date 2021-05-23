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

namespace Mesen.Windows
{
	public class MainWindow : Window
	{
		private MainWindowViewModel _model = null!;

		private NotificationListener? _listener = null;
		private ShortcutHandler _shortcutHandler;

		private FrameInfo _baseScreenSize;
		private double _initialScale = 2.0;

		public NativeRenderer _renderer;

		public MainWindow()
		{
			InitializeComponent();
			_shortcutHandler = new ShortcutHandler(this);

			AddHandler(DragDrop.DropEvent, OnDrop);

			//Allows us to catch LeftAlt/RightAlt key presses
			AddHandler(InputElement.KeyDownEvent, OnPreviewKeyDown, RoutingStrategies.Tunnel, true);
			AddHandler(InputElement.KeyUpEvent, OnPreviewKeyUp, RoutingStrategies.Tunnel, true);

			_renderer = this.FindControl<NativeRenderer>("Renderer")!;

#if DEBUG
			this.AttachDevTools();
#endif
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is MainWindowViewModel model) {
				_model = model;
			} else {
				throw new Exception("Invalid model");
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

			Task.Run(() => {
				EmuApi.InitializeEmu(ConfigManager.HomeFolder, PlatformImpl.Handle.Handle, _renderer.Handle, false, false, false);

				_listener = new NotificationListener();
				_listener.OnNotification += OnNotification;

				ConfigManager.Config.InitializeDefaults();
				ConfigManager.Config.ApplyConfig();

				ProcessCommandLineArgs(Program.CommandLineArgs);

				ConfigManager.Config.Preferences.UpdateFileAssociations();
				SingleInstance.Instance.ArgumentsReceived += Instance_ArgumentsReceived;
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
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					RomInfo romInfo = EmuApi.GetRomInfo();
					Dispatcher.UIThread.Post(() => {
						_model.RecentGames.Visible = false;
						_model.RomInfo = romInfo;
					});
					break;

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
						double scale;
						if(_baseScreenSize.Width == 0) {
							scale = _initialScale;
						} else {
							scale = ClientSize.Width / _baseScreenSize.Width;
						}
						SetScale(scale);
						_baseScreenSize = EmuApi.GetBaseScreenSize();
					});
					break;

				case ConsoleNotificationType.ExecuteShortcut:
					ExecuteShortcutParams p = Marshal.PtrToStructure<ExecuteShortcutParams>(e.Parameter);
					_shortcutHandler.ExecuteShortcut(p.Shortcut);
					break;
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public void SetScale(double scale)
		{
			FrameInfo screenSize = EmuApi.GetBaseScreenSize();
			ClientSize = new Size(screenSize.Width * scale, screenSize.Height * scale + this.FindControl<MainMenuView>("MainMenu").Bounds.Height);
			ResizeRenderer();
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

		protected override void OnLostFocus(RoutedEventArgs e)
		{
			InputApi.ResetKeyState();
		}
	}
}
