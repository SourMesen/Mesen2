using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Platform;
using Avalonia.Threading;
using Mesen.ViewModels;
using Mesen.Config;
using ReactiveUI;
using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using Mesen.Debugger.Windows;
using Mesen.Debugger.ViewModels;
using System.IO;
using System.Linq;
using Mesen.Config.Shortcuts;
using Mesen.Utilities;
using Mesen.Interop;

namespace Mesen.Windows
{
	public class MainWindow : Window
	{
#pragma warning disable CS8625 // Cannot convert null literal to non-nullable reference type.
		private MainWindowViewModel _model = null;
#pragma warning restore CS8625 // Cannot convert null literal to non-nullable reference type.

		private NotificationListener? _listener = null;

		public MainWindow()
		{
			InitializeComponent();
			AddHandler(DragDrop.DropEvent, OnDrop);

			//Allow us to catch LeftAlt/RightAlt key presses
			AddHandler(InputElement.KeyDownEvent, OnPreviewKeyDown, RoutingStrategies.Tunnel, true);
			AddHandler(InputElement.KeyUpEvent, OnPreviewKeyUp, RoutingStrategies.Tunnel, true);
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
			NativeRenderer renderer = this.FindControl<NativeRenderer>("Renderer");
			renderer.InvalidateMeasure();
			renderer.InvalidateArrange();
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

			NativeRenderer renderer = this.FindControl<NativeRenderer>("Renderer");
			IntPtr wndHandle = ((IWindowImpl)((TopLevel)this.VisualRoot).PlatformImpl).Handle.Handle;
			EmuApi.InitializeEmu(ConfigManager.HomeFolder, wndHandle, renderer.Handle, false, false, false);

			_listener = new NotificationListener();
			_listener.OnNotification += OnNotification;

			ConfigManager.Config.InitializeDefaults();
			ConfigManager.Config.ApplyConfig();

			//ConfigApi.SetEmulationFlag(EmulationFlags.MaximumSpeed, true);

			/*if(!ProcessCommandLineArgs(Program.CommandLineArgs)) {
				EmuApi.LoadRom(@"C:\Code\Games\Super Mario Bros. (USA).nes");
				//EmuApi.LoadRom(@"C:\Code\Mesen-S\PGOHelper\PGOGames\Super Mario World (USA).sfc");
			}*/

			ConfigManager.Config.Preferences.UpdateFileAssociations();

			SingleInstance.Instance.ArgumentsReceived += Instance_ArgumentsReceived;
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
						ResizeRenderer();
					});
					break;

			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void OnPreviewKeyDown(object? sender, KeyEventArgs e)
		{
			InputApi.SetKeyState((int)e.Key, true);
			if(e.Key == Key.Tab) {
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
