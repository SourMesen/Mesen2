using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Controls;
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
	public partial class frmTilemapViewer : BaseForm, IRefresh, IDebuggerWindow
	{
		private int[,] _layerBpp = new int[8, 4] { { 2,2,2,2 }, { 4,4,2,0 }, { 4,4,0,0 }, { 8,4,0,0 }, { 8,2,0,0 }, { 4,2,0,0 }, { 4,0,0,0 }, { 8,0,0,0 } };

		private GetTilemapOptions _options;
		private DebugState _state;
		private byte[] _cgram;
		private byte[] _vram;
		private byte[] _tilemapData;
		private Bitmap _tilemapImage;
		private int _selectedRow = 0;
		private int _selectedColumn = 0;
		private DateTime _lastUpdate = DateTime.MinValue;
		private WindowRefreshManager _refreshManager;
		
		public CpuType CpuType { get; private set; }

		public frmTilemapViewer(CpuType cpuType)
		{
			this.CpuType = cpuType;
			InitializeComponent();
			if(cpuType == CpuType.Gameboy) {
				this.Text = "GB " + this.Text;
			}
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			if(DesignMode) {
				return;
			}

			_tilemapData = new byte[1024 * 1024 * 4];
			_tilemapImage = new Bitmap(1024, 1024, PixelFormat.Format32bppPArgb);
			ctrlImagePanel.Image = _tilemapImage;
			
			InitShortcuts();

			TilemapViewerConfig config = ConfigManager.Config.Debug.TilemapViewer;
			RestoreLocation(config.WindowLocation, config.WindowSize);

			mnuAutoRefresh.Checked = config.AutoRefresh;
			chkShowTileGrid.Checked = config.ShowTileGrid;
			chkShowScrollOverlay.Checked = config.ShowScrollOverlay;
			ctrlImagePanel.ImageScale = config.ImageScale;
			ctrlScanlineCycleSelect.Initialize(config.RefreshScanline, config.RefreshCycle, this.CpuType);

			_refreshManager = new WindowRefreshManager(this);
			_refreshManager.AutoRefresh = config.AutoRefresh;
			_refreshManager.AutoRefreshSpeed = config.AutoRefreshSpeed;

			mnuAutoRefreshLow.Click += (s, evt) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.Low;
			mnuAutoRefreshNormal.Click += (s, evt) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.Normal;
			mnuAutoRefreshHigh.Click += (s, evt) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.High;
			mnuAutoRefreshSpeed.DropDownOpening += (s, evt) => UpdateRefreshSpeedMenu();

			RefreshData();
			RefreshViewer();
		}

		private void InitShortcuts()
		{
			mnuRefresh.InitShortcut(this, nameof(DebuggerShortcutsConfig.Refresh));
			mnuZoomIn.InitShortcut(this, nameof(DebuggerShortcutsConfig.ZoomIn));
			mnuZoomOut.InitShortcut(this, nameof(DebuggerShortcutsConfig.ZoomOut));

			mnuCopyToClipboard.InitShortcut(this, nameof(DebuggerShortcutsConfig.Copy));
			mnuSaveAsPng.InitShortcut(this, nameof(DebuggerShortcutsConfig.SaveAsPng));

			mnuCopyToClipboard.Click += (s, e) => { ctrlImagePanel.CopyToClipboard(); };
			mnuSaveAsPng.Click += (s, e) => { ctrlImagePanel.SaveAsPng(); };
		}

		protected override void OnFormClosed(FormClosedEventArgs e)
		{
			_refreshManager?.Dispose();

			TilemapViewerConfig config = ConfigManager.Config.Debug.TilemapViewer;
			config.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			config.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			config.AutoRefresh = mnuAutoRefresh.Checked;
			config.AutoRefreshSpeed = _refreshManager.AutoRefreshSpeed;
			config.ShowTileGrid = chkShowTileGrid.Checked;
			config.ShowScrollOverlay = chkShowScrollOverlay.Checked;
			config.RefreshScanline = ctrlScanlineCycleSelect.Scanline;
			config.RefreshCycle = ctrlScanlineCycleSelect.Cycle;
			config.ImageScale = ctrlImagePanel.ImageScale;
			ConfigManager.ApplyChanges();
			base.OnFormClosed(e);
		}

		public void RefreshData()
		{
			_state = DebugApi.GetState();
			_vram = DebugApi.GetMemoryState(this.CpuType == CpuType.Gameboy ? SnesMemoryType.GbVideoRam : SnesMemoryType.VideoRam);
			_cgram = DebugApi.GetMemoryState(SnesMemoryType.CGRam);
		}

		private bool IsDoubleWidthScreen
		{
			get { return _state.Ppu.HiResMode || _state.Ppu.BgMode == 5 || _state.Ppu.BgMode == 6; }
		}

		private bool IsDoubleHeightScreen
		{
			get { return _state.Ppu.ScreenInterlace || _state.Ppu.BgMode == 5 || _state.Ppu.BgMode == 6; }
		}

		private bool IsLargeTileWidth
		{
			get { return _state.Ppu.Layers[_options.Layer].LargeTiles || _state.Ppu.BgMode == 5 || _state.Ppu.BgMode == 6; }
		}

		private bool IsLargeTileHeight
		{
			get { return _state.Ppu.Layers[_options.Layer].LargeTiles; }
		}

		public ctrlScanlineCycleSelect ScanlineCycleSelect
		{
			get { return this.ctrlScanlineCycleSelect; }
		}

		private int GetWidth()
		{
			if(this.CpuType == CpuType.Gameboy) {
				return 256;
			} else if(_state.Ppu.BgMode == 7) {
				return 1024;
			}

			LayerConfig layer = _state.Ppu.Layers[_options.Layer];
			bool largeTileWidth = layer.LargeTiles || _state.Ppu.BgMode == 5 || _state.Ppu.BgMode == 6;

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
			if(this.CpuType == CpuType.Gameboy) {
				return 256;
			} else if(_state.Ppu.BgMode == 7) {
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

		public void RefreshViewer()
		{
			bool isGameboy = this.CpuType == CpuType.Gameboy;
			if(_layerBpp[_state.Ppu.BgMode, _options.Layer] == 0) {
				_options.Layer = 0;
			}

			if(isGameboy) {
				DebugApi.GetGameboyTilemap(_vram, _state.Gameboy.Ppu, (ushort)(_options.Layer == 0 ? 0x1800 : 0x1C00), _tilemapData);
			} else {
				DebugApi.GetTilemap(_options, _state.Ppu, _vram, _cgram, _tilemapData);
			}

			int mapWidth = GetWidth();
			int mapHeight = GetHeight();
			if(_tilemapImage.Width != mapWidth || _tilemapImage.Height != mapHeight) {
				_tilemapImage = new Bitmap(mapWidth, mapHeight, PixelFormat.Format32bppPArgb);
				ctrlImagePanel.Image = _tilemapImage;
			}
			using(Graphics g = Graphics.FromImage(_tilemapImage)) {
				GCHandle handle = GCHandle.Alloc(_tilemapData, GCHandleType.Pinned);
				Bitmap source = new Bitmap(mapWidth, mapHeight, 4 * 1024, PixelFormat.Format32bppPArgb, handle.AddrOfPinnedObject());
				g.DrawImage(source, 0, 0);
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

			btnLayer1.Text = isGameboy ? "BG" : "1";
			btnLayer2.Text = isGameboy ? "Window" : "2";
			btnLayer2.Width = isGameboy ? 64 : 32;

			btnLayer3.Visible = !isGameboy;
			btnLayer4.Visible = !isGameboy;

			lblMap.Visible = !isGameboy;
			txtMapNumber.Visible = !isGameboy;
			lblValue.Visible = !isGameboy;
			txtValue.Visible = !isGameboy;

			ctrlImagePanel.ImageSize = new Size(GetWidth(), GetHeight());
			ctrlImagePanel.Selection = new Rectangle(_selectedColumn * 8, _selectedRow * 8, IsLargeTileWidth ? 16 : 8, IsLargeTileHeight ? 16 : 8);

			ctrlImagePanel.GridSizeX = chkShowTileGrid.Checked ? (IsLargeTileWidth ? 16 : 8): 0;
			ctrlImagePanel.GridSizeY = chkShowTileGrid.Checked ? (IsLargeTileHeight ? 16 : 8): 0;

			if(chkShowScrollOverlay.Checked) {
				if(isGameboy) {
					if(_options.Layer == 0) {
						GbPpuState ppu = _state.Gameboy.Ppu;
						ctrlImagePanel.Overlay = new Rectangle(ppu.ScrollX, ppu.ScrollY, 160, 144);
					} else {
						//Hide for window, doesn't make sense to show this
						ctrlImagePanel.Overlay = Rectangle.Empty;
					}
				} else {
					LayerConfig layer = _state.Ppu.Layers[_options.Layer];
					int hScroll = _state.Ppu.BgMode == 7 ? (int)_state.Ppu.Mode7.HScroll : layer.HScroll;
					int vScroll = _state.Ppu.BgMode == 7 ? (int)_state.Ppu.Mode7.VScroll : layer.VScroll;
					int height = _state.Ppu.OverscanMode ? 239 : 224;
					ctrlImagePanel.Overlay = new Rectangle(hScroll, vScroll, IsDoubleWidthScreen ? 512 : 256, IsDoubleHeightScreen ? height * 2 : height);
				}
			} else {
				ctrlImagePanel.Overlay = Rectangle.Empty;
			}
			ctrlImagePanel.Refresh();

			if(isGameboy) {
				UpdateGameboyFields();
			} else {
				UpdateFields();
			}
		}

		private void UpdateFields()
		{
			if(_state.Ppu.BgMode == 7) {
				//Selected tile
				txtMapNumber.Text = "0";
				txtPosition.Text = _selectedColumn.ToString() + ", " + _selectedRow.ToString();
				int address = _selectedRow * 256 + _selectedColumn * 2;
				int value = _vram[address] | (_vram[address + 1] << 8);
				txtAddress.Text = address.ToString("X4");
				txtValue.Text = value.ToString("X4");
				txtTileNumber.Text = (value & 0xFF).ToString();
				txtTileAddress.Text = ((value & 0xFF) * 128).ToString("X4");

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
				int baseOffset = layer.TilemapAddress + addrVerticalScrollingOffset + ((row & 0x1F) << 5);
				int address = ((baseOffset + (column & 0x1F) + (layer.DoubleWidth ? ((column & 0x20) << 5) : 0)) << 1) & 0xFFFF;
				int value = _vram[address] | (_vram[address + 1] << 8);

				//Selected tile
				txtMapNumber.Text = ((column >= 32 ? 1 : 0) + (row >= 32 ? 2 : 0)).ToString();
				txtPosition.Text = (column & 0x1F).ToString() + ", " + (row & 0x1F).ToString();
				txtAddress.Text = address.ToString("X4");
				txtValue.Text = value.ToString("X4");

				int bpp = _layerBpp[_state.Ppu.BgMode, _options.Layer];
				int tileNumber = (value & 0x3FF);
				txtTileNumber.Text = tileNumber.ToString();
				txtTileAddress.Text = ((layer.ChrAddress << 1) + (tileNumber * bpp * 8)).ToString("X4");

				txtPalette.Text = ((value >> 10) & 0x07).ToString();
				chkPriorityFlag.Checked = (value & 0x2000) != 0;
				chkHorizontalMirror.Checked = (value & 0x4000) != 0;
				chkVerticalMirror.Checked = (value & 0x8000) != 0;
				
				//Tilemap
				txtMapSize.Text = (layer.DoubleWidth ? "64" : "32") + "x" + (layer.DoubleHeight ? "64" : "32");
				txtMapAddress.Text = (layer.TilemapAddress << 1).ToString("X4");
				txtTilesetAddress.Text = (layer.ChrAddress << 1).ToString("X4");
				txtTileSize.Text = (IsLargeTileWidth ? "16" : "8") + "x" + (IsLargeTileHeight ? "16" : "8");
				txtBitDepth.Text = bpp.ToString();
			}
		}

		private void UpdateGameboyFields()
		{
			GbPpuState state = _state.Gameboy.Ppu;
			bool isGbc = state.CgbEnabled;
			bool tilemapSelect = _options.Layer == 1 ? state.WindowTilemapSelect : state.BgTilemapSelect;
			int tilemapAddress = tilemapSelect ? 0x1C00 : 0x1800;
			int tilesetAddress = state.BgTileSelect ? 0x0000 : 0x1000;

			int row = _selectedRow;
			int column = _selectedColumn;

			int address = tilemapAddress + (row * 32) + column;
			byte tileNumber = _vram[address & 0x1FFF];
			int attributes = isGbc ? _vram[0x2000 | (address & 0x1FFF)] : 0;

			//Selected tile
			txtPosition.Text = column.ToString() + ", " + row.ToString();
			txtAddress.Text = address.ToString("X4");
			txtTileNumber.Text = tileNumber.ToString() + " ($" + tileNumber.ToString("X2") + ")";

			int tileAddress = tilesetAddress + (state.BgTileSelect ? tileNumber : (int)(sbyte)tileNumber) * 16;
			if((attributes & 0x08) != 0) {
				tileAddress |= 0x2000;
			}
			txtTileAddress.Text = tileAddress.ToString("X4");

			txtPalette.Text = (attributes & 0x07).ToString();
			chkPriorityFlag.Checked = (attributes & 0x80) != 0;
			chkVerticalMirror.Checked = (attributes & 0x40) != 0;
			chkHorizontalMirror.Checked = (attributes & 0x20) != 0;

			//Tilemap
			txtMapSize.Text = "32x32";
			txtMapAddress.Text = (0x8000 | tilemapAddress).ToString("X4");
			txtTilesetAddress.Text = (0x8000 | tilesetAddress).ToString("X4");
			txtTileSize.Text = "8x8";
			txtBitDepth.Text = "2";
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
			RefreshViewer();
		}

		private void chkShowScrollOverlay_Click(object sender, EventArgs e)
		{
			RefreshViewer();
		}

		private void mnuRefresh_Click(object sender, EventArgs e)
		{
			RefreshData();
			RefreshViewer();
		}

		private void mnuZoomIn_Click(object sender, EventArgs e)
		{
			ctrlImagePanel.ZoomIn();
		}

		private void mnuZoomOut_Click(object sender, EventArgs e)
		{
			ctrlImagePanel.ZoomOut();
		}

		private void mnuClose_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void ctrlImagePanel_MouseClick(object sender, MouseEventArgs e)
		{
			_selectedColumn = e.X / (8 * ctrlImagePanel.ImageScale);
			_selectedRow = e.Y / (8 * ctrlImagePanel.ImageScale);

			if(IsLargeTileWidth) {
				_selectedColumn &= 0xFE;
			}
			if(IsLargeTileHeight) {
				_selectedRow &= 0xFE;
			}

			RefreshViewer();
		}

		private void mnuAutoRefresh_CheckedChanged(object sender, EventArgs e)
		{
			_refreshManager.AutoRefresh = mnuAutoRefresh.Checked;
		}

		private void UpdateRefreshSpeedMenu()
		{
			mnuAutoRefreshLow.Checked = _refreshManager.AutoRefreshSpeed == RefreshSpeed.Low;
			mnuAutoRefreshNormal.Checked = _refreshManager.AutoRefreshSpeed == RefreshSpeed.Normal;
			mnuAutoRefreshHigh.Checked = _refreshManager.AutoRefreshSpeed == RefreshSpeed.High;
		}
	}
}
