using Avalonia.Media;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Config
{
	public class FontConfig : BaseConfig<FontConfig>
	{
		[Reactive] public string FontFamily { get; set; } = DebuggerConfig.MonospaceFontFamily;
		[Reactive] public float FontSize { get; set; } = DebuggerConfig.DefaultFontSize;
		[Reactive] public int TextZoom { get; set; } = 100;

		public FontConfig()
		{
		}
	}
}
