using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Avalonia.Interactivity;
using Mesen.ViewModels;
using Mesen.Interop;
using System.Collections.Generic;
using Mesen.Utilities;
using System;
using Mesen.Localization;
using Mesen.Controls;

namespace Mesen.Views
{
	public class MainMenuView : UserControl
	{
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
	}
}
