using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.Config;
using Mesen.Controls;
using Avalonia.Themes.Fluent;
using Avalonia.Styling;
using System.Collections.Generic;
using Avalonia.Markup.Xaml.Styling;
using Avalonia.Interactivity;
using Mesen.Windows;

namespace Mesen.Views
{
	public class PreferencesConfigView : UserControl
	{
		public PreferencesConfigView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void btnChangeStorageFolder_OnClick(object sender, RoutedEventArgs e)
		{
			ShowSelectFolderWindow();
		}

		private async void ShowSelectFolderWindow()
		{
			SelectStorageFolderWindow wnd = new();
			if(await wnd.ShowCenteredDialog<bool>(VisualRoot)) {
				(VisualRoot as Window)?.Close();
				ApplicationHelper.GetMainWindow()?.Close();
				ConfigManager.RestartMesen();
			}
		}
	}
}
