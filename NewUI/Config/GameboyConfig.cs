using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public class GameboyConfig : BaseConfig<GameboyConfig>
	{
		[Reactive] public GameboyModel Model { get; set; } = GameboyModel.Auto;
		[Reactive] public bool UseSgb2 { get; set; } = true;

		[Reactive] public bool BlendFrames { get; set; } = true;
		[Reactive] public bool GbcAdjustColors { get; set; } = true;

		[Reactive] public RamState RamPowerOnState { get; set; } = RamState.Random;

		[Reactive] public UInt32 BgColor0 { get; set; } = 0xFFFFFF;
		[Reactive] public UInt32 BgColor1 { get; set; } = 0xB0B0B0;
		[Reactive] public UInt32 BgColor2 { get; set; } = 0x686868;
		[Reactive] public UInt32 BgColor3 { get; set; } = 0x000000;
		[Reactive] public UInt32 Obj0Color0 { get; set; } = 0xFFFFFF;
		[Reactive] public UInt32 Obj0Color1 { get; set; } = 0xB0B0B0;
		[Reactive] public UInt32 Obj0Color2 { get; set; } = 0x686868;
		[Reactive] public UInt32 Obj0Color3 { get; set; } = 0x000000;
		[Reactive] public UInt32 Obj1Color0 { get; set; } = 0xFFFFFF;
		[Reactive] public UInt32 Obj1Color1 { get; set; } = 0xB0B0B0;
		[Reactive] public UInt32 Obj1Color2 { get; set; } = 0x686868;
		[Reactive] public UInt32 Obj1Color3 { get; set; } = 0x000000;

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

				BgColor0 = BgColor0,
				BgColor1 = BgColor1,
				BgColor2 = BgColor2,
				BgColor3 = BgColor3,
				Obj0Color0 = Obj0Color0,
				Obj0Color1 = Obj0Color1,
				Obj0Color2 = Obj0Color2,
				Obj0Color3 = Obj0Color3,
				Obj1Color0 = Obj1Color0,
				Obj1Color1 = Obj1Color1,
				Obj1Color2 = Obj1Color2,
				Obj1Color3 = Obj1Color3,

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

		public UInt32 BgColor0;
		public UInt32 BgColor1;
		public UInt32 BgColor2;
		public UInt32 BgColor3;
		public UInt32 Obj0Color0;
		public UInt32 Obj0Color1;
		public UInt32 Obj0Color2;
		public UInt32 Obj0Color3;
		public UInt32 Obj1Color0;
		public UInt32 Obj1Color1;
		public UInt32 Obj1Color2;
		public UInt32 Obj1Color3;

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
