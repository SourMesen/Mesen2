using Avalonia;
using Avalonia.Controls;

namespace Mesen.Config
{
	public class BaseWindowConfig<T> : BaseConfig<T> where T : class
	{
		public System.Drawing.Size WindowSize { get; set; } = new System.Drawing.Size(0, 0);
		public System.Drawing.Point WindowLocation { get; set; } = new System.Drawing.Point(0, 0);

		public void SaveWindowSettings(Window wnd)
		{
			WindowLocation = new System.Drawing.Point(wnd.Position.X, wnd.Position.Y);
			WindowSize = new System.Drawing.Size((int)wnd.Width, (int)wnd.Height);
		}

		public void LoadWindowSettings(Window wnd)
		{
			if(!WindowSize.IsEmpty) {
				wnd.Position = new PixelPoint(WindowLocation.X, WindowLocation.Y);
				wnd.Width = WindowSize.Width;
				wnd.Height = WindowSize.Height;
			}
		}
	}
}
