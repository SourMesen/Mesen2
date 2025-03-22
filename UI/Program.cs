using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.ReactiveUI;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Utilities;
using System;
using System.Linq;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Mesen.Interop;

namespace Mesen
{
	class Program
	{
		// Initialization code. Don't use any Avalonia, third-party APIs or any
		// SynchronizationContext-reliant code before AppMain is called: things aren't initialized
		// yet and stuff might break.

		public static string OriginalFolder { get; private set; }
		public static string[] CommandLineArgs { get; private set; } = Array.Empty<string>();

		public static string ExePath => Process.GetCurrentProcess().MainModule?.FileName ?? Path.Join(Path.GetDirectoryName(AppContext.BaseDirectory), "Mesen.exe");

		static Program()
		{
			try {
				Program.OriginalFolder = Environment.CurrentDirectory;
			} catch {
				Program.OriginalFolder = Path.GetDirectoryName(ExePath) ?? "";
			}
		}

		[STAThread]
		public static int Main(string[] args)
		{
			if(!System.Diagnostics.Debugger.IsAttached) {
				NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), DllImportResolver);
				NativeLibrary.SetDllImportResolver(typeof(SkiaSharp.SKGraphics).Assembly, DllImportResolver);
				NativeLibrary.SetDllImportResolver(typeof(HarfBuzzSharp.Blob).Assembly, DllImportResolver);
			}

			if(args.Length >= 4 && args[0] == "--update") {
				UpdateHelper.AttemptUpdate(args[1], args[2], args[3], args.Contains("admin"));
				return 0;
			}

			Environment.CurrentDirectory = ConfigManager.HomeFolder;

			if(!File.Exists(ConfigManager.GetConfigFile())) {
				//Could not find configuration file, show wizard
				DependencyHelper.ExtractNativeDependencies(ConfigManager.HomeFolder);
				App.ShowConfigWindow = true;
				BuildAvaloniaApp().StartWithClassicDesktopLifetime(args, ShutdownMode.OnMainWindowClose);
				if(File.Exists(ConfigManager.GetConfigFile())) {
					//Configuration done, restart process
					Process.Start(Program.ExePath);
				}
				return 0;
			}

			//Start loading config file in a separate thread
			Task.Run(() => ConfigManager.LoadConfig());

			//Extract core dll & other native dependencies
			DependencyHelper.ExtractNativeDependencies(ConfigManager.HomeFolder);

			if(CommandLineHelper.IsTestRunner(args)) {
				return TestRunner.Run(args);
			}

			using SingleInstance instance = SingleInstance.Instance;
			instance.Init(args);
			if(instance.FirstInstance) {
				Program.CommandLineArgs = (string[])args.Clone();
				BuildAvaloniaApp().StartWithClassicDesktopLifetime(args, ShutdownMode.OnMainWindowClose);
			}

			return 0;
		}

		private static IntPtr DllImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
		{
			if(libraryName.Contains("Mesen") || libraryName.Contains("SkiaSharp") || libraryName.Contains("HarfBuzz")) {
				if(libraryName.EndsWith(".dll")) {
					libraryName = libraryName.Substring(0, libraryName.Length - 4);
				}

				if(OperatingSystem.IsLinux()) {
					if(!libraryName.EndsWith(".so")) {
						libraryName = libraryName + ".so";
					}
				} else if(OperatingSystem.IsWindows()) {
					if(!libraryName.EndsWith(".dll")) {
						libraryName = libraryName + ".dll";
					}
				} else if(OperatingSystem.IsMacOS()) {
					if(!libraryName.EndsWith(".dylib")) {
						libraryName = libraryName + ".dylib";
					}
				}
				return NativeLibrary.Load(Path.Combine(ConfigManager.HomeFolder, libraryName));
			}
			return IntPtr.Zero;
		}

		// Avalonia configuration, don't remove; also used by visual designer.
		public static AppBuilder BuildAvaloniaApp()
			 => AppBuilder.Configure<App>()
					.UseReactiveUI()
					.UsePlatformDetect()
					.With(new Win32PlatformOptions { })
					.With(new X11PlatformOptions {
						EnableInputFocusProxy = Environment.GetEnvironmentVariable("XDG_CURRENT_DESKTOP") == "gamescope",
					 })
					.With(new AvaloniaNativePlatformOptions { RenderingMode = new AvaloniaNativeRenderingMode[] { AvaloniaNativeRenderingMode.OpenGl, AvaloniaNativeRenderingMode.Software } })
					.LogToTrace();
	}
}
