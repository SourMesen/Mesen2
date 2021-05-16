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
		private NotificationListener? _listener;
		private ConfigWindow? _cfgWindow = null;
		private MainWindowViewModel? _model = null;

		public MainWindow()
		{
			InitializeComponent();
			AddHandler(DragDrop.DropEvent, OnDrop);
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

			if(!ProcessCommandLineArgs(Program.CommandLineArgs)) {
				EmuApi.LoadRom(@"C:\Code\Games\Super Mario Bros. (USA).nes");
				//EmuApi.LoadRom(@"C:\Code\Mesen-S\PGOHelper\PGOGames\Super Mario World (USA).sfc");
			}

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
						_model!.RomInfo = romInfo;
					});
					break;

				case ConsoleNotificationType.EmulationStopped:
					Dispatcher.UIThread.Post(() => {
						_model!.RomInfo = new RomInfo();
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
				LoadRomHelper.LoadFile(filenames[0]);
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

		private void OpenConfig(ConfigWindowTab tab)
		{
			if(_cfgWindow == null) {
				_cfgWindow = new ConfigWindow { DataContext = new ConfigViewModel(tab) };
				_cfgWindow.Closed += cfgWindow_Closed;
				_cfgWindow.ShowCentered(this);
			} else {
				(_cfgWindow.DataContext as ConfigViewModel)!.SelectTab(tab);
				_cfgWindow.Activate();
			}
		}

		private void cfgWindow_Closed(object? sender, EventArgs e)
		{
			_cfgWindow = null;
		}

		private void OnPreferencesClick(object sender, RoutedEventArgs e)
		{
			OpenConfig(ConfigWindowTab.Preferences);
		}

		private void OnAudioConfigClick(object sender, RoutedEventArgs e)
		{
			OpenConfig(ConfigWindowTab.Audio);
		}

		private void OnVideoConfigClick(object sender, RoutedEventArgs e)
		{
			OpenConfig(ConfigWindowTab.Video);
		}

		private void OnEmulationConfigClick(object sender, RoutedEventArgs e)
		{
			OpenConfig(ConfigWindowTab.Emulation);
		}

		private void OnNesConfigClick(object sender, RoutedEventArgs e)
		{
			OpenConfig(ConfigWindowTab.Nes);
		}

		private void OnSnesConfigClick(object sender, RoutedEventArgs e)
		{
			OpenConfig(ConfigWindowTab.Snes);
		}

		private void OnGameboyConfigClick(object sender, RoutedEventArgs e)
		{
			OpenConfig(ConfigWindowTab.Gameboy);
		}

		private void OnLogWindowClick(object sender, RoutedEventArgs e)
		{
			new LogWindow().ShowCentered(this);
		}

		private async void OnStartAudioRecordingClick(object sender, RoutedEventArgs e)
		{
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.Filters = new List<FileDialogFilter>() {
				new FileDialogFilter() { Name = "Wave files (*.wav)", Extensions = { "wav" } }
			};

			string filename = await sfd.ShowAsync(VisualRoot as Window);
			if(filename != null) {
				RecordApi.WaveRecord(filename);
			}
		}

		private void OnStopAudioRecordingClick(object sender, RoutedEventArgs e)
		{
			RecordApi.WaveStop();
		}

		private void OnStartVideoRecordingClick(object sender, RoutedEventArgs e)
		{
			RecordApi.AviRecord("c:\\temp\\out.gif", VideoCodec.GIF, 0);
		}

		private void OnStopVideoRecordingClick(object sender, RoutedEventArgs e)
		{
			RecordApi.AviStop();
		}

		private void OnPlayMovieClick(object sender, RoutedEventArgs e)
		{
			RecordApi.MoviePlay("c:\\temp\\out.mmo");
		}

		private void OnRecordMovieClick(object sender, RoutedEventArgs e)
		{
			RecordMovieOptions options = new RecordMovieOptions("c:\\temp\\out.mmo", "", "", RecordMovieFrom.CurrentState);
			RecordApi.MovieRecord(options);
		}

		private void OnStopMovieClick(object sender, RoutedEventArgs e)
		{
			RecordApi.AviStop();
		}

		private void OnCheatsClick(object sender, RoutedEventArgs e)
		{
			//TODO
		}

		private void OnTakeScreenshotClick(object sender, RoutedEventArgs e)
		{
			EmuApi.TakeScreenshot();
		}

		private void OnResetClick(object sender, RoutedEventArgs e)
		{
			EmuApi.Reset();
		}

		private void OnPowerCycleClick(object sender, RoutedEventArgs e)
		{
			EmuApi.PowerCycle();
		}

		private void OnPowerOffClick(object sender, RoutedEventArgs e)
		{
			EmuApi.Stop();
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
