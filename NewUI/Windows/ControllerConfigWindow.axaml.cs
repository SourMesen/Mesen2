using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Avalonia.Interactivity;
using Mesen.ViewModels;

namespace Mesen.Windows
{
	public class ControllerConfigWindow : Window
	{
		private ControllerConfigViewModel Model => (ControllerConfigViewModel)DataContext!;

		public ControllerConfigWindow()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void btnOk_OnClick(object sender, RoutedEventArgs e)
		{
			Close(true);
		}

		private void btnCancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}

		private void btnPreset_OnClick(object sender, RoutedEventArgs e)
		{
			((Button)sender).ContextMenu?.Open();
		}

		private void SetDefaultMappings(KeyPresetType? preset)
		{
			int index = this.GetControl<TabControl>("tabMain").SelectedIndex;
			ControllerConfig cfg = Model.Config;
			switch(index) {
				case 0: cfg.Mapping1.SetDefaultKeys(Model.Type, preset); Model.KeyMapping1.RefreshCustomKeys(); break;
				case 1: cfg.Mapping2.SetDefaultKeys(Model.Type, preset); Model.KeyMapping2.RefreshCustomKeys(); break;
				case 2: cfg.Mapping3.SetDefaultKeys(Model.Type, preset); Model.KeyMapping3.RefreshCustomKeys(); break;
				case 3: cfg.Mapping4.SetDefaultKeys(Model.Type, preset); Model.KeyMapping4.RefreshCustomKeys(); break;
			}
		}

		private void btnClearBindings_OnClick(object sender, RoutedEventArgs e)
		{
			int index = this.GetControl<TabControl>("tabMain").SelectedIndex;
			ControllerConfig cfg = Model.Config;
			switch(index) {
				case 0: cfg.Mapping1.ClearKeys(Model.Type); Model.KeyMapping1.RefreshCustomKeys(); break;
				case 1: cfg.Mapping2.ClearKeys(Model.Type); Model.KeyMapping2.RefreshCustomKeys(); break;
				case 2: cfg.Mapping3.ClearKeys(Model.Type); Model.KeyMapping3.RefreshCustomKeys(); break;
				case 3: cfg.Mapping4.ClearKeys(Model.Type); Model.KeyMapping4.RefreshCustomKeys(); break;
			}
		}

		private void mnuWasdLayout_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.WasdKeys);
		}

		private void mnuArrowLayout_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.ArrowKeys);
		}

		private void mnuXboxLayout1_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.XboxP1);
		}

		private void mnuXboxLayout2_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.XboxP2);
		}

		private void mnuPs4Layout1_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.Ps4P1);
		}

		private void mnuPs4Layout2_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.Ps4P2);
		}

		private void btnSetDefaultBindings_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(null);
		}
	}
}
