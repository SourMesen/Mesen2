using Avalonia;
using Avalonia.Media;

namespace Mesen.Config
{
	public class AssemblerConfig
	{
		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;

		public string FontFamily = DebuggerConfig.MonospaceFontFamily;
		public FontStyle FontStyle = FontStyle.Normal;
		public float FontSize = DebuggerConfig.DefaultFontSize;
		public int Zoom = 100;
	}
}