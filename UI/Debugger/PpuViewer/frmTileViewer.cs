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
	public partial class frmTileViewer : BaseForm, IRefresh, IDebuggerWindow
	{
		private int[,] _layerBpp = new int[8, 4] { { 2,2,2,2 }, { 4,4,2,0 }, { 4,4,0,0 }, { 8,4,0,0 }, { 8,2,0,0 }, { 4,2,0,0 }, { 4,0,0,0 }, { 8,0,0,0 } };

		private GetTileViewOptions _options;
		private byte[] _tileData;
		private byte[] _cgram;
		private byte[] _tileSource;
		private Bitmap _tileImage;
		private SnesMemoryType _memoryType = SnesMemoryType.VideoRam;
		private int _address = 0;
		private DebugState _state;
		private int _selectedTile = 0;
		private WindowRefreshManager _refreshManager;
		private NotificationListener _notifListener;

		public ctrlScanlineCycleSelect ScanlineCycleSelect { get { return this.ctrlScanlineCycleSelect; } }
		public CpuType CpuType { get; private set; }

		public frmTileViewer(CpuType cpuType)
		{
			this.CpuType = cpuType;
			InitializeComponent();
			ctrlPaletteViewer.CpuType = cpuType;
			if(cpuType == CpuType.Gameboy) {
				this.Text = "GB " + this.Text;
			}
		}

		protected override void OnLoad(EventArgs evt)
		{
			base.OnLoad(evt);
			if(DesignMode) {
				return;
			}

			_tileData = new byte[0x400000];
			_tileImage = new Bitmap(512, 512, PixelFormat.Format32bppPArgb);
			ctrlImagePanel.Image = _tileImage;

			BaseConfigForm.InitializeComboBox(cboFormat, typeof(TileFormat));
			BaseConfigForm.InitializeComboBox(cboLayout, typeof(TileLayout));
			InitMemoryTypeDropdown();

			InitShortcuts();

			TileViewerConfig config = ConfigManager.Config.Debug.TileViewer;
			RestoreLocation(config.WindowLocation, config.WindowSize);

			cboMemoryType.SetEnumValue(config.Source);
			cboFormat.SetEnumValue(config.Format);
			cboLayout.SetEnumValue(config.Layout);
			nudColumns.Value = config.ColumnCount;

			UpdateMemoryType(config.Source);

			int memSize = DebugApi.GetMemorySize(_memoryType);
			config.Address = Math.Max(0, Math.Min(memSize - config.PageSize, config.Address));

			nudAddress.Value = config.Address;
			nudSize.Value = config.PageSize;
			mnuAutoRefresh.Checked = config.AutoRefresh;
			chkShowTileGrid.Checked = config.ShowTileGrid;
			ctrlImagePanel.ImageScale = config.ImageScale;
			ctrlScanlineCycleSelect.Initialize(config.RefreshScanline, config.RefreshCycle, this.CpuType);

			double scale = (double)ctrlPaletteViewer.Width / 176;
			ctrlPaletteViewer.PaletteScale = (int)(11 * scale);
			ctrlPaletteViewer.SelectedPalette = config.SelectedPalette;

			nudSize.Increment = 0x1000;
			nudSize.Minimum = 0x1000;
			nudSize.Maximum = Math.Min(memSize, 0x40000);
			nudAddress.Increment = nudSize.Value;
			nudAddress.Maximum = memSize - nudSize.Value;

			_address = config.Address;
			_options.Format = config.Format;
			_options.Layout = config.Layout;
			_options.Palette = config.SelectedPalette;
			_options.Width = config.ColumnCount;
			_options.PageSize = config.PageSize;
			ctrlImagePanel.GridSizeX = config.ShowTileGrid ? 8 : 0;
			ctrlImagePanel.GridSizeY = config.ShowTileGrid ? 8 : 0;

			_refreshManager = new WindowRefreshManager(this);
			_refreshManager.AutoRefresh = config.AutoRefresh;
			_refreshManager.AutoRefreshSpeed = config.AutoRefreshSpeed;
			mnuAutoRefreshLow.Click += (s, e) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.Low;
			mnuAutoRefreshNormal.Click += (s, e) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.Normal;
			mnuAutoRefreshHigh.Click += (s, e) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.High;
			mnuAutoRefreshSpeed.DropDownOpening += (s, e) => UpdateRefreshSpeedMenu();

			RefreshData();
			RefreshViewer();

			cboMemoryType.SelectedIndexChanged += cboMemoryType_SelectedIndexChanged;
			nudAddress.ValueChanged += nudAddress_ValueChanged;
			chkShowTileGrid.Click += chkShowTileGrid_Click;
			cboFormat.SelectedIndexChanged += cboFormat_SelectedIndexChanged;
			cboLayout.SelectedIndexChanged += cboLayout_SelectedIndexChanged;
			nudColumns.ValueChanged += nudColumns_ValueChanged;
			nudSize.ValueChanged += nudSize_ValueChanged;
			ctrlPaletteViewer.SelectionChanged += ctrlPaletteViewer_SelectionChanged;
			mnuAutoRefresh.CheckedChanged += mnuAutoRefresh_CheckedChanged;

			UpdatePaletteControl();

			btnPresetBg1.Click += (s, e) => GoToBgLayer(0);
			btnPresetBg2.Click += (s, e) => GoToBgLayer(1);
			btnPresetBg3.Click += (s, e) => GoToBgLayer(2);
			btnPresetBg4.Click += (s, e) => GoToBgLayer(3);
			btnPresetOam1.Click += (s, e) => GoToOamPreset(0);
			btnPresetOam2.Click += (s, e) => GoToOamPreset(1);

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;
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
			_notifListener?.Dispose();
			_refreshManager?.Dispose();

			TileViewerConfig config = ConfigManager.Config.Debug.TileViewer;
			config.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			config.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			config.AutoRefresh = mnuAutoRefresh.Checked;
			config.AutoRefreshSpeed = _refreshManager.AutoRefreshSpeed;
			config.ShowTileGrid = chkShowTileGrid.Checked;
			config.RefreshScanline = ctrlScanlineCycleSelect.Scanline;
			config.RefreshCycle = ctrlScanlineCycleSelect.Cycle;
			config.ImageScale = ctrlImagePanel.ImageScale;

			config.Source = cboMemoryType.GetEnumValue<SnesMemoryType>();
			config.Format = cboFormat.GetEnumValue<TileFormat>();
			config.Layout = cboLayout.GetEnumValue<TileLayout>();
			config.ColumnCount = (int)nudColumns.Value;
			config.Address = (int)nudAddress.Value;
			config.PageSize = (int)nudSize.Value;
			config.SelectedPalette = ctrlPaletteViewer.SelectedPalette;

			ConfigManager.ApplyChanges();
			base.OnFormClosed(e);
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					this.BeginInvoke((Action)(() => {
						this.InitMemoryTypeDropdown();
					}));
					break;
			}
		}

		private int GetBytesPerTile()
		{
			switch(_options.Format) {
				case TileFormat.Bpp2: return 16;
				case TileFormat.Bpp4: return 32;

				default:
				case TileFormat.Bpp8: return 64;

				case TileFormat.Mode7:
				case TileFormat.Mode7DirectColor: return 128;
			}
		}

		public void RefreshData()
		{
			_state = DebugApi.GetState();

			if(this.CpuType == CpuType.Gameboy) {
				_cgram = ctrlPaletteViewer.GetGameboyPalette();
			} else {
				_cgram = DebugApi.GetMemoryState(SnesMemoryType.CGRam);
			}

			byte[] source = DebugApi.GetMemoryState(_memoryType);

			int size = Math.Min(source.Length - _address, _options.PageSize);
			_tileSource = new byte[0x40000];
			Array.Copy(source, _address, _tileSource, 0, size);
			
			ctrlPaletteViewer.RefreshData();
		}

		public void RefreshViewer()
		{
			if(_tileSource == null) {
				return;
			}

			DebugApi.GetTileView(_options, _tileSource, _tileSource.Length, _cgram, _tileData);

			int tileCount = Math.Max(1, _options.PageSize / GetBytesPerTile());
			int mapWidth = _options.Width * 8;
			int mapHeight = (int)Math.Ceiling((double)tileCount / _options.Width) * 8;

			if(_tileImage.Width != mapWidth || _tileImage.Height != mapHeight) {
				_tileImage = new Bitmap(mapWidth, mapHeight, PixelFormat.Format32bppPArgb);
				ctrlImagePanel.Image = _tileImage;
			}

			using(Graphics g = Graphics.FromImage(_tileImage)) {
				GCHandle handle = GCHandle.Alloc(_tileData, GCHandleType.Pinned);
				Bitmap source = new Bitmap(mapWidth, mapHeight, 4 * mapWidth, PixelFormat.Format32bppPArgb, handle.AddrOfPinnedObject());
				try {
					g.DrawImage(source, 0, 0);
				} finally {
					handle.Free();
				}
			}

			int selectedColumn = _selectedTile % _options.Width;
			int selectedRow = _selectedTile / _options.Width;
			ctrlImagePanel.Selection = new Rectangle(selectedColumn * 8, selectedRow * 8, 8, 8);

			//TODO: Properly update tile address based on the selected tile layout
			txtTileAddress.Text = (_selectedTile * GetBytesPerTile() + _address).ToString("X4");

			btnPresetBg1.Enabled = _layerBpp[_state.Ppu.BgMode, 0] > 0;
			btnPresetBg2.Enabled = _layerBpp[_state.Ppu.BgMode, 1] > 0;
			btnPresetBg3.Enabled = _layerBpp[_state.Ppu.BgMode, 2] > 0;
			btnPresetBg4.Enabled = _layerBpp[_state.Ppu.BgMode, 3] > 0;

			bool isGameboy = this.CpuType == CpuType.Gameboy;
			lblPresets.Visible = !isGameboy;
			tlpPresets1.Visible = !isGameboy;
			tlpPresets2.Visible = !isGameboy;

			UpdateMapSize();
			ctrlImagePanel.Refresh();

			ctrlPaletteViewer.RefreshViewer();
		}

		private void UpdateMapSize()
		{
			int tileCount = Math.Max(1, _options.PageSize / GetBytesPerTile());
			int mapWidth = _options.Width * 8;
			int mapHeight = (int)Math.Ceiling((double)tileCount / _options.Width) * 8;

			ctrlImagePanel.ImageSize = new Size(mapWidth, mapHeight);
		}

		private void InitMemoryTypeDropdown()
		{
			cboMemoryType.BeginUpdate();
			cboMemoryType.Items.Clear();

			if(this.CpuType == CpuType.Gameboy) {
				AddGameboyTypes();
			} else {
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.VideoRam));
				cboMemoryType.Items.Add("-");
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.CpuMemory));
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.PrgRom));
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.WorkRam));
				if(DebugApi.GetMemorySize(SnesMemoryType.SaveRam) > 0) {
					cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.SaveRam));
				}

				if(DebugApi.GetMemorySize(SnesMemoryType.GsuWorkRam) > 0) {
					cboMemoryType.Items.Add("-");
					cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GsuWorkRam));
				}
				if(DebugApi.GetMemorySize(SnesMemoryType.Sa1InternalRam) > 0) {
					cboMemoryType.Items.Add("-");
					cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.Sa1InternalRam));
					cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.Sa1Memory));
				}
				if(DebugApi.GetMemorySize(SnesMemoryType.BsxPsRam) > 0) {
					cboMemoryType.Items.Add("-");
					cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.BsxPsRam));
				}
				if(DebugApi.GetMemorySize(SnesMemoryType.BsxMemoryPack) > 0) {
					cboMemoryType.Items.Add("-");
					cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.BsxMemoryPack));
				}
			}

			cboMemoryType.SelectedIndex = 0;
			cboMemoryType.EndUpdate();
		}

		private void AddGameboyTypes()
		{
			if(DebugApi.GetMemorySize(SnesMemoryType.GbPrgRom) > 0) {
				if(cboMemoryType.Items.Count > 0) {
					cboMemoryType.Items.Add("-");
				}
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GbVideoRam));
				cboMemoryType.Items.Add("-");
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GameboyMemory));
				cboMemoryType.Items.Add("-");
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GbPrgRom));
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GbWorkRam));
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GbCartRam));
			}
		}

		private void UpdatePaletteControl()
		{
			if(_options.Format == TileFormat.Bpp2) {
				ctrlPaletteViewer.SelectionMode = PaletteSelectionMode.FourColors;
			} else if(_options.Format == TileFormat.Bpp4) {
				ctrlPaletteViewer.SelectionMode = PaletteSelectionMode.SixteenColors;
			} else {
				ctrlPaletteViewer.SelectionMode = PaletteSelectionMode.None;
			}

			_options.Palette = ctrlPaletteViewer.SelectedPalette;
		}

		private void UpdateMemoryType(SnesMemoryType memType)
		{
			_memoryType = cboMemoryType.GetEnumValue<SnesMemoryType>();

			int memSize = DebugApi.GetMemorySize(_memoryType);
			nudSize.Maximum = Math.Min(0x40000, memSize);
			nudAddress.Maximum = memSize - nudSize.Value;
		}

		private void chkShowTileGrid_Click(object sender, EventArgs e)
		{
			ctrlImagePanel.GridSizeX = chkShowTileGrid.Checked ? 8 : 0;
			ctrlImagePanel.GridSizeY = chkShowTileGrid.Checked ? 8 : 0;
			RefreshViewer();
		}

		private void cboMemoryType_SelectedIndexChanged(object sender, EventArgs e)
		{
			UpdateMemoryType(cboMemoryType.GetEnumValue<SnesMemoryType>());
			
			RefreshData();
			RefreshViewer();
		}

		private void cboFormat_SelectedIndexChanged(object sender, EventArgs e)
		{
			_options.Format = cboFormat.GetEnumValue<TileFormat>();
			UpdatePaletteControl();
			RefreshViewer();
		}

		private void cboLayout_SelectedIndexChanged(object sender, EventArgs e)
		{
			_options.Layout = cboLayout.GetEnumValue<TileLayout>();

			if(_options.Layout == TileLayout.SingleLine8x16 && _options.Width % 2 != 0) {
				nudColumns.Value++;
			} else if(_options.Layout == TileLayout.SingleLine16x16 && _options.Width % 4 != 0) {
				nudColumns.Value += 4 - (_options.Width % 4);
			}

			RefreshViewer();
		}

		private void nudColumns_ValueChanged(object sender, EventArgs e)
		{
			bool smaller = _options.Width > nudColumns.Value;
			_options.Width = (int)nudColumns.Value;

			if(_options.Layout == TileLayout.SingleLine8x16 && _options.Width % 2 != 0) {
				nudColumns.Value += smaller ? -1 : 1;
			} else if(_options.Layout == TileLayout.SingleLine16x16 && _options.Width % 4 != 0) {
				nudColumns.Value += smaller ? -(_options.Width % 4) : (4 - _options.Width % 4);
			}

			RefreshViewer();
		}

		private void nudAddress_ValueChanged(object sender, EventArgs e)
		{
			_address = (int)nudAddress.Value;
			RefreshData();
			RefreshViewer();
		}

		private void nudSize_ValueChanged(object sender, EventArgs e)
		{
			_options.PageSize = (int)nudSize.Value;

			int memSize = DebugApi.GetMemorySize(_memoryType);
			nudAddress.Increment = nudSize.Value;
			nudAddress.Maximum = memSize - nudSize.Value;
			RefreshData();
			RefreshViewer();
		}

		private void ctrlPaletteViewer_SelectionChanged()
		{
			_options.Palette = ctrlPaletteViewer.SelectedPalette;
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

		private void mnuAutoRefresh_CheckedChanged(object sender, EventArgs e)
		{
			_refreshManager.AutoRefresh = mnuAutoRefresh.Checked;
		}

		private void GoToBgLayer(int layer)
		{
			int bpp = _layerBpp[_state.Ppu.BgMode, layer];
			TileFormat format = TileFormat.Bpp2;

			if(_state.Ppu.BgMode == 7) {
				format = _state.Ppu.DirectColorMode ? TileFormat.Mode7DirectColor : TileFormat.Mode7;
			} else if(bpp == 4) {
				format = TileFormat.Bpp4;
			} else if(bpp == 8) {
				format = _state.Ppu.DirectColorMode ? TileFormat.DirectColor : TileFormat.Bpp8;
			}

			cboMemoryType.SetEnumValue(SnesMemoryType.VideoRam);
			cboFormat.SetEnumValue(format);
			nudAddress.Value = 0;
			nudSize.Value = 0x10000;

			if(_state.Ppu.BgMode == 7) {
				nudAddress.Value = 0;
				_selectedTile = 0;
				RefreshViewer();
				ctrlImagePanel.ScrollTo(0);
			} else {
				int gap = _state.Ppu.Layers[layer].ChrAddress - (int)nudAddress.Value;
				int bytesPerRow = GetBytesPerTile() / 8 * _options.Width;
				int scrollRow = (gap / bytesPerRow) & ~0x7;
				_selectedTile = scrollRow * bytesPerRow / GetBytesPerTile();

				nudAddress.Value = _state.Ppu.Layers[layer].ChrAddress;
				RefreshViewer();
				ctrlImagePanel.ScrollTo(scrollRow * ctrlImagePanel.ImageScale);
			}
		}

		private void GoToOamPreset(int layer)
		{
			int address = _state.Ppu.OamBaseAddress + (layer == 1 ? _state.Ppu.OamAddressOffset : 0);
			address *= 2;

			cboMemoryType.SetEnumValue(SnesMemoryType.VideoRam);
			cboFormat.SetEnumValue(TileFormat.Bpp4);
			nudAddress.Value = 0;
			nudSize.Value = 0x10000;

			int gap = address - (int)nudAddress.Value;
			int bytesPerRow = GetBytesPerTile() / 8 * _options.Width;
			int scrollRow = (gap / bytesPerRow) & ~0x7;
			_selectedTile = scrollRow * bytesPerRow / GetBytesPerTile();

			if(ctrlPaletteViewer.SelectedPalette < 8) {
				ctrlPaletteViewer.SelectedPalette = 8;
			}
			nudAddress.Value = address * 2;
			ctrlImagePanel.ScrollTo(scrollRow * ctrlImagePanel.ImageScale);
		}

		private void ctrlImagePanel_MouseClick(object sender, MouseEventArgs e)
		{
			int selectedColumn = e.X / (8 * ctrlImagePanel.ImageScale);
			int selectedRow = e.Y / (8 * ctrlImagePanel.ImageScale);

			_selectedTile = selectedRow * _options.Width + selectedColumn;

			RefreshViewer();
		}

		private void UpdateRefreshSpeedMenu()
		{
			mnuAutoRefreshLow.Checked = _refreshManager.AutoRefreshSpeed == RefreshSpeed.Low;
			mnuAutoRefreshNormal.Checked = _refreshManager.AutoRefreshSpeed == RefreshSpeed.Normal;
			mnuAutoRefreshHigh.Checked = _refreshManager.AutoRefreshSpeed == RefreshSpeed.High;
		}
	}
}
