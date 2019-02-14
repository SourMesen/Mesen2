using Mesen.GUI.Config;
using Mesen.GUI.Debugger;
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
	public partial class frmMain : BaseForm
	{
		public frmMain(string[] args)
		{
			InitializeComponent();
			ResourceHelper.LoadResources(Language.English);
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);

			EmuApi.InitDll();
			EmuApi.InitializeEmu(ConfigManager.HomeFolder, Handle, ctrlRenderer.Handle, false, false, false);
		}

		private void mnuTraceLogger_Click(object sender, EventArgs e)
		{
			frmTraceLogger frm = new frmTraceLogger();
			frm.Show();
		}

		private void mnuStep_Click(object sender, EventArgs e)
		{
			DebugApi.Step(1);
		}

		private void mnuOpen_Click(object sender, EventArgs e)
		{
			using(OpenFileDialog ofd = new OpenFileDialog()) {
				ofd.Filter = ResourceHelper.GetMessage("FilterRom");
				if(ofd.ShowDialog() == DialogResult.OK) {
					EmuApi.LoadRom(ofd.FileName);
					Task.Run(() => {
						EmuApi.Run();
					});
				}
			}
		}

		private void mnuRun_Click(object sender, EventArgs e)
		{
			DebugApi.ResumeExecution();
		}

		private void mnuRun100Instructions_Click(object sender, EventArgs e)
		{
			DebugApi.Step(1000);
		}
	}
}
