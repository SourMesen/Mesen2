using Mesen.GUI.Properties;
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
using System.Xml;
using System.Xml.Serialization;

namespace Mesen.GUI.Forms
{
	public partial class frmCheatDbList : BaseForm
	{
		private CheatDatabase _cheatDb;
		public List<CheatDbCheatEntry> ImportedCheats { get; internal set; }
		private string _previousSearch = "";

		public frmCheatDbList()
		{
			InitializeComponent();
			this.Icon = Resources.Find;
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);

			_cheatDb = new CheatDatabase();
			try {
				XmlSerializer xmlSerializer = new XmlSerializer(typeof(CheatDatabase));
				using(TextReader textReader = new StreamReader(ResourceExtractor.GetZippedResource("CheatDb.xml"))) {
					_cheatDb = (CheatDatabase)xmlSerializer.Deserialize(textReader);
				}
			} catch { }

			lblCheatCount.Text = ResourceHelper.GetMessage("CheatsFound", _cheatDb.Games.Count.ToString());

			txtSearch.Focus();
			UpdateList();

			string sha1 = EmuApi.GetRomInfo().Sha1;
			for(int i = 0, len = lstGames.Items.Count; i < len; i++) {
				if(((CheatDbGameEntry)lstGames.Items[i]).Sha1 == sha1) {
					lstGames.TopIndex = Math.Max(0, i - 10);
					lstGames.SelectedIndex = i;
					break;
				}
			}
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			if(keyData == Keys.Escape) {
				Close();
				return true;
			} else if(txtSearch.Focused) {
				if(keyData == Keys.Down || keyData == Keys.PageDown || keyData == Keys.Up || keyData == Keys.PageUp) {
					lstGames.Focus();
					if(lstGames.Items.Count > 0) {
						lstGames.SelectedIndex = 0;
					}
					return true;
				}
			} else if(lstGames.Focused && lstGames.SelectedIndex <= 0) {
				if(keyData == Keys.Up || keyData == Keys.PageUp) {
					txtSearch.Focus();
					txtSearch.SelectAll();
					return true;
				}
			}
			return base.ProcessCmdKey(ref msg, keyData);
		}

		private void UpdateList()
		{
			lstGames.BeginUpdate();
			lstGames.Sorted = false;
			lstGames.Items.Clear();
			if(string.IsNullOrWhiteSpace(_previousSearch)) {
				lstGames.Items.AddRange(_cheatDb.Games.ToArray());
			} else {
				lstGames.Items.AddRange(_cheatDb.Games.Where(c => c.Name.IndexOf(_previousSearch, StringComparison.InvariantCultureIgnoreCase) >= 0).ToArray());
			}
			lstGames.Sorted = true;
			lstGames.EndUpdate();
		}

		void SetImportedCheats()
		{
			this.ImportedCheats = ((CheatDbGameEntry)lstGames.SelectedItem).Cheats;
			this.DialogResult = DialogResult.OK;
		}

		private void lstGames_SelectedIndexChanged(object sender, EventArgs e)
		{
			btnOK.Enabled = lstGames.SelectedItems.Count > 0;
		}

		private void lstGames_DoubleClick(object sender, EventArgs e)
		{
			SetImportedCheats();
		}

		private void btnOK_Click(object sender, EventArgs e)
		{
			SetImportedCheats();
		}
		
		private void txtSearch_TextChanged(object sender, EventArgs e)
		{
			if(txtSearch.Text.Trim() != _previousSearch) {
				_previousSearch = txtSearch.Text.Trim();
				UpdateList();
			}
		}
	}
}
