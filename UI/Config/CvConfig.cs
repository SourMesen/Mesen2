using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config;

public class CvConfig : BaseConfig<CvConfig>
{
	[Reactive] public CvControllerConfig Port1 { get; set; } = new();
	[Reactive] public CvControllerConfig Port2 { get; set; } = new();

	[ValidValues(ConsoleRegion.Auto, ConsoleRegion.Ntsc, ConsoleRegion.Pal)]
	[Reactive] public ConsoleRegion Region { get; set; } = ConsoleRegion.Auto;

	[Reactive] public RamState RamPowerOnState { get; set; } = RamState.AllZeros;

	[Reactive] public bool RemoveSpriteLimit { get; set; } = false;
	[Reactive] public bool DisableSprites { get; set; } = false;
	[Reactive] public bool DisableBackground { get; set; } = false;

	[Reactive][MinMax(0, 100)] public UInt32 Tone1Vol { get; set; } = 100;
	[Reactive][MinMax(0, 100)] public UInt32 Tone2Vol { get; set; } = 100;
	[Reactive][MinMax(0, 100)] public UInt32 Tone3Vol { get; set; } = 100;
	[Reactive][MinMax(0, 100)] public UInt32 NoiseVol { get; set; } = 100;

	public void ApplyConfig()
	{
		ConfigApi.SetCvConfig(new InteropCvConfig() {
			Port1 = Port1.ToInterop(),
			Port2 = Port2.ToInterop(),

			Region = Region,
			RamPowerOnState = RamPowerOnState,

			RemoveSpriteLimit = RemoveSpriteLimit,
			DisableBackground = DisableBackground,
			DisableSprites = DisableSprites,

			Tone1Vol = Tone1Vol,
			Tone2Vol = Tone2Vol,
			Tone3Vol = Tone3Vol,
			NoiseVol = NoiseVol,
		});
	}

	internal void InitializeDefaults(DefaultKeyMappingType defaultMappings)
	{
		List<CvKeyMapping> mappings = new List<CvKeyMapping>();
		if(defaultMappings.HasFlag(DefaultKeyMappingType.Xbox)) {
			CvKeyMapping mapping = new();
			mapping.ColecoVisionControllerButtons = mapping.GetDefaultCustomKeys(ControllerType.ColecoVisionController, KeyPresetType.XboxP1);
			mappings.Add(mapping);
		}
		if(defaultMappings.HasFlag(DefaultKeyMappingType.Ps4)) {
			CvKeyMapping mapping = new();
			mapping.ColecoVisionControllerButtons = mapping.GetDefaultCustomKeys(ControllerType.ColecoVisionController, KeyPresetType.Ps4P1);
			mappings.Add(mapping);
		}
		if(defaultMappings.HasFlag(DefaultKeyMappingType.WasdKeys)) {
			CvKeyMapping mapping = new();
			mapping.ColecoVisionControllerButtons = mapping.GetDefaultCustomKeys(ControllerType.ColecoVisionController, KeyPresetType.WasdKeys);
			mappings.Add(mapping);
		}
		if(defaultMappings.HasFlag(DefaultKeyMappingType.ArrowKeys)) {
			CvKeyMapping mapping = new();
			mapping.ColecoVisionControllerButtons = mapping.GetDefaultCustomKeys(ControllerType.ColecoVisionController, KeyPresetType.ArrowKeys);
			mappings.Add(mapping);
		}

		Port1.Type = ControllerType.ColecoVisionController;
		Port1.TurboSpeed = 2;
		if(mappings.Count > 0) {
			Port1.Mapping1 = mappings[0];
			if(mappings.Count > 1) {
				Port1.Mapping2 = mappings[1];
				if(mappings.Count > 2) {
					Port1.Mapping3 = mappings[2];
					if(mappings.Count > 3) {
						Port1.Mapping4 = mappings[3];
					}
				}
			}
		}
	}
}

[StructLayout(LayoutKind.Sequential)]
public struct InteropCvConfig
{
	public InteropControllerConfig Port1;
	public InteropControllerConfig Port2;

	public ConsoleRegion Region;
	public RamState RamPowerOnState;

	[MarshalAs(UnmanagedType.I1)] public bool RemoveSpriteLimit;
	[MarshalAs(UnmanagedType.I1)] public bool DisableSprites;
	[MarshalAs(UnmanagedType.I1)] public bool DisableBackground;

	public UInt32 Tone1Vol;
	public UInt32 Tone2Vol;
	public UInt32 Tone3Vol;
	public UInt32 NoiseVol;
}
