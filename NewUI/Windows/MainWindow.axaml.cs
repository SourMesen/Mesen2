using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using Avalonia.Threading;
using Mesen.ViewModels;
using Mesen.GUI;
using Mesen.GUI.Config;
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
using Mesen.GUI.Config.Shortcuts;
using Mesen.Utilities;

namespace Mesen.Windows
{
	public class MainWindow : Window
	{
		private NotificationListener _listener;

		public MainWindow()
		{
			InitializeComponent();
			AddHandler(DragDrop.DropEvent, OnDrop);
#if DEBUG
			this.AttachDevTools();
#endif
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
					EmuApi.LoadRom(filename);
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

			if(!ProcessCommandLineArgs(Program.CommandLineArgs)) {
				EmuApi.LoadRom(@"C:\Code\Games\Super Mario Bros. (USA).nes");
				//EmuApi.LoadRom(@"C:\Code\Mesen-S\PGOHelper\PGOGames\Super Mario World (USA).sfc");
			}

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
					return EmuApi.LoadRom(arg);
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
						(this.DataContext as MainWindowViewModel).RomInfo = romInfo;
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

		public void OnExitClick(object sender, RoutedEventArgs e)
		{
			Close();
		}

		public async void OnOpenClick(object sender, RoutedEventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filters = new List<FileDialogFilter>() {
				new FileDialogFilter() { Name = "All ROM Files", Extensions = { "sfc" , "fig", "smc", "spc", "nes", "fds", "unif", "nsf", "nsfe", "gb", "gbc", "gbs" } },
				new FileDialogFilter() { Name = "SNES ROM Files", Extensions = { "sfc" , "fig", "smc", "spc" } },
				new FileDialogFilter() { Name = "NES ROM Files", Extensions = { "nes" , "fds", "unif", "nsf", "nsfe" } },
				new FileDialogFilter() { Name = "GB ROM Files", Extensions = { "gb" , "gbc", "gbs" } }
			};

			string[] filenames = await ofd.ShowAsync(this);
			if(filenames?.Length > 0) {
				EmuApi.LoadRom(filenames[0]);
			}
		}

		private void OnTileViewerClick(object sender, RoutedEventArgs e)
		{
			new TileViewerWindow {
				DataContext = new TileViewerViewModel(),
			}.Show();
		}

		private void OnMemoryToolsClick(object sender, RoutedEventArgs e)
		{
			new MemoryToolsWindow {
				DataContext = new MemoryToolsViewModel(),
			}.Show();
		}

		private void OnDebuggerClick(object sender, RoutedEventArgs e)
		{
			new DebuggerWindow {
				DataContext = new DebuggerWindowViewModel(),
			}.Show();
		}

		private void OnOptionsClick(object sender, RoutedEventArgs e)
		{
			new ConfigWindow {
				DataContext = new ConfigViewModel(),
			}.ShowCentered(this);
		}

		private void OnLogWindowClick(object sender, RoutedEventArgs e)
		{
			new LogWindow().ShowCentered(this);
		}

		private void OnStartRecordingClick(object sender, RoutedEventArgs e)
		{
			RecordApi.AviRecord("c:\\temp\\out.gif", VideoCodec.GIF, 0);
		}

		private void OnStopRecordingClick(object sender, RoutedEventArgs e)
		{
			RecordApi.AviStop();
		}

		private void OnResetClick(object sender, RoutedEventArgs e)
		{
			EmuApi.Reset();
		}

		private void OnPowerCycleClick(object sender, RoutedEventArgs e)
		{
			EmuApi.PowerCycle();
		}

		private void OnFdsSwitchDiskSide(object sender, RoutedEventArgs e)
		{
			EmuApi.ExecuteShortcut(new ExecuteShortcutParams() { Shortcut = EmulatorShortcut.FdsSwitchDiskSide });
		}

		private void OnFdsEjectDisk(object sender, RoutedEventArgs e)
		{
			EmuApi.ExecuteShortcut(new ExecuteShortcutParams() { Shortcut = EmulatorShortcut.FdsEjectDisk });
		}

		private void OnFdsInsertDisk0(object sender, RoutedEventArgs e)
		{
			EmuApi.ExecuteShortcut(new ExecuteShortcutParams() { Shortcut = EmulatorShortcut.FdsInsertDiskNumber, Param = 0 });
		}

		private void OnFdsInsertDisk1(object sender, RoutedEventArgs e)
		{
			EmuApi.ExecuteShortcut(new ExecuteShortcutParams() { Shortcut = EmulatorShortcut.FdsInsertDiskNumber, Param = 1 });
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);
			InputApi.SetKeyState((int)e.Key, true);
		}

		protected override void OnKeyUp(KeyEventArgs e)
		{
			base.OnKeyUp(e);
			InputApi.SetKeyState((int)e.Key, false);
		}
	}
}
