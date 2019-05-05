using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using Mesen.GUI.Controls;
using System.Drawing.Imaging;
using System.Drawing.Drawing2D;

namespace Mesen.GUI.Debugger
{
	public partial class ctrlPaletteViewer : PictureBox
	{
		public delegate void SelectionChangedHandler();
		public event SelectionChangedHandler SelectionChanged;

		private byte[] _cgRam;
		private Bitmap _paletteImage;
		private int _selectedPalette = 0;
		private PaletteSelectionMode _selectionMode = PaletteSelectionMode.None;
		private int _paletteScale = 16;

		public ctrlPaletteViewer()
		{
			this.SetStyle(ControlStyles.Selectable, true);
			this.PaletteScale = 16;
		}

		public int PaletteScale
		{
			get { return _paletteScale; }
			set
			{
				_paletteScale = value;
				_paletteImage = new Bitmap(PaletteScale * 16, PaletteScale * 16, PixelFormat.Format32bppArgb);
			}
		}

		public int SelectedPalette
		{
			get { return _selectedPalette; }
			set
			{
				int maxPalette = 0;
				switch(this.SelectionMode) {
					case PaletteSelectionMode.SingleColor: maxPalette = 255; break;
					case PaletteSelectionMode.FourColors: maxPalette = 63; break;
					case PaletteSelectionMode.SixteenColors: maxPalette = 15; break;
				}

				int newPalette = Math.Max(0, Math.Min(value, maxPalette));

				if(newPalette != _selectedPalette) {
					_selectedPalette = newPalette;
					SelectionChanged?.Invoke();
				}
			}
		}

		public PaletteSelectionMode SelectionMode
		{
			get { return _selectionMode; }
			set
			{
				_selectionMode = value;
				this.SelectedPalette = _selectedPalette;
			}
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			switch(keyData) {
				case Keys.Left:
					this.SelectedPalette--;
					RefreshViewer();
					return true;

				case Keys.Right:
					this.SelectedPalette++;
					RefreshViewer();
					return true;

				case Keys.Down:
					switch(this.SelectionMode) {
						case PaletteSelectionMode.SingleColor: this.SelectedPalette+=16; break;
						case PaletteSelectionMode.FourColors: this.SelectedPalette+=4; break;
						case PaletteSelectionMode.SixteenColors: this.SelectedPalette++; break;
					}
					RefreshViewer();
					return true;

				case Keys.Up:
					switch(this.SelectionMode) {
						case PaletteSelectionMode.SingleColor: this.SelectedPalette-=16; break;
						case PaletteSelectionMode.FourColors: this.SelectedPalette-=4; break;
						case PaletteSelectionMode.SixteenColors: this.SelectedPalette--; break;
					}
					RefreshViewer();
					return true;
			}

			return base.ProcessCmdKey(ref msg, keyData);
		}

		public byte[] CgRam
		{
			get { return _cgRam; }
		}

		public void RefreshData()
		{
			_cgRam = DebugApi.GetMemoryState(SnesMemoryType.CGRam);
		}

		private uint To8Bit(int color)
		{
			return (uint)((color << 3) + (color >> 2));
		}

		public uint ToArgb(int rgb555)
		{
			uint b = To8Bit(rgb555 >> 10);
			uint g = To8Bit((rgb555 >> 5) & 0x1F);
			uint r = To8Bit(rgb555 & 0x1F);

			return (0xFF000000 | (r << 16) | (g << 8) | b);
		}

		public void RefreshViewer()
		{
			UInt32[] argbPalette = new UInt32[256];
			for(int i = 0; i < 256; i++) {
				argbPalette[i] = ToArgb(_cgRam[i * 2] | _cgRam[i * 2 + 1] << 8);
			}

			using(Graphics g = Graphics.FromImage(_paletteImage)) {
				GCHandle handle = GCHandle.Alloc(argbPalette, GCHandleType.Pinned);
				Bitmap source = new Bitmap(16, 16, 16 * 4, PixelFormat.Format32bppArgb, handle.AddrOfPinnedObject());
				try {
					g.InterpolationMode = InterpolationMode.NearestNeighbor;
					g.SmoothingMode = SmoothingMode.None;
					g.PixelOffsetMode = PixelOffsetMode.Half;

					g.ScaleTransform(PaletteScale, PaletteScale);
					g.DrawImage(source, 0, 0);

					g.ResetTransform();
					using(Pen pen = new Pen(Color.LightGray, 2)) {
						pen.DashStyle = DashStyle.Dash;
						if(SelectionMode == PaletteSelectionMode.SingleColor) {
							g.DrawRectangle(pen, (SelectedPalette & 0x0F) * PaletteScale, (SelectedPalette / 16) * PaletteScale, PaletteScale, PaletteScale);
						} else if(SelectionMode == PaletteSelectionMode.FourColors) {
							g.DrawRectangle(pen, (SelectedPalette & 0x03) * PaletteScale * 4, (SelectedPalette / 4) * PaletteScale, PaletteScale * 4, PaletteScale);
						} else if(SelectionMode == PaletteSelectionMode.SixteenColors) {
							g.DrawRectangle(pen, 0, SelectedPalette * PaletteScale, PaletteScale * 16, PaletteScale);
						}
					}
				} finally {
					handle.Free();
				}
			}

			this.Image = _paletteImage;
			this.Size = _paletteImage.Size;
			this.Invalidate();
		}

		protected override void OnMouseClick(MouseEventArgs e)
		{
			base.OnMouseClick(e);

			int paletteIndex = 0;
			if(SelectionMode == PaletteSelectionMode.SingleColor) {
				paletteIndex = (e.Y / PaletteScale) * 16 + (e.X / PaletteScale);
			} else if(SelectionMode == PaletteSelectionMode.FourColors) {
				paletteIndex += e.X / (4 * PaletteScale);
				paletteIndex += (e.Y / PaletteScale) * 4;
			} else if(SelectionMode == PaletteSelectionMode.SixteenColors) {
				paletteIndex = (e.Y / PaletteScale);
			}
			SelectedPalette = paletteIndex;

			RefreshViewer();

			this.Focus();
		}
	}

	public enum PaletteSelectionMode
	{
		None,
		SingleColor,
		FourColors,
		SixteenColors
	}
}
