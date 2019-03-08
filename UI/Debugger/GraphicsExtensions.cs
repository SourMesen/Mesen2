using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public static class GraphicsExtensions
	{
		public static void DrawOutlinedString(this Graphics g, string text, Font font, Brush foreColor, Brush backColor, int x, int y)
		{
			for(int i = -1; i <= 1; i++) {
				for(int j = -1; j <= 1; j++) {
					g.DrawString(text, font, backColor, x+j, y+i);
				}
			}
			g.DrawString(text, font, foreColor, x, y);
		}
	}

}