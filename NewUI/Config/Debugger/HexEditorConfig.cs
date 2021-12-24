using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Avalonia;
using Avalonia.Media;
using Mesen.Interop;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class HexEditorConfig : BaseWindowConfig<HexEditorConfig>
	{
		[Reactive] public bool ShowOptionPanel { get; set; } = true;
		[Reactive] public bool AutoRefresh { get; set; } = true;
		[Reactive] public bool IgnoreRedundantWrites { get; set; } = false;

		[Reactive] public int BytesPerRow { get; set; } = 16;

		[Reactive] public bool HighDensityTextMode { get; set; } = false;

		[Reactive] public bool ShowCharacters { get; set; } = true;

		[Reactive] public bool HideUnusedBytes { get; set; } = false;
		[Reactive] public bool HideReadBytes { get; set; } = false;
		[Reactive] public bool HideWrittenBytes { get; set; } = false;
		[Reactive] public bool HideExecutedBytes { get; set; } = false;

		[Reactive] public HighlightFadeSpeed FadeSpeed { get; set; } = HighlightFadeSpeed.Normal;
		[Reactive] public HighlightConfig ReadHighlight { get; set; } = new() { Highlight = true, ColorCode = Colors.Blue.ToUint32() };
		[Reactive] public HighlightConfig WriteHighlight { get; set; } = new() { Highlight = true, ColorCode = Colors.Red.ToUint32() };
		[Reactive] public HighlightConfig ExecHighlight { get; set; } = new() { Highlight = true, ColorCode = Colors.Green.ToUint32() };

		[Reactive] public HighlightConfig LabelHighlight { get; set; } = new() { Highlight = false, ColorCode = Colors.LightPink.ToUint32() };
		[Reactive] public HighlightConfig CodeHighlight { get; set; } = new() { Highlight = false, ColorCode = Colors.DarkSeaGreen.ToUint32() };
		[Reactive] public HighlightConfig DataHighlight { get; set; } = new() { Highlight = false, ColorCode = Colors.LightSteelBlue.ToUint32() };
		[Reactive] public bool HighlightBreakpoints { get; set; } = false;

		[Reactive] public SnesMemoryType MemoryType { get; set; } = SnesMemoryType.CpuMemory;

		public HexEditorConfig()
		{
		}
	}

	public enum HighlightFadeSpeed
	{
		NoFade,
		Slow,
		Normal,
		Fast
	}

	public static class HighlightFadeSpeedExtensions
	{
		public static int ToFrameCount(this HighlightFadeSpeed speed)
		{
			return speed switch {
				HighlightFadeSpeed.NoFade => 0,
				HighlightFadeSpeed.Slow => 600,
				HighlightFadeSpeed.Normal => 300,
				HighlightFadeSpeed.Fast => 120,
				_ => 0
			};
		}
	}

	public class HighlightConfig
	{
		[Reactive] public bool Highlight { get; set; }
		[Reactive] public UInt32 ColorCode { get; set; }

		public Color Color => Color.FromUInt32(ColorCode);
	}
}
