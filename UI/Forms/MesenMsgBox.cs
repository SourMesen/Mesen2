using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms
{
	class MesenMsgBox
	{
		public static DialogResult Show(string text, MessageBoxButtons buttons, MessageBoxIcon icon, params string[] args)
		{
			string resourceText = ResourceHelper.GetMessage(text, args);

			if(resourceText.StartsWith("[[")) {
				if(args != null && args.Length > 0) {
					return MessageBox.Show(string.Format("Critical error (" + text + ") {0}", args), "Mesen", buttons, icon);
				} else {
					return MessageBox.Show(string.Format("Critical error (" + text + ")"), "Mesen", buttons, icon);
				}
			} else {
				Form mainForm = Application.OpenForms.Count > 0 ? Application.OpenForms[0] : null;
				if(mainForm?.InvokeRequired == true) {
					DialogResult result = DialogResult.Cancel;
					mainForm.Invoke((Action)(() => {
						result = MessageBox.Show(mainForm, ResourceHelper.GetMessage(text, args), "Mesen", buttons, icon);
					}));
					return result;
				} else {
					return MessageBox.Show(ResourceHelper.GetMessage(text, args), "Mesen", buttons, icon);
				}
			}
		}
	}
}
