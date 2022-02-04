using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public static class ApplicationHelper
	{
		public static Window? GetMainWindow()
		{
			if(Application.Current?.ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop) {
				return desktop.MainWindow;
			}

			return null;
		}

		public static Window? GetActiveWindow()
		{
			if(Application.Current?.ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop) {
				return desktop.Windows.Where(w => w.IsActive).FirstOrDefault();
			}

			return null;
		}

		public static T? GetExistingWindow<T>() where T : Window
		{
			if(Application.Current?.ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop) {
				return desktop.Windows.Where(w => w is T).FirstOrDefault() as T;
			}

			return null;
		}

		//Taken from Avalonia's code (MIT): https://github.com/AvaloniaUI/Avalonia/blob/master/src/Avalonia.Dialogs/AboutAvaloniaDialog.xaml.cs
		public static void OpenBrowser(string url)
		{
			if(RuntimeInformation.IsOSPlatform(OSPlatform.Linux)) {
				// If no associated application/json MimeType is found xdg-open opens retrun error
				// but it tries to open it anyway using the console editor (nano, vim, other..)
				ShellExec($"xdg-open {url}", waitForExit: false);
			} else {
				using Process? process = Process.Start(new ProcessStartInfo {
					FileName = RuntimeInformation.IsOSPlatform(OSPlatform.Windows) ? url : "open",
					Arguments = RuntimeInformation.IsOSPlatform(OSPlatform.OSX) ? $"{url}" : "",
					CreateNoWindow = true,
					UseShellExecute = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
				});
			}
		}

		private static void ShellExec(string cmd, bool waitForExit = true)
		{
			var escapedArgs = Regex.Replace(cmd, "(?=[`~!#&*()|;'<>])", "\\").Replace("\"", "\\\\\\\"");

			using(Process? process = Process.Start(
				 new ProcessStartInfo {
					 FileName = "/bin/sh",
					 Arguments = $"-c \"{escapedArgs}\"",
					 RedirectStandardOutput = true,
					 UseShellExecute = false,
					 CreateNoWindow = true,
					 WindowStyle = ProcessWindowStyle.Hidden
				 }
			)) {
				if(waitForExit) {
					process?.WaitForExit();
				}
			}
		}
	}
}
