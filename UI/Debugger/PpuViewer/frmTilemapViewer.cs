using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmTilemapViewer : BaseForm
	{
		private NotificationListener _notifListener;
		private GetTilemapOptions _options;
		private DebugState _state;
		private byte[] _tilemapData;
		private Bitmap _tilemapImage;
		private bool _zoomed;

		public frmTilemapViewer()
		{
			InitializeComponent();
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			if(DesignMode) {
				return;
			}

			_options.BgMode = 0;

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;

			_tilemapImage = new Bitmap(512, 512, PixelFormat.Format32bppArgb);
			picTilemap.Image = _tilemapImage;

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
			_state = DebugApi.GetState();
			_tilemapData = DebugApi.GetTilemap(_options);
		}

		private void RefreshViewer()
		{
			int mapWidth = _state.Ppu.Layers[_options.Layer].DoubleWidth ? 512 : 256;
			int mapHeight = _state.Ppu.Layers[_options.Layer].DoubleHeight ? 512 : 256;
			if(_tilemapImage.Width != mapWidth || _tilemapImage.Height != mapHeight) {
				_tilemapImage = new Bitmap(mapWidth, mapHeight, PixelFormat.Format32bppArgb);
				picTilemap.Image = _tilemapImage;
			}
			using(Graphics g = Graphics.FromImage(_tilemapImage)) {
				GCHandle handle = GCHandle.Alloc(_tilemapData, GCHandleType.Pinned);
				Bitmap source = new Bitmap(512, 512, 4 * 512, PixelFormat.Format32bppArgb, handle.AddrOfPinnedObject());
				try {
					g.DrawImage(source, 0, 0);
				} finally {
					handle.Free();
				}
			}

			UpdateMapSize();
			picTilemap.Invalidate();
		}

		private void UpdateMapSize()
		{
			int mapWidth = _state.Ppu.Layers[_options.Layer].DoubleWidth ? 512 : 256;
			int mapHeight = _state.Ppu.Layers[_options.Layer].DoubleHeight ? 512 : 256;
			picTilemap.Width = _zoomed ? mapWidth * 2 : mapWidth;
			picTilemap.Height = _zoomed ? mapHeight * 2  : mapHeight;
		}

		private void btnLayer1_Click(object sender, EventArgs e)
		{
			_options.Layer = 0;
		}

		private void btnLayer2_Click(object sender, EventArgs e)
		{
			_options.Layer = 1;
		}

		private void btnLayer3_Click(object sender, EventArgs e)
		{
			_options.Layer = 2;
		}

		private void btnLayer4_Click(object sender, EventArgs e)
		{
			_options.Layer = 3;
		}

		private void picTilemap_DoubleClick(object sender, EventArgs e)
		{
			_zoomed = !_zoomed;
			UpdateMapSize();
		}

		private void chkShowTileGrid_Click(object sender, EventArgs e)
		{
			_options.ShowTileGrid = chkShowTileGrid.Checked;
		}

		private void chkShowScrollOverlay_Click(object sender, EventArgs e)
		{
			_options.ShowScrollOverlay = chkShowScrollOverlay.Checked;
		}
	}
}
