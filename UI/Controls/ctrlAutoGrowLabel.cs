using System;
using System.Drawing;
using System.Windows.Forms;

namespace Mesen.GUI.Controls
{
	public class ctrlAutoGrowLabel : Label
	{
		private bool _growing;

		public ctrlAutoGrowLabel()
		{
			this.AutoSize = false;
		}

		private void resizeLabel()
		{
			if(_growing) {
				return;
			}

			try {
				_growing = true;
				Size textSize = new Size(this.ClientSize.Width - this.Padding.Left - this.Padding.Right, Int32.MaxValue);
				textSize = TextRenderer.MeasureText(this.Text, this.Font, textSize, TextFormatFlags.WordBreak);
				this.ClientSize = new Size(textSize.Width + this.Margin.Size.Width + this.Padding.Size.Width, textSize.Height);
			} finally {
				_growing = false;
			}
		}

		protected override void OnTextChanged(EventArgs e)
		{
			base.OnTextChanged(e);
			resizeLabel();
		}

		protected override void OnFontChanged(EventArgs e)
		{
			base.OnFontChanged(e);
			resizeLabel();
		}

		protected override void OnSizeChanged(EventArgs e)
		{
			base.OnSizeChanged(e);
			resizeLabel();
		}
	}
}
