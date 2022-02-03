using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
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
	}
}
