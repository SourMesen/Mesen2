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
using Mesen.GUI.Utilities;
using ReactiveUI;
using System;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace Mesen
{
	public class MainWindow : Window
	{
		public MainWindow()
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif
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

			ConfigApi.SetAudioConfig(new Mesen.GUI.Config.InteropAudioConfig() { });
			ConfigManager.Config.InitializeDefaults();
			ConfigManager.Config.Input.ApplyConfig();

			//EmuApi.LoadRom(@"C:\Code\Mesen-S\PGOHelper\PGOGames\Chrono Trigger (USA).sfc");
			/*if(RuntimeInformation.IsOSPlatform(OSPlatform.Linux)) {
				EmuApi.LoadRom(@"/mnt/hgfs/Code/Mesen-S/PGOHelper/PGOGames/Super Mario World (USA).sfc");
			} else {
				EmuApi.LoadRom(@"C:\Code\Mesen-S\PGOHelper\PGOGames\Super Mario World (USA).sfc");
			}*/
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public async void OnOpenClick(object sender, RoutedEventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filters = new List<FileDialogFilter>() {
				new FileDialogFilter() { Name = "SNES ROM Files", Extensions = { "sfc" , "fig", "smc" } }
			};

			string[] filenames = await ofd.ShowAsync(this);
			if(filenames.Length > 0) {
				EmuApi.LoadRom(filenames[0]);
			}
		}

		private void OnDebuggerClick(object sender, RoutedEventArgs e)
		{
			new DebuggerWindow {
				DataContext = new DebuggerViewModel(),
			}.Show();
		}

		private void OnOptionsClick(object sender, RoutedEventArgs e)
		{
			new ConfigWindow {
				DataContext = new ConfigViewModel(),
			}.Show();
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
