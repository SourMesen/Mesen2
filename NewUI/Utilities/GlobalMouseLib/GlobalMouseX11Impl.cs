using Mesen.Interop;
using System;
using System.Runtime.InteropServices;

namespace Mesen.Utilities.GlobalMouseLib;

public class GlobalMouseX11Impl : IGlobalMouseImpl
{
	private X11Info _x11;
	private IntPtr _mainWindow;

	public GlobalMouseX11Impl()
	{
		_mainWindow = ApplicationHelper.GetMainWindow()?.PlatformImpl.Handle.Handle ?? IntPtr.Zero;
		_x11 = new X11Info();
	}

	public MousePosition GetMousePosition()
	{
		(int x, int y) = GetCursorPos(_x11, null, out _);
		return new MousePosition(x, y);
	}

	public bool IsMouseButtonPressed(MouseButtons button)
	{
		GetCursorPos(_x11, null, out int keys);

		switch(button) {
			case MouseButtons.Left: return (keys & (1 << 8)) != 0;
			case MouseButtons.Middle: return (keys & (1 << 9)) != 0;
			case MouseButtons.Right: return (keys & (1 << 10)) != 0;
		}

		return false;
	}

	public void CaptureCursor(int x, int y, int width, int height, IntPtr rendererHandle)
	{
		for(int i = 0; i < 10; i++) {
			int result = X11Api.XGrabPointer(_x11.Display, rendererHandle, true, X11Api.EventMask.NoEventMask, X11Api.GrabMode.GrabModeAsync, X11Api.GrabMode.GrabModeAsync, rendererHandle, _x11.HiddenCursor, IntPtr.Zero);
			X11Api.XFlush(_x11.Display);
			if(result == 1 && i < 9) {
				//XGrabPointer can fail with AlreadyGrabbed - this can be normal, retry a few times
				System.Threading.Thread.Sleep(100);
			} else {
				if(result != 0) {
					EmuApi.WriteLogEntry("XGrabPointer failed: " + result.ToString());
				}
				return;
			}
		}
	}

	public void ReleaseCursor()
	{
		X11Api.XUngrabPointer(_x11.Display, IntPtr.Zero);
		X11Api.XFlush(_x11.Display);
	}

	public void SetCursorIcon(CursorIcon icon)
	{
		X11Api.XDefineCursor(_x11.Display, _mainWindow, icon switch {
			CursorIcon.Hidden => _x11.HiddenCursor,
			CursorIcon.Cross => _x11.CrossCursor,
			CursorIcon.Arrow or _ => _x11.DefaultCursor
		});
		X11Api.XFlush(_x11.Display);
	}

	public void SetMousePosition(uint x, uint y)
	{
		X11Api.XWarpPointer(_x11.Display, IntPtr.Zero, _x11.RootWindow, 0, 0, 0, 0, (int)x, (int)y);
		X11Api.XFlush(_x11.Display);
	}

	public static void QueryPointer(IntPtr display, IntPtr w, out IntPtr root, out IntPtr child, out int root_x, out int root_y, out int child_x, out int child_y, out int mask)
	{
		IntPtr c;

		X11Api.XGrabServer(display);

		X11Api.XQueryPointer(display, w, out root, out c,
			 out root_x, out root_y, out child_x, out child_y,
			 out mask);

		if(root != w)
			c = root;

		IntPtr child_last = IntPtr.Zero;
		while(c != IntPtr.Zero) {
			child_last = c;
			X11Api.XQueryPointer(display, c, out root, out c,
				 out root_x, out root_y, out child_x, out child_y,
				 out mask);
		}
		X11Api.XUngrabServer(display);
		X11Api.XFlush(display);

		child = child_last;
	}

	public static (int x, int y) GetCursorPos(X11Info x11, IntPtr? handle, out int keys_buttons)
	{
		QueryPointer(x11.Display, handle ?? x11.RootWindow, out IntPtr root, out IntPtr child, out int root_x, out int root_y, out int win_x, out int win_y, out keys_buttons);

		if(handle != null) {
			return (win_x, win_y);
		} else {
			return (root_x, root_y);
		}
	}
}

public class X11Api
{
	const string libX11 = "libX11.so.6";
	/*const string libX11Randr = "libXrandr.so.2";
	const string libX11Ext = "libXext.so.6";
	const string libXInput = "libXi.so.6";
	const string libXCursor = "libXcursor.so.1";*/

	[DllImport(libX11)]
	public static extern IntPtr XOpenDisplay(IntPtr display);

	[DllImport(libX11)]
	public static extern int XDefaultScreen(IntPtr display);

	[DllImport(libX11)]
	public static extern IntPtr XRootWindow(IntPtr display, int screen_number);

	[DllImport(libX11)]
	public static extern bool XQueryPointer(IntPtr display, IntPtr window, out IntPtr root, out IntPtr child,
		 out int root_x, out int root_y, out int win_x, out int win_y, out int keys_buttons);
	[DllImport(libX11)]
	public static extern void XGrabServer(IntPtr display);
	[DllImport(libX11)]
	public static extern void XUngrabServer(IntPtr display);
	[DllImport(libX11)]
	public static extern int XFlush(IntPtr display);

	[DllImport(libX11)]
	public static extern int XGrabPointer(IntPtr display, IntPtr window, bool owner_events, EventMask event_mask,
		 GrabMode pointer_mode, GrabMode keyboard_mode, IntPtr confine_to, IntPtr cursor, IntPtr timestamp);

	[DllImport(libX11)]
	public static extern int XUngrabPointer(IntPtr display, IntPtr timestamp);

	[DllImport(libX11)]
	public static extern IntPtr XCreateFontCursor(IntPtr display, CursorFontShape shape);

	[DllImport(libX11)]
	public static extern int XDefineCursor(IntPtr display, IntPtr window, IntPtr cursor);

	[DllImport(libX11)]
	public static extern uint XWarpPointer(IntPtr display, IntPtr src_w, IntPtr dest_w, int src_x, int src_y,
		 uint src_width, uint src_height, int dest_x, int dest_y);

	[DllImport(libX11)]
	public static extern IntPtr XCreatePixmapCursor(IntPtr display, IntPtr source, IntPtr mask,
	 ref XColor foreground_color, ref XColor background_color, int x_hot, int y_hot);

	[DllImport(libX11)]
	public static extern IntPtr XCreateBitmapFromData(IntPtr display, IntPtr drawable, byte[] data, int width, int height);

	[StructLayout(LayoutKind.Sequential, Pack = 2)]
	public struct XColor
	{
		internal IntPtr pixel;
		internal ushort red;
		internal ushort green;
		internal ushort blue;
		internal byte flags;
		internal byte pad;
	}

	public enum GrabMode
	{
		GrabModeSync = 0,
		GrabModeAsync = 1
	}

	[Flags]
	public enum EventMask
	{
		NoEventMask = 0,
		KeyPressMask = 1 << 0,
		KeyReleaseMask = 1 << 1,
		ButtonPressMask = 1 << 2,
		ButtonReleaseMask = 1 << 3,
		EnterWindowMask = 1 << 4,
		LeaveWindowMask = 1 << 5,
		PointerMotionMask = 1 << 6,
		PointerMotionHintMask = 1 << 7,
		Button1MotionMask = 1 << 8,
		Button2MotionMask = 1 << 9,
		Button3MotionMask = 1 << 10,
		Button4MotionMask = 1 << 11,
		Button5MotionMask = 1 << 12,
		ButtonMotionMask = 1 << 13,
		KeymapStateMask = 1 << 14,
		ExposureMask = 1 << 15,
		VisibilityChangeMask = 1 << 16,
		StructureNotifyMask = 1 << 17,
		ResizeRedirectMask = 1 << 18,
		SubstructureNotifyMask = 1 << 19,
		SubstructureRedirectMask = 1 << 20,
		FocusChangeMask = 1 << 21,
		PropertyChangeMask = 1 << 22,
		ColormapChangeMask = 1 << 23,
		OwnerGrabButtonMask = 1 << 24
	}

	public enum CursorFontShape
	{
		XC_X_cursor = 0,
		XC_arrow = 2,
		XC_based_arrow_down = 4,
		XC_based_arrow_up = 6,
		XC_boat = 8,
		XC_bogosity = 10,
		XC_bottom_left_corner = 12,
		XC_bottom_right_corner = 14,
		XC_bottom_side = 16,
		XC_bottom_tee = 18,
		XC_box_spiral = 20,
		XC_center_ptr = 22,

		XC_circle = 24,
		XC_clock = 26,
		XC_coffee_mug = 28,
		XC_cross = 30,
		XC_cross_reverse = 32,
		XC_crosshair = 34,
		XC_diamond_cross = 36,
		XC_dot = 38,
		XC_dotbox = 40,
		XC_double_arrow = 42,
		XC_draft_large = 44,
		XC_draft_small = 46,

		XC_draped_box = 48,
		XC_exchange = 50,
		XC_fleur = 52,
		XC_gobbler = 54,
		XC_gumby = 56,
		XC_hand1 = 58,
		XC_hand2 = 60,
		XC_heart = 62,
		XC_icon = 64,
		XC_iron_cross = 66,
		XC_left_ptr = 68,
		XC_left_side = 70,

		XC_left_tee = 72,
		XC_left_button = 74,
		XC_ll_angle = 76,
		XC_lr_angle = 78,
		XC_man = 80,
		XC_middlebutton = 82,
		XC_mouse = 84,
		XC_pencil = 86,
		XC_pirate = 88,
		XC_plus = 90,
		XC_question_arrow = 92,
		XC_right_ptr = 94,

		XC_right_side = 96,
		XC_right_tee = 98,
		XC_rightbutton = 100,
		XC_rtl_logo = 102,
		XC_sailboat = 104,
		XC_sb_down_arrow = 106,
		XC_sb_h_double_arrow = 108,
		XC_sb_left_arrow = 110,
		XC_sb_right_arrow = 112,
		XC_sb_up_arrow = 114,
		XC_sb_v_double_arrow = 116,
		XC_sb_shuttle = 118,

		XC_sizing = 120,
		XC_spider = 122,
		XC_spraycan = 124,
		XC_star = 126,
		XC_target = 128,
		XC_tcross = 130,
		XC_top_left_arrow = 132,
		XC_top_left_corner = 134,
		XC_top_right_corner = 136,
		XC_top_side = 138,
		XC_top_tee = 140,
		XC_trek = 142,

		XC_ul_angle = 144,
		XC_umbrella = 146,
		XC_ur_angle = 148,
		XC_watch = 150,
		XC_xterm = 152,
		XC_num_glyphs = 154
	}
}

public class X11Info
{
	public IntPtr Display { get; }
	public int DefaultScreen { get; }
	public IntPtr RootWindow { get; }
	
	public IntPtr DefaultCursor { get; }
	public IntPtr CrossCursor { get; }
	public IntPtr HiddenCursor { get; }

	private static readonly byte[] NullCursorData = new byte[] { 0 };

	public X11Info()
	{
		Display = X11Api.XOpenDisplay(IntPtr.Zero);
		DefaultScreen = X11Api.XDefaultScreen(Display);
		RootWindow = X11Api.XRootWindow(Display, DefaultScreen);

		DefaultCursor = X11Api.XCreateFontCursor(Display, X11Api.CursorFontShape.XC_left_ptr);
		CrossCursor = X11Api.XCreateFontCursor(Display, X11Api.CursorFontShape.XC_crosshair);
		HiddenCursor = GetNullCursor(Display);
	}

	private IntPtr GetNullCursor(IntPtr display)
	{
		X11Api.XColor color = new X11Api.XColor();
		IntPtr pixmap = X11Api.XCreateBitmapFromData(display, RootWindow, NullCursorData, 1, 1);
		return X11Api.XCreatePixmapCursor(display, pixmap, pixmap, ref color, ref color, 0, 0);
	}
}
