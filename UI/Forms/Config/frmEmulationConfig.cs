using Mesen.GUI.Config;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms.Config
{
	public partial class frmEmulationConfig : BaseConfigForm
	{
		private DateTime _unixEpoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);

		public frmEmulationConfig()
		{
			InitializeComponent();
			if(DesignMode) {
				return;
			}

			Entity = ConfigManager.Config.Emulation.Clone();

			AddBinding(nameof(EmulationConfig.EmulationSpeed), nudEmulationSpeed);
			AddBinding(nameof(EmulationConfig.TurboSpeed), nudTurboSpeed);
			AddBinding(nameof(EmulationConfig.RewindSpeed), nudRewindSpeed);
			AddBinding(nameof(EmulationConfig.Region), cboRegion);
			AddBinding(nameof(EmulationConfig.RunAheadFrames), nudRunAheadFrames);

			AddBinding(nameof(EmulationConfig.RamPowerOnState), cboRamPowerOnState);
			AddBinding(nameof(EmulationConfig.EnableRandomPowerOnState), chkEnableRandomPowerOnState);
			AddBinding(nameof(EmulationConfig.EnableStrictBoardMappings), chkEnableStrictBoardMappings);

			AddBinding(nameof(EmulationConfig.PpuExtraScanlinesBeforeNmi), nudExtraScanlinesBeforeNmi);
			AddBinding(nameof(EmulationConfig.PpuExtraScanlinesAfterNmi), nudExtraScanlinesAfterNmi);
			AddBinding(nameof(EmulationConfig.GsuClockSpeed), nudGsuClockSpeed);
			
			AddBinding(nameof(EmulationConfig.GbModel), cboGameboyModel);

			long customDate = ConfigManager.Config.Emulation.BsxCustomDate;
			if(customDate >= 0) {
				DateTime stamp;
				try {
					stamp = _unixEpoch.AddSeconds(customDate).ToLocalTime();
				} catch {
					stamp = DateTime.Now;
				}
				radBsxCustomTime.Checked = true;
				dtpBsxCustomDate.Value = stamp.Date;
				dtpBsxCustomTime.Value = new DateTime(2000, 01, 01) + stamp.TimeOfDay;
			} else {
				radBsxLocalTime.Checked = true;
				dtpBsxCustomDate.Value = DateTime.Now.Date;
				dtpBsxCustomTime.Value = new DateTime(2000, 01, 01) + DateTime.Now.TimeOfDay;
			}
		}

		protected override void OnApply()
		{
			if(radBsxCustomTime.Checked) {
				DateTime dateTime = dtpBsxCustomDate.Value + dtpBsxCustomTime.Value.TimeOfDay;
				((EmulationConfig)this.Entity).BsxCustomDate = (long)dateTime.Subtract(_unixEpoch.ToLocalTime()).TotalSeconds;
			} else {
				((EmulationConfig)this.Entity).BsxCustomDate = -1;
			}

			ConfigManager.Config.Emulation = (EmulationConfig)this.Entity;
			ConfigManager.ApplyChanges();
		}

		private void nudGsuClockSpeed_Leave(object sender, EventArgs e)
		{
			nudGsuClockSpeed.Value = Math.Ceiling(nudGsuClockSpeed.Value / 100) * 100;
		}

		private void dtpBsxCustomDate_Enter(object sender, EventArgs e)
		{
			radBsxCustomTime.Checked = true;
		}

		private void dtpBsxCustomTime_Enter(object sender, EventArgs e)
		{
			radBsxCustomTime.Checked = true;
		}
	}
}
