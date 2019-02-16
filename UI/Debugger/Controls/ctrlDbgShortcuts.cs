using System;
using System.ComponentModel;
using System.Windows.Forms;
using Mesen.GUI.Config;
using System.Reflection;
using Mesen.GUI.Controls;
using System.Collections.Generic;

namespace Mesen.GUI.Debugger
{
	public partial class ctrlDbgShortcuts : BaseControl
	{
		private FieldInfo[] _shortcuts;

		public ctrlDbgShortcuts()
		{
			InitializeComponent();
		}

		public FieldInfo[] Shortcuts
		{
			set
			{
				_shortcuts = value;
				InitializeGrid();
			}
		}

		protected override void OnResize(EventArgs e)
		{
			base.OnResize(e);
			this.colShortcut.Width = (int)(this.Width / 2.1);
			this.colKeys.Width = (int)(this.Width / 2.1);
		}

		public void InitializeGrid()
		{
			gridShortcuts.Rows.Clear();

			foreach(FieldInfo shortcut in _shortcuts) {
				int i = gridShortcuts.Rows.Add();
				gridShortcuts.Rows[i].Cells[0].Tag = shortcut;
				gridShortcuts.Rows[i].Cells[0].Value = shortcut.GetCustomAttribute<ShortcutNameAttribute>()?.Name ?? shortcut.Name;
				gridShortcuts.Rows[i].Cells[1].Value = DebuggerShortcutsConfig.GetShortcutDisplay(((XmlKeys)shortcut.GetValue(ConfigManager.Config.Debug.Shortcuts)));
			}
		}
		
		private void gridShortcuts_CellMouseDown(object sender, DataGridViewCellMouseEventArgs e)
		{
			//Right-click on buttons to clear mappings
			if(gridShortcuts.Columns[e.ColumnIndex] is DataGridViewButtonColumn && e.RowIndex >= 0) {
				DataGridViewButtonCell button = gridShortcuts.Rows[e.RowIndex].Cells[e.ColumnIndex] as DataGridViewButtonCell;
				if(button != null) {
					if(e.Button == MouseButtons.Right) {
						button.Value = "";
						_shortcuts[e.RowIndex].SetValue(ConfigManager.Config.Debug.Shortcuts, (XmlKeys)Keys.None);
					} else if(e.Button == MouseButtons.Left) {
						using(frmDbgShortcutGetKey frm = new frmDbgShortcutGetKey()) {
							frm.ShowDialog();
							button.Value = DebuggerShortcutsConfig.GetShortcutDisplay(frm.ShortcutKeys);
							_shortcuts[e.RowIndex].SetValue(ConfigManager.Config.Debug.Shortcuts, (XmlKeys)frm.ShortcutKeys);
						}
					}
				}
			}
		}
	}
}
