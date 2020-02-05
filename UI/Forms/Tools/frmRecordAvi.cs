using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
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

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);

			VideoCodec codec = cboVideoCodec.GetEnumValue<VideoCodec>();
			string defaultFile = Path.Combine(ConfigManager.AviFolder, EmuApi.GetRomInfo().GetRomName() + (codec == VideoCodec.GIF ? ".gif" : ".avi"));
			txtFilename.Text = defaultFile;
		}

		private void btnBrowse_Click(object sender, EventArgs e)
		{
			SaveFileDialog sfd = new SaveFileDialog();
			VideoCodec codec = cboVideoCodec.GetEnumValue<VideoCodec>();
			sfd.SetFilter(ResourceHelper.GetMessage(codec == VideoCodec.GIF ? "FilterGif" : "FilterAvi"));
			sfd.InitialDirectory = ConfigManager.AviFolder;
			sfd.FileName = EmuApi.GetRomInfo().GetRomName() + (codec == VideoCodec.GIF ? ".gif" : ".avi");
			if(sfd.ShowDialog() == DialogResult.OK) {
				txtFilename.Text = sfd.FileName;
			}
		}

		private void cboVideoCodec_SelectedIndexChanged(object sender, EventArgs e)
		{
			VideoCodec codec = cboVideoCodec.GetEnumValue<VideoCodec>();
			bool hasCompressionLevel = (codec == VideoCodec.CSCD || codec == VideoCodec.ZMBV);
			lblCompressionLevel.Visible = hasCompressionLevel;
			tlpCompressionLevel.Visible = hasCompressionLevel;
		}
	}
}
