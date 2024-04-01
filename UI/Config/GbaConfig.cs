﻿using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class GbaConfig : BaseConfig<GbaConfig>
	{
		[Reactive] public ControllerConfig Controller { get; set; } = new();

		[Reactive] public bool SkipBootScreen { get; set; } = false;
		[Reactive] public bool DisableFrameSkipping { get; set; } = false;
		[Reactive] public bool BlendFrames { get; set; } = true;
		[Reactive] public bool GbaAdjustColors { get; set; } = true;
		
		[Reactive] public bool HideBgLayer1 { get; set; } = false;
		[Reactive] public bool HideBgLayer2 { get; set; } = false;
		[Reactive] public bool HideBgLayer3 { get; set; } = false;
		[Reactive] public bool HideBgLayer4 { get; set; } = false;
		[Reactive] public bool DisableSprites { get; set; } = false;

		[Reactive] public RamState RamPowerOnState { get; set; } = RamState.AllZeros;
		[Reactive] public GbaSaveType SaveType { get; set; } = GbaSaveType.AutoDetect;
		[Reactive] public bool AllowInvalidInput { get; set; } = false;

		[Reactive][MinMax(0, 100)] public UInt32 Square1Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 Square2Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 NoiseVol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 WaveVol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 ChannelAVol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 ChannelBVol { get; set; } = 100;

		public void ApplyConfig()
{
			ConfigApi.SetGbaConfig(new InteropGbaConfig() {
				Controller = Controller.ToInterop(),

				SkipBootScreen = SkipBootScreen,
				DisableFrameSkipping = DisableFrameSkipping,
				BlendFrames = BlendFrames,
				GbaAdjustColors = GbaAdjustColors,
				HideBgLayer1 = HideBgLayer1,
				HideBgLayer2 = HideBgLayer2,
				HideBgLayer3 = HideBgLayer3,
				HideBgLayer4 = HideBgLayer4,
				DisableSprites = DisableSprites,

				RamPowerOnState = RamPowerOnState,
				SaveType = SaveType,
				AllowInvalidInput = AllowInvalidInput,

				ChannelAVol = ChannelAVol,
				ChannelBVol = ChannelBVol,
				Square1Vol = Square1Vol,
				Square2Vol = Square2Vol,
				NoiseVol = NoiseVol,
				WaveVol = WaveVol
			});
		}

		internal void InitializeDefaults(DefaultKeyMappingType defaultMappings)
		{
			List<KeyMapping> mappings = new List<KeyMapping>();
			if(defaultMappings.HasFlag(DefaultKeyMappingType.Xbox)) {
				KeyMapping mapping = new();
				KeyPresets.ApplyXboxLayout(mapping, 0, ControllerType.GbaController);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.Ps4)) {
				KeyMapping mapping = new();
				KeyPresets.ApplyPs4Layout(mapping, 0, ControllerType.GbaController);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.WasdKeys)) {
				KeyMapping mapping = new();
				KeyPresets.ApplyWasdLayout(mapping, ControllerType.GbaController);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.ArrowKeys)) {
				KeyMapping mapping = new();
				KeyPresets.ApplyArrowLayout(mapping, ControllerType.GbaController);
				mappings.Add(mapping);
			}

			Controller.Type = ControllerType.GbaController;
			Controller.TurboSpeed = 2;
			if(mappings.Count > 0) {
				Controller.Mapping1 = mappings[0];
				if(mappings.Count > 1) {
					Controller.Mapping2 = mappings[1];
					if(mappings.Count > 2) {
						Controller.Mapping3 = mappings[2];
						if(mappings.Count > 3) {
							Controller.Mapping4 = mappings[3];
						}
					}
				}
			}
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropGbaConfig
	{
		public InteropControllerConfig Controller;

		[MarshalAs(UnmanagedType.I1)] public bool SkipBootScreen;
		[MarshalAs(UnmanagedType.I1)] public bool DisableFrameSkipping;
		[MarshalAs(UnmanagedType.I1)] public bool BlendFrames;
		[MarshalAs(UnmanagedType.I1)] public bool GbaAdjustColors;
		
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer1;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer2;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer3;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer4;
		[MarshalAs(UnmanagedType.I1)] public bool DisableSprites;

		public RamState RamPowerOnState;
		public GbaSaveType SaveType;
		[MarshalAs(UnmanagedType.I1)] public bool AllowInvalidInput;

		public UInt32 ChannelAVol;
		public UInt32 ChannelBVol;
		public UInt32 Square1Vol;
		public UInt32 Square2Vol;
		public UInt32 NoiseVol;
		public UInt32 WaveVol;
	}

	public enum GbaSaveType
	{
		AutoDetect = 0,
		None = 1,
		Sram = 2,
		//EepromUnknown = 3, //Hidden in UI
		Eeprom512 = 4,
		Eeprom8192 = 5,
		Flash64 = 6,
		Flash128 = 7
	}
}
