using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class GameboyConfig : BaseConfig<GameboyConfig>
	{
		[Reactive] public ConsoleOverrideConfig ConfigOverrides { get; set; } = new();

		[Reactive] public ControllerConfig Controller { get; set; } = new();

		[Reactive] public GameboyModel Model { get; set; } = GameboyModel.AutoFavorGbc;
		[Reactive] public bool UseSgb2 { get; set; } = true;

		[Reactive] public bool BlendFrames { get; set; } = true;
		[Reactive] public bool GbcAdjustColors { get; set; } = true;
		
		[Reactive] public bool DisableBackground { get; set; } = false;
		[Reactive] public bool DisableSprites { get; set; } = false;
		[Reactive] public bool HideSgbBorders { get; set; } = false;

		[Reactive] public RamState RamPowerOnState { get; set; } = RamState.Random;
		[Reactive] public bool AllowInvalidInput { get; set; } = false;

		[Reactive] public UInt32[] BgColors { get; set; } = new UInt32[] { 0xFFFFFFFF, 0xFFB0B0B0, 0xFF686868, 0xFF000000 };
		[Reactive] public UInt32[] Obj0Colors { get; set; } = new UInt32[] { 0xFFFFFFFF, 0xFFB0B0B0, 0xFF686868, 0xFF000000 };
		[Reactive] public UInt32[] Obj1Colors { get; set; } = new UInt32[] { 0xFFFFFFFF, 0xFFB0B0B0, 0xFF686868, 0xFF000000 };

		[Reactive][MinMax(0, 100)] public UInt32 Square1Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 Square2Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 NoiseVol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 WaveVol { get; set; } = 100;

		public void ApplyConfig()
		{
			ConfigManager.Config.Video.ApplyConfig();

			ConfigApi.SetGameboyConfig(new InteropGameboyConfig() {
				Controller = Controller.ToInterop(),
				Model = Model,
				UseSgb2 = UseSgb2,

				BlendFrames = BlendFrames,
				GbcAdjustColors = GbcAdjustColors,
				DisableBackground = DisableBackground,
				DisableSprites = DisableSprites,
				HideSgbBorders = HideSgbBorders,

				RamPowerOnState = RamPowerOnState,
				AllowInvalidInput = AllowInvalidInput,

				BgColors = BgColors,
				Obj0Colors = Obj0Colors,
				Obj1Colors = Obj1Colors,

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
				KeyPresets.ApplyXboxLayout(mapping, 0, ControllerType.GameboyController);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.Ps4)) {
				KeyMapping mapping = new();
				KeyPresets.ApplyPs4Layout(mapping, 0, ControllerType.GameboyController);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.WasdKeys)) {
				KeyMapping mapping = new();
				KeyPresets.ApplyWasdLayout(mapping, ControllerType.GameboyController);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.ArrowKeys)) {
				KeyMapping mapping = new();
				KeyPresets.ApplyArrowLayout(mapping, ControllerType.GameboyController);
				mappings.Add(mapping);
			}

			Controller.Type = ControllerType.GameboyController;
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
	public struct InteropGameboyConfig
	{
		public InteropControllerConfig Controller;

		public GameboyModel Model;
		[MarshalAs(UnmanagedType.I1)] public bool UseSgb2;

		[MarshalAs(UnmanagedType.I1)] public bool BlendFrames;
		[MarshalAs(UnmanagedType.I1)] public bool GbcAdjustColors;
		
		[MarshalAs(UnmanagedType.I1)] public bool DisableBackground;
		[MarshalAs(UnmanagedType.I1)] public bool DisableSprites;
		[MarshalAs(UnmanagedType.I1)] public bool HideSgbBorders;

		public RamState RamPowerOnState;
		[MarshalAs(UnmanagedType.I1)] public bool AllowInvalidInput;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
		public UInt32[] BgColors;
		
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
		public UInt32[] Obj0Colors;
		
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
		public UInt32[] Obj1Colors;

		public UInt32 Square1Vol;
		public UInt32 Square2Vol;
		public UInt32 NoiseVol;
		public UInt32 WaveVol;
	}

	public enum GameboyModel
	{
		AutoFavorGbc,
		AutoFavorSgb,
		AutoFavorGb,
		Gameboy,
		GameboyColor,
		SuperGameboy
	}
}
