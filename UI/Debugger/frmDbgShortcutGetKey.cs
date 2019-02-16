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
	public partial class frmDbgShortcutGetKey : BaseForm
	{
		private Keys[] _ignoredKeys = new Keys[9] {
			Keys.LMenu, Keys.RMenu, Keys.Menu, Keys.LShiftKey, Keys.RShiftKey, Keys.ShiftKey, Keys.LControlKey, Keys.RControlKey, Keys.ControlKey
		};

		private Keys _shortcutKeys = Keys.None;
		public Keys ShortcutKeys
		{
			get { return this._shortcutKeys; }
			set { this._shortcutKeys = value; }
		}

		public frmDbgShortcutGetKey()
		{
			InitializeComponent();
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			foreach(Keys ignoredKey in _ignoredKeys) {
				if((keyData & (Keys)0xFF) == ignoredKey) {
					keyData ^= ignoredKey;
				}
			}

			_shortcutKeys = keyData;
			lblCurrentKeys.Text = DebuggerShortcutsConfig.GetShortcutDisplay(keyData);

			return true;
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
		}

		protected override void OnKeyUp(KeyEventArgs e)
		{
			base.OnKeyUp(e);
			this.Close();
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			base.OnFormClosing(e);
		}
	}
}
