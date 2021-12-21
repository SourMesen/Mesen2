using Avalonia;
using Avalonia.Controls;

namespace Mesen.Config
{
	public class BaseWindowConfig<T> : BaseConfig<T> where T : class
	{
		public PixelSize WindowSize { get; set; } = new PixelSize(0, 0);
		public PixelPoint WindowLocation { get; set; } = new PixelPoint(0, 0);

		public void SaveWindowSettings(Window wnd)
		{
			WindowLocation = wnd.Position;
			WindowSize = new PixelSize((int)wnd.Width, (int)wnd.Height);
		}

		public void LoadWindowSettings(Window wnd)
		{
			if(WindowSize.Width != 0 && WindowSize.Height != 0) {
				wnd.Position = WindowLocation;
				wnd.Width = WindowSize.Width;
				wnd.Height = WindowSize.Height;
			}
		}
	}
}
