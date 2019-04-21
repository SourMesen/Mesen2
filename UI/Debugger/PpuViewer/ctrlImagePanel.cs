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

namespace Mesen.GUI.Debugger.PpuViewer
{
	public partial class ctrlImagePanel : BaseControl
	{
		private int _scale = 1;
		private Size _imageSize;

		public Rectangle Selection { get { return ctrlImageViewer.Selection; } set { ctrlImageViewer.Selection = value; } }
		public Size ImageSize { get { return _imageSize; } set { _imageSize = value; UpdateMapSize(); } }
		public Image Image { get { return ctrlImageViewer.Image; } set { ctrlImageViewer.Image = value; } }
		public int ImageScale { get { return _scale; } set { _scale = value; } }

		public new event MouseEventHandler MouseClick { add { ctrlImageViewer.MouseClick += value; } remove { ctrlImageViewer.MouseClick -= value; } }

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

				ctrlPanel.HorizontalScroll.Value = Math.Max(0, Math.Min((int)(hori * _scale) - ctrlPanel.Width / 2, ctrlPanel.HorizontalScroll.Maximum));
				ctrlPanel.HorizontalScroll.Value = Math.Max(0, Math.Min((int)(hori * _scale) - ctrlPanel.Width / 2, ctrlPanel.HorizontalScroll.Maximum));
				ctrlPanel.VerticalScroll.Value = Math.Max(0, Math.Min((int)(vert * _scale) - ctrlPanel.Height / 2, ctrlPanel.VerticalScroll.Maximum));
				ctrlPanel.VerticalScroll.Value = Math.Max(0, Math.Min((int)(vert * _scale) - ctrlPanel.Height / 2, ctrlPanel.VerticalScroll.Maximum));
			};
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
	}
}
