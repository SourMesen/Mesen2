using Mesen.GUI.Config;
using Mesen.GUI.Controls;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms
{
	public partial class frmDebugLog : BaseForm
	{
		private string _currentLog;
		public frmDebugLog()
		{
			InitializeComponent();
			txtLog.Font = new Font(BaseControl.MonospaceFontFamily, 10);
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			
			DebugLogConfig config = ConfigManager.Config.Debug.DebugLog;
			RestoreLocation(config.WindowLocation, config.WindowSize);
			UpdateLog();
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			base.OnFormClosing(e);

			DebugLogConfig config = ConfigManager.Config.Debug.DebugLog;
			config.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			config.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			ConfigManager.ApplyChanges();
		}

		private void UpdateLog()
		{
			Task.Run(() => {
				string log = DebugApi.GetLog();
				if(_currentLog != log) {
					_currentLog = log;
					this.BeginInvoke((Action)(() => {
						txtLog.Text = _currentLog;
						txtLog.SelectionLength = 0;
						txtLog.SelectionStart = txtLog.Text.Length;
						txtLog.ScrollToCaret();
					}));
				}
			});
		}

		private void btnClose_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		private void tmrRefresh_Tick(object sender, EventArgs e)
		{
			UpdateLog();
		}
	}
}
