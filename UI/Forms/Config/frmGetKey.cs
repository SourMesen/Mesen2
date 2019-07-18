using Mesen.GUI.Config.Shortcuts;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms.Config
{
	public partial class frmGetKey : BaseInputForm
	{
		private bool _singleKeyMode = false;
		private List<UInt32> _prevScanCodes = new List<UInt32>();

		public KeyCombination ShortcutKey { get; set; }

		public frmGetKey(bool singleKeyMode)
		{
			InitializeComponent();
			_singleKeyMode = singleKeyMode;
			if(_singleKeyMode) {
				tableLayoutPanel1.RowStyles[1].SizeType = SizeType.Absolute;
				tableLayoutPanel1.RowStyles[1].Height = 0;
			}
			if(_singleKeyMode) {
				lblCurrentKeys.Height = 1;
				lblCurrentKeys.Visible = false;
			}
			ShortcutKey = new KeyCombination();
			InputApi.UpdateInputDevices();
			InputApi.ResetKeyState();

			//Prevent other keybindings from interfering/activating
			InputApi.DisableAllKeys(true);
		}

		public override bool IsConfigForm { get { return true; } }

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			InputApi.DisableAllKeys(false);
			base.OnFormClosing(e);
		}

		private void SelectKeyCombination(KeyCombination key)
		{
			if(!string.IsNullOrWhiteSpace(key.ToString())) {
				ShortcutKey = key;
				this.Close();
			}
		}

		private void tmrCheckKey_Tick(object sender, EventArgs e)
		{	
			List<UInt32> scanCodes = InputApi.GetPressedKeys();

			if(_singleKeyMode) {
				if(scanCodes.Count >= 1) {
					//Always use the largest scancode (when multiple buttons are pressed at once)
					scanCodes = new List<UInt32> { scanCodes.OrderBy(code => -code).First() };
					this.SelectKeyCombination(new KeyCombination(scanCodes));
				}
			} else {
				KeyCombination key = new KeyCombination(_prevScanCodes);
				lblCurrentKeys.Text = key.ToString();

				if(scanCodes.Count < _prevScanCodes.Count) {
					//Confirm key selection when the user releases a key
					this.SelectKeyCombination(key);
				}

				_prevScanCodes = scanCodes;
			}
		}
	}
}
