using Avalonia;
using Avalonia.Markup.Xaml.Styling;
using Avalonia.Styling;
using Avalonia.Themes.Fluent;
using Mesen.Config;
using System;
using System.Collections.Generic;

namespace Mesen.Utilities
{
	public class StyleHelper
	{
		public static void ApplyTheme(MesenTheme theme)
		{
			//Reset styles & load everything needed to display all non-debugger windows
			Application.Current?.Styles.Clear();

			var styles = new List<IStyle> {
				new FluentTheme(new Uri("avares://Mesen/App.axaml")) { Mode = (theme == MesenTheme.Light) ? FluentThemeMode.Light : FluentThemeMode.Dark },
				new StyleInclude(new Uri("avares://Mesen/App.axaml")) { Source = new Uri("/Styles/MesenStyles.xaml", UriKind.Relative) },
			};

			if(theme == MesenTheme.Dark) {
				styles.Add(new StyleInclude(new Uri("avares://Mesen/App.axaml")) { Source = new Uri("/Styles/MesenStyles.Dark.xaml", UriKind.Relative) });
			}

			Application.Current?.Styles.AddRange(styles);
		}

		public static void LoadStartupStyles()
		{
			//Load the minimum amount of styles required to display the main window
			var styles = new List<IStyle> {
				new StyleInclude(new Uri("avares://Mesen/App.axaml")) { Source = new Uri("/Styles/StartupStyles.xaml", UriKind.Relative) },
			};
			Application.Current?.Styles.AddRange(styles);
		}

		public static void LoadDebuggerStyles()
		{
			//Load the debugger window specific styles
			var styles = new List<IStyle> {
				new StyleInclude(new Uri("avares://Mesen/App.axaml")) { Source = new Uri("avares://Avalonia.Controls.DataGrid/Themes/Fluent.xaml") },
				new StyleInclude(new Uri("avares://Mesen/App.axaml")) { Source = new Uri("avares://Dock.Avalonia/Themes/DockFluentTheme.axaml") },
				new StyleInclude(new Uri("avares://Mesen/App.axaml")) { Source = new Uri("/ThirdParty/ColorPicker/ColorPicker.axaml", UriKind.Relative) },
				new StyleInclude(new Uri("avares://Mesen/App.axaml")) { Source = new Uri("/ThirdParty/DataBox/Themes/Fluent.axaml", UriKind.Relative) },
				new StyleInclude(new Uri("avares://Mesen/App.axaml")) { Source = new Uri("avares://AvaloniaEdit/AvaloniaEdit.xaml") },
				new StyleInclude(new Uri("avares://Mesen/App.axaml")) { Source = new Uri("/Styles/AvaloniaDataGridStyles.xaml", UriKind.Relative) },
				new StyleInclude(new Uri("avares://Mesen/App.axaml")) { Source = new Uri("/Styles/DockStyles.xaml", UriKind.Relative) }
			};

			Application.Current?.Styles.InsertRange(1, styles);
		}
	}
}