using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Avalonia.Interactivity;

namespace Mesen.Windows
{
	public class ControllerConfigWindow : Window
	{
		private KeyPresets _presets = new KeyPresets();

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
			this.Close(true);
		}

		private void btnCancel_OnClick(object sender, RoutedEventArgs e)
		{
			this.Close(false);
		}

		private void btnPreset_OnClick(object sender, RoutedEventArgs e)
		{
			((Button)sender).ContextMenu?.Open();
		}

		private void ApplyPreset(KeyMapping preset)
		{
			int index = this.FindControl<TabControl>("tabMain").SelectedIndex;
			ControllerConfig? cfg = (this.DataContext as ControllerConfig);
			if(cfg != null) {
				switch(index) {
					case 0: cfg.Mapping1 = preset; break;
					case 1: cfg.Mapping2 = preset; break;
					case 2: cfg.Mapping3 = preset; break;
					case 3: cfg.Mapping4 = preset; break;
				}
			}
		}

		private void mnuWasdLayout_OnClick(object sender, RoutedEventArgs e)
		{
			this.ApplyPreset(_presets.WasdLayout);
		}

		private void mnuArrowLayout_OnClick(object sender, RoutedEventArgs e)
		{
			this.ApplyPreset(_presets.ArrowLayout);
		}

		private void mnuXboxLayout1_OnClick(object sender, RoutedEventArgs e)
		{
			this.ApplyPreset(_presets.XboxLayout1);
		}

		private void mnuXboxLayout2_OnClick(object sender, RoutedEventArgs e)
		{
			this.ApplyPreset(_presets.XboxLayout2);
		}

		private void mnuPs4Layout1_OnClick(object sender, RoutedEventArgs e)
		{
			this.ApplyPreset(_presets.Ps4Layout1);
		}

		private void mnuPs4Layout2_OnClick(object sender, RoutedEventArgs e)
		{
			this.ApplyPreset(_presets.Ps4Layout2);
		}

		private void mnuSnes30Layout1_OnClick(object sender, RoutedEventArgs e)
		{
			this.ApplyPreset(_presets.Snes30Layout1);
		}

		private void mnuSnes30Layout2_OnClick(object sender, RoutedEventArgs e)
		{
			this.ApplyPreset(_presets.Snes30Layout2);
		}

		private void btnClearBindings_OnClick(object sender, RoutedEventArgs e)
		{
			this.ApplyPreset(new KeyMapping());
		}
	}
}
