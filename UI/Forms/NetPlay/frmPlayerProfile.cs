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
	public partial class frmPlayerProfile : BaseConfigForm
	{
		public frmPlayerProfile()
		{
			InitializeComponent();

			Entity = ConfigManager.Config.Netplay;
			AddBinding(nameof(NetplayConfig.PlayerName), txtPlayerName);
		}
	}
}
