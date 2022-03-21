using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Controls.Platform;
using Avalonia.Markup.Xaml;
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
			AvaloniaXamlLoader.Load(this);
			StyleHelper.LoadStartupStyles();
			if(Design.IsDesignMode) {
				StyleHelper.ApplyTheme(MesenTheme.Light);
				StyleHelper.LoadDebuggerStyles();
			}
			ResourceHelper.LoadResources(Language.English);
		}

		public override void OnFrameworkInitializationCompleted()
		{
			if(ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop) {
				if(ShowConfigWindow) {
					StyleHelper.ApplyTheme(MesenTheme.Light);
					desktop.MainWindow = new SetupWizardWindow();
				} else {
					desktop.MainWindow = new MainWindow();
				}
			}
			base.OnFrameworkInitializationCompleted();
		}
	}
}
