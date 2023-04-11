using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Avalonia.Interactivity;
using Mesen.ViewModels;
using Mesen.Utilities;
using Avalonia.Input;
using System.ComponentModel;

namespace Mesen.Windows
{
	public class ControllerConfigWindow : MesenWindow
	{
		private ControllerConfigViewModel Model => (ControllerConfigViewModel)DataContext!;
		private bool _promptToSave = true;

		public ControllerConfigWindow()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);
			if(e.Key == Key.Escape) {
				Close();
			}
		}

		private async void DisplaySaveChangesPrompt()
		{
			DialogResult result = await MesenMsgBox.Show(this, "PromptKeepChanges", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
			switch(result) {
				case DialogResult.Yes: _promptToSave = false; Close(true); break;
				case DialogResult.No: _promptToSave = false; Close(false); break;
				default: break;
			}
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			if(Design.IsDesignMode) {
				return;
			}

			if(_promptToSave && !Model.Config.IsIdentical(Model.OriginalConfig)) {
				e.Cancel = true;
				DisplaySaveChangesPrompt();
				return;
			}
		}

		private void btnOk_OnClick(object sender, RoutedEventArgs e)
		{
			_promptToSave = false;
			Close(true);
		}

		private void btnCancel_OnClick(object sender, RoutedEventArgs e)
		{
			_promptToSave = false;
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

		private void mnuXboxLayout1Alt_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.XboxP1Alt);
		}

		private void mnuXboxLayout2_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.XboxP2);
		}

		private void mnuXboxLayout2Alt_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.XboxP2Alt);
		}

		private void mnuPs4Layout1_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.Ps4P1);
		}

		private void mnuPs4Layout1Alt_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.Ps4P1Alt);
		}

		private void mnuPs4Layout2_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.Ps4P2);
		}

		private void mnuPs4Layout2Alt_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(KeyPresetType.Ps4P2Alt);
		}

		private void btnSetDefaultBindings_OnClick(object sender, RoutedEventArgs e)
		{
			SetDefaultMappings(null);
		}
	}
}
