using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class FontConfig : ViewModelBase
	{
		[Reactive] public string FontFamily { get; set; } = DebuggerConfig.MonospaceFontFamily;
		[Reactive] public float FontSize { get; set; } = DebuggerConfig.DefaultFontSize;
		[Reactive] public int TextZoom { get; set; } = 100;

		public FontConfig()
		{
		}
	}
}
