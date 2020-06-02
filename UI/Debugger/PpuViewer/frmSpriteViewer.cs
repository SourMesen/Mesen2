using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Debugger.PpuViewer;
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
	public partial class frmSpriteViewer : BaseForm, IRefresh
	{
		private DebugState _state;
		private byte[] _vram;
		private byte[] _cgram;
		private byte[] _oamRam;
		private byte[] _previewData;
		private Bitmap _previewImage;
		private GetSpritePreviewOptions _options = new GetSpritePreviewOptions();
		private WindowRefreshManager _refreshManager;
		private bool _isGameboyMode = false;

		public ctrlScanlineCycleSelect ScanlineCycleSelect { get { return this.ctrlScanlineCycleSelect; } }

		public frmSpriteViewer()
		{
			InitializeComponent();
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			if(DesignMode) {
				return;
			}

			InitShortcuts();

			SpriteViewerConfig config = ConfigManager.Config.Debug.SpriteViewer;
			RestoreLocation(config.WindowLocation, config.WindowSize);

			_refreshManager = new WindowRefreshManager(this);
			_refreshManager.AutoRefresh = config.AutoRefresh;
			_refreshManager.AutoRefreshSpeed = config.AutoRefreshSpeed;
			mnuAutoRefreshLow.Click += (s, evt) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.Low;
			mnuAutoRefreshNormal.Click += (s, evt) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.Normal;
			mnuAutoRefreshHigh.Click += (s, evt) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.High;
			mnuAutoRefreshSpeed.DropDownOpening += (s, evt) => UpdateRefreshSpeedMenu();

			mnuAutoRefresh.Checked = config.AutoRefresh;
			ctrlImagePanel.ImageScale = config.ImageScale;
			ctrlSplitContainer.SplitterDistance = config.SplitterDistance;
			ctrlScanlineCycleSelect.Initialize(config.RefreshScanline, config.RefreshCycle);
			ctrlSpriteList.HideOffscreenSprites = config.HideOffscreenSprites;

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

			SpriteViewerConfig config = ConfigManager.Config.Debug.SpriteViewer;
			config.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			config.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			config.AutoRefresh = mnuAutoRefresh.Checked;
			config.AutoRefreshSpeed = _refreshManager.AutoRefreshSpeed;
			config.HideOffscreenSprites = ctrlSpriteList.HideOffscreenSprites;
			config.RefreshScanline = ctrlScanlineCycleSelect.Scanline;
			config.RefreshCycle = ctrlScanlineCycleSelect.Cycle;
			config.ImageScale = ctrlImagePanel.ImageScale;
			config.SplitterDistance = ctrlSplitContainer.SplitterDistance;
			ConfigManager.ApplyChanges();
			base.OnFormClosed(e);
		}

		public void RefreshData()
		{
			_isGameboyMode = EmuApi.GetRomInfo().CoprocessorType == CoprocessorType.Gameboy;
			_state = DebugApi.GetState();
			_vram = DebugApi.GetMemoryState(_isGameboyMode ? SnesMemoryType.GbVideoRam : SnesMemoryType.VideoRam);
			_oamRam = DebugApi.GetMemoryState(_isGameboyMode ? SnesMemoryType.GbSpriteRam : SnesMemoryType.SpriteRam);
			_cgram = DebugApi.GetMemoryState(SnesMemoryType.CGRam);
		}
		
		public void RefreshViewer()
		{
			int height = _isGameboyMode ? 256 : 240;
			if(_previewImage == null || _previewImage.Height != height) {
				_previewData = new byte[256 * height * 4];
				_previewImage = new Bitmap(256, height, PixelFormat.Format32bppPArgb);
				ctrlImagePanel.ImageSize = new Size(256, height);
				ctrlImagePanel.Image = _previewImage;
			}

			ctrlSpriteList.SetData(_state, _oamRam, _state.Ppu.OamMode, _isGameboyMode);

			if(_isGameboyMode) {
				DebugApi.GetGameboySpritePreview(_options, _state.Gameboy.Ppu, _vram, _oamRam, _previewData);
			} else {
				DebugApi.GetSpritePreview(_options, _state.Ppu, _vram, _oamRam, _cgram, _previewData);
			}

			using(Graphics g = Graphics.FromImage(_previewImage)) {
				GCHandle handle = GCHandle.Alloc(_previewData, GCHandleType.Pinned);
				Bitmap source = new Bitmap(256, height, 4 * 256, PixelFormat.Format32bppPArgb, handle.AddrOfPinnedObject());
				g.DrawImage(source, 0, 0);
				handle.Free();
			}

			if(_options.SelectedSprite >= 0) {
				ctrlImagePanel.Selection = GetSpriteInfo(_options.SelectedSprite).GetBounds();
			} else {
				ctrlImagePanel.Selection = Rectangle.Empty;
			}

			ctrlImagePanel.Refresh();
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
			int x = e.X / ctrlImagePanel.ImageScale;
			int y = e.Y / ctrlImagePanel.ImageScale;

			SpriteInfo match = null;
			for(int i = 0; i < (_isGameboyMode ? 40 : 128); i++) {
				SpriteInfo sprite = GetSpriteInfo(i);
				if(x >= sprite.X && x <= sprite.X + sprite.Width) {
					int endY = (sprite.Y + sprite.Height) & 0xFF;
					bool visible = (y >= sprite.Y && y < endY) || (endY < sprite.Y && y < endY);
					if(visible) {
						match = sprite;
						break;
					}
				}
			}

			SelectSprite(match);
		}

		private SpriteInfo GetSpriteInfo(int index)
		{
			SpriteInfo sprite;
			if(_isGameboyMode) {
				sprite = SpriteInfo.GetGbSpriteInfo(_oamRam, index, _state.Gameboy.Ppu.LargeSprites, _state.Gameboy.Ppu.CgbEnabled);
			} else {
				sprite = SpriteInfo.GetSpriteInfo(_oamRam, _state.Ppu.OamMode, index);
			}

			return sprite;
		}

		private void SelectSprite(SpriteInfo sprite)
		{
			if(sprite != null) {
				_options.SelectedSprite = sprite.Index;
				ctrlSpriteList.SetSelectedIndex(sprite.Index, true);
				ctrlImagePanel.Selection = sprite.GetBounds();
				ctrlImagePanel.SelectionWrapPosition = 256;
			} else {
				_options.SelectedSprite = -1;
				ctrlSpriteList.SetSelectedIndex(-1, false);
				ctrlImagePanel.Selection = Rectangle.Empty;
			}
		}

		private void mnuAutoRefresh_CheckedChanged(object sender, EventArgs e)
		{
			_refreshManager.AutoRefresh = mnuAutoRefresh.Checked;
		}

		private void ctrlSpriteList_SpriteSelected(SpriteInfo sprite)
		{
			if(_options.SelectedSprite == sprite?.Index) {
				return;
			}

			SelectSprite(sprite);
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
