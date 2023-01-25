using Avalonia.Media;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Config
{
	public class FontConfig : BaseConfig<FontConfig>
	{
		[Reactive] public string FontFamily { get; set; } = "";
		[Reactive] public double FontSize { get; set; } = 12;
	}
}
