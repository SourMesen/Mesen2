using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;

namespace Mesen.GUI.Forms.NetPlay
{
	public partial class frmClientConfig : BaseConfigForm
	{
		public frmClientConfig()
		{
			InitializeComponent();

			Entity = ConfigManager.Config.Netplay;

			AddBinding(nameof(NetplayConfig.Host), txtHost);
			AddBinding(nameof(NetplayConfig.Password), txtPassword);
			this.txtPort.Text = ConfigManager.Config.Netplay.Port.ToString();
		}

		protected override void UpdateConfig()
		{
			((NetplayConfig)Entity).Port = Convert.ToUInt16(txtPort.Text);
		}

		private void Field_TextChanged(object sender, EventArgs e)
		{
			UInt16 port;
			if(!UInt16.TryParse(txtPort.Text, out port)) {
				btnOK.Enabled = false;
			} else {
				btnOK.Enabled = !string.IsNullOrWhiteSpace(txtHost.Text);
			}
		}
	}
}
