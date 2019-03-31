using Mesen.GUI.Config;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Controls
{
	public class ctrlMesenToolStrip : ToolStrip
	{
		private const int WM_MOUSEACTIVATE = 0x21;
		protected override void WndProc(ref Message m)
		{
			if(m.Msg == WM_MOUSEACTIVATE && this.CanFocus && !this.Focused) {
				this.FindForm()?.Focus();
			}
			base.WndProc(ref m);
		}
		
		public void AddItemToToolbar(ToolStripMenuItem item, string caption = null)
		{
			if(item == null) {
				this.Items.Add("-");
			} else {
				ToolStripItem newItem = item.HasDropDownItems ? (ToolStripItem)new ToolStripDropDownButton(item.Text, item.Image) : (ToolStripItem)new ToolStripButton(item.Image);
				if(item.Image == null) {
					newItem.Text = item.Text;
				}
				newItem.ToolTipText = (caption ?? item.Text) + (item.ShortcutKeys != Keys.None ? $" ({DebuggerShortcutsConfig.GetShortcutDisplay(item.ShortcutKeys)})" : "");
				newItem.Click += (s, e) => item.PerformClick();
				if(newItem is ToolStripButton) {
					((ToolStripButton)newItem).Checked = item.Checked;
					item.CheckedChanged += (s, e) => ((ToolStripButton)newItem).Checked = item.Checked;
				}
				newItem.Enabled = item.Enabled;
				newItem.MouseEnter += (s, e) => newItem.ToolTipText = (caption ?? item.Text) + (item.ShortcutKeys != Keys.None ? $" ({DebuggerShortcutsConfig.GetShortcutDisplay(item.ShortcutKeys)})" : "");
				item.EnabledChanged += (s, e) => newItem.Enabled = item.Enabled;
				item.VisibleChanged += (s, e) => newItem.Visible = item.Visible;

				if(item.HasDropDownItems) {
					foreach(ToolStripItem ddItem in item.DropDownItems) {
						ToolStripItem newDdItem = ((ToolStripDropDownButton)newItem).DropDownItems.Add(ddItem.Text, ddItem.Image);
						newDdItem.Click += (s, e) => ddItem.PerformClick();
					}
				}

				this.Items.Add(newItem);
			}
		}

		public void AddItemsToToolbar(params ToolStripMenuItem[] items)
		{
			foreach(ToolStripMenuItem item in items) {
				AddItemToToolbar(item);
			}
		}
	}
}
