using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Controls;
using Mesen.GUI.Forms;
using System.Drawing.Imaging;

namespace Mesen.GUI.Debugger.PpuViewer
{
	public partial class ctrlImagePanel : BaseControl
	{
		private int _scale = 1;
		private Size _imageSize;

		private bool _mouseDown = false;
		private Point _lastLocation = Point.Empty;

		public Rectangle Selection { get { return ctrlImageViewer.Selection; } set { ctrlImageViewer.Selection = value; } }
		public Rectangle Overlay { get { return ctrlImageViewer.Overlay; } set { ctrlImageViewer.Overlay = value; } }
		public int SelectionWrapPosition { get { return ctrlImageViewer.SelectionWrapPosition; } set { ctrlImageViewer.SelectionWrapPosition = value; } }

		public Size ImageSize { get { return _imageSize; } set { _imageSize = value; UpdateMapSize(); } }
		public Image Image { get { return ctrlImageViewer.Image; } set { ctrlImageViewer.Image = value; } }
		public int ImageScale { get { return _scale; } set { _scale = value; UpdateMapSize(); } }
		public int GridSizeX { get { return ctrlImageViewer.GridSizeX; } set { ctrlImageViewer.GridSizeX = value; } }
		public int GridSizeY { get { return ctrlImageViewer.GridSizeY; } set { ctrlImageViewer.GridSizeY = value; } }
		public Point ScrollOffsets { get { return new Point(ctrlPanel.HorizontalScroll.Value, ctrlPanel.VerticalScroll.Value); } }

		public new event EventHandler MouseLeave { add { ctrlImageViewer.MouseLeave += value; } remove { ctrlImageViewer.MouseLeave -= value; } }
		public new event MouseEventHandler MouseMove { add { ctrlImageViewer.MouseMove += value; } remove { ctrlImageViewer.MouseMove -= value; } }
		public new event MouseEventHandler MouseClick;

		public ctrlImagePanel()
		{
			InitializeComponent();

			if(DesignMode) {
				return;
			}

			ctrlPanel.OnZoom += (scaleDelta) => {
				double hori = (double)ctrlPanel.HorizontalScroll.Value / _scale + (double)ctrlPanel.Width / 2 / _scale;
				double vert = (double)ctrlPanel.VerticalScroll.Value / _scale + (double)ctrlPanel.Height / 2 / _scale;

				_scale = Math.Min(16, Math.Max(1, _scale + scaleDelta));
				UpdateMapSize();

				int horizontalScroll = Math.Max(0, Math.Min((int)(hori * _scale) - ctrlPanel.Width / 2, ctrlPanel.HorizontalScroll.Maximum));
				int verticalScroll = Math.Max(0, Math.Min((int)(vert * _scale) - ctrlPanel.Height / 2, ctrlPanel.VerticalScroll.Maximum));
				
				//Set the values twice to avoid a scroll position bug
				ctrlPanel.HorizontalScroll.Value = horizontalScroll;
				ctrlPanel.HorizontalScroll.Value = horizontalScroll;
				ctrlPanel.VerticalScroll.Value = verticalScroll;
				ctrlPanel.VerticalScroll.Value = verticalScroll;
			};

			ctrlImageViewer.MouseDown += (s, e) => {
				if(e.Button == MouseButtons.Left) {
					_mouseDown = true;
					_lastLocation = e.Location;
				}
			};

			ctrlImageViewer.MouseUp += (s, e) => {
				_mouseDown = false;
			};

			ctrlImageViewer.MouseClick += (s, e) => {
				this.MouseClick?.Invoke(s, e);
			};

			ctrlImageViewer.MouseMove += ctrlImageViewer_MouseMove;
		}

		private void ctrlImageViewer_MouseMove(object sender, MouseEventArgs e)
		{
			if(_mouseDown) {
				ctrlImageViewer.MouseMove -= ctrlImageViewer_MouseMove;
				int hScroll = Math.Min(ctrlPanel.HorizontalScroll.Maximum, Math.Max(0, ctrlPanel.HorizontalScroll.Value - (e.Location.X - _lastLocation.X)));
				int vScroll = Math.Min(ctrlPanel.VerticalScroll.Maximum, Math.Max(0, ctrlPanel.VerticalScroll.Value - (e.Location.Y - _lastLocation.Y)));

				ctrlPanel.HorizontalScroll.Value = hScroll;
				ctrlPanel.HorizontalScroll.Value = hScroll;
				ctrlPanel.VerticalScroll.Value = vScroll;
				ctrlPanel.VerticalScroll.Value = vScroll;
				ctrlImageViewer.MouseMove += ctrlImageViewer_MouseMove;
			}
		}

		private void UpdateMapSize()
		{
			ctrlImageViewer.Width = ImageSize.Width * _scale;
			ctrlImageViewer.Height = ImageSize.Height * _scale;
			ctrlImageViewer.ImageScale = _scale;
			ctrlImageViewer.Invalidate();
		}

		protected override void OnInvalidated(InvalidateEventArgs e)
		{
			base.OnInvalidated(e);
			ctrlImageViewer.Invalidate();
		}

		public void ZoomIn()
		{
			_scale = Math.Min(16, _scale + 1);
			UpdateMapSize();
		}

		public void ZoomOut()
		{
			_scale = Math.Max(1, _scale - 1);
			UpdateMapSize();
		}

		public void ScrollTo(int scrollPos)
		{
			ctrlPanel.VerticalScroll.Value = scrollPos;
			ctrlPanel.VerticalScroll.Value = scrollPos;

			ctrlPanel.HorizontalScroll.Value = 0;
			ctrlPanel.HorizontalScroll.Value = 0;
		}

		public void CopyToClipboard()
		{
			Clipboard.SetImage(this.Image);
		}

		public void SaveAsPng()
		{
			using(SaveFileDialog sfd = new SaveFileDialog()) {
				sfd.SetFilter("PNG files|*.png");
				if(sfd.ShowDialog() == DialogResult.OK) {
					this.Image.Save(sfd.FileName, ImageFormat.Png);
				}
			}
		}
	}
}
