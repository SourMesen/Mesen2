using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	[StructLayout(LayoutKind.Sequential)]
	public class GameboyConfig
	{
		public GameboyModel Model = GameboyModel.Auto;
		[MarshalAs(UnmanagedType.I1)] public bool UseSgb2 = true;
		
		[MarshalAs(UnmanagedType.I1)] public bool BlendFrames = true;
		[MarshalAs(UnmanagedType.I1)] public bool GbcAdjustColors = true;

		public UInt32 BgColor0 = 0xFFFFFF;
		public UInt32 BgColor1 = 0xB0B0B0;
		public UInt32 BgColor2 = 0x686868;
		public UInt32 BgColor3 = 0x000000;
		public UInt32 Obj0Color0 = 0xFFFFFF;
		public UInt32 Obj0Color1 = 0xB0B0B0;
		public UInt32 Obj0Color2 = 0x686868;
		public UInt32 Obj0Color3 = 0x000000;
		public UInt32 Obj1Color0 = 0xFFFFFF;
		public UInt32 Obj1Color1 = 0xB0B0B0;
		public UInt32 Obj1Color2 = 0x686868;
		public UInt32 Obj1Color3 = 0x000000;

		public UInt32 Square1Vol = 100;
		public UInt32 Square2Vol = 100;
		public UInt32 NoiseVol = 100;
		public UInt32 WaveVol = 100;

		public void ApplyConfig()
		{
			ConfigApi.SetGameboyConfig(this);
		}
	}

	public enum GameboyModel
	{
		Auto = 0,
		Gameboy = 1,
		GameboyColor = 2,
		SuperGameboy = 3
	}
}
