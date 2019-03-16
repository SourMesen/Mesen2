using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmPaletteViewer : BaseForm
	{
		private NotificationListener _notifListener;
		private byte[] _cgRam;
		private Bitmap _paletteImage;

		public frmPaletteViewer()
		{
			InitializeComponent();
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			if(DesignMode) {
				return;
			}

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;

			_paletteImage = new Bitmap(256, 256, PixelFormat.Format32bppArgb);
			picPalette.Image = _paletteImage;

			ctrlScanlineCycleSelect.Initialize(241, 0);

			RefreshData();
			RefreshViewer();
		}

		protected override void OnFormClosed(FormClosedEventArgs e)
		{
			base.OnFormClosed(e);
			_notifListener?.Dispose();
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.ViewerRefresh:
					if(e.Parameter.ToInt32() == ctrlScanlineCycleSelect.ViewerId) {
						RefreshData();
						this.BeginInvoke((Action)(() => {
							this.RefreshViewer();
						}));
					}
					break;
			}
		}

		private void RefreshData()
		{
			_cgRam = DebugApi.GetMemoryState(SnesMemoryType.CGRam);
		}

		private void RefreshViewer()
		{
			Func<int, uint> to8Bit = (int color) => { return (uint)((color << 3) + (color >> 2)); };
			Func<int, uint> toArgb = (int rgb555) => {
				uint b = to8Bit(rgb555 >> 10);
				uint g = to8Bit((rgb555 >> 5) & 0x1F);
				uint r = to8Bit(rgb555 & 0x1F);

				return (0xFF000000 | (r << 16) | (g << 8) | b);
			};


			UInt32[] argbPalette = new UInt32[256];
			for(int i = 0; i < 256; i++) {
				argbPalette[i] = toArgb(_cgRam[i * 2] | _cgRam[i * 2 + 1] << 8);
			}

			using(Graphics g = Graphics.FromImage(_paletteImage)) {
				GCHandle handle = GCHandle.Alloc(argbPalette, GCHandleType.Pinned);
				Bitmap source = new Bitmap(16, 16, 16 * 4, PixelFormat.Format32bppArgb, handle.AddrOfPinnedObject());
				try {
					g.InterpolationMode = InterpolationMode.NearestNeighbor;
					g.SmoothingMode = SmoothingMode.None;
					g.PixelOffsetMode = PixelOffsetMode.Half;

					g.ScaleTransform(16, 16);
					g.DrawImage(source, 0, 0);
				} finally {
					handle.Free();
				}
			}
			
			picPalette.Invalidate();
		}
	}
}
