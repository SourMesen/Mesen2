namespace Mesen.Utilities.GlobalMouseLib
{
	public interface IGlobalMouseImpl
	{
		MousePosition GetMousePosition();
		bool IsMouseButtonPressed(MouseButtons button);
		void SetCursorIcon(CursorIcon icon);
		void SetMousePosition(uint x, uint y);
		void CaptureCursor(int x, int y, int width, int height);
		void ReleaseCursor();
	}
}