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

namespace Mesen.Windows
{
	public class MainWindow : Window
	{
		public MainWindow()
		{
			InitializeComponent();
			AddHandler(DragDrop.DropEvent, OnDrop);
#if DEBUG
			this.AttachDevTools();
#endif
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

			EmuApi.InitDll();
			NativeRenderer renderer = this.FindControl<NativeRenderer>("Renderer");

			ConfigManager.InitHomeFolder();
			System.IO.Directory.CreateDirectory(ConfigManager.HomeFolder);
			System.IO.Directory.SetCurrentDirectory(ConfigManager.HomeFolder);

			IntPtr wndHandle = ((IWindowImpl)((TopLevel)this.VisualRoot).PlatformImpl).Handle.Handle;
			EmuApi.InitializeEmu(ConfigManager.HomeFolder, wndHandle, renderer.Handle, false, false, false);

			ConfigManager.Config.InitializeDefaults();
			ConfigManager.Config.ApplyConfig();
		
			//ConfigApi.SetEmulationFlag(EmulationFlags.MaximumSpeed, true);
			EmuApi.LoadRom(@"C:\Code\Games\Super Mario Bros. (USA).nes");
			//EmuApi.LoadRom(@"C:\Code\Mesen-S\PGOHelper\PGOGames\Super Mario World (USA).sfc");
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public async void OnOpenClick(object sender, RoutedEventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filters = new List<FileDialogFilter>() {
				new FileDialogFilter() { Name = "All ROM Files", Extensions = { "sfc" , "fig", "smc", "nes", "fds", "unif", "gb", "gbc" } },
				new FileDialogFilter() { Name = "SNES ROM Files", Extensions = { "sfc" , "fig", "smc" } },
				new FileDialogFilter() { Name = "NES ROM Files", Extensions = { "nes" , "fds", "unif" } },
				new FileDialogFilter() { Name = "GB ROM Files", Extensions = { "gb" , "gbc" } }
			};

			string[] filenames = await ofd.ShowAsync(this);
			if(filenames.Length > 0) {
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
			}.Show();
		}

		private void OnResetClick(object sender, RoutedEventArgs e)
		{
			EmuApi.Reset();
		}

		private void OnPowerCycleClick(object sender, RoutedEventArgs e)
		{
			EmuApi.PowerCycle();
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
