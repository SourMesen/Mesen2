using Mesen.Config;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger
{
	public static class PaletteHelper
	{
		private static uint To8Bit(int color)
		{
			return (uint)((color << 3) + (color >> 2));
		}

		public static uint ToArgb(int rgb555)
		{
			uint b = To8Bit(rgb555 >> 10);
			uint g = To8Bit((rgb555 >> 5) & 0x1F);
			uint r = To8Bit(rgb555 & 0x1F);

			return (0xFF000000 | (r << 16) | (g << 8) | b);
		}

		public static UInt32[] GetConvertedPalette(CpuType cpuType, ConsoleType consoleType)
		{
			switch(cpuType) {
				case CpuType.Cpu: {
					byte[] cgram = DebugApi.GetMemoryState(SnesMemoryType.CGRam);
					UInt32[] colors = new UInt32[256];
					for(int i = 0; i < 256; i++) {
						colors[i] = ToArgb(cgram[i * 2] | cgram[i * 2 + 1] << 8);
					}
					return colors;
				}

				case CpuType.Nes: {
					byte[] cgram = DebugApi.GetMemoryState(SnesMemoryType.NesPaletteRam);
					UInt32[] colors = new UInt32[32];
					for(int i = 0; i < 32; i++) {
						colors[i] = ConfigManager.Config.Nes.UserPalette[cgram[i]];
					}
					return colors;
				}

				case CpuType.Gameboy: {
					GbPpuState ppu = DebugApi.GetPpuState<GbPpuState>(CpuType.Gameboy);
					if(consoleType == ConsoleType.GameboyColor) {
						UInt32[] colors = new UInt32[64];
						for(int i = 0; i < 32; i++) {
							colors[i] = ToArgb(ppu.CgbBgPalettes[i]);
						}

						for(int i = 0; i < 32; i++) {
							colors[i + 32] = ToArgb(ppu.CgbObjPalettes[i]);
						}
						return colors;
					} else {
						UInt32[] colors = new UInt32[16];
						GameboyConfig cfg = ConfigManager.Config.Gameboy;

						for(int i = 0; i < 4; i++) {
							colors[i] = cfg.BgColors[(ppu.BgPalette >> (i * 2)) & 0x03];
							colors[i + 4] = cfg.Obj0Colors[(ppu.ObjPalette0 >> (i * 2)) & 0x03];
							colors[i + 8] = cfg.Obj1Colors[(ppu.ObjPalette1 >> (i * 2)) & 0x03];
						}

						colors[12] = 0xFFFFFFFF;
						colors[13] = 0xFFB0B0B0;
						colors[14] = 0xFF606060;
						colors[15] = 0xFF000000;
						return colors;
					}
				}
			}

			throw new Exception("Invalid cpu type");
		}
	}
}
