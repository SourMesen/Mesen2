namespace Mesen.Utilities.GlobalMouseLib
{
	public class GlobalMouseStubImpl : IGlobalMouseImpl
	{
		public MousePosition GetMousePosition()
		{
			return new MousePosition(0, 0);
		}

		public bool IsMouseButtonPressed(MouseButtons button)
		{
			return false;
		}

		public void SetCursorIcon(CursorIcon icon)
		{
		}

		public void SetMousePosition(uint x, uint y)
		{
		}

		public void CaptureCursor(int x, int y, int width, int height)
		{
		}

		public void ReleaseCursor()
		{
		}
	}
}
