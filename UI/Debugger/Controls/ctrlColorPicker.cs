using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public class ctrlColorPicker : PictureBox
	{
		public ctrlColorPicker() : base()
		{
			this.BorderStyle = BorderStyle.FixedSingle;
		}

		protected override void OnClick(EventArgs e)
		{
			base.OnClick(e);

			using(ColorDialog cd = new ColorDialog()) {
				cd.SolidColorOnly = true;
				cd.AllowFullOpen = true;
				cd.FullOpen = true;
				cd.Color = this.BackColor;
				if(cd.ShowDialog() == DialogResult.OK) {
					this.BackColor = cd.Color;
				}
			}
		}
	}
}
