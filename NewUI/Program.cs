using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.ReactiveUI;
using System;

namespace Mesen
{
	class Program
	{
		// Initialization code. Don't use any Avalonia, third-party APIs or any
		// SynchronizationContext-reliant code before AppMain is called: things aren't initialized
		// yet and stuff might break.
		public static void Main(string[] args) => BuildAvaloniaApp()
			 .StartWithClassicDesktopLifetime(args);

		// Avalonia configuration, don't remove; also used by visual designer.
		public static AppBuilder BuildAvaloniaApp()
			 => AppBuilder.Configure<App>()
					.UseReactiveUI()
					.UsePlatformDetect()
					.With(new Win32PlatformOptions { UseWgl = true, UseDeferredRendering = false })
					.With(new X11PlatformOptions { UseGpu = true, UseEGL = true })
					.With(new AvaloniaNativePlatformOptions { UseGpu = true })
					.LogToTrace();
	}
}
