using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger.Controls
{
	public class ComboBoxWithSeparator : ComboBox
	{
		private int _previousSelectedIndex = 0;

		public ComboBoxWithSeparator() : base()
		{
			this.DrawMode = DrawMode.OwnerDrawFixed;
		}

		protected override void OnDrawItem(DrawItemEventArgs e)
		{
			base.OnDrawItem(e);

			if(e.Index >= 0) {
				var item = this.Items[e.Index];
				if(item.ToString() == "-") {
					e.Graphics.FillRectangle(SystemBrushes.ControlLightLight, e.Bounds);
					e.Graphics.DrawLine(Pens.DarkGray, e.Bounds.X + 2, e.Bounds.Y + e.Bounds.Height / 2, e.Bounds.Right - 2, e.Bounds.Y + e.Bounds.Height / 2);
				} else {
					e.DrawBackground();

					if(e.State == DrawItemState.Focus) {
						e.DrawFocusRectangle();
					}
					using(Brush brush = new SolidBrush(e.ForeColor)) {
						e.Graphics.DrawString(item.ToString(), e.Font, brush, e.Bounds);
					}
				}
			} else {
				e.DrawBackground();
			}
		}

		protected override void OnSelectedIndexChanged(EventArgs e)
		{
			if(this.SelectedItem.ToString() == "-") {
				if(_previousSelectedIndex > this.SelectedIndex) {
					this.SelectedIndex--;
				} else {
					this.SelectedIndex++;
				}
				return;
			}

			this._previousSelectedIndex = this.SelectedIndex;
			base.OnSelectedIndexChanged(e);
		}
	}
}
