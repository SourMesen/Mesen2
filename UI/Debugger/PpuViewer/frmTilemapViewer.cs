using Mesen.GUI.Config;
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
		private byte[] _cgram;
		private byte[] _vram;
		private byte[] _tilemapData;
		private Bitmap _tilemapImage;
		private int _scale = 1;

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

			_tilemapData = new byte[1024 * 1024 * 4];
			_tilemapImage = new Bitmap(1024, 1024, PixelFormat.Format32bppArgb);
			picTilemap.Image = _tilemapImage;

			ctrlScanlineCycleSelect.Initialize(241, 0);

			RefreshData();
			RefreshViewer();

			InitShortcuts();
		}

		private void InitShortcuts()
		{
			mnuRefresh.InitShortcut(this, nameof(DebuggerShortcutsConfig.Refresh));
			mnuZoomIn.InitShortcut(this, nameof(DebuggerShortcutsConfig.ZoomIn));
			mnuZoomOut.InitShortcut(this, nameof(DebuggerShortcutsConfig.ZoomOut));
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
			_vram = DebugApi.GetMemoryState(SnesMemoryType.VideoRam);
			_cgram = DebugApi.GetMemoryState(SnesMemoryType.CGRam);
		}

		private int GetWidth()
		{
			return _state.Ppu.BgMode == 7 ? 1024 : _state.Ppu.Layers[_options.Layer].DoubleWidth ? 512 : 256;
		}

		private int GetHeight()
		{
			return _state.Ppu.BgMode == 7 ? 1024 : _state.Ppu.Layers[_options.Layer].DoubleHeight ? 512 : 256;
		}

		private void RefreshViewer()
		{
			DebugApi.GetTilemap(_options, _vram, _cgram, _tilemapData);

			int mapWidth = GetWidth();
			int mapHeight = GetHeight();
			if(_tilemapImage.Width != mapWidth || _tilemapImage.Height != mapHeight) {
				_tilemapImage = new Bitmap(mapWidth, mapHeight, PixelFormat.Format32bppArgb);
				picTilemap.Image = _tilemapImage;
			}
			using(Graphics g = Graphics.FromImage(_tilemapImage)) {
				GCHandle handle = GCHandle.Alloc(_tilemapData, GCHandleType.Pinned);
				Bitmap source = new Bitmap(mapWidth, mapHeight, 4 * 1024, PixelFormat.Format32bppArgb, handle.AddrOfPinnedObject());
				try {
					g.DrawImage(source, 0, 0);
				} finally {
					handle.Free();
				}
			}

			btnLayer1.BackColor = _options.Layer == 0 ? SystemColors.GradientActiveCaption : SystemColors.ControlLight;
			btnLayer2.BackColor = _options.Layer == 1 ? SystemColors.GradientActiveCaption : SystemColors.ControlLight;
			btnLayer3.BackColor = _options.Layer == 2 ? SystemColors.GradientActiveCaption : SystemColors.ControlLight;
			btnLayer4.BackColor = _options.Layer == 3 ? SystemColors.GradientActiveCaption : SystemColors.ControlLight;

			UpdateMapSize();
			picTilemap.Invalidate();
		}

		private void UpdateMapSize()
		{
			picTilemap.Width = GetWidth() * _scale;
			picTilemap.Height = GetHeight() * _scale;
		}

		private void btnLayer1_Click(object sender, EventArgs e)
		{
			_options.Layer = 0;
			RefreshViewer();
		}

		private void btnLayer2_Click(object sender, EventArgs e)
		{
			_options.Layer = 1;
			RefreshViewer();
		}

		private void btnLayer3_Click(object sender, EventArgs e)
		{
			_options.Layer = 2;
			RefreshViewer();
		}

		private void btnLayer4_Click(object sender, EventArgs e)
		{
			_options.Layer = 3;
			RefreshViewer();
		}
		
		private void chkShowTileGrid_Click(object sender, EventArgs e)
		{
			_options.ShowTileGrid = chkShowTileGrid.Checked;
		}

		private void chkShowScrollOverlay_Click(object sender, EventArgs e)
		{
			_options.ShowScrollOverlay = chkShowScrollOverlay.Checked;
		}

		private void mnuRefresh_Click(object sender, EventArgs e)
		{
			RefreshData();
			RefreshViewer();
		}

		private void mnuZoomIn_Click(object sender, EventArgs e)
		{
			_scale = Math.Min(8, _scale + 1);
			UpdateMapSize();
		}

		private void mnuZoomOut_Click(object sender, EventArgs e)
		{
			_scale = Math.Max(1, _scale - 1);
			UpdateMapSize();
		}

		private void mnuClose_Click(object sender, EventArgs e)
		{
			Close();
		}
	}
}
