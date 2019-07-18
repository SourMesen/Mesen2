using Mesen.GUI.Config;
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
	public partial class frmEventViewer : BaseForm
	{
		private NotificationListener _notifListener;
		private DateTime _lastUpdate = DateTime.MinValue;

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
				if(!config.WindowSize.IsEmpty) {
					this.StartPosition = FormStartPosition.Manual;
					this.Size = config.WindowSize;
					this.Location = config.WindowLocation;
				}

				mnuRefreshOnBreakPause.Checked = ConfigManager.Config.Debug.EventViewer.RefreshOnBreakPause;
				ctrlPpuView.ImageScale = config.ImageScale;

				_notifListener = new NotificationListener();
				_notifListener.OnNotification += OnNotificationReceived;
			}
		}

		protected override void OnFormClosed(FormClosedEventArgs e)
		{
			_notifListener?.Dispose();

			EventViewerConfig config = ConfigManager.Config.Debug.EventViewer;
			config.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			config.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			config.ImageScale = ctrlPpuView.ImageScale;

			base.OnFormClosed(e);
		}

		private void InitShortcuts()
		{
			mnuZoomIn.InitShortcut(this, nameof(DebuggerShortcutsConfig.ZoomIn));
			mnuZoomOut.InitShortcut(this, nameof(DebuggerShortcutsConfig.ZoomOut));

			mnuZoomIn.Click += (s, e) => ctrlPpuView.ZoomIn();
			mnuZoomOut.Click += (s, e) => ctrlPpuView.ZoomOut();
			mnuClose.Click += (s, e) => this.Close();
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.CodeBreak:
					if(ConfigManager.Config.Debug.EventViewer.RefreshOnBreakPause) {
						ctrlPpuView.RefreshData();
						this.BeginInvoke((Action)(() => {
							ctrlPpuView.RefreshViewer();
						}));
					}
					break;

				case ConsoleNotificationType.EventViewerRefresh:
					if((DateTime.Now - _lastUpdate).Milliseconds > 10) {
						_lastUpdate = DateTime.Now;

						ctrlPpuView.RefreshData();
						this.BeginInvoke((Action)(() => {
							ctrlPpuView.RefreshViewer();
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
	}
}
