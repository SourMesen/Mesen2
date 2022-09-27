using Avalonia;
using Avalonia.Controls;
using Avalonia.Platform;
using System;

namespace Mesen.Config
{
	public class BaseWindowConfig<T> : BaseConfig<T> where T : class
	{
		public PixelSize WindowSize { get; set; } = new PixelSize(0, 0);
		public PixelPoint WindowLocation { get; set; } = new PixelPoint(0, 0);
		public bool WindowIsMaximized { get; set; } = false;

		private PixelRect _restoreBounds = PixelRect.Empty;

		public void SaveWindowSettings(Window wnd)
		{
			if(wnd.WindowState == WindowState.Normal) {
				WindowLocation = wnd.Position;
				WindowSize = new PixelSize((int)wnd.ClientSize.Width, (int)wnd.ClientSize.Height);
			} else if(!_restoreBounds.IsEmpty) {
				WindowLocation = _restoreBounds.Position;
				WindowSize = _restoreBounds.Size;
			}

			WindowIsMaximized = wnd.WindowState == WindowState.Maximized;
		}

		public void LoadWindowSettings(Window wnd)
		{
			//Update _restoreBounds when size/position changes
			wnd.GetPropertyChangedObservable(Window.ClientSizeProperty).Subscribe(x => UpdateRestoreBounds(wnd));
			wnd.PositionChanged += (s, e) => UpdateRestoreBounds(wnd);

			if(WindowSize.Width != 0 && WindowSize.Height != 0) {
				if(WindowSize.Width * WindowSize.Height < 400) {
					//Window is too small, reset to a default size
					WindowSize = new PixelSize(300, 300);
				}

				PixelRect wndRect = new PixelRect(WindowLocation, WindowSize);
				
				Screen? screen = wnd.Screens.ScreenFromBounds(wndRect);
				if(screen == null) {
					//Window is not on any screen, move it to the top left of the first screen
					wnd.Position = wnd.Screens.All[0].WorkingArea.TopLeft;
				} else {
					//Window top left corner is offscreen, adjust position
					if(WindowLocation.Y < screen.WorkingArea.Position.Y) {
						WindowLocation = WindowLocation.WithY(screen.WorkingArea.Position.Y);
					}
					if(WindowLocation.X < screen.WorkingArea.Position.X) {
						WindowLocation = WindowLocation.WithX(screen.WorkingArea.Position.X);
					}

					wndRect = new PixelRect(WindowLocation, WindowSize);
					PixelRect intersect = screen.WorkingArea.Intersect(wndRect);
					if(intersect.Width * intersect.Height < wndRect.Width * wndRect.Height / 2) {
						//More than half the window is offscreen, move it to the top left corner to be safe
						wnd.Position = wnd.Screens.All[0].WorkingArea.TopLeft;
					} else {
						wnd.Position = WindowLocation;
					}
				}

				wnd.Width = WindowSize.Width;
				wnd.Height = WindowSize.Height;

				wnd.Opened += (s, e) => {
					//Set position again after opening
					//Fixes KDE (or X11?) not showing the window in the specified position
					wnd.Position = WindowLocation;
					
					if(WindowIsMaximized) {
						wnd.WindowState = WindowState.Maximized;
					}
				};
			}
		}

		private void UpdateRestoreBounds(Window wnd)
		{
			if(wnd.WindowState == WindowState.Normal && (wnd.Position.X > 0 || wnd.Position.Y > 0) && wnd.PlatformImpl != null && wnd.Width != wnd.Screens.ScreenFromWindow(wnd.PlatformImpl)?.Bounds.Width) {
				//If window is not maximized/minimized, save current position+size
				_restoreBounds = new PixelRect(wnd.Position.X, wnd.Position.Y, (int)wnd.Width, (int)wnd.Height);
			}
		}
	}
}
