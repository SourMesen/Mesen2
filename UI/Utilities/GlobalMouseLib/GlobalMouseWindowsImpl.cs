using System;
using System.Runtime.InteropServices;

namespace Mesen.Utilities.GlobalMouseLib
{
	public class GlobalMouseWindowsImpl : IGlobalMouseImpl
	{
		private IntPtr _arrowCursor = LoadCursor(IntPtr.Zero, new IntPtr((int)Cursor.IDC_ARROW));
		private IntPtr _crossCursor = LoadCursor(IntPtr.Zero, new IntPtr((int)Cursor.IDC_CROSS));

		public bool IsMouseButtonPressed(MouseButtons button)
		{
			switch(button) {
				case MouseButtons.Left: return (GetAsyncKeyState((int)WindowsMouseButton.VK_LBUTTON) & 0x8000) != 0;
				case MouseButtons.Right: return (GetAsyncKeyState((int)WindowsMouseButton.VK_RBUTTON) & 0x8000) != 0;
				case MouseButtons.Middle: return (GetAsyncKeyState((int)WindowsMouseButton.VK_MBUTTON) & 0x8000) != 0;
				case MouseButtons.Button4: return (GetAsyncKeyState((int)WindowsMouseButton.VK_XBUTTON1) & 0x8000) != 0;
				case MouseButtons.Button5: return (GetAsyncKeyState((int)WindowsMouseButton.VK_XBUTTON2) & 0x8000) != 0;
			}

			return false;
		}

		public MousePosition GetMousePosition(IntPtr windowFilter)
		{
			GetCursorPos(out CursorPoint p);
			if(windowFilter != IntPtr.Zero && WindowFromPoint(p) != windowFilter) {
				//Mouse is over another window
				return new MousePosition(-1, -1);
			}
			return new MousePosition(p.X, p.Y);
		}

		public void SetMousePosition(uint x, uint y)
		{
			SetCursorPos(x, y);
		}

		public void SetCursorIcon(CursorIcon icon)
		{
			switch(icon) {
				case CursorIcon.Hidden: SetCursor(IntPtr.Zero); break;
				case CursorIcon.Arrow: SetCursor(_arrowCursor); break;
				case CursorIcon.Cross: SetCursor(_crossCursor); break;
			}
		}

		public void CaptureCursor(int x, int y, int width, int height, IntPtr rendererHandle)
		{
			ClipCursor(new WinRect() { Left = x, Top = y, Right = x + width, Bottom = y + height });
		}

		public void ReleaseCursor()
		{
			ClipCursor(null);
		}

		[DllImport("User32.dll")]
		private static extern short GetAsyncKeyState(int arrowKeys);

		[DllImport("user32.dll")]
		private static extern IntPtr LoadCursor(IntPtr hInstance, IntPtr lpCursorName);

		[DllImport("user32.dll")]
		private static extern IntPtr SetCursor(IntPtr hCursor);

		[StructLayout(LayoutKind.Sequential)]
		private struct CursorPoint
		{
			public int X;
			public int Y;
		}

		[StructLayout(LayoutKind.Sequential)]
		private class WinRect
		{
			public int Left;
			public int Top;
			public int Right;
			public int Bottom;
		}

		[DllImport("user32.dll")]
		private static extern bool GetCursorPos(out CursorPoint point);

		[DllImport("user32.dll")]
		[return: MarshalAs(UnmanagedType.Bool)]
		private static extern bool SetCursorPos(uint x, uint y);

		[DllImport("user32.dll")]
		[return: MarshalAs(UnmanagedType.Bool)]
		private static extern bool ClipCursor(WinRect? rect);

		[DllImport("user32.dll")]
		private static extern IntPtr WindowFromPoint(CursorPoint p);

		private enum WindowsMouseButton
		{
			VK_LBUTTON = 1,
			VK_RBUTTON = 2,
			VK_MBUTTON = 4,
			VK_XBUTTON1 = 5,
			VK_XBUTTON2 = 6,
		}

		private enum Cursor
		{
			IDC_ARROW = 32512,
			IDC_IBEAM = 32513,
			IDC_WAIT = 32514,
			IDC_CROSS = 32515,
			IDC_UPARROW = 32516,
			IDC_SIZE = 32640,
			IDC_ICON = 32641,
			IDC_SIZENWSE = 32642,
			IDC_SIZENESW = 32643,
			IDC_SIZEWE = 32644,
			IDC_SIZENS = 32645,
			IDC_SIZEALL = 32646,
			IDC_NO = 32648,
			IDC_HAND = 32649,
			IDC_APPSTARTING = 32650,
			IDC_HELP = 32651
		}
	}
}
