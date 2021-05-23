using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Avalonia.Interactivity;
using Avalonia.Threading;
using Mesen.ViewModels;
using Mesen.Interop;
using Mesen.Windows;
using System.Collections.Generic;
using Mesen.Utilities;
using Mesen.Debugger.Windows;
using Mesen.Debugger.ViewModels;
using System;

namespace Mesen.Views
{
	public class MainMenuView : UserControl
	{
		private ConfigWindow? _cfgWindow = null;
		private MainMenuViewModel _model = null!;

		public MainMenuView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is MainMenuViewModel model) {
				_model = model;
			}
		}

		private void OnSaveStateMenuClick(object sender, RoutedEventArgs e)
		{
			_model.MainWindow.RecentGames.Init(GameScreenMode.SaveState);
		}

		private void OnLoadStateMenuClick(object sender, RoutedEventArgs e)
		{
			_model.MainWindow.RecentGames.Init(GameScreenMode.LoadState);
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
				_cfgWindow.ShowCentered((Window)VisualRoot);
			} else {
				(_cfgWindow.DataContext as ConfigViewModel)!.SelectTab(tab);
				_cfgWindow.Activate();
			}
		}

		private void cfgWindow_Closed(object? sender, EventArgs e)
		{
			_cfgWindow = null;
			if(ConfigManager.Config.Preferences.DisableGameSelectionScreen && _model.MainWindow.RecentGames.Visible) {
				_model.MainWindow.RecentGames.Visible = false;
			} else if(!ConfigManager.Config.Preferences.DisableGameSelectionScreen && !_model.IsGameRunning) {
				_model.MainWindow.RecentGames.Init(GameScreenMode.RecentGames);
			}
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
			new LogWindow().ShowCentered((Window)VisualRoot);
		}

		private async void OnStartAudioRecordingClick(object sender, RoutedEventArgs e)
		{
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.Filters = new List<FileDialogFilter>() {
				new FileDialogFilter() { Name = "Wave files", Extensions = { "wav" } },
				new FileDialogFilter() { Name = "All files", Extensions = { "*" } }
			};
			sfd.Directory = ConfigManager.WaveFolder;
			sfd.InitialFileName = EmuApi.GetRomInfo().GetRomName() + ".wav";

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
			new VideoRecordWindow() {
				DataContext = new VideoRecordConfigViewModel()
			}.ShowCenteredDialog((Window)VisualRoot);
		}

		private void OnStopVideoRecordingClick(object sender, RoutedEventArgs e)
		{
			RecordApi.AviStop();
		}

		private void OnToolSubMenuOpened(object sender, RoutedEventArgs e)
		{
			_model.UpdateToolSubMenus();
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
	}
}
