using Avalonia;
using Avalonia.Controls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	static class WindowExtensions
	{
		public static void ShowCentered(this Window child, Window parent)
		{
			child.WindowStartupLocation = WindowStartupLocation.Manual;
			Size wndCenter = (parent.ClientSize / 2);
			PixelPoint screenCenter = new PixelPoint(parent.Position.X + (int)wndCenter.Width, parent.Position.Y + (int)wndCenter.Height);
			child.Position = new PixelPoint(screenCenter.X - (int)child.Width / 2, screenCenter.Y - (int)child.Height / 2);
			child.Show();
		}
	}
}
