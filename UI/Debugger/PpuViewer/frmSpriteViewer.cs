using Mesen.GUI.Config;
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
	public partial class frmSpriteViewer : BaseForm
	{
		private NotificationListener _notifListener;
		private PpuState _state;
		private byte[] _vram;
		private byte[] _cgram;
		private byte[] _oamRam;
		private byte[] _previewData;
		private Bitmap _previewImage;
		private DateTime _lastUpdate = DateTime.MinValue;
		private bool _autoRefresh = true;
		private GetSpritePreviewOptions _options = new GetSpritePreviewOptions();

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

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;

			_previewData = new byte[256 * 240 * 4];
			_previewImage = new Bitmap(256, 240, PixelFormat.Format32bppPArgb);
			ctrlImagePanel.ImageSize = new Size(256, 240);
			ctrlImagePanel.Image = _previewImage;

			InitShortcuts();

			SpriteViewerConfig config = ConfigManager.Config.Debug.SpriteViewer;
			if(!config.WindowSize.IsEmpty) {
				this.StartPosition = FormStartPosition.Manual;
				this.Size = config.WindowSize;
				this.Location = config.WindowLocation;
			}

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
			_notifListener?.Dispose();

			SpriteViewerConfig config = ConfigManager.Config.Debug.SpriteViewer;
			config.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			config.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			config.AutoRefresh = mnuAutoRefresh.Checked;
			config.HideOffscreenSprites = ctrlSpriteList.HideOffscreenSprites;
			config.RefreshScanline = ctrlScanlineCycleSelect.Scanline;
			config.RefreshCycle = ctrlScanlineCycleSelect.Cycle;
			config.ImageScale = ctrlImagePanel.ImageScale;
			config.SplitterDistance = ctrlSplitContainer.SplitterDistance;
			ConfigManager.ApplyChanges();
			base.OnFormClosed(e);
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

		private void RefreshData()
		{
			_state = DebugApi.GetState().Ppu;
			_vram = DebugApi.GetMemoryState(SnesMemoryType.VideoRam);
			_oamRam = DebugApi.GetMemoryState(SnesMemoryType.SpriteRam);
			_cgram = DebugApi.GetMemoryState(SnesMemoryType.CGRam);
		}
		
		private void RefreshViewer()
		{
			ctrlSpriteList.SetData(_oamRam, _state.OamMode);

			DebugApi.GetSpritePreview(_options, _state, _vram, _oamRam, _cgram, _previewData);

			using(Graphics g = Graphics.FromImage(_previewImage)) {
				GCHandle handle = GCHandle.Alloc(_previewData, GCHandleType.Pinned);
				Bitmap source = new Bitmap(256, 240, 4 * 256, PixelFormat.Format32bppPArgb, handle.AddrOfPinnedObject());
				g.DrawImage(source, 0, 0);
				handle.Free();
			}

			if(_options.SelectedSprite >= 0) {
				ctrlImagePanel.Selection = SpriteInfo.GetSpriteInfo(_oamRam, _state.OamMode, _options.SelectedSprite).GetBounds();
			} else {
				ctrlImagePanel.Selection = Rectangle.Empty;
			}
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
			for(int i = 0; i < 128; i++) {
				SpriteInfo sprite = SpriteInfo.GetSpriteInfo(_oamRam, _state.OamMode, i);
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
			_autoRefresh = mnuAutoRefresh.Checked;
		}

		private void ctrlSpriteList_SpriteSelected(SpriteInfo sprite)
		{
			if(_options.SelectedSprite == sprite?.Index) {
				return;
			}

			SelectSprite(sprite);
			RefreshViewer();
		}
	}
}
