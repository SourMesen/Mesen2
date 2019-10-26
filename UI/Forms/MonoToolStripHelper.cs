using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

namespace Mesen.GUI
{
	//Everything in here is a workaround for issues with dropdown menus in the latest version of Mono (5.18)
	//Bug report:
	//https://github.com/mono/mono/issues/12644
	static class MonoToolStripHelper
	{
		private static HashSet<ToolStripDropDown> _openedDropdowns = new HashSet<ToolStripDropDown>();

		public static void DropdownOpening(object sender, EventArgs e)
		{
			ToolStripDropDownItem ddItem = (ToolStripDropDownItem)sender;
			if(!ddItem.GetCurrentParent().Visible) {
				ddItem.DropDown.Close();
				return;
			}

			HashSet<object> parents = new HashSet<object>();
			parents.Add(ddItem.GetCurrentParent());
			ToolStripItem parent = ddItem.OwnerItem;
			if(parent != null) {
				parents.Add(parent);
				parents.Add(parent.GetCurrentParent());
				while((parent = parent.OwnerItem) != null) {
					parents.Add(parent);
					parents.Add(parent.GetCurrentParent());
				}
			}

			foreach(ToolStripDropDown openedDropdown in _openedDropdowns.ToList()) {
				//Close all non-parent dropdowns when opening a new dropdown
				if(!parents.Contains(openedDropdown.OwnerItem) && !parents.Contains(openedDropdown)) {
					openedDropdown.Close();
				}
			}

			_openedDropdowns.Add(ddItem.DropDown);
		}

		public static void DropdownClosed(object sender, EventArgs e)
		{
			ToolStripDropDownItem ddItem = (ToolStripDropDownItem)sender;
			ToolStripDropDown parent = ddItem.GetCurrentParent() as ToolStripDropDown;
			if(parent != null) {
				Point pos = parent.PointToClient(Cursor.Position);
				if(pos.X < 0 || pos.Y < 0 || pos.X > parent.Width || pos.Y > parent.Height) {
					//When closing a dropdown, if the mouse isn't inside its parent, close all the parent, too.
					parent.Close();
				}
			}

			_openedDropdowns.Remove(ddItem.DropDown);
		}

		public static void ContextMenuOpening(object sender, EventArgs e)
		{
			//Close all existing dropdowns with no exception when a context menu opens
			foreach(ToolStripDropDown openedDropdown in _openedDropdowns.ToList()) {
				openedDropdown.Close();
			}
			_openedDropdowns = new HashSet<ToolStripDropDown>();
			_openedDropdowns.Add((ContextMenuStrip)sender);
		}

		public static void ContextMenuClosed(object sender, EventArgs e)
		{
			_openedDropdowns.Remove((ContextMenuStrip)sender);
		}
	}
}
