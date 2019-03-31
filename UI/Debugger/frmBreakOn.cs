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
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmBreakOn : BaseConfigForm
	{
		public frmBreakOn()
		{
			InitializeComponent();

			nudCount.Value = ConfigManager.Config.Debug.Debugger.BreakOnValue;			
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);
			nudCount.Focus();
		}

		protected override void OnFormClosed(FormClosedEventArgs e)
		{
			base.OnFormClosed(e);
			if(this.DialogResult == DialogResult.OK) {
				int count = (int)nudCount.Value;
				ConfigManager.Config.Debug.Debugger.BreakOnValue = count;
				DebugApi.Step(count, StepType.SpecificScanline);
				ConfigManager.ApplyChanges();
			}
		}
	}
}
