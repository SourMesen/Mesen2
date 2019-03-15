using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Text;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;

namespace Mesen.GUI.Forms
{
	public partial class frmConfigWizard : Form
	{
		public frmConfigWizard()
		{
			InitializeComponent();

			this.Icon = Properties.Resources.MesenIcon;
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			lblLocation.Text = ConfigManager.DefaultDocumentsFolder;
			lblLocation.ForeColor = Color.Blue;
			lblCancel.ForeColor = Color.Blue;

			ResourceHelper.ApplyResources(this);
		}

		private void InitializeConfig()
		{
			ConfigManager.CreateConfig(radStoragePortable.Checked);
			DefaultKeyMappingType mappingType = DefaultKeyMappingType.None;
			if(chkXbox.Checked) {
				mappingType |= DefaultKeyMappingType.Xbox;
			}
			if(chkPs4.Checked) {
				mappingType |= DefaultKeyMappingType.Ps4;
			}
			if(chkWasd.Checked) {
				mappingType |= DefaultKeyMappingType.WasdKeys;
			}
			if(chkArrows.Checked) {
				mappingType |= DefaultKeyMappingType.ArrowKeys;
			}

			ConfigManager.Config.DefaultKeyMappings = mappingType;
			ConfigManager.ApplyChanges();
			ConfigManager.SaveConfig();

			if(chkCreateShortcut.Checked) {
				this.CreateShortcut();
			}
		}

		private void CreateShortcut()
		{
			if(Program.IsMono) {
				string shortcutFile = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Desktop), "mesen-s.desktop");
				FileAssociationHelper.CreateShortcutFile(shortcutFile);
				Process.Start("chmod", "775 " + shortcutFile); 
			} else {
				Type t = Type.GetTypeFromCLSID(new Guid("72C24DD5-D70A-438B-8A42-98424B88AFB8"));
				dynamic shell = Activator.CreateInstance(t);
				try {
					string linkPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory), "Mesen-S.lnk");
					var lnk = shell.CreateShortcut(linkPath);
					try {
						lnk.TargetPath = Assembly.GetEntryAssembly().Location;
						lnk.IconLocation = Assembly.GetEntryAssembly().Location + ", 0";
						lnk.Save();
					} finally {
						Marshal.FinalReleaseComObject(lnk);
					}
				} finally {
					Marshal.FinalReleaseComObject(shell);
				}
			}
		}

		private void picXbox_Click(object sender, EventArgs e)
		{
			chkXbox.Checked = !chkXbox.Checked;
		}

		private void picPs4_Click(object sender, EventArgs e)
		{
			chkPs4.Checked = !chkPs4.Checked;
		}

		private void picWasd_Click(object sender, EventArgs e)
		{
			chkWasd.Checked = !chkWasd.Checked;
			if(chkWasd.Checked) {
				chkArrows.Checked = false;
			}
		}

		private void picArrows_Click(object sender, EventArgs e)
		{
			chkArrows.Checked = !chkArrows.Checked;
			if(chkArrows.Checked) {
				chkWasd.Checked = false;
			}
		}

		private void chkWasd_CheckedChanged(object sender, EventArgs e)
		{
			if(chkWasd.Checked) {
				chkArrows.Checked = false;
			}
		}

		private void chkArrows_CheckedChanged(object sender, EventArgs e)
		{
			if(chkArrows.Checked) {
				chkWasd.Checked = false;
			}
		}

		private void btnOk_Click(object sender, EventArgs e)
		{
			string targetFolder = radStoragePortable.Checked ? ConfigManager.DefaultPortableFolder : ConfigManager.DefaultDocumentsFolder;
			string testFile = Path.Combine(targetFolder, "test.txt");
			try {
				if(!Directory.Exists(targetFolder)) {
					Directory.CreateDirectory(targetFolder);
				}
				File.WriteAllText(testFile, "test");
				File.Delete(testFile);
				this.InitializeConfig();
				this.Close();
			} catch(Exception ex) {
				MesenMsgBox.Show("CannotWriteToFolder", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
			}
		}

		private void lblCancel_Click(object sender, EventArgs e)
		{
			Application.Exit();
		}

		private void radStoragePortable_CheckedChanged(object sender, EventArgs e)
		{
			lblLocation.Text = ConfigManager.DefaultPortableFolder;
		}

		private void radStorageDocuments_CheckedChanged(object sender, EventArgs e)
		{
			lblLocation.Text = ConfigManager.DefaultDocumentsFolder;
		}
	}
}
