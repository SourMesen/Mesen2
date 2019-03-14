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
using Mesen.GUI.Controls;

namespace Mesen.GUI.Forms.Config
{
	public partial class frmControllerConfig : BaseInputConfigForm
	{
		private int _portNumber;

		public frmControllerConfig(ControllerConfig cfg, int portNumber) : base(cfg)
		{
			InitializeComponent();

			if(!this.DesignMode) {
				_portNumber = portNumber;
				SetMainTab(this.tabMain);

				//AddBinding("TurboSpeed", trkTurboSpeed);

				ctrlController0.Initialize(cfg.Keys.Mapping1);
				ctrlController1.Initialize(cfg.Keys.Mapping2);
				ctrlController2.Initialize(cfg.Keys.Mapping3);
				ctrlController3.Initialize(cfg.Keys.Mapping4);

				ctrlController0.PortNumber = portNumber;
				ctrlController1.PortNumber = portNumber;
				ctrlController2.PortNumber = portNumber;
				ctrlController3.PortNumber = portNumber;

				this.btnSelectPreset.Image = BaseControl.DownArrow;

				ResourceHelper.ApplyResources(this, mnuStripPreset);
				this.Text += ": " + ResourceHelper.GetMessage("PlayerNumber", (portNumber + 1).ToString());
			}
		}
		
		private void btnClear_Click(object sender, EventArgs e)
		{
			ClearCurrentTab();
		}

		private void btnSelectPreset_Click(object sender, EventArgs e)
		{
			mnuStripPreset.Show(btnSelectPreset.PointToScreen(new Point(0, btnSelectPreset.Height-1)));
		}

		private void mnuWasdLayout_Click(object sender, EventArgs e)
		{
			GetControllerControl().Initialize(Presets.WasdLayout);
		}

		private void mnuArrowLayout_Click(object sender, EventArgs e)
		{
			GetControllerControl().Initialize(Presets.ArrowLayout);
		}
		
		private void mnuXboxLayout1_Click(object sender, EventArgs e)
		{
			GetControllerControl().Initialize(Presets.XboxLayout1);
		}

		private void mnuXboxLayout2_Click(object sender, EventArgs e)
		{
			GetControllerControl().Initialize(Presets.XboxLayout2);
		}

		private void mnuPs4Layout1_Click(object sender, EventArgs e)
		{
			GetControllerControl().Initialize(Presets.Ps4Layout1);
		}

		private void mnuPs4Layout2_Click(object sender, EventArgs e)
		{
			GetControllerControl().Initialize(Presets.Ps4Layout2);
		}

		private void mnuSnes30Layout1_Click(object sender, EventArgs e)
		{
			GetControllerControl().Initialize(Presets.Snes30Layout1);
		}

		private void mnuSnes30Layout2_Click(object sender, EventArgs e)
		{
			GetControllerControl().Initialize(Presets.Snes30Layout2);
		}
	}
}
