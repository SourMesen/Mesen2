using System;
using Mesen.Interop;

namespace Mesen.Utilities.GlobalMouseLib
{
	public class GlobalMouseMacOSImpl : IGlobalMouseImpl
	{
		public MousePosition GetMousePosition(IntPtr windowFilter)
		{
			// InteropMouseState state = InputApi.GetSystemMouseState();
			// return new MousePosition((int) state.XPosition, (int) state.YPosition);
			return new MousePosition(0, 0);
		}

		public bool IsMouseButtonPressed(MouseButtons button)
		{
			// InteropMouseState state = InputApi.GetSystemMouseState();
			// switch(button) {
			// 	case MouseButtons.Left: return state.LeftButton;
			// 	case MouseButtons.Middle: return state.MiddleButton;
			// 	case MouseButtons.Right: return state.RightButton;
			// 	case MouseButtons.Button4: return state.Button4;
			// 	case MouseButtons.Button5: return state.Button5;
			// }
			return false;
		}

		public void SetCursorIcon(CursorIcon icon)
		{
			// InputApi.SetSystemCursorImage(icon);
		}

		public void SetMousePosition(uint x, uint y)
		{
			// InputApi.SetSystemMousePosition((double) x, (double) y);
		}

		public void CaptureCursor(int x, int y, int width, int height, IntPtr rendererHandle)
		{
			// InputApi.SetSystemMouseCaptured(true);
		}

		public void ReleaseCursor()
		{
			// InputApi.SetSystemMouseCaptured(false);
		}
	}
}
