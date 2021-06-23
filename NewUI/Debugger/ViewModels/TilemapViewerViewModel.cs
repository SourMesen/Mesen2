using Avalonia.Controls;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.ViewModels
{
	public class TilemapViewerViewModel : ViewModelBase
	{
		public CpuType CpuType { get; }
		public ConsoleType ConsoleType { get; }

		[Reactive] public bool ShowGrid { get; set; }
		[Reactive] public bool ShowAltGrid { get; set; }
		[Reactive] public bool HighlightTileChanges { get; set; }
		[Reactive] public bool HighlightAttributeChanges { get; set; }
		[Reactive] public TilemapDisplayMode DisplayMode { get; set; }

		[Reactive] public List<TilemapViewerTab> Tabs { get; set; } = new List<TilemapViewerTab>();

		//For designer
		public TilemapViewerViewModel() : this(CpuType.Cpu, ConsoleType.Snes) { }

		public TilemapViewerViewModel(CpuType cpuType, ConsoleType consoleType)
		{
			CpuType = cpuType;
			ConsoleType = consoleType;

			switch(CpuType) {
				case CpuType.Cpu: 
					Tabs = new List<TilemapViewerTab>() {
						new() { Title = "Layer 1", Layer = 0 },
						new() { Title = "Layer 2", Layer = 1 },
						new() { Title = "Layer 3", Layer = 2 },
						new() { Title = "Layer 4", Layer = 3 },
					};
					break;

				case CpuType.Nes:
					Tabs = new List<TilemapViewerTab>() {
						new() { Title = "", Layer = 0 }
					};
					break;

				case CpuType.Gameboy:
					Tabs = new List<TilemapViewerTab>() {
						new() { Title = "Layer 1", Layer = 0 },
						new() { Title = "Layer 2", Layer = 1 }
					};
					break;
			}
		}
	}

	public class TilemapViewerTab
	{
		public string Title { get; set; } = "";
		public int Layer { get; set; }  = 0;
	}
}
