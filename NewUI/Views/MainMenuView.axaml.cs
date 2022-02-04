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
using System.IO;
using System.IO.Compression;
using System.Text.RegularExpressions;
using Mesen.Localization;
using Mesen.Debugger.Utilities;
using Mesen.Controls;

namespace Mesen.Views
{
	public class MainMenuView : UserControl
	{
		private ConfigWindow? _cfgWindow = null;
		private MainMenuViewModel _model = null!;

		public Menu MainMenu { get; }

		public MainMenuView()
		{
			InitializeComponent();

			MainMenu = this.FindControl<Menu>("MainMenu");
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

		private void OpenConfig(ConfigWindowTab tab)
		{
			if(_cfgWindow == null) {
				_cfgWindow = new ConfigWindow(tab);
				_cfgWindow.Closed += cfgWindow_Closed;
				_cfgWindow.ShowCentered(VisualRoot);
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


		private void OnGameMenuOpened(object sender, RoutedEventArgs e)
		{
			bool isPaused = EmuApi.IsPaused();
			this.FindControl<ShortcutMenuItem>("mnuPause").IsVisible = !isPaused;
			this.FindControl<ShortcutMenuItem>("mnuResume").IsVisible = isPaused;
		}

		private void OnRegionMenuOpened(object sender, RoutedEventArgs e)
		{
			MenuItem menu = ((MenuItem)sender);
			List<IControl> items = new List<IControl>();

			ConsoleType consoleType = EmuApi.GetRomInfo().ConsoleType;
			ConsoleRegion region;
			switch(consoleType) {
				case ConsoleType.Snes: region = ConfigManager.Config.Snes.Region; break;
				case ConsoleType.Nes: region = ConfigManager.Config.Nes.Region; break;
				default: region = ConsoleRegion.Auto; break;
			}

			void AddRegion(ConsoleRegion r)
			{
				MenuItem item = new MenuItem() {
					Header = ResourceHelper.GetEnumText(r),
					Icon = (region == r ? ImageUtilities.FromAsset("Assets/MenuItemChecked.png") : null)!
				};

				item.Click += (_, _) => {
					switch(consoleType) {
						case ConsoleType.Snes:
							ConfigManager.Config.Snes.Region = r;
							ConfigManager.Config.Snes.ApplyConfig();
							break;

						case ConsoleType.Nes:
							ConfigManager.Config.Nes.Region = r;
							ConfigManager.Config.Nes.ApplyConfig();
							break;

						default: break;
					}
				};
				items.Add(item);
			}

			AddRegion(ConsoleRegion.Auto);
			items.Add(new Separator());
			AddRegion(ConsoleRegion.Ntsc);
			AddRegion(ConsoleRegion.Pal);
			if(consoleType == ConsoleType.Nes) {
				AddRegion(ConsoleRegion.Dendy);
			}

			menu.Items = items;
		}
	}
}
