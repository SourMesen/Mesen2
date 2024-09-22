using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.ViewModels;
using System;
using System.ComponentModel;
using Mesen.Config;
using Mesen.Utilities;
using System.IO;
using Avalonia.Input;
using Avalonia.Styling;

namespace Mesen.Windows
{
	public class ConfigWindow : MesenWindow
	{
		private ConfigViewModel _model;
		private bool _promptToSave = true;

		[Obsolete("For designer only")]
		public ConfigWindow() : this(ConfigWindowTab.Audio) { }

		public ConfigWindow(ConfigWindowTab tab)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_model = new ConfigViewModel(tab);
			DataContext = _model;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			_promptToSave = false;
			_model.SaveConfig();
			Close();
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			_promptToSave = false;
			_model.RevertConfig();
			Close();
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);
			if(e.Key == Key.Escape) {
				Close();
			}
		}

		private void OpenMesenFolder(object sender, RoutedEventArgs e)
		{
			System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo() {
				FileName = ConfigManager.HomeFolder + Path.DirectorySeparatorChar,
				UseShellExecute = true,
				Verb = "open"
			});
		}

		private async void ResetAllSettings(object sender, RoutedEventArgs e)
		{
			if(await MesenMsgBox.Show(VisualRoot, "ResetSettingsConfirmation", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning) == DialogResult.OK) {
				ConfigManager.ResetSettings();
				_promptToSave = false;
				Close();
			}
		}

		private async void DisplaySaveChangesPrompt()
		{
			DialogResult result = await MesenMsgBox.Show(this, "PromptSaveChanges", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
			switch(result) {
				case DialogResult.Yes: _promptToSave = false; _model.SaveConfig(); Close(); break;
				case DialogResult.No: _promptToSave = false; _model.RevertConfig(); Close(); break;
				default: break;
			}
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			if(Design.IsDesignMode) {
				return;
			}

			if(_promptToSave && _model.IsDirty()) {
				e.Cancel = true;
				DisplaySaveChangesPrompt();
				return;
			}

			ConfigManager.Config.ApplyConfig();
			_model.Dispose();

			PreferencesConfig.UpdateTheme();

			//Ensure config isn't modified by the UI while closing
			DataContext = null;
		}
	}
}