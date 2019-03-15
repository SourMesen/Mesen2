using Mesen.GUI.Emulation;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms
{
	public class frmFullscreenRenderer : BaseInputForm
	{
		private const int LeftMouseButtonKeyCode = 0x200;
		private const int RightMouseButtonKeyCode = 0x201;
		private const int MiddleMouseButtonKeyCode = 0x202;
		private bool _closing = false;

		public frmFullscreenRenderer()
		{
			this.BackColor = Color.Black;
			this.Text = "Mesen Fullscreen Window";
			this.FormBorderStyle = FormBorderStyle.None;
			this.WindowState = FormWindowState.Maximized;
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			_closing = true;
			base.OnFormClosing(e);
		}

		protected override void OnDeactivate(EventArgs e)
		{
			base.OnDeactivate(e);

			if(!_closing) {
				//Close fullscreen mode if window loses focus
				this.Close();
			}
		}

		protected override void OnMouseDown(MouseEventArgs e)
		{
			base.OnMouseDown(e);
			CursorManager.CaptureMouse();
			SetMouseButtonState(Control.MouseButtons);
		}

		protected override void OnMouseUp(MouseEventArgs e)
		{
			base.OnMouseUp(e);
			SetMouseButtonState(Control.MouseButtons);
		}

		private void SetMouseButtonState(MouseButtons pressedButtons)
		{
			InputApi.SetKeyState(LeftMouseButtonKeyCode, pressedButtons.HasFlag(MouseButtons.Left));
			InputApi.SetKeyState(RightMouseButtonKeyCode, pressedButtons.HasFlag(MouseButtons.Right));
			InputApi.SetKeyState(MiddleMouseButtonKeyCode, pressedButtons.HasFlag(MouseButtons.Middle));
		}

		protected override void OnMouseMove(MouseEventArgs e)
		{
			base.OnMouseMove(e);

			ScreenSize size = EmuApi.GetScreenSize(false);
			int leftMargin = (this.Width - size.Width) / 2;
			int topMargin = (this.Height - size.Height) / 2;

			CursorManager.OnMouseMove(this);

			if(CursorManager.NeedMouseIcon) {
				this.Cursor = Cursors.Cross;
			}

			double xPos = (double)(e.X - leftMargin) / size.Width;
			double yPos = (double)(e.Y - topMargin) / size.Height;

			xPos = Math.Max(0.0, Math.Min(1.0, xPos));
			yPos = Math.Max(0.0, Math.Min(1.0, yPos));

			InputApi.SetMousePosition(xPos, yPos);
		}

		protected override void OnMouseLeave(EventArgs e)
		{
			base.OnMouseLeave(e);

			CursorManager.OnMouseLeave();
			InputApi.SetMousePosition(-1, -1);
		}
	}
}
