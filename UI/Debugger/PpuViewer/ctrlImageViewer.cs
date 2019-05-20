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
		private Rectangle _overlay = Rectangle.Empty;
		private int _selectionWrapPosition = 0;
		private int _gridSizeX = 0;
		private int _gridSizeY = 0;

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
		
		public Rectangle Overlay
		{
			get { return _overlay; }
			set { _overlay = value; this.Invalidate(); }
		}

		public int GridSizeX
		{
			get { return _gridSizeX; }
			set { _gridSizeX = value; this.Invalidate(); }
		}

		public int GridSizeY
		{
			get { return _gridSizeY; }
			set { _gridSizeY = value; this.Invalidate(); }
		}

		public int SelectionWrapPosition
		{
			get { return _selectionWrapPosition; }
			set { _selectionWrapPosition = value; this.Invalidate(); }
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

			using(Pen gridPen = new Pen(Color.FromArgb(180, Color.LightBlue))) {
				if(_gridSizeX > 1) {
					for(int i = this.ImageScale * _gridSizeX; i < this.Width; i += this.ImageScale * _gridSizeX) {
						e.Graphics.DrawLine(gridPen, i, 0, i, this.Height);
					}
				}
				if(_gridSizeY > 1) {
					for(int i = this.ImageScale * _gridSizeY; i < this.Height; i += this.ImageScale * _gridSizeY) {
						e.Graphics.DrawLine(gridPen, 0, i, this.Width, i);
					}
				}
			}

			if(_overlay != Rectangle.Empty) {
				using(SolidBrush brush = new SolidBrush(Color.FromArgb(100, 240, 240, 240))) {
					int scale = this.ImageScale;
					Rectangle rect = new Rectangle(_overlay.Left * scale % this.Width, _overlay.Top * scale % this.Height, _overlay.Width * scale, _overlay.Height * scale);

					e.Graphics.FillRectangle(brush, rect.Left, rect.Top, rect.Width, rect.Height);
					e.Graphics.DrawRectangle(Pens.Gray, rect.Left, rect.Top, rect.Width, rect.Height);

					if((rect.Top + rect.Height) > this.Height) {
						e.Graphics.FillRectangle(brush, rect.Left, rect.Top - this.Height, rect.Width, rect.Height);
						e.Graphics.DrawRectangle(Pens.Gray, rect.Left, rect.Top - this.Height, rect.Width, rect.Height);
					}

					if((rect.Left + rect.Width) > this.Width) {
						e.Graphics.FillRectangle(brush, rect.Left - this.Width, rect.Top, rect.Width, rect.Height);
						e.Graphics.DrawRectangle(Pens.Gray, rect.Left - this.Width, rect.Top, rect.Width, rect.Height);

						if((rect.Top + rect.Height) > this.Height) {
							e.Graphics.FillRectangle(brush, rect.Left - this.Width, rect.Top - this.Height, rect.Width, rect.Height);
							e.Graphics.DrawRectangle(Pens.Gray, rect.Left - this.Width, rect.Top - this.Height, rect.Width, rect.Height);
						}
					}
				}
			}

			if(_selection != Rectangle.Empty) {
				int scale = this.ImageScale;
				e.Graphics.DrawRectangle(Pens.White, _selection.Left * scale, _selection.Top * scale, _selection.Width * scale + 0.5f, _selection.Height * scale + 0.5f);
				e.Graphics.DrawRectangle(Pens.Gray, _selection.Left * scale - 1, _selection.Top * scale - 1, _selection.Width * scale + 2.5f, _selection.Height * scale + 2.5f);

				if(_selectionWrapPosition > 0 && _selection.Top + _selection.Height > _selectionWrapPosition) {
					e.Graphics.DrawRectangle(Pens.White, _selection.Left * scale, _selection.Top * scale - _selectionWrapPosition * scale, _selection.Width * scale + 0.5f, _selection.Height * scale + 0.5f);
					e.Graphics.DrawRectangle(Pens.Gray, _selection.Left * scale - 1, _selection.Top * scale - 1 - _selectionWrapPosition * scale, _selection.Width * scale + 2.5f, _selection.Height * scale + 2.5f);
				}
			}
		}
	}
}
