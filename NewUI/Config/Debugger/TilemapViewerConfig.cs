using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Avalonia;
using Mesen.Debugger;
using Mesen.Interop;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class TilemapViewerConfig : BaseWindowConfig<TilemapViewerConfig>
	{
		[Reactive] public bool ShowSettingsPanel { get; set; } = true;

		[Reactive] public int ImageScale { get; set; } = 1;

		[Reactive] public bool ShowGrid { get; set; }
		[Reactive] public bool ShowAltGrid { get; set; }
		[Reactive] public bool ShowScrollOverlay { get; set; }
		[Reactive] public bool HighlightTileChanges { get; set; }
		[Reactive] public bool HighlightAttributeChanges { get; set; }
		[Reactive] public TilemapDisplayMode DisplayMode { get; set; } = TilemapDisplayMode.Default;

		[Reactive] public bool AutoRefresh { get; set; } = true;
		[Reactive] public int RefreshScanline { get; set; } = 240;
		[Reactive] public int RefreshCycle { get; set; } = 0;

		public TilemapViewerConfig()
		{
		}
	}
}
