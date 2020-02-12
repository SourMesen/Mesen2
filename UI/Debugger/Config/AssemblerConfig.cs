using Mesen.GUI.Controls;
using Mesen.GUI.Utilities;
using System.Drawing;

namespace Mesen.GUI.Config
{
	public class AssemblerConfig
	{
		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;

		public string FontFamily = BaseControl.MonospaceFontFamily;
		public FontStyle FontStyle = FontStyle.Regular;
		public float FontSize = BaseControl.DefaultFontSize;
		public int Zoom = 100;
	}
}