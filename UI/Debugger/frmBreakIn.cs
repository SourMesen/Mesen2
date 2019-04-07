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
		private CpuType _cpuType;

		public frmBreakIn(CpuType cpuType)
		{
			InitializeComponent();

			_cpuType = cpuType;

			DebuggerInfo cfg = ConfigManager.Config.Debug.Debugger;
			nudCount.Value = cfg.BreakInCount;
			radCpuInstructions.Checked = cfg.BreakInMetric == BreakInMetric.CpuInstructions;
			radPpuCycles.Checked = cfg.BreakInMetric == BreakInMetric.PpuCycles;
			radScanlines.Checked = cfg.BreakInMetric == BreakInMetric.Scanlines;
			radFrames.Checked = cfg.BreakInMetric == BreakInMetric.Frames;
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
				DebuggerInfo cfg = ConfigManager.Config.Debug.Debugger;
				int count = (int)nudCount.Value;
				cfg.BreakInCount = (int)count;
				if(radCpuInstructions.Checked) {
					DebugApi.Step(count, _cpuType == CpuType.Cpu ? StepType.CpuStep : StepType.SpcStep);
					cfg.BreakInMetric = BreakInMetric.CpuInstructions;
				} else if(radPpuCycles.Checked) {
					DebugApi.Step(count, StepType.PpuStep);
					cfg.BreakInMetric = BreakInMetric.PpuCycles;
				} else if(radScanlines.Checked) {
					DebugApi.Step(count * 341, StepType.PpuStep);
					cfg.BreakInMetric = BreakInMetric.Scanlines;
				} else {
					DebugApi.Step(count * 341 * 262, StepType.PpuStep);
					cfg.BreakInMetric = BreakInMetric.Frames;
				}
				ConfigManager.ApplyChanges();
			}
		}
	}
}
