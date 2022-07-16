using Avalonia;
using Avalonia.Controls;
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
		public static void CenterWindow(Window child, IVisual parent)
		{
			child.WindowStartupLocation = WindowStartupLocation.Manual;
			Size wndCenter = new Size(parent.Bounds.Width / 2, parent.Bounds.Height / 2);
			PixelPoint controlPosition = parent.PointToScreen(new Point(0, 0));
			PixelPoint screenCenter = new PixelPoint(controlPosition.X + (int)wndCenter.Width, controlPosition.Y + (int)wndCenter.Height);
			PixelPoint startPosition = new PixelPoint(screenCenter.X - (int)child.Width / 2, screenCenter.Y - (int)child.Height / 2);
			child.Position = startPosition;

			//Set position again after opening
			//Fixes KDE (or X11?) not showing the window in the specified position
			child.Opened += (s,e) => { child.Position = startPosition; };
		}

		public static void ShowCentered(this Window child, IVisual? parent)
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

		public static Task ShowCenteredDialog(this Window child, IVisual? parent)
		{
			return ShowCenteredDialog<object>(child, parent);
		}

		public static Task<TResult> ShowCenteredDialog<TResult>(this Window child, IVisual? parent)
		{
			if(parent != null) {
				CenterWindow(child, parent);
			}
			return InternalShowDialog<TResult>(child, parent);
		}

		public static Task<TResult> ShowDialogAtPosition<TResult>(this Window child, IVisual? parent, PixelPoint startPosition)
		{
			child.WindowStartupLocation = WindowStartupLocation.Manual;
			child.Position = startPosition;

			//Set position again after opening
			//Fixes KDE (or X11?) not showing the window in the specified position
			child.Opened += (s, e) => { child.Position = startPosition; };

			return InternalShowDialog<TResult>(child, parent);
		}

		private static Task<TResult> InternalShowDialog<TResult>(Window child, IVisual? parent)
		{
			IVisual? parentWnd = parent;
			while(!(parentWnd is Window) && parentWnd != null) {
				parentWnd = parentWnd.VisualParent;
			}

			if(parentWnd is Window wnd && wnd.Topmost) {
				child.Topmost = true;
			}
			return child.ShowDialog<TResult>(parentWnd as Window);
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
