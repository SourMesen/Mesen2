using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.ViewModels;
using System;
using System.Threading.Tasks;

namespace Mesen.Views;

public class SmsConfigView : UserControl
{
	private SmsConfigViewModel _model = null!;

	public SmsConfigView()
	{
		InitializeComponent();
	}

	private void InitializeComponent()
	{
		AvaloniaXamlLoader.Load(this);
	}

	protected override void OnDataContextChanged(EventArgs e)
	{
		if(DataContext is SmsConfigViewModel model) {
			_model = model;
		}
	}

	private void btnSelectPreset_OnClick(object sender, RoutedEventArgs e)
	{
		((Button)sender).ContextMenu?.Open();
	}

	private void mnuGameGearPreset_Click(object sender, RoutedEventArgs e)
	{
		_model.Config.GameGearOverscan.Left = 48;
		_model.Config.GameGearOverscan.Right = 48;
		_model.Config.GameGearOverscan.Top = 48;
		_model.Config.GameGearOverscan.Bottom = 48;
	}

	private void mnuFullFramePreset_Click(object sender, RoutedEventArgs e)
	{
		_model.Config.GameGearOverscan.Left = 0;
		_model.Config.GameGearOverscan.Right = 0;
		_model.Config.GameGearOverscan.Top = 24;
		_model.Config.GameGearOverscan.Bottom = 24;
	}

	private void mnuFullFrameFirstColumnPreset_Click(object sender, RoutedEventArgs e)
	{
		_model.Config.GameGearOverscan.Left = 8;
		_model.Config.GameGearOverscan.Right = 0;
		_model.Config.GameGearOverscan.Top = 24;
		_model.Config.GameGearOverscan.Bottom = 24;
	}
}
