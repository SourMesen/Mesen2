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

namespace Mesen.Windows
{
	public class ConfigWindow : Window
	{
		private ConfigViewModel _model;

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
			_model.SaveConfig();
			Close();
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close();
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
				Close();
			}
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			if(Design.IsDesignMode) {
				return;
			}

			ConfigManager.Config.ApplyConfig();
			_model.Dispose();
			DataContext = null;
		}
	}
}