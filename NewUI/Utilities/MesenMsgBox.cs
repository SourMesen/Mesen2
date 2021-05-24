using Avalonia.Controls;
using Mesen.Localization;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class MesenMsgBox
	{
		public static Task<DialogResult> Show(Window parent, string text, MessageBoxButtons buttons, MessageBoxIcon icon, params string[] args)
		{
			string resourceText = ResourceHelper.GetMessage(text, args);

			if(resourceText.StartsWith("[[")) {
				if(args != null && args.Length > 0) {
					return MessageBox.Show(parent, string.Format("Critical error (" + text + ") {0}", args), "Mesen", buttons, icon);
				} else {
					return MessageBox.Show(parent, string.Format("Critical error (" + text + ")"), "Mesen", buttons, icon);
				}
			} else {
				return MessageBox.Show(parent, ResourceHelper.GetMessage(text, args), "Mesen", buttons, icon);
			}
		}
	}
}
