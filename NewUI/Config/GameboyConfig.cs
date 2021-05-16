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
		[Reactive] public GameboyModel Model { get; set; } = GameboyModel.Auto;
		[Reactive] public bool UseSgb2 { get; set; } = true;

		[Reactive] public bool BlendFrames { get; set; } = true;
		[Reactive] public bool GbcAdjustColors { get; set; } = true;

		[Reactive] public RamState RamPowerOnState { get; set; } = RamState.Random;

		[Reactive] public UInt32[] BgColors { get; set; } = new UInt32[] { 0xFFFFFFFF, 0xFFB0B0B0, 0xFF686868, 0xFF000000 };
		[Reactive] public UInt32[] Obj0Colors { get; set; } = new UInt32[] { 0xFFFFFFFF, 0xFFB0B0B0, 0xFF686868, 0xFF000000 };
		[Reactive] public UInt32[] Obj1Colors { get; set; } = new UInt32[] { 0xFFFFFFFF, 0xFFB0B0B0, 0xFF686868, 0xFF000000 };

		[Reactive] public UInt32 Square1Vol { get; set; } = 100;
		[Reactive] public UInt32 Square2Vol { get; set; } = 100;
		[Reactive] public UInt32 NoiseVol { get; set; } = 100;
		[Reactive] public UInt32 WaveVol { get; set; } = 100;

		public void ApplyConfig()
		{
			ConfigApi.SetGameboyConfig(new InteropGameboyConfig() {
				Model = Model,
				UseSgb2 = UseSgb2,

				BlendFrames = BlendFrames,
				GbcAdjustColors = GbcAdjustColors,

				RamPowerOnState = RamPowerOnState,

				BgColors = BgColors,
				Obj0Colors = Obj0Colors,
				Obj1Colors = Obj1Colors,

				Square1Vol = Square1Vol,
				Square2Vol = Square2Vol,
				NoiseVol = NoiseVol,
				WaveVol = WaveVol
			});
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropGameboyConfig
	{
		public GameboyModel Model;
		[MarshalAs(UnmanagedType.I1)] public bool UseSgb2;

		[MarshalAs(UnmanagedType.I1)] public bool BlendFrames;
		[MarshalAs(UnmanagedType.I1)] public bool GbcAdjustColors;

		public RamState RamPowerOnState;

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
		Auto = 0,
		Gameboy = 1,
		GameboyColor = 2,
		SuperGameboy = 3
	}
}
