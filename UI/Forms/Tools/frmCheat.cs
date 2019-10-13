using Mesen.GUI.Config;
using Mesen.GUI.Properties;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms
{
	public partial class frmCheat : BaseConfigForm
	{
		public frmCheat(CheatCode cheat)
		{
			InitializeComponent();

			this.Icon = Resources.CheatCode;

			this.Entity = cheat;

			AddBinding(nameof(CheatCode.Description), txtCheatName);
			AddBinding(nameof(CheatCode.Enabled), chkEnabled);
			AddBinding(nameof(CheatCode.Codes), txtCodes);

			radGameGenie.Checked = cheat.Format == CheatFormat.GameGenie;
			radProActionReplay.Checked = cheat.Format == CheatFormat.ProActionReplay;
		}

		protected override bool ValidateInput()
		{
			Regex validate = new Regex(radGameGenie.Checked ? "^[a-f0-9]{4}-[a-f0-9]{4}$" : "^[a-f0-9]{8}$", RegexOptions.IgnoreCase);
			int validCodeCount = 0;
			foreach(string code in txtCodes.Text.Split(new string[1] { Environment.NewLine }, StringSplitOptions.None)) {
				string trimmedCode = code.Trim();
				if(trimmedCode.Length > 0) {
					if(validate.IsMatch(trimmedCode)) {
						validCodeCount++;
					} else {
						tlpInvalidCodes.Visible = true;
						return false;
					}
				}
			}

			tlpInvalidCodes.Visible = false;
			return validCodeCount > 0;
		}

		protected override void OnApply()
		{
			((CheatCode)this.Entity).Format = radGameGenie.Checked ? CheatFormat.GameGenie : CheatFormat.ProActionReplay;
		}

		private void GenerateRawCodes()
		{
			StringBuilder sb = new StringBuilder();
			Regex validate = new Regex(radGameGenie.Checked ? "^[a-f0-9]{4}-[a-f0-9]{4}$" : "^[a-f0-9]{8}$", RegexOptions.IgnoreCase);
			foreach(string code in txtCodes.Text.Split(new string[1] { Environment.NewLine }, StringSplitOptions.None)) {
				string trimmedCode = code.Trim();
				if(trimmedCode.Length > 0) {
					if(!validate.IsMatch(trimmedCode)) {
						sb.AppendLine("[invalid code]");
					} else {
						uint encodedCheat = CheatCode.GetEncodedCheat(trimmedCode, radGameGenie.Checked ? CheatFormat.GameGenie : CheatFormat.ProActionReplay);
						sb.AppendLine("$" + (encodedCheat >> 8).ToString("X6") + " = $" + (encodedCheat & 0xFF).ToString("X2"));
					}
				} else {
					sb.AppendLine("");
				}
			}
			txtRawCodes.Text = sb.ToString();
		}

		private void txtCodes_TextChanged(object sender, EventArgs e)
		{
			GenerateRawCodes();
		}

		private void radFormat_CheckedChanged(object sender, EventArgs e)
		{
			GenerateRawCodes();
		}
	}
}
