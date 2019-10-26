using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger.Controls
{
	public class ctrlPanel : Panel
	{
		public delegate void ZoomEventHandler(int scaleDelta);
		public event ZoomEventHandler OnZoom;

		public ctrlPanel()
		{
			this.DoubleBuffered = true;
		}

		protected override Point ScrollToControl(Control activeControl)
		{
			// Returning the current location prevents the panel from
			// scrolling to the active control when the panel loses and regains focus
			return this.DisplayRectangle.Location;
		}

		protected override void OnMouseWheel(MouseEventArgs e)
		{
			if(Control.ModifierKeys != Keys.Control) {
				int hori = this.HorizontalScroll.Value;
				int vert = this.VerticalScroll.Value;

				if(Control.ModifierKeys == Keys.Shift) {
					hori = Math.Max(0, Math.Min(hori - e.Delta, this.HorizontalScroll.Maximum));
				} else {
					vert = Math.Max(0, Math.Min(vert - e.Delta, this.VerticalScroll.Maximum));
				}

				this.HorizontalScroll.Value = hori;
				this.HorizontalScroll.Value = hori;
				this.VerticalScroll.Value = vert;
				this.VerticalScroll.Value = vert;
			} else {
				if(Program.IsMono) {
					//Patch for Mono to prevent a scrolling bug when zooming out.
					//Breaks the zoom in/out logic to keep the current scroll position when zooming, but that's better than the original bug.
					this.HorizontalScroll.Value = 0;
					this.VerticalScroll.Value = 0;
				}
				this.OnZoom?.Invoke(e.Delta > 0 ? 1 : -1);
			}
		}
	}
}
