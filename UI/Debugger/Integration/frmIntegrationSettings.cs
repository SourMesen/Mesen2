using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmIntegrationSettings : BaseConfigForm
	{
		public frmIntegrationSettings()
		{
			InitializeComponent();

			Entity = ConfigManager.Config.Debug.DbgIntegration;

			AddBinding(nameof(DbgIntegrationConfig.AutoImport), chkAutoLoadDbgFiles);
			AddBinding(nameof(DbgIntegrationConfig.ResetLabelsOnImport), chkResetLabelsOnImport);

			AddBinding(nameof(DbgIntegrationConfig.ImportCpuPrgRomLabels), chkCpuPrgRom);
			AddBinding(nameof(DbgIntegrationConfig.ImportCpuWorkRamLabels), chkCpuWorkRam);
			AddBinding(nameof(DbgIntegrationConfig.ImportCpuSaveRamLabels), chkCpuSaveRam);
			AddBinding(nameof(DbgIntegrationConfig.ImportCpuComments), chkCpuComments);

			AddBinding(nameof(DbgIntegrationConfig.ImportSpcRamLabels), chkSpcRam);
			AddBinding(nameof(DbgIntegrationConfig.ImportSpcComments), chkSpcComments);
		}
	}
}
