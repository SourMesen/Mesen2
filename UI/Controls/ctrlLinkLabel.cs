using System;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;

namespace Mesen.GUI.Controls
{
	public class ctrlLinkLabel: Label
	{
		private string _link = null;

		public ctrlLinkLabel()
		{
		}

		public string Link
		{
			get { return _link; }
			set
			{
				_link = value;
				if(!string.IsNullOrWhiteSpace(_link)) {
					this.ForeColor = Color.Blue;
					this.Font = new Font(this.Font, FontStyle.Underline);
					this.Cursor = Cursors.Hand;
				}
			}
		}

		protected override void OnClick(EventArgs e)
		{
			base.OnClick(e);

			if(!string.IsNullOrWhiteSpace(_link)) {
				Process.Start(this.Link);
			}
		}
	}
}
