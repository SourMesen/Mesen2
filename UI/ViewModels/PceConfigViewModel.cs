using Avalonia.Controls;
using Mesen.Config;
using Mesen.Controls;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reactive.Linq;

namespace Mesen.ViewModels
{
	public class PceConfigViewModel : DisposableViewModel
	{
		[Reactive] public PcEngineConfig Config { get; set; }
		[Reactive] public PcEngineConfig OriginalConfig { get; set; }
		[Reactive] public PceConfigTab SelectedTab { get; set; } = 0;

		public PceInputConfigViewModel Input { get; private set; }

		public List<PalettePreset> PalettePresets { get; } = new List<PalettePreset>() {
			//The default palette is generated from Kitrinx's palette generator which
			//is meant to produce a more accurate palette for the PCE: https://github.com/Kitrinx/TG16_Palette
			new() { Name = "Default (by Kitrinx)", Palette = PcEngineConfig.DefaultPalette.ToArray() },
		};

		public PceConfigViewModel()
		{
			Config = ConfigManager.Config.PcEngine;
			OriginalConfig = Config.Clone();
			Input = new PceInputConfigViewModel(Config);

			if(Design.IsDesignMode) {
				return;
			}

			UInt32[] linearPalette = new UInt32[512];
			for(UInt32 i = 0; i < 0x200; i++) {
				UInt32 g = To8Bit(i >> 6);
				UInt32 r = To8Bit((i >> 3) & 0x07);
				UInt32 b = To8Bit(i & 0x07);
				linearPalette[i] = (0xFF000000 | (r << 16) | (g << 8) | b);
			}
			PalettePresets.Add(new() { Name = "Linear RGB333", Palette = linearPalette });

			AddDisposable(Input);
			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
		}

		UInt32 To8Bit(UInt32 color)
		{
			return (color << 5) | (color << 2) | (color >> 1);
		}
	}

	public enum PceConfigTab
	{
		General,
		Audio,
		Emulation,
		Input,
		Video
	}
}
