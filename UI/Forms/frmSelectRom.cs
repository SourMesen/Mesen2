using Mesen.GUI.Utilities;
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

namespace Mesen.GUI.Forms
{
	public partial class frmSelectRom : BaseForm
	{
		private List<ArchiveRomEntry> _romFiles;
		private int SelectedIndex { get; set; }
		private string _previousSearch = "";

		private frmSelectRom(List<ArchiveRomEntry> romFiles)
		{
			InitializeComponent();

			_romFiles = romFiles;
			lblRomCount.Text = ResourceHelper.GetMessage("RomsFound", _romFiles.Count.ToString());

			lstRoms.Sorted = true;
			this.DialogResult = DialogResult.Cancel;

			UpdateList();
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			if(txtSearch.Focused) {
				if(keyData == Keys.Down || keyData == Keys.PageDown || keyData == Keys.Up || keyData == Keys.PageUp) {
					lstRoms.Focus();
					if(lstRoms.Items.Count > 0) {
						lstRoms.SelectedIndex = 0;
					}
					return true;
				}
			} else if(lstRoms.Focused && lstRoms.SelectedIndex <= 0) {
				if(keyData == Keys.Up || keyData == Keys.PageUp) {
					txtSearch.Focus();
					txtSearch.SelectAll();
					return true;
				}
			}

			if(keyData == Keys.Enter && lstRoms.Focused && lstRoms.SelectedItems.Count == 1) {
				this.DialogResult = DialogResult.OK;
				this.Close();
				return true;
			}

			return base.ProcessCmdKey(ref msg, keyData);
		}		

		private void UpdateList()
		{
			lstRoms.Items.Clear();
			if(string.IsNullOrWhiteSpace(_previousSearch)) {
				lstRoms.Items.AddRange(_romFiles.ToArray());
			} else {
				List<ArchiveRomEntry> romsToAdd = new List<ArchiveRomEntry>();
				foreach(ArchiveRomEntry rom in _romFiles) {
					if(rom.Filename.IndexOf(_previousSearch, StringComparison.InvariantCultureIgnoreCase) >= 0) {
						romsToAdd.Add(rom);
					}
				}
				lstRoms.Items.AddRange(romsToAdd.ToArray());
			}
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			txtSearch.Focus();
		}

		public static bool SelectRom(ref ResourcePath resource)
		{
			List<ArchiveRomEntry> archiveRomList = ArchiveHelper.GetArchiveRomList(resource.Path);

			if(archiveRomList.Select(entry => entry.Filename).Contains(resource.InnerFile)) {
				return true;
			}

			if(archiveRomList.Count > 1) {
				using(frmSelectRom frm = new frmSelectRom(archiveRomList)) {
					if(frm.ShowDialog(null, frmMain.Instance) == DialogResult.OK) {
						ArchiveRomEntry entry = frm.lstRoms.SelectedItem as ArchiveRomEntry;
						resource.InnerFile = entry.Filename;
						if(!entry.IsUtf8) {
							resource.InnerFileIndex = archiveRomList.IndexOf(entry) + 1;
						}
					} else {
						return false;
					}
				}
			} else if(archiveRomList.Count == 1) {
				resource.InnerFile = archiveRomList[0].Filename;
				if(!archiveRomList[0].IsUtf8) {
					resource.InnerFileIndex = 1;
				}
			} else {
				resource.InnerFile = "";
			}

			return true;
		}

		private void lstRoms_SelectedIndexChanged(object sender, EventArgs e)
		{
			btnOK.Enabled = lstRoms.SelectedItems.Count > 0;
		}

		private void tmrSearch_Tick(object sender, EventArgs e)
		{
			if(txtSearch.Text.Trim() != _previousSearch) {
				_previousSearch = txtSearch.Text.Trim();
				UpdateList();
			}
		}

		private void lstRoms_DoubleClick(object sender, EventArgs e)
		{
			if(lstRoms.SelectedItems.Count == 1) {
				this.DialogResult = DialogResult.OK;
				this.Close();
			}
		}
	}
}
