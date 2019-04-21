using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger.PpuViewer
{
	public class ctrlImageViewer : Control
	{
		private Image _image = null;
		private Rectangle _selection = Rectangle.Empty;

		public ctrlImageViewer()
		{
			this.DoubleBuffered = true;
			this.ResizeRedraw = true;
		}

		public Image Image
		{
			get { return _image; }
			set { _image = value; this.Invalidate(); }
		}

		public Rectangle Selection
		{
			get { return _selection; }
			set { _selection = value; this.Invalidate(); }
		}

		public int ImageScale { get; set; } = 1;

		protected override void OnPaint(PaintEventArgs e)
		{
			base.OnPaint(e);

			e.Graphics.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
			e.Graphics.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.Half;
			e.Graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
			e.Graphics.ScaleTransform(this.ImageScale, this.ImageScale);

			if(this.Image != null) {
				e.Graphics.DrawImage(this.Image, 0, 0);
			}
			e.Graphics.ResetTransform();

			if(_selection != Rectangle.Empty) {
				e.Graphics.DrawRectangle(Pens.White, _selection.Left * this.ImageScale, _selection.Top * this.ImageScale, _selection.Width * this.ImageScale + 0.5f, _selection.Height * this.ImageScale + 0.5f);
				e.Graphics.DrawRectangle(Pens.Gray, _selection.Left * this.ImageScale - 1, _selection.Top * this.ImageScale - 1, _selection.Width * this.ImageScale + 2.5f, _selection.Height * this.ImageScale + 2.5f);
			}
		}
	}
}
