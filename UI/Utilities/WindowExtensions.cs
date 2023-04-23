using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Platform;
using Avalonia.Rendering;
using Avalonia.VisualTree;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	static class WindowExtensions
	{
		public static void CenterWindow(Window child, Visual parent)
		{
			EventHandler? handler = null;
			handler = (s, e) => {
				//This logic is inside the Opened event because running it immediately
				//before showing the window appears to break some things in some configurations (multi-monitor+high DPI)

				WindowBase? parentWnd = parent.GetVisualRoot() as WindowBase;
				Screen? screen = null;
				double scale = 1;
				if(parentWnd != null) {
					screen = parentWnd.Screens.ScreenFromVisual(parent);
					scale = LayoutHelper.GetLayoutScale(parentWnd);
				}

				Size wndCenter = new Size(parent.Bounds.Width / 2, parent.Bounds.Height / 2) * scale;
				int childWidth = (int)((child.FrameSize?.Width ?? child.Width) * scale);
				int childHeight = (int)((child.FrameSize?.Height ?? child.Height) * scale);
				PixelPoint controlPosition = parent.PointToScreen(new Point(0, 0));
				PixelPoint screenCenter = new PixelPoint(controlPosition.X + (int)wndCenter.Width, controlPosition.Y + (int)wndCenter.Height);
				PixelPoint startPosition = new PixelPoint(screenCenter.X - childWidth / 2, screenCenter.Y - childHeight / 2);

				startPosition = FitToScreenBounds(screen, childWidth, childHeight, startPosition);

				child.Position = startPosition;
				child.Opened -= handler;
			};
			child.Opened += handler;
		}

		private static PixelPoint FitToScreenBounds(Screen? screen, int childWidth, int childHeight, PixelPoint startPosition)
		{
			if(screen != null) {
				PixelPoint bottomRight = startPosition + new PixelVector(childWidth, childHeight);

				//Try to reposition window to ensure it appears on the parent's screen
				if(startPosition.X < screen.Bounds.X + 10) {
					startPosition = startPosition.WithX(screen.Bounds.X + 10);
				}
				if(startPosition.Y < screen.Bounds.Y + 10) {
					startPosition = startPosition.WithY(screen.Bounds.Y + 10);
				}
				if(bottomRight.X > screen.Bounds.Right - 10) {
					startPosition = startPosition.WithX(screen.Bounds.Right - childWidth - 10);
				}
				if(bottomRight.Y > screen.Bounds.Bottom - 50) {
					startPosition = startPosition.WithY(screen.Bounds.Bottom - childHeight - 50);
				}
			}

			return startPosition;
		}

		public static void ShowCentered(this Window child, Visual? parent)
		{
			if(parent != null) {
				CenterWindow(child, parent);
			}
			child.Show();
		}

		public static void ShowCenteredWithParent(this Window child, Window parent)
		{
			CenterWindow(child, parent);
			child.Show(parent);
		}

		public static Task ShowCenteredDialog(this Window child, Visual? parent)
		{
			return ShowCenteredDialog<object>(child, parent);
		}

		public static Task<TResult> ShowCenteredDialog<TResult>(this Window child, Visual? parent)
		{
			if(parent != null) {
				CenterWindow(child, parent);
			}
			return InternalShowDialog<TResult>(child, parent);
		}

		public static Task<TResult> ShowDialogAtPosition<TResult>(this Window child, Visual? parent, PixelPoint startPosition)
		{
			child.WindowStartupLocation = WindowStartupLocation.Manual;
			child.Position = startPosition;

			EventHandler? handler = null;
			handler = (s, e) => {
				WindowBase? parentWnd = parent?.GetVisualRoot() as WindowBase;
				if(parentWnd != null && parent != null) {
					Screen? screen = parentWnd.Screens.ScreenFromVisual(parent);
					double scale = LayoutHelper.GetLayoutScale(parentWnd);

					int childWidth = (int)((child.FrameSize?.Width ?? child.Width) * scale);
					int childHeight = (int)((child.FrameSize?.Height ?? child.Height) * scale);

					startPosition = FitToScreenBounds(screen, childWidth, childHeight, startPosition);
				}

				child.Position = startPosition;
				child.Opened -= handler;
			};
			child.Opened += handler;

			return InternalShowDialog<TResult>(child, parent);
		}

		private static Task<TResult> InternalShowDialog<TResult>(Window child, Visual? parent)
		{
			Visual? parentWnd = parent;
			while(!(parentWnd is Window) && parentWnd != null) {
				parentWnd = parentWnd.GetVisualParent();
			}

			if(!(parentWnd is Window wnd)) {
				throw new Exception("Could not find parent window");
			}

			if(wnd.Topmost) {
				child.Topmost = true;
			}
			return child.ShowDialog<TResult>(wnd);
		}

		public static void BringToFront(this Window wnd)
		{
			if(wnd.WindowState == WindowState.Minimized) {
				wnd.WindowState = WindowState.Normal;
			}
			wnd.Activate();
		}
	}
}
