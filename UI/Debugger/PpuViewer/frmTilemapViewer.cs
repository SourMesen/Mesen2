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
		private int[,] _layerBpp = new int[8, 4] { { 2,2,2,2 }, { 4,4,2,0 }, { 4,4,0,0 }, { 8,4,0,0 }, { 8,2,0,0 }, { 4,2,0,0 }, { 4,0,0,0 }, { 8,0,0,0 } };

		private NotificationListener _notifListener;
		private GetTilemapOptions _options;
		private DebugState _state;
		private byte[] _cgram;
		private byte[] _vram;
		private byte[] _tilemapData;
		private Bitmap _tilemapImage;
		private int _scale = 1;
		private int _selectedRow = 0;
		private int _selectedColumn = 0;

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

		private bool IsLargeTileWidth
		{
			get { return _state.Ppu.Layers[_options.Layer].LargeTiles || _state.Ppu.BgMode == 5 || _state.Ppu.BgMode == 6; }
		}

		private bool IsLargeTileHeight
		{
			get { return _state.Ppu.Layers[_options.Layer].LargeTiles; }
		}

		private int GetWidth()
		{
			if(_state.Ppu.BgMode == 7) {
				return 1024;
			}

			LayerConfig layer = _state.Ppu.Layers[_options.Layer];
			bool largeTileWidth = layer.LargeTiles || _state.Ppu.BgMode == 5 || _state.Ppu.BgMode == 6;
			bool largeTileHeight = layer.LargeTiles;

			int width = 256;
			if(layer.DoubleWidth) {
				width *= 2;
			}
			if(largeTileWidth) {
				width *= 2;
			}
			return width;
		}

		private int GetHeight()
		{
			if(_state.Ppu.BgMode == 7) {
				return 1024;
			}

			LayerConfig layer = _state.Ppu.Layers[_options.Layer];

			int height = 256;
			if(layer.DoubleHeight) {
				height *= 2;
			}
			if(layer.LargeTiles) {
				height *= 2;
			}
			return height;
		}

		private void RefreshViewer()
		{
			if(_layerBpp[_state.Ppu.BgMode, _options.Layer] == 0) {
				_options.Layer = 0;
			}

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
				g.DrawImage(source, 0, 0);

				g.DrawRectangle(Pens.Gray, _selectedColumn * 8 - 2, _selectedRow * 8 - 2, IsLargeTileWidth ? 19 : 11, IsLargeTileHeight ? 19 : 11);
				g.DrawRectangle(Pens.White, _selectedColumn * 8 - 1, _selectedRow * 8 - 1, IsLargeTileWidth ? 17 : 9, IsLargeTileHeight ? 17 : 9);

				handle.Free();
			}

			btnLayer1.BackColor = _options.Layer == 0 ? SystemColors.GradientActiveCaption : Color.Empty;
			btnLayer2.BackColor = _options.Layer == 1 ? SystemColors.GradientActiveCaption : Color.Empty;
			btnLayer3.BackColor = _options.Layer == 2 ? SystemColors.GradientActiveCaption : Color.Empty;
			btnLayer4.BackColor = _options.Layer == 3 ? SystemColors.GradientActiveCaption : Color.Empty;

			btnLayer1.Enabled = _layerBpp[_state.Ppu.BgMode, 0] > 0;
			btnLayer2.Enabled = _layerBpp[_state.Ppu.BgMode, 1] > 0;
			btnLayer3.Enabled = _layerBpp[_state.Ppu.BgMode, 2] > 0;
			btnLayer4.Enabled = _layerBpp[_state.Ppu.BgMode, 3] > 0;

			UpdateMapSize();
			picTilemap.Invalidate();

			UpdateFields();
		}

		private void UpdateMapSize()
		{
			picTilemap.Width = GetWidth() * _scale;
			picTilemap.Height = GetHeight() * _scale;
		}

		private void UpdateFields()
		{
			if(_state.Ppu.BgMode == 7) {
				//Selected tile
				txtMapNumber.Text = "0";
				txtPosition.Text = _selectedColumn.ToString() + ", " + _selectedRow.ToString();
				int address = _selectedRow * 128 + _selectedColumn;
				int value = _vram[address] | (_vram[address + 1] << 8);
				txtAddress.Text = address.ToString("X4");
				txtValue.Text = value.ToString("X4");
				txtTileNumber.Text = (value & 0xFF).ToString();

				txtPalette.Text = "0";
				chkPriorityFlag.Checked = false;
				chkHorizontalMirror.Checked = false;
				chkVerticalMirror.Checked = false;

				//Tilemap
				txtMapSize.Text = "128x128";
				txtMapAddress.Text = "0000";
				txtTilesetAddress.Text = "0000";
				txtTileSize.Text = "8x8";
				txtBitDepth.Text = "8";
			} else {
				int row = (IsLargeTileHeight ? _selectedRow / 2 : _selectedRow);
				int column = (IsLargeTileWidth ? _selectedColumn / 2 : _selectedColumn);

				LayerConfig layer = _state.Ppu.Layers[_options.Layer];
				int addrVerticalScrollingOffset = layer.DoubleHeight ? ((row & 0x20) << (layer.DoubleWidth ? 6 : 5)) : 0;
				int baseOffset = (layer.TilemapAddress >> 1) + addrVerticalScrollingOffset + ((row & 0x1F) << 5);
				int address = (baseOffset + (column & 0x1F) + (layer.DoubleWidth ? ((column & 0x20) << 5) : 0)) << 1;
				int value = _vram[address] | (_vram[address + 1] << 8);

				//Selected tile
				txtMapNumber.Text = ((column >= 32 ? 1 : 0) + (row >= 32 ? 1 : 0)).ToString();
				txtPosition.Text = (column & 0x1F).ToString() + ", " + (row & 0x1F).ToString();
				txtAddress.Text = address.ToString("X4");
				txtValue.Text = value.ToString("X4");
				txtTileNumber.Text = (value & 0x3FF).ToString();
				txtPalette.Text = ((value >> 10) & 0x07).ToString();
				chkPriorityFlag.Checked = (value & 0x2000) != 0;
				chkHorizontalMirror.Checked = (value & 0x4000) != 0;
				chkVerticalMirror.Checked = (value & 0x8000) != 0;
				
				//Tilemap
				txtMapSize.Text = (layer.DoubleWidth ? "64" : "32") + "x" + (layer.DoubleHeight ? "64" : "32");
				txtMapAddress.Text = layer.TilemapAddress.ToString("X4");
				txtTilesetAddress.Text = layer.ChrAddress.ToString("X4");
				txtTileSize.Text = (IsLargeTileWidth ? "16" : "8") + "x" + (IsLargeTileHeight ? "16" : "8");
				txtBitDepth.Text = _layerBpp[_state.Ppu.BgMode, _options.Layer].ToString();
			}
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
			RefreshViewer();
		}

		private void chkShowScrollOverlay_Click(object sender, EventArgs e)
		{
			_options.ShowScrollOverlay = chkShowScrollOverlay.Checked;
			RefreshViewer();
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

		private void picTilemap_MouseClick(object sender, MouseEventArgs e)
		{
			_selectedColumn = e.X / (8 * _scale);
			_selectedRow = e.Y / (8 * _scale);

			if(IsLargeTileWidth) {
				_selectedColumn &= 0xFE;
			}
			if(IsLargeTileHeight) {
				_selectedRow &= 0xFE;
			}

			RefreshViewer();
		}
	}
}
