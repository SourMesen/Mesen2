using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	class FontDialogHelper
	{
		public static Font SelectFont(Font currentFont)
		{
			using(FontDialog fd = new FontDialog()) {
				fd.Font = currentFont;
				fd.ShowApply = false;
				fd.ShowColor = false;
				fd.ShowEffects = false;
				fd.ShowHelp = false;
				if(fd.ShowDialog() == DialogResult.OK) {
					Font font = fd.Font;

					Size sizeM = TextRenderer.MeasureText("M", font);
					Size sizeDot = TextRenderer.MeasureText(".", font);

					if(sizeM != sizeDot) {
						if(MessageBox.Show("The font you've selected (" + fd.Font.FontFamily.Name.ToString() + ") doesn't appear to be a monospace font. Using anything other than a monospace font will cause display issues in the UI. Are you sure you want to use this font?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.Yes) {
							return fd.Font;
						} else {
							return currentFont;
						}
					} else {
						return fd.Font;
					}
				} else {
					return currentFont;
				}
			}
		}
	}
}
