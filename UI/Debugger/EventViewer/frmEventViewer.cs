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
				_notifListener = new NotificationListener();
				_notifListener.OnNotification += OnNotificationReceived;
			}
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
	}
}
