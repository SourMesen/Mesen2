using System;

namespace Mesen.Utilities.GlobalMouseLib
{
	public interface IGlobalMouseImpl
	{
		MousePosition GetMousePosition(IntPtr windowFilter);
		bool IsMouseButtonPressed(MouseButtons button);
		void SetCursorIcon(CursorIcon icon);
		void SetMousePosition(uint x, uint y);
		void CaptureCursor(int x, int y, int width, int height, IntPtr rendererHandle);
		void ReleaseCursor();
	}
}