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
		private bool _zoomed;
		private SnesMemoryType _memoryType = SnesMemoryType.VideoRam;
		private int _addressOffset = 0;

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

			_options.Format = TileFormat.Bpp4;
			_options.Width = 32;
			cboFormat.SetEnumValue(TileFormat.Bpp4);
			ctrlPaletteViewer.SelectionMode = PaletteSelectionMode.SixteenColors;

			_tileData = new byte[512 * 512 * 4];
			_tileImage = new Bitmap(512, 512, PixelFormat.Format32bppArgb);
			picTilemap.Image = _tileImage;

			ctrlScanlineCycleSelect.Initialize(241, 0);

			InitMemoryTypeDropdown();

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
				case ConsoleNotificationType.CodeBreak:
				case ConsoleNotificationType.ViewerRefresh:
					if(e.Parameter.ToInt32() == ctrlScanlineCycleSelect.ViewerId) {
						RefreshData();
						this.BeginInvoke((Action)(() => {
							this.RefreshViewer();
						}));
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
			_options.Palette = ctrlPaletteViewer.SelectedPalette;
			_cgram = DebugApi.GetMemoryState(SnesMemoryType.CGRam);

			byte[] source = DebugApi.GetMemoryState(_memoryType);
			int size = Math.Min(source.Length - _addressOffset, 0x10000);
			_tileSource = new byte[0x10000];
			Array.Copy(source, _addressOffset, _tileSource, 0, size);
			
			ctrlPaletteViewer.RefreshData();
		}

		private void RefreshViewer()
		{
			DebugApi.GetTileView(_options, _tileSource, _tileSource.Length, _cgram, _tileData);

			int tileCount = 0x10000 / GetBytesPerTile();

			int mapWidth = _options.Width * 8;
			int mapHeight = tileCount / _options.Width * 8;

			if(_tileImage.Width != mapWidth || _tileImage.Height != mapHeight) {
				_tileImage = new Bitmap(mapWidth, mapHeight, PixelFormat.Format32bppArgb);
				picTilemap.Image = _tileImage;
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
			picTilemap.Invalidate();

			ctrlPaletteViewer.RefreshViewer();
		}

		private void UpdateMapSize()
		{
			int tileCount = 0x10000 / GetBytesPerTile();
			int mapWidth = _options.Width * 8;
			int mapHeight = tileCount / _options.Width * 8;

			picTilemap.Width = _zoomed ? mapWidth * 2 : mapWidth;
			picTilemap.Height = _zoomed ? mapHeight * 2  : mapHeight;
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

		private void picTilemap_DoubleClick(object sender, EventArgs e)
		{
			_zoomed = !_zoomed;
			UpdateMapSize();
		}

		private void chkShowTileGrid_Click(object sender, EventArgs e)
		{
			_options.ShowTileGrid = chkShowTileGrid.Checked;
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
		}

		private void nudColumns_ValueChanged(object sender, EventArgs e)
		{
			_options.Width = (int)nudColumns.Value;
		}

		private void nudBank_ValueChanged(object sender, EventArgs e)
		{
			_addressOffset = (int)(nudBank.Value * 0x10000 + nudOffset.Value);
		}

		private void nudOffset_ValueChanged(object sender, EventArgs e)
		{
			_addressOffset = (int)(nudBank.Value * 0x10000 + nudOffset.Value);
		}
	}
}
