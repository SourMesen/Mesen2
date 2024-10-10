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
	public class GbaConfig : BaseConfig<GbaConfig>
	{
		[Reactive] public ConsoleOverrideConfig ConfigOverrides { get; set; } = new();

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
		[Reactive] public GbaRtcType RtcType { get; set; } = GbaRtcType.AutoDetect;
		[Reactive] public bool AllowInvalidInput { get; set; } = false;
		[Reactive] public bool EnableMgbaLogApi { get; set; } = false;

		[Reactive][MinMax(0, 100)] public UInt32 Square1Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 Square2Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 NoiseVol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 WaveVol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 ChannelAVol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 ChannelBVol { get; set; } = 100;

		public void ApplyConfig()
{
			ConfigManager.Config.Video.ApplyConfig();

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
				RtcType = RtcType,
				AllowInvalidInput = AllowInvalidInput,
				EnableMgbaLogApi = EnableMgbaLogApi,

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
			Controller.InitDefaults(defaultMappings, ControllerType.GbaController);
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
		public GbaRtcType RtcType;
		[MarshalAs(UnmanagedType.I1)] public bool AllowInvalidInput;
		[MarshalAs(UnmanagedType.I1)] public bool EnableMgbaLogApi;

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

	public enum GbaRtcType
	{
		AutoDetect = 0,
		Enabled = 1,
		Disabled = 2,
	}
}
