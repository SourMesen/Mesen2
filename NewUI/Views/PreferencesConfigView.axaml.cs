using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.GUI.Config;
using Mesen.Controls;
using Avalonia.Themes.Fluent;
using Avalonia.Styling;
using System.Collections.Generic;
using Avalonia.Markup.Xaml.Styling;

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

		private void cboTheme_SelectionChanged(object? sender, SelectionChangedEventArgs e)
		{
			PreferencesConfig.ApplyTheme((MesenTheme)e.AddedItems[0]);
		}
	}
}
