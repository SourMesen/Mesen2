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

		public frmTileViewer()
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

			BaseConfigForm.InitializeComboBox(cboFormat, typeof(TileFormat));
			InitMemoryTypeDropdown();

			_tileData = new byte[512 * 512 * 4];
			_tileImage = new Bitmap(512, 512, PixelFormat.Format32bppArgb);
			ctrlImagePanel.Image = _tileImage;

			InitShortcuts();

			TileViewerConfig config = ConfigManager.Config.Debug.TileViewer;
			if(!config.WindowSize.IsEmpty) {
				this.StartPosition = FormStartPosition.Manual;
				this.Size = config.WindowSize;
				this.Location = config.WindowLocation;
			}

			cboMemoryType.SetEnumValue(config.Source);
			cboFormat.SetEnumValue(config.Format);
			nudColumns.Value = config.ColumnCount;
			nudBank.Value = config.Bank;
			nudOffset.Value = config.Offset;
			mnuAutoRefresh.Checked = config.AutoRefresh;
			chkShowTileGrid.Checked = config.ShowTileGrid;
			ctrlImagePanel.ImageScale = config.ImageScale;
			ctrlScanlineCycleSelect.Initialize(config.RefreshScanline, config.RefreshCycle);
			ctrlPaletteViewer.SelectedPalette = config.SelectedPalette;

			_options.ShowTileGrid = config.ShowTileGrid;
			_autoRefresh = config.AutoRefresh;

			RefreshData();
			RefreshViewer();
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
			config.ColumnCount = (int)nudColumns.Value;
			config.Bank = (int)nudBank.Value;
			config.Offset = (int)nudOffset.Value;
			config.SelectedPalette = ctrlPaletteViewer.SelectedPalette;

			ConfigManager.ApplyChanges();
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.CodeBreak:
				case ConsoleNotificationType.ViewerRefresh:
					if(_autoRefresh && e.Parameter.ToInt32() == ctrlScanlineCycleSelect.ViewerId) {
						if((DateTime.Now - _lastUpdate).Milliseconds > 10) {
							_lastUpdate = DateTime.Now;
							RefreshData();
							this.BeginInvoke((Action)(() => {
								this.RefreshViewer();
							}));
						}
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
				_tileImage = new Bitmap(mapWidth, mapHeight, PixelFormat.Format32bppArgb);
				ctrlImagePanel.Image = _tileImage;
			}

			using(Graphics g = Graphics.FromImage(_tileImage)) {
				GCHandle handle = GCHandle.Alloc(_tileData, GCHandleType.Pinned);
				Bitmap source = new Bitmap(mapWidth, mapHeight, 4 * mapWidth, PixelFormat.Format32bppArgb, handle.AddrOfPinnedObject());
				try {
					g.DrawImage(source, 0, 0);
				} finally {
					handle.Free();
				}
			}

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

		private void chkShowTileGrid_Click(object sender, EventArgs e)
		{
			_options.ShowTileGrid = chkShowTileGrid.Checked;
			RefreshViewer();
		}

		private void cboMemoryType_SelectedIndexChanged(object sender, EventArgs e)
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
			RefreshViewer();
		}

		private void cboBpp_SelectedIndexChanged(object sender, EventArgs e)
		{
			_options.Format = cboFormat.GetEnumValue<TileFormat>();
			if(_options.Format == TileFormat.Bpp2) {
				ctrlPaletteViewer.SelectionMode = PaletteSelectionMode.FourColors;
			} else if(_options.Format == TileFormat.Bpp4) {
				ctrlPaletteViewer.SelectionMode = PaletteSelectionMode.SixteenColors;
			} else {
				ctrlPaletteViewer.SelectionMode = PaletteSelectionMode.None;
			}

			_options.Palette = ctrlPaletteViewer.SelectedPalette;
			RefreshViewer();
		}

		private void nudColumns_ValueChanged(object sender, EventArgs e)
		{
			_options.Width = (int)nudColumns.Value;
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
	}
}
