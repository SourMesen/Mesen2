using System;

namespace Mesen.Interop
{
	public static class SnesMemoryTypeExtensions
	{
		public static CpuType ToCpuType(this SnesMemoryType memType)
		{
			switch(memType) {
				case SnesMemoryType.SpcMemory:
				case SnesMemoryType.SpcRam:
				case SnesMemoryType.SpcRom:
					return CpuType.Spc;

				case SnesMemoryType.GsuMemory:
				case SnesMemoryType.GsuWorkRam:
					return CpuType.Gsu;

				case SnesMemoryType.Sa1InternalRam:
				case SnesMemoryType.Sa1Memory:
					return CpuType.Sa1;

				case SnesMemoryType.DspDataRam:
				case SnesMemoryType.DspDataRom:
				case SnesMemoryType.DspProgramRom:
					return CpuType.NecDsp;

				case SnesMemoryType.GbPrgRom:
				case SnesMemoryType.GbWorkRam:
				case SnesMemoryType.GbCartRam:
				case SnesMemoryType.GbHighRam:
				case SnesMemoryType.GbBootRom:
				case SnesMemoryType.GbVideoRam:
				case SnesMemoryType.GbSpriteRam:
				case SnesMemoryType.GameboyMemory:
					return CpuType.Gameboy;

				case SnesMemoryType.NesMemory:
				case SnesMemoryType.NesPrgRom:
				case SnesMemoryType.NesWorkRam:
				case SnesMemoryType.NesSaveRam:
				case SnesMemoryType.NesChrRam:
				case SnesMemoryType.NesChrRom:
				case SnesMemoryType.NesInternalRam:
				case SnesMemoryType.NesNametableRam:
				case SnesMemoryType.NesPaletteRam:
				case SnesMemoryType.NesSpriteRam:
					return CpuType.Nes;

				default:
					return CpuType.Snes;
			}
		}

		public static bool IsPpuMemory(this SnesMemoryType memType)
		{
			switch(memType) {
				case SnesMemoryType.VideoRam:
				case SnesMemoryType.SpriteRam:
				case SnesMemoryType.CGRam:
				
				case SnesMemoryType.GbVideoRam:
				case SnesMemoryType.GbSpriteRam:

				case SnesMemoryType.NesPpuMemory:
				case SnesMemoryType.NesSecondarySpriteRam:
				case SnesMemoryType.NesSpriteRam:
				case SnesMemoryType.NesNametableRam:
				case SnesMemoryType.NesChrRam:
				case SnesMemoryType.NesChrRom:
				case SnesMemoryType.NesPaletteRam:
					return true;

				default:
					return false;
			}
		}

		public static bool IsRelativeMemory(this SnesMemoryType memType)
		{
			switch(memType) {
				case SnesMemoryType.CpuMemory:
				case SnesMemoryType.SpcMemory:
				case SnesMemoryType.Sa1Memory:
				case SnesMemoryType.GsuMemory:
				case SnesMemoryType.NecDspMemory:
				case SnesMemoryType.Cx4Memory:
				case SnesMemoryType.GameboyMemory:
				case SnesMemoryType.NesMemory:
				case SnesMemoryType.NesPpuMemory:
					return true;
			}
			return false;
		}

		public static bool SupportsLabels(this SnesMemoryType memType)
		{
			switch(memType) {
				//SNES
				case SnesMemoryType.PrgRom:
				case SnesMemoryType.WorkRam:
				case SnesMemoryType.SaveRam:
				case SnesMemoryType.Register:
				case SnesMemoryType.SpcRam:
				case SnesMemoryType.SpcRom:
				case SnesMemoryType.Sa1InternalRam:
				
				//Gameboy
				case SnesMemoryType.GbPrgRom:
				case SnesMemoryType.GbWorkRam:
				case SnesMemoryType.GbCartRam:
				case SnesMemoryType.GbHighRam:
				case SnesMemoryType.GbBootRom:
				case SnesMemoryType.GameboyMemory:

				//NES
				case SnesMemoryType.NesPrgRom:
				case SnesMemoryType.NesWorkRam:
				case SnesMemoryType.NesSaveRam:
				case SnesMemoryType.NesInternalRam:
				case SnesMemoryType.NesMemory:
					return true;
			}

			return false;
		}

		public static bool SupportsWatch(this SnesMemoryType memType)
		{
			switch(memType) {
				case SnesMemoryType.CpuMemory:
				case SnesMemoryType.SpcMemory:
				case SnesMemoryType.Sa1Memory:
				case SnesMemoryType.GsuMemory:
				case SnesMemoryType.NecDspMemory:
				case SnesMemoryType.Cx4Memory:
				case SnesMemoryType.GameboyMemory:
				case SnesMemoryType.NesMemory:
					return true;
			}

			return false;
		}

		public static bool SupportsCdl(this SnesMemoryType memType)
		{
			switch(memType) {
				case SnesMemoryType.CpuMemory:
				case SnesMemoryType.GameboyMemory:
				case SnesMemoryType.NesMemory:
				case SnesMemoryType.PrgRom:
				case SnesMemoryType.GbPrgRom:
				case SnesMemoryType.NesPrgRom:
					return true;
			}

			return false;
		}

		public static bool SupportsBreakpoints(this SnesMemoryType memType)
		{
			switch(memType) {
				case SnesMemoryType.NesSecondarySpriteRam:
					return false;
			}

			return true;
		}

		public static string GetShortName(this SnesMemoryType memType)
		{
			return memType switch {
				SnesMemoryType.CpuMemory => "CPU",
				SnesMemoryType.SpcMemory => "SPC",
				SnesMemoryType.Sa1Memory => "SA1",
				SnesMemoryType.GsuMemory => "GSU",
				SnesMemoryType.NecDspMemory => "DSP",

				SnesMemoryType.PrgRom => "PRG",
				SnesMemoryType.WorkRam => "WRAM",
				SnesMemoryType.SaveRam => "SRAM",
				SnesMemoryType.VideoRam => "VRAM",
				SnesMemoryType.SpriteRam => "OAM",
				SnesMemoryType.CGRam => "CG",

				SnesMemoryType.SpcRam => "RAM",
				SnesMemoryType.SpcRom => "ROM",

				SnesMemoryType.DspProgramRom => "DSP",
				SnesMemoryType.Sa1InternalRam => "IRAM",
				SnesMemoryType.GsuWorkRam => "GWRAM",

				SnesMemoryType.BsxPsRam => "PSRAM",
				SnesMemoryType.BsxMemoryPack => "MPACK",

				SnesMemoryType.GameboyMemory => "CPU",
				SnesMemoryType.GbPrgRom => "PRG",
				SnesMemoryType.GbWorkRam => "WRAM",
				SnesMemoryType.GbCartRam => "SRAM",
				SnesMemoryType.GbHighRam => "HRAM",
				SnesMemoryType.GbBootRom => "BOOT",
				SnesMemoryType.GbVideoRam => "VRAM",
				SnesMemoryType.GbSpriteRam => "OAM",

				SnesMemoryType.NesMemory => "CPU",
				SnesMemoryType.NesPpuMemory => "PPU",
				SnesMemoryType.NesPrgRom => "PRG",
				SnesMemoryType.NesWorkRam => "WRAM",
				SnesMemoryType.NesSaveRam => "SRAM",
				SnesMemoryType.NesInternalRam => "RAM",

				SnesMemoryType.NesSpriteRam => "SPR",
				SnesMemoryType.NesSecondarySpriteRam => "SPR2",
				SnesMemoryType.NesPaletteRam => "PAL",
				SnesMemoryType.NesNametableRam => "NTRAM",
				SnesMemoryType.NesChrRom => "CHR",
				SnesMemoryType.NesChrRam => "CHR",

				SnesMemoryType.Register => "REG",

				_ => throw new Exception("invalid type"),
			};
		}
	}
}
