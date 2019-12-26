using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmEventViewer : BaseForm, IRefresh
	{
		private NotificationListener _notifListener;
		private WindowRefreshManager _refreshManager;

		public ctrlScanlineCycleSelect ScanlineCycleSelect { get { return null; } }

		public frmEventViewer()
		{
			InitializeComponent();
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			if(!this.DesignMode) {
				InitShortcuts();

				EventViewerConfig config = ConfigManager.Config.Debug.EventViewer;
				RestoreLocation(config.WindowLocation, config.WindowSize);

				mnuRefreshOnBreakPause.Checked = ConfigManager.Config.Debug.EventViewer.RefreshOnBreakPause;
				ctrlPpuView.ImageScale = config.ImageScale;

				_notifListener = new NotificationListener();
				_notifListener.OnNotification += OnNotificationReceived;

				_refreshManager = new WindowRefreshManager(this);
				_refreshManager.AutoRefresh = config.AutoRefresh;
				_refreshManager.AutoRefreshSpeed = config.AutoRefreshSpeed;
				mnuAutoRefresh.Checked = config.AutoRefresh;
				mnuAutoRefreshLow.Click += (s, evt) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.Low;
				mnuAutoRefreshNormal.Click += (s, evt) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.Normal;
				mnuAutoRefreshHigh.Click += (s, evt) => _refreshManager.AutoRefreshSpeed = RefreshSpeed.High;
				mnuAutoRefreshSpeed.DropDownOpening += (s, evt) => UpdateRefreshSpeedMenu();

				ctrlFilters.Init();

				RefreshData();
				RefreshViewer();
			}
		}

		protected override void OnFormClosed(FormClosedEventArgs e)
		{
			_notifListener?.Dispose();
			_refreshManager?.Dispose();

			EventViewerConfig config = ConfigManager.Config.Debug.EventViewer;
			config.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			config.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			config.ImageScale = ctrlPpuView.ImageScale;
			config.AutoRefresh = _refreshManager.AutoRefresh;
			config.AutoRefreshSpeed = _refreshManager.AutoRefreshSpeed;

			base.OnFormClosed(e);
		}

		private void InitShortcuts()
		{
			mnuRefresh.InitShortcut(this, nameof(DebuggerShortcutsConfig.Refresh));
			mnuZoomIn.InitShortcut(this, nameof(DebuggerShortcutsConfig.ZoomIn));
			mnuZoomOut.InitShortcut(this, nameof(DebuggerShortcutsConfig.ZoomOut));

			mnuZoomIn.Click += (s, e) => ctrlPpuView.ZoomIn();
			mnuZoomOut.Click += (s, e) => ctrlPpuView.ZoomOut();
			mnuClose.Click += (s, e) => this.Close();
		}

		public void RefreshData()
		{
			EventViewerDisplayOptions options = ConfigManager.Config.Debug.EventViewer.GetInteropOptions();
			ctrlPpuView.ScanlineCount = DebugApi.TakeEventSnapshot(options);
		}

		public void RefreshViewer()
		{
			if(tabMain.SelectedTab == tpgPpuView) {
				ctrlPpuView.RefreshViewer();
			} else {
				ctrlListView.RefreshViewer();
			}
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.CodeBreak:
					if(ConfigManager.Config.Debug.EventViewer.RefreshOnBreakPause) {
						RefreshData();
						this.BeginInvoke((Action)(() => {
							RefreshViewer();
						}));
					}
					break;
			}
		}

		private void mnuRefreshOnBreakPause_Click(object sender, EventArgs e)
		{
			ConfigManager.Config.Debug.EventViewer.RefreshOnBreakPause = mnuRefreshOnBreakPause.Checked;
			ConfigManager.ApplyChanges();
		}

		private void mnuRefresh_Click(object sender, EventArgs e)
		{
			RefreshData();
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

		private void ctrlFilters_OptionsChanged(object sender, EventArgs e)
		{
			RefreshViewer();
		}

		private void tabMain_SelectedIndexChanged(object sender, EventArgs e)
		{
			RefreshViewer();
		}
	}
}
