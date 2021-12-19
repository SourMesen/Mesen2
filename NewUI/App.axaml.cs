using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;
using System.IO;

namespace Mesen
{
	public class App : Application
	{
		public override void Initialize()
		{
			AvaloniaXamlLoader.Load(this);
			StyleHelper.LoadStartupStyles();
			ResourceHelper.LoadResources(Language.English);
		}

		public override void OnFrameworkInitializationCompleted()
		{
			EmuApi.InitDll();

			Directory.CreateDirectory(ConfigManager.HomeFolder);
			Directory.SetCurrentDirectory(ConfigManager.HomeFolder);

			if(ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop) {
				desktop.MainWindow = new MainWindow {
					DataContext = new MainWindowViewModel(),
				};
			}

			base.OnFrameworkInitializationCompleted();
		}
	}
}
