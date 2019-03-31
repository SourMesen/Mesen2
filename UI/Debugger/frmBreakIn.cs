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
	public partial class frmBreakIn : BaseConfigForm
	{
		public frmBreakIn()
		{
			InitializeComponent();

			nudCount.Value = ConfigManager.Config.Debug.BreakInCount;
			radCpuInstructions.Checked = ConfigManager.Config.Debug.BreakInMetric == BreakInMetric.CpuInstructions;
			radPpuCycles.Checked = ConfigManager.Config.Debug.BreakInMetric == BreakInMetric.PpuCycles;
			radScanlines.Checked = ConfigManager.Config.Debug.BreakInMetric == BreakInMetric.Scanlines;
			radFrames.Checked = ConfigManager.Config.Debug.BreakInMetric == BreakInMetric.Frames;
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
				ConfigManager.Config.Debug.BreakInCount = (int)count;
				if(radCpuInstructions.Checked) {
					DebugApi.Step(count, StepType.CpuStep);
					ConfigManager.Config.Debug.BreakInMetric = BreakInMetric.CpuInstructions;
				} else if(radPpuCycles.Checked) {
					DebugApi.Step(count, StepType.PpuStep);
					ConfigManager.Config.Debug.BreakInMetric = BreakInMetric.PpuCycles;
				} else if(radScanlines.Checked) {
					DebugApi.Step(count * 341, StepType.PpuStep);
					ConfigManager.Config.Debug.BreakInMetric = BreakInMetric.Scanlines;
				} else {
					DebugApi.Step(count * 341 * 262, StepType.PpuStep);
					ConfigManager.Config.Debug.BreakInMetric = BreakInMetric.Frames;
				}
				ConfigManager.ApplyChanges();
			}
		}
	}
}
