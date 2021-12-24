using Avalonia.Media;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System.Reactive.Linq;

namespace Mesen.Config
{
	public class FontConfig : ViewModelBase
	{
		[Reactive] public string FontFamily { get; set; } = DebuggerConfig.MonospaceFontFamily;
		[Reactive] public float FontSize { get; set; } = DebuggerConfig.DefaultFontSize;
		[Reactive] public int TextZoom { get; set; } = 100;

		[ObservableAsProperty] public FontFamily FontFamilyObject { get; } = new FontFamily(DebuggerConfig.MonospaceFontFamily);

		public FontConfig()
		{
			this.WhenAnyValue(x => x.FontFamily).Select(x => new FontFamily(x)).ToPropertyEx(this, x => x.FontFamilyObject);
		}
	}
}
