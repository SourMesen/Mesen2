using Mesen.GUI.Forms;
using System;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmPaletteViewer : BaseForm
	{
		private NotificationListener _notifListener;

		public frmPaletteViewer()
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

			ctrlScanlineCycleSelect.Initialize(241, 0);

			ctrlPaletteViewer.RefreshData();
			ctrlPaletteViewer.RefreshViewer();
		}

		protected override void OnFormClosed(FormClosedEventArgs e)
		{
			base.OnFormClosed(e);
			_notifListener?.Dispose();
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.ViewerRefresh:
					if(e.Parameter.ToInt32() == ctrlScanlineCycleSelect.ViewerId) {
						ctrlPaletteViewer.RefreshData();
						this.BeginInvoke((Action)(() => {
							ctrlPaletteViewer.RefreshViewer();
						}));
					}
					break;
			}
		}
	}
}