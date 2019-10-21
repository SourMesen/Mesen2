using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using Mesen.GUI.Forms.NetPlay;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Utilities
{
	public static class NetPlayHelper
	{
		public static void Connect()
		{
			if(NetplayApi.IsConnected()) {
				Task.Run(() => NetplayApi.Disconnect());
			} else {
				using(frmClientConfig frm = new frmClientConfig()) {
					if(frm.ShowDialog(frmMain.Instance) == DialogResult.OK) {
						NetplayConfig cfg = ConfigManager.Config.Netplay;
						Task.Run(() => {
							NetplayApi.Connect(cfg.Host, cfg.Port, cfg.Password, cfg.PlayerName, false);
						});
					}
				}
			}
		}

		public static void ToggleServer()
		{
			if(NetplayApi.IsServerRunning()) {
				Task.Run(() => NetplayApi.StopServer());
			} else {
				using(frmServerConfig frm = new frmServerConfig()) {
					if(frm.ShowDialog(frmMain.Instance) == DialogResult.OK) {
						NetplayConfig cfg = ConfigManager.Config.Netplay;
						NetplayApi.StartServer(cfg.ServerPort, cfg.ServerPassword, cfg.PlayerName);
					}
				}
			}
		}
	}
}
