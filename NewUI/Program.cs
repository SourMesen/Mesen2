using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.ReactiveUI;
using Mesen.Config;
using Mesen.Utilities;
using System;
using System.Linq;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Threading.Tasks;

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
			//Start loading config file in a separate thread
			Task.Run(() => ConfigManager.LoadConfig());

			//Extract core dll & other native dependencies
			ExtractNativeDependencies();

			Environment.CurrentDirectory = ConfigManager.HomeFolder;

			using SingleInstance instance = SingleInstance.Instance;
			instance.Init(args);
			if(instance.FirstInstance) {
				Program.CommandLineArgs = (string[])args.Clone();
				bool useWgl = args.Any(arg => arg.ToLowerInvariant() == "-wgl");
				BuildAvaloniaApp(useWgl).StartWithClassicDesktopLifetime(args, ShutdownMode.OnMainWindowClose);
			}
		}

		private static void ExtractNativeDependencies()
		{
			using(Stream? depStream = Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Dependencies.zip")) {
				if(depStream == null) {
					throw new Exception("Missing dependencies.zip");
				}

				using ZipArchive zip = new(depStream);
				foreach(ZipArchiveEntry entry in zip.Entries) {
					try {
						string path = Path.Combine(ConfigManager.HomeFolder, entry.Name);
						if(File.Exists(path)) {
							FileInfo fileInfo = new(path);
							if(fileInfo.LastWriteTime != entry.LastWriteTime || fileInfo.Length != entry.Length) {
								entry.ExtractToFile(Path.Combine(ConfigManager.HomeFolder, entry.Name), true);
							}
						} else {
							entry.ExtractToFile(Path.Combine(ConfigManager.HomeFolder, entry.Name), true);
						}
					} catch {

					}
				}
			}
		}

		// Avalonia configuration, don't remove; also used by visual designer.
		public static AppBuilder BuildAvaloniaApp()
		{
			return BuildAvaloniaApp(false);
		}

		public static AppBuilder BuildAvaloniaApp(bool useWgl)
			 => AppBuilder.Configure<App>()
					.UseReactiveUI()
					.UsePlatformDetect()
					.With(new Win32PlatformOptions { AllowEglInitialization = true, UseWgl = useWgl })
					.With(new X11PlatformOptions { UseGpu = false, UseEGL = false })
					.With(new AvaloniaNativePlatformOptions { UseGpu = true })
					.LogToTrace();
	}
}
