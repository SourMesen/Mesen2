using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;

namespace Mesen.GUI.Updates
{
	public static class UpdateHelper
	{
		public static void CheckForUpdates(bool silent)
		{
			Task.Run(() => {
				try {
					using(var client = new WebClient()) {
						XmlDocument xmlDoc = new XmlDocument();

						string platform = Program.IsMono ? "linux" : "win";
						xmlDoc.LoadXml(client.DownloadString("https://www.mesen.ca/snes/Services/GetLatestVersion.php?v=" + EmuApi.GetMesenVersion().ToString(3) + "&p=" + platform + "&l=" + ResourceHelper.GetLanguageCode()));
						Version currentVersion = EmuApi.GetMesenVersion();
						Version latestVersion = new Version(xmlDoc.SelectSingleNode("VersionInfo/LatestVersion").InnerText);
						string changeLog = xmlDoc.SelectSingleNode("VersionInfo/ChangeLog").InnerText;
						string fileHash = xmlDoc.SelectSingleNode("VersionInfo/Sha1Hash").InnerText;
						string donateText = xmlDoc.SelectSingleNode("VersionInfo/DonateText")?.InnerText;

						if(latestVersion > currentVersion) {
							Application.OpenForms[0].BeginInvoke((MethodInvoker)(() => {
								using(frmUpdatePrompt frmUpdate = new frmUpdatePrompt(currentVersion, latestVersion, changeLog, fileHash, donateText)) {
									if(frmUpdate.ShowDialog(null, Application.OpenForms[0]) == DialogResult.OK) {
										Application.Exit();
									}
								}
							}));
						} else if(!silent) {
							MesenMsgBox.Show("MesenUpToDate", MessageBoxButtons.OK, MessageBoxIcon.Information);
						}
					}
				} catch(Exception ex) {
					if(!silent) {
						MesenMsgBox.Show("ErrorWhileCheckingUpdates", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
					}
				}
			});
		}
	}
}
