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

namespace Mesen.GUI.Forms
{
	public partial class frmRecordAvi : BaseConfigForm
	{
		public frmRecordAvi()
		{
			InitializeComponent();

			Entity = ConfigManager.Config.AviRecord;
			AddBinding(nameof(AviRecordConfig.Codec), cboVideoCodec);
			AddBinding(nameof(AviRecordConfig.CompressionLevel), trkCompressionLevel);
		}

		public string Filename { get; internal set; }

		protected override bool ValidateInput()
		{
			return !string.IsNullOrWhiteSpace(txtFilename.Text);
		}

		protected override void OnFormClosed(FormClosedEventArgs e)
		{
			base.OnFormClosed(e);

			this.Filename = txtFilename.Text;
		}

		private void btnBrowse_Click(object sender, EventArgs e)
		{
			using(SaveFileDialog sfd = new SaveFileDialog()) {
				sfd.SetFilter(ResourceHelper.GetMessage("FilterAvi"));
				sfd.InitialDirectory = ConfigManager.AviFolder;
				sfd.FileName = EmuApi.GetRomInfo().GetRomName() + ".avi";
				if(sfd.ShowDialog() == DialogResult.OK) {
					txtFilename.Text = sfd.FileName;
				}
			}
		}

		private void cboVideoCodec_SelectedIndexChanged(object sender, EventArgs e)
		{
			lblCompressionLevel.Visible = cboVideoCodec.SelectedIndex > 0;
			tlpCompressionLevel.Visible = cboVideoCodec.SelectedIndex > 0;
		}
	}
}
