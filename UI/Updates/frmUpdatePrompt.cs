using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;
using Mesen.GUI.Controls;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Updates
{
	public partial class frmUpdatePrompt : BaseForm
	{
		private string _fileHash;
		private string _donateText;

		public frmUpdatePrompt(Version currentVersion, Version latestVersion, string changeLog, string fileHash, string donateText)
		{
			InitializeComponent();

			_donateText = donateText;

			this.txtChangelog.Font = new Font(BaseControl.MonospaceFontFamily, 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));

			_fileHash = fileHash;

			lblCurrentVersionString.Text = currentVersion.ToString();
			lblLatestVersionString.Text = latestVersion.ToString();
			txtChangelog.Text = changeLog.Replace("\n", Environment.NewLine);
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			if(_donateText != null) {
				if(!string.IsNullOrEmpty(_donateText)) {
					this.lblDonate.Text = _donateText;
				}
				this.lblDonate.Visible = true;
				this.picDonate.Visible = true;
			} else {
				this.lblDonate.Visible = false;
				this.picDonate.Visible = false;
			}

			btnUpdate.Focus();
		}
		
		private void btnUpdate_Click(object sender, EventArgs e)
		{
#if DISABLEAUTOUPDATE
			MesenMsgBox.Show("AutoUpdateDisabledMessage", MessageBoxButtons.OK, MessageBoxIcon.Information);
			this.DialogResult = DialogResult.Cancel;
			this.Close();
#else
			string destFilePath = System.Reflection.Assembly.GetEntryAssembly().Location;
			string srcFilePath = Path.Combine(ConfigManager.DownloadFolder, "Mesen." + lblLatestVersionString.Text + ".exe");
			string backupFilePath = Path.Combine(ConfigManager.BackupFolder, "Mesen." + lblCurrentVersionString.Text + ".exe");
			string updateHelper = Path.Combine(ConfigManager.HomeFolder, "Resources", "MesenUpdater.exe");

			if(!File.Exists(updateHelper)) {
				MesenMsgBox.Show("UpdaterNotFound", MessageBoxButtons.OK, MessageBoxIcon.Error);
				DialogResult = DialogResult.Cancel;
			} else if(!string.IsNullOrWhiteSpace(srcFilePath)) {
				frmDownloadProgress frmDownload = new frmDownloadProgress("https://www.mesen.ca/snes/Services/GetLatestVersion.php?a=download&p=win&v=" + EmuApi.GetMesenVersion().ToString(3), srcFilePath);
				if(frmDownload.ShowDialog() == DialogResult.OK) {
					FileInfo fileInfo = new FileInfo(srcFilePath);
					if(fileInfo.Length > 0 && ResourceExtractor.GetSha1Hash(File.ReadAllBytes(srcFilePath)) == _fileHash) {
						if(Program.IsMono) {
							Process.Start("mono", string.Format("\"{0}\" \"{1}\" \"{2}\" \"{3}\"", updateHelper, srcFilePath, destFilePath, backupFilePath));
						} else {
							Process.Start(updateHelper, string.Format("\"{0}\" \"{1}\" \"{2}\"", srcFilePath, destFilePath, backupFilePath));
						}
					} else {
						//Download failed, mismatching hashes
						MesenMsgBox.Show("UpdateDownloadFailed", MessageBoxButtons.OK, MessageBoxIcon.Error);
						DialogResult = DialogResult.Cancel;
					}
				}
			}
#endif
		}

		private void picDonate_Click(object sender, EventArgs e)
		{
			Process.Start("https://www.mesen.ca/Donate.php?l=" + ResourceHelper.GetLanguageCode());
		}
	}
}
