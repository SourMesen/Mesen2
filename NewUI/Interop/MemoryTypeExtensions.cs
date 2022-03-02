using System;

namespace Mesen.Interop
{
	public static class MemoryTypeExtensions
	{
		public static CpuType ToCpuType(this MemoryType memType)
		{
			switch(memType) {
				case MemoryType.SpcMemory:
				case MemoryType.SpcRam:
				case MemoryType.SpcRom:
					return CpuType.Spc;

				case MemoryType.GsuMemory:
				case MemoryType.GsuWorkRam:
					return CpuType.Gsu;

				case MemoryType.Sa1InternalRam:
				case MemoryType.Sa1Memory:
					return CpuType.Sa1;

				case MemoryType.DspDataRam:
				case MemoryType.DspDataRom:
				case MemoryType.DspProgramRom:
					return CpuType.NecDsp;

				case MemoryType.GbPrgRom:
				case MemoryType.GbWorkRam:
				case MemoryType.GbCartRam:
				case MemoryType.GbHighRam:
				case MemoryType.GbBootRom:
				case MemoryType.GbVideoRam:
				case MemoryType.GbSpriteRam:
				case MemoryType.GameboyMemory:
					return CpuType.Gameboy;

				case MemoryType.NesMemory:
				case MemoryType.NesPrgRom:
				case MemoryType.NesWorkRam:
				case MemoryType.NesSaveRam:
				case MemoryType.NesChrRam:
				case MemoryType.NesChrRom:
				case MemoryType.NesInternalRam:
				case MemoryType.NesNametableRam:
				case MemoryType.NesPaletteRam:
				case MemoryType.NesSpriteRam:
					return CpuType.Nes;

				default:
					return CpuType.Snes;
			}
		}

		public static bool IsPpuMemory(this MemoryType memType)
		{
			switch(memType) {
				case MemoryType.SnesVideoRam:
				case MemoryType.SnesSpriteRam:
				case MemoryType.SnesCgRam:
				
				case MemoryType.GbVideoRam:
				case MemoryType.GbSpriteRam:

				case MemoryType.NesPpuMemory:
				case MemoryType.NesSecondarySpriteRam:
				case MemoryType.NesSpriteRam:
				case MemoryType.NesNametableRam:
				case MemoryType.NesChrRam:
				case MemoryType.NesChrRom:
				case MemoryType.NesPaletteRam:
					return true;

				default:
					return false;
			}
		}

		public static bool IsRelativeMemory(this MemoryType memType)
		{
			switch(memType) {
				case MemoryType.SnesMemory:
				case MemoryType.SpcMemory:
				case MemoryType.Sa1Memory:
				case MemoryType.GsuMemory:
				case MemoryType.NecDspMemory:
				case MemoryType.Cx4Memory:
				case MemoryType.GameboyMemory:
				case MemoryType.NesMemory:
				case MemoryType.NesPpuMemory:
					return true;
			}
			return false;
		}

		public static bool SupportsLabels(this MemoryType memType)
		{
			switch(memType) {
				//SNES
				case MemoryType.SnesPrgRom:
				case MemoryType.SnesWorkRam:
				case MemoryType.SnesSaveRam:
				case MemoryType.Register:
				case MemoryType.SpcRam:
				case MemoryType.SpcRom:
				case MemoryType.Sa1InternalRam:
				
				//Gameboy
				case MemoryType.GbPrgRom:
				case MemoryType.GbWorkRam:
				case MemoryType.GbCartRam:
				case MemoryType.GbHighRam:
				case MemoryType.GbBootRom:
				case MemoryType.GameboyMemory:

				//NES
				case MemoryType.NesPrgRom:
				case MemoryType.NesWorkRam:
				case MemoryType.NesSaveRam:
				case MemoryType.NesInternalRam:
				case MemoryType.NesMemory:
					return true;
			}

			return false;
		}

		public static bool SupportsWatch(this MemoryType memType)
		{
			switch(memType) {
				case MemoryType.SnesMemory:
				case MemoryType.SpcMemory:
				case MemoryType.Sa1Memory:
				case MemoryType.GsuMemory:
				case MemoryType.NecDspMemory:
				case MemoryType.Cx4Memory:
				case MemoryType.GameboyMemory:
				case MemoryType.NesMemory:
					return true;
			}

			return false;
		}

		public static bool SupportsCdl(this MemoryType memType)
		{
			switch(memType) {
				case MemoryType.SnesMemory:
				case MemoryType.GameboyMemory:
				case MemoryType.NesMemory:
				case MemoryType.SnesPrgRom:
				case MemoryType.GbPrgRom:
				case MemoryType.NesPrgRom:
					return true;
			}

			return false;
		}

		public static bool SupportsBreakpoints(this MemoryType memType)
		{
			switch(memType) {
				case MemoryType.NesSecondarySpriteRam:
					return false;
			}

			return true;
		}

		public static string GetShortName(this MemoryType memType)
		{
			return memType switch {
				MemoryType.SnesMemory => "CPU",
				MemoryType.SpcMemory => "SPC",
				MemoryType.Sa1Memory => "SA1",
				MemoryType.GsuMemory => "GSU",
				MemoryType.NecDspMemory => "DSP",

				MemoryType.SnesPrgRom => "PRG",
				MemoryType.SnesWorkRam => "WRAM",
				MemoryType.SnesSaveRam => "SRAM",
				MemoryType.SnesVideoRam => "VRAM",
				MemoryType.SnesSpriteRam => "OAM",
				MemoryType.SnesCgRam => "CGRAM",

				MemoryType.SpcRam => "RAM",
				MemoryType.SpcRom => "ROM",

				MemoryType.DspProgramRom => "DSP",
				MemoryType.Sa1InternalRam => "IRAM",
				MemoryType.GsuWorkRam => "GWRAM",

				MemoryType.BsxPsRam => "PSRAM",
				MemoryType.BsxMemoryPack => "MPACK",

				MemoryType.GameboyMemory => "CPU",
				MemoryType.GbPrgRom => "PRG",
				MemoryType.GbWorkRam => "WRAM",
				MemoryType.GbCartRam => "SRAM",
				MemoryType.GbHighRam => "HRAM",
				MemoryType.GbBootRom => "BOOT",
				MemoryType.GbVideoRam => "VRAM",
				MemoryType.GbSpriteRam => "OAM",

				MemoryType.NesMemory => "CPU",
				MemoryType.NesPpuMemory => "PPU",
				MemoryType.NesPrgRom => "PRG",
				MemoryType.NesWorkRam => "WRAM",
				MemoryType.NesSaveRam => "SRAM",
				MemoryType.NesInternalRam => "RAM",

				MemoryType.NesSpriteRam => "SPR",
				MemoryType.NesSecondarySpriteRam => "SPR2",
				MemoryType.NesPaletteRam => "PAL",
				MemoryType.NesNametableRam => "NTRAM",
				MemoryType.NesChrRom => "CHR",
				MemoryType.NesChrRam => "CHR",

				MemoryType.Register => "REG",

				_ => throw new Exception("invalid type"),
			};
		}

		public static string GetFormatString(this MemoryType memType)
		{
			return memType switch {
				MemoryType.NesPpuMemory => "X4",
				MemoryType.NesSpriteRam => "X2",
				_ => "X" + (DebugApi.GetMemorySize(memType) - 1).ToString("X").Length
			};
		}
	}
}
