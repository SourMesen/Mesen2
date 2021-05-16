using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.ReactiveUI;
using Mesen.Config;
using Mesen.Utilities;
using System;

namespace Mesen
{
	class Program
	{
		// Initialization code. Don't use any Avalonia, third-party APIs or any
		// SynchronizationContext-reliant code before AppMain is called: things aren't initialized
		// yet and stuff might break.

		public static string[] CommandLineArgs { get; private set; } = Array.Empty<string>();

		[STAThread]
		public static void Main(string[] args)
		{
			using SingleInstance instance = SingleInstance.Instance;
			instance.Init(args, ConfigManager.Config.Preferences.SingleInstance);
			if(instance.FirstInstance) {
				Program.CommandLineArgs = (string[])args.Clone();
				BuildAvaloniaApp().StartWithClassicDesktopLifetime(args, ShutdownMode.OnMainWindowClose);
			}
		}

		// Avalonia configuration, don't remove; also used by visual designer.
		public static AppBuilder BuildAvaloniaApp()
			 => AppBuilder.Configure<App>()
					.UseReactiveUI()
					.UsePlatformDetect()
					.With(new Win32PlatformOptions { AllowEglInitialization = true })
					.With(new X11PlatformOptions { UseGpu = false, UseEGL = false })
					.With(new AvaloniaNativePlatformOptions { UseGpu = true })
					.LogToTrace();
	}
}
