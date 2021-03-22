using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Mesen.GUI.Config;
using Mesen.Localization;
using Mesen.ViewModels;
using Mesen.Windows;

namespace Mesen
{
	public class App : Application
	{
		public override void Initialize()
		{
			AvaloniaXamlLoader.Load(this);
			PreferencesConfig.ApplyTheme(ConfigManager.Config.Preferences.Theme);
			ResourceHelper.LoadResources(Language.English);
		}

		public override void OnFrameworkInitializationCompleted()
		{
			if(ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop) {
				desktop.MainWindow = new MainWindow {
					DataContext = new MainWindowViewModel(),
				};
			}

			base.OnFrameworkInitializationCompleted();
		}
	}
}
