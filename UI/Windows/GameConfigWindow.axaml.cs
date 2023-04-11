using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Controls;
using Mesen.ViewModels;
using System;
using System.ComponentModel;
using System.Linq;

namespace Mesen.Windows;

public class GameConfigWindow : MesenWindow
{
	private GameConfigViewModel _model;

	public GameConfigWindow()
	{
		_model = new GameConfigViewModel();
		DataContext = _model;
		InitializeComponent();
	}

	private void InitializeComponent()
	{
		AvaloniaXamlLoader.Load(this);
	}

	protected override void OnClosing(WindowClosingEventArgs e)
	{
		GameConfig.LoadGameConfig(MainWindowViewModel.Instance.RomInfo).ApplyConfig();
		_model?.Dispose();
		base.OnClosing(e);
	}

	private void Ok_OnClick(object sender, RoutedEventArgs e)
	{
		_model.Save();
		Close();
	}

	private void Cancel_OnClick(object sender, RoutedEventArgs e)
	{
		Close();
	}
}
