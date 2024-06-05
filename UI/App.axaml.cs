using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Controls.Platform;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;
using System.IO;
using System.Reflection;

namespace Mesen
{
	public class App : Application
	{
		public static bool ShowConfigWindow { get; set; }

		public override void Initialize()
		{
			if(Design.IsDesignMode || ShowConfigWindow) {
				RequestedThemeVariant = ThemeVariant.Light;
			} else {
				RequestedThemeVariant = ConfigManager.Config.Preferences.Theme == MesenTheme.Dark ? ThemeVariant.Dark : ThemeVariant.Light;
			}

			AvaloniaXamlLoader.Load(this);
			ResourceHelper.LoadResources();
		}

		public override void OnFrameworkInitializationCompleted()
		{
			if(ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop) {
				if(ShowConfigWindow) {
					new PreferencesConfig().InitializeFontDefaults();
					desktop.MainWindow = new SetupWizardWindow();
				} else {
					desktop.MainWindow = new MainWindow();
				}
			}
			base.OnFrameworkInitializationCompleted();
		}
	}
}
