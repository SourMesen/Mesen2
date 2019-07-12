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
	public partial class frmTileViewer : BaseForm
	{
		private int[,] _layerBpp = new int[8, 4] { { 2,2,2,2 }, { 4,4,2,0 }, { 4,4,0,0 }, { 8,4,0,0 }, { 8,2,0,0 }, { 4,2,0,0 }, { 4,0,0,0 }, { 8,0,0,0 } };

		private NotificationListener _notifListener;
		private GetTileViewOptions _options;
		private byte[] _tileData;
		private byte[] _cgram;
		private byte[] _tileSource;
		private Bitmap _tileImage;
		private SnesMemoryType _memoryType = SnesMemoryType.VideoRam;
		private int _addressOffset = 0;
		private DateTime _lastUpdate = DateTime.MinValue;
		private bool _autoRefresh = true;
		private DebugState _state;
		private int _selectedTile = 0;
		
		public frmTileViewer()
		{
			InitializeComponent();
		}

		protected override void OnLoad(EventArgs evt)
		{
			base.OnLoad(evt);
			if(DesignMode) {
				return;
			}

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;

			_tileData = new byte[512 * 512 * 4];
			_tileImage = new Bitmap(512, 512, PixelFormat.Format32bppPArgb);
			ctrlImagePanel.Image = _tileImage;

			BaseConfigForm.InitializeComboBox(cboFormat, typeof(TileFormat));
			BaseConfigForm.InitializeComboBox(cboLayout, typeof(TileLayout));
			InitMemoryTypeDropdown();

			InitShortcuts();

			TileViewerConfig config = ConfigManager.Config.Debug.TileViewer;
			if(!config.WindowSize.IsEmpty) {
				this.StartPosition = FormStartPosition.Manual;
				this.Size = config.WindowSize;
				this.Location = config.WindowLocation;
			}

			cboMemoryType.SetEnumValue(config.Source);
			cboFormat.SetEnumValue(config.Format);
			cboLayout.SetEnumValue(config.Layout);
			nudColumns.Value = config.ColumnCount;
			nudBank.Value = config.Bank;
			nudOffset.Value = config.Offset;
			mnuAutoRefresh.Checked = config.AutoRefresh;
			chkShowTileGrid.Checked = config.ShowTileGrid;
			ctrlImagePanel.ImageScale = config.ImageScale;
			ctrlScanlineCycleSelect.Initialize(config.RefreshScanline, config.RefreshCycle);
			ctrlPaletteViewer.SelectedPalette = config.SelectedPalette;

			UpdateMemoryType(config.Source);
			_addressOffset = config.Bank * 0x10000 + config.Offset;
			_options.Format = config.Format;
			_options.Layout = config.Layout;
			_options.Palette = config.SelectedPalette;
			_options.Width = config.ColumnCount;
			ctrlImagePanel.GridSizeX = config.ShowTileGrid ? 8 : 0;
			ctrlImagePanel.GridSizeY = config.ShowTileGrid ? 8 : 0;
			_autoRefresh = config.AutoRefresh;

			RefreshData();
			RefreshViewer();

			cboMemoryType.SelectedIndexChanged += cboMemoryType_SelectedIndexChanged;
			nudBank.ValueChanged += nudBank_ValueChanged;
			chkShowTileGrid.Click += chkShowTileGrid_Click;
			cboFormat.SelectedIndexChanged += cboFormat_SelectedIndexChanged;
			cboLayout.SelectedIndexChanged += cboLayout_SelectedIndexChanged;
			nudColumns.ValueChanged += nudColumns_ValueChanged;
			nudOffset.ValueChanged += nudOffset_ValueChanged;
			ctrlPaletteViewer.SelectionChanged += ctrlPaletteViewer_SelectionChanged;
			mnuAutoRefresh.CheckedChanged += mnuAutoRefresh_CheckedChanged;

			UpdatePaletteControl();

			btnPresetBg1.Click += (s, e) => GoToBgLayer(0);
			btnPresetBg2.Click += (s, e) => GoToBgLayer(1);
			btnPresetBg3.Click += (s, e) => GoToBgLayer(2);
			btnPresetBg4.Click += (s, e) => GoToBgLayer(3);
			btnPresetOam1.Click += (s, e) => GoToOamPreset(0);
			btnPresetOam2.Click += (s, e) => GoToOamPreset(1);
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
			base.OnFormClosed(e);
			_notifListener?.Dispose();

			TileViewerConfig config = ConfigManager.Config.Debug.TileViewer;
			config.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			config.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			config.AutoRefresh = mnuAutoRefresh.Checked;
			config.ShowTileGrid = chkShowTileGrid.Checked;
			config.RefreshScanline = ctrlScanlineCycleSelect.Scanline;
			config.RefreshCycle = ctrlScanlineCycleSelect.Cycle;
			config.ImageScale = ctrlImagePanel.ImageScale;

			config.Source = cboMemoryType.GetEnumValue<SnesMemoryType>();
			config.Format = cboFormat.GetEnumValue<TileFormat>();
			config.Layout = cboLayout.GetEnumValue<TileLayout>();
			config.ColumnCount = (int)nudColumns.Value;
			config.Bank = (int)nudBank.Value;
			config.Offset = (int)nudOffset.Value;
			config.SelectedPalette = ctrlPaletteViewer.SelectedPalette;

			ConfigManager.ApplyChanges();
		}

		private void RefreshContent()
		{
			_lastUpdate = DateTime.Now;
			RefreshData();
			this.BeginInvoke((Action)(() => {
				this.RefreshViewer();
			}));
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.CodeBreak:
					RefreshContent();
					break;

				case ConsoleNotificationType.ViewerRefresh:
					if(_autoRefresh && e.Parameter.ToInt32() == ctrlScanlineCycleSelect.ViewerId && (DateTime.Now - _lastUpdate).Milliseconds > 10) {
						RefreshContent();
					}
					break;

				case ConsoleNotificationType.GameLoaded:
					//Configuration is lost when debugger is restarted (when switching game or power cycling)
					ctrlScanlineCycleSelect.RefreshSettings();
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

		private void RefreshData()
		{
			_state = DebugApi.GetState();
			_cgram = DebugApi.GetMemoryState(SnesMemoryType.CGRam);

			byte[] source = DebugApi.GetMemoryState(_memoryType);
			int size = Math.Min(source.Length - _addressOffset, 0x10000);
			_tileSource = new byte[0x10000];
			Array.Copy(source, _addressOffset, _tileSource, 0, size);
			
			ctrlPaletteViewer.RefreshData();
		}

		private void RefreshViewer()
		{
			if(_tileSource == null) {
				return;
			}

			DebugApi.GetTileView(_options, _tileSource, _tileSource.Length, _cgram, _tileData);

			int tileCount = 0x10000 / GetBytesPerTile();

			int mapWidth = _options.Width * 8;
			int mapHeight = tileCount / _options.Width * 8;

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
			txtTileAddress.Text = (_selectedTile * GetBytesPerTile() + _addressOffset).ToString("X4");

			btnPresetBg1.Enabled = _layerBpp[_state.Ppu.BgMode, 0] > 0;
			btnPresetBg2.Enabled = _layerBpp[_state.Ppu.BgMode, 1] > 0;
			btnPresetBg3.Enabled = _layerBpp[_state.Ppu.BgMode, 2] > 0;
			btnPresetBg4.Enabled = _layerBpp[_state.Ppu.BgMode, 3] > 0;

			UpdateMapSize();
			ctrlImagePanel.Invalidate();

			ctrlPaletteViewer.RefreshViewer();
		}

		private void UpdateMapSize()
		{
			int tileCount = 0x10000 / GetBytesPerTile();
			int mapWidth = _options.Width * 8;
			int mapHeight = tileCount / _options.Width * 8;

			ctrlImagePanel.ImageSize = new Size(mapWidth, mapHeight);
		}

		private void InitMemoryTypeDropdown()
		{
			cboMemoryType.BeginUpdate();
			cboMemoryType.Items.Clear();

			cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.VideoRam));
			cboMemoryType.Items.Add("-");
			cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.PrgRom));
			cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.WorkRam));

			cboMemoryType.SelectedIndex = 0;
			cboMemoryType.EndUpdate();
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

			bool isVram = _memoryType == SnesMemoryType.VideoRam;
			nudOffset.Visible = !isVram;
			nudBank.Visible = !isVram;
			lblOffset.Visible = !isVram;
			lblBank.Visible = !isVram;
			if(isVram) {
				nudBank.Value = 0;
				nudOffset.Value = 0;
			}

			nudBank.Maximum = Math.Max(1, (DebugApi.GetMemorySize(_memoryType) / 0x10000) - 1);
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

		private void nudBank_ValueChanged(object sender, EventArgs e)
		{
			_addressOffset = (int)(nudBank.Value * 0x10000 + nudOffset.Value);
			RefreshData();
			RefreshViewer();
		}

		private void nudOffset_ValueChanged(object sender, EventArgs e)
		{
			_addressOffset = (int)(nudBank.Value * 0x10000 + nudOffset.Value);
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
			_autoRefresh = mnuAutoRefresh.Checked;
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

			if(_state.Ppu.BgMode == 7) {
				_selectedTile = 0;
				RefreshViewer();
				ctrlImagePanel.ScrollTo(0);
			} else {
				int bytesPerRow = GetBytesPerTile() / 8 * _options.Width;
				int scrollRow = ((_state.Ppu.Layers[layer].ChrAddress << 1) / bytesPerRow) & 0xFFF8;
				_selectedTile = scrollRow * bytesPerRow / GetBytesPerTile();
				RefreshViewer();
				ctrlImagePanel.ScrollTo(scrollRow * ctrlImagePanel.ImageScale);
			}			
		}

		private void GoToOamPreset(int layer)
		{
			int bpp = _layerBpp[_state.Ppu.BgMode, layer];
			TileFormat format = TileFormat.Bpp4;
			int address = _state.Ppu.OamBaseAddress + (layer == 1 ? _state.Ppu.OamAddressOffset : 0);

			cboMemoryType.SetEnumValue(SnesMemoryType.VideoRam);
			cboFormat.SetEnumValue(format);

			int bytesPerRow = GetBytesPerTile() / 8 * _options.Width;
			int scrollRow = (address * 2 / bytesPerRow) & 0xFFF8;
			_selectedTile = scrollRow * bytesPerRow / GetBytesPerTile();
			RefreshViewer();
			ctrlImagePanel.ScrollTo(scrollRow * ctrlImagePanel.ImageScale);
		}

		private void ctrlImagePanel_MouseClick(object sender, MouseEventArgs e)
		{
			int selectedColumn = e.X / (8 * ctrlImagePanel.ImageScale);
			int selectedRow = e.Y / (8 * ctrlImagePanel.ImageScale);

			_selectedTile = selectedRow * _options.Width + selectedColumn;

			RefreshViewer();
		}
	}
}
