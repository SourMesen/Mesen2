using Mesen.GUI.Forms;
using System;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmPaletteViewer : BaseForm
	{
		private NotificationListener _notifListener;
		private DateTime _lastUpdate = DateTime.MinValue;

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

		private void UpdateFields()
		{
			int index = ctrlPaletteViewer.SelectedPalette;
			byte[] cgram = ctrlPaletteViewer.CgRam;
			int color = (cgram[index * 2] | (cgram[index * 2 + 1] << 8));
			txtIndex.Text = index.ToString();
			txtValue.Text = color.ToString("X4");
			txtR.Text = (color & 0x1F).ToString();
			txtG.Text = ((color >> 5) & 0x1F).ToString();
			txtB.Text = ((color >> 10) & 0x1F).ToString();
			txtRgb.Text = (ctrlPaletteViewer.ToArgb(color) & 0xFFFFFF).ToString("X6");
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
						if((DateTime.Now - _lastUpdate).Milliseconds > 10) {
							_lastUpdate = DateTime.Now;
							ctrlPaletteViewer.RefreshData();
							this.BeginInvoke((Action)(() => {
								ctrlPaletteViewer.RefreshViewer();
								UpdateFields();
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

		private void ctrlPaletteViewer_SelectionChanged()
		{
			UpdateFields();
		}
	}
}