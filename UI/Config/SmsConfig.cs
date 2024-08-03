﻿using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config;

public class SmsConfig : BaseConfig<SmsConfig>
{
	[Reactive] public ConsoleOverrideConfig ConfigOverrides { get; set; } = new();

	[Reactive] public SmsControllerConfig Port1 { get; set; } = new();
	[Reactive] public SmsControllerConfig Port2 { get; set; } = new();

	[ValidValues(ConsoleRegion.Auto, ConsoleRegion.Ntsc, ConsoleRegion.Pal)]
	[Reactive] public ConsoleRegion Region { get; set; } = ConsoleRegion.Auto;

	[ValidValues(ConsoleRegion.Auto, ConsoleRegion.Ntsc, ConsoleRegion.NtscJapan, ConsoleRegion.Pal)]
	[Reactive] public ConsoleRegion GameGearRegion { get; set; } = ConsoleRegion.Auto;

	[Reactive] public RamState RamPowerOnState { get; set; } = RamState.Random;

	[Reactive] public SmsRevision Revision { get; set; } = SmsRevision.Compatibility;

	[Reactive] public bool UseSgPalette { get; set; } = true;
	[Reactive] public bool GgBlendFrames { get; set; } = true;
	[Reactive] public bool RemoveSpriteLimit { get; set; } = false;
	[Reactive] public bool DisableSprites { get; set; } = false;
	[Reactive] public bool DisableBackground { get; set; } = false;

	[Reactive][MinMax(0, 100)] public UInt32 Tone1Vol { get; set; } = 100;
	[Reactive][MinMax(0, 100)] public UInt32 Tone2Vol { get; set; } = 100;
	[Reactive][MinMax(0, 100)] public UInt32 Tone3Vol { get; set; } = 100;
	[Reactive][MinMax(0, 100)] public UInt32 NoiseVol { get; set; } = 100;
	[Reactive][MinMax(0, 100)] public UInt32 FmAudioVolume { get; set; } = 100;
	[Reactive] public bool EnableFmAudio { get; set; } = true;

	[Reactive] public OverscanConfig NtscOverscan { get; set; } = new() { Top = 24, Bottom = 24 };
	[Reactive] public OverscanConfig PalOverscan { get; set; } = new() { Top = 24, Bottom = 24 };

	public void ApplyConfig()
	{
		ConfigManager.Config.Video.ApplyConfig();

		ConfigApi.SetSmsConfig(new InteropSmsConfig() {
			Port1 = Port1.ToInterop(),
			Port2 = Port2.ToInterop(),

			Region = Region,
			GameGearRegion = GameGearRegion,
			RamPowerOnState = RamPowerOnState,
			Revision = Revision,

			UseSgPalette = UseSgPalette,
			GgBlendFrames = GgBlendFrames,
			RemoveSpriteLimit = RemoveSpriteLimit,
			DisableBackground = DisableBackground,
			DisableSprites = DisableSprites,

			Tone1Vol = Tone1Vol,
			Tone2Vol = Tone2Vol,
			Tone3Vol = Tone3Vol,
			NoiseVol = NoiseVol,
			FmAudioVolume = FmAudioVolume,
			EnableFmAudio = EnableFmAudio,

			NtscOverscan = NtscOverscan.ToInterop(),
			PalOverscan = PalOverscan.ToInterop(),
		});
	}

	internal void InitializeDefaults(DefaultKeyMappingType defaultMappings)
	{
		List<SmsKeyMapping> mappings = new List<SmsKeyMapping>();
		if(defaultMappings.HasFlag(DefaultKeyMappingType.Xbox)) {
			SmsKeyMapping mapping = new();
			KeyPresets.ApplyXboxLayout(mapping, 0, ControllerType.SmsController);
			mappings.Add(mapping);
		}
		if(defaultMappings.HasFlag(DefaultKeyMappingType.Ps4)) {
			SmsKeyMapping mapping = new();
			KeyPresets.ApplyPs4Layout(mapping, 0, ControllerType.SmsController);
			mappings.Add(mapping);
		}
		if(defaultMappings.HasFlag(DefaultKeyMappingType.WasdKeys)) {
			SmsKeyMapping mapping = new();
			KeyPresets.ApplyWasdLayout(mapping, ControllerType.SmsController);
			mappings.Add(mapping);
		}
		if(defaultMappings.HasFlag(DefaultKeyMappingType.ArrowKeys)) {
			SmsKeyMapping mapping = new();
			KeyPresets.ApplyArrowLayout(mapping, ControllerType.SmsController);
			mappings.Add(mapping);
		}

		Port1.Type = ControllerType.SmsController;
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
public struct InteropSmsConfig
{
	public InteropControllerConfig Port1;
	public InteropControllerConfig Port2;

	public ConsoleRegion Region;
	public ConsoleRegion GameGearRegion;
	public RamState RamPowerOnState;
	public SmsRevision Revision;

	[MarshalAs(UnmanagedType.I1)] public bool UseSgPalette;
	[MarshalAs(UnmanagedType.I1)] public bool GgBlendFrames;
	[MarshalAs(UnmanagedType.I1)] public bool RemoveSpriteLimit;
	[MarshalAs(UnmanagedType.I1)] public bool DisableSprites;
	[MarshalAs(UnmanagedType.I1)] public bool DisableBackground;

	public UInt32 Tone1Vol;
	public UInt32 Tone2Vol;
	public UInt32 Tone3Vol;
	public UInt32 NoiseVol;
	public UInt32 FmAudioVolume;
	[MarshalAs(UnmanagedType.I1)] public bool EnableFmAudio;

	public InteropOverscanDimensions NtscOverscan;
	public InteropOverscanDimensions PalOverscan;
}

public enum SmsRevision
{
	Compatibility,
	Sms1,
	Sms2
}
