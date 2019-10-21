using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;

namespace Mesen.GUI.Forms.NetPlay
{
	public partial class frmServerConfig : BaseConfigForm
	{
		public frmServerConfig()
		{
			InitializeComponent();

			Entity = ConfigManager.Config.Netplay;

			AddBinding(nameof(NetplayConfig.ServerName), txtServerName);
			AddBinding(nameof(NetplayConfig.ServerPassword), txtPassword);
			AddBinding(nameof(NetplayConfig.ServerPort), txtPort, eNumberFormat.Decimal);
		}

		private void Field_ValueChanged(object sender, EventArgs e)
		{
			UInt16 port;
			if(!UInt16.TryParse(txtPort.Text, out port)) {
				btnOK.Enabled = false;
			} else {
				btnOK.Enabled = !string.IsNullOrWhiteSpace(txtServerName.Text);
			}
		}
	}
}
