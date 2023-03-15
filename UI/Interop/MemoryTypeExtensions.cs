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
				case MemoryType.SpcDspRegisters:
					return CpuType.Spc;

				case MemoryType.GsuMemory:
				case MemoryType.GsuWorkRam:
					return CpuType.Gsu;

				case MemoryType.Sa1InternalRam:
				case MemoryType.Sa1Memory:
					return CpuType.Sa1;

				case MemoryType.Cx4DataRam:
				case MemoryType.Cx4Memory:
					return CpuType.Cx4;

				case MemoryType.DspDataRam:
				case MemoryType.DspDataRom:
				case MemoryType.DspProgramRom:
				case MemoryType.NecDspMemory:
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
				case MemoryType.NesPpuMemory:
				case MemoryType.NesSecondarySpriteRam:
					return CpuType.Nes;

				case MemoryType.SnesMemory:
				case MemoryType.SnesPrgRom:
				case MemoryType.SnesWorkRam:
				case MemoryType.SnesSaveRam:
				case MemoryType.SnesVideoRam:
				case MemoryType.SnesSpriteRam:
				case MemoryType.SnesCgRam:
				case MemoryType.BsxPsRam:
				case MemoryType.BsxMemoryPack:
				case MemoryType.SnesRegister:
					return CpuType.Snes;

				case MemoryType.PceMemory:
				case MemoryType.PcePrgRom:
				case MemoryType.PceWorkRam:
				case MemoryType.PceSaveRam:
				case MemoryType.PceCdromRam:
				case MemoryType.PceCardRam:
				case MemoryType.PceAdpcmRam:
				case MemoryType.PceArcadeCardRam:
				case MemoryType.PceVideoRam:
				case MemoryType.PceVideoRamVdc2:
				case MemoryType.PcePaletteRam:
				case MemoryType.PceSpriteRam:
				case MemoryType.PceSpriteRamVdc2:
					return CpuType.Pce;

				default:
					throw new NotImplementedException("Unsupported cpu type");
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

				case MemoryType.PceVideoRam:
				case MemoryType.PceVideoRamVdc2:
				case MemoryType.PcePaletteRam:
				case MemoryType.PceSpriteRam:
				case MemoryType.PceSpriteRamVdc2:
					return true;

				default:
					return false;
			}
		}

		public static bool SupportsMemoryViewer(this MemoryType memType)
		{
			switch(memType) {
				case MemoryType.SnesRegister:
					return false;
			}
			return true;
		}

		public static bool SupportsTileViewer(this MemoryType memType)
		{
			//Hide sprite/palette ram/etc. from tile viewer dropdown, these will never contain tiles
			switch(memType) {
				case MemoryType.SnesCgRam:
				case MemoryType.SnesSpriteRam:
				case MemoryType.SpcRom:
				case MemoryType.SpcDspRegisters:
				case MemoryType.SpcMemory:
				case MemoryType.SnesRegister:

				case MemoryType.GbSpriteRam:

				case MemoryType.NesSecondarySpriteRam:
				case MemoryType.NesSpriteRam:
				case MemoryType.NesPaletteRam:
				case MemoryType.NesInternalRam:
				case MemoryType.NesNametableRam:

				case MemoryType.PceSpriteRam:
				case MemoryType.PceSpriteRamVdc2:
				case MemoryType.PcePaletteRam:
					return false;
			}

			return true;
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
				case MemoryType.PceMemory:
					return true;
			}
			return false;
		}
		
		public static bool IsRomMemory(this MemoryType memType)
		{
			switch(memType) {
				case MemoryType.SnesPrgRom:
				case MemoryType.GbPrgRom:
				case MemoryType.GbBootRom:
				case MemoryType.NesPrgRom:
				case MemoryType.NesChrRom:
				case MemoryType.PcePrgRom:
				case MemoryType.DspDataRom:
				case MemoryType.DspProgramRom:
				case MemoryType.SpcRom:
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
				case MemoryType.SnesRegister:
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

				//PC Engine
				case MemoryType.PceMemory:
				case MemoryType.PcePrgRom:
				case MemoryType.PceWorkRam:
				case MemoryType.PceSaveRam:
				case MemoryType.PceCdromRam:
				case MemoryType.PceCardRam:
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
				case MemoryType.PceMemory:
					return true;
			}

			return false;
		}

		public static bool SupportsCdl(this MemoryType memType)
		{
			switch(memType) {
				case MemoryType.SnesMemory:
				case MemoryType.Sa1Memory:
				case MemoryType.Cx4Memory:
				case MemoryType.GsuMemory:
				case MemoryType.SnesPrgRom:

				case MemoryType.GameboyMemory:
				case MemoryType.GbPrgRom:

				case MemoryType.NesMemory:
				case MemoryType.NesPrgRom:
				case MemoryType.NesChrRom:

				case MemoryType.PceMemory:
				case MemoryType.PcePrgRom:
					return true;

				case MemoryType.NesPpuMemory:
					//NES PPU memory contains no logged data unless game uses CHR ROM
					return DebugApi.GetMemorySize(MemoryType.NesChrRom) > 0;
			}

			return false;
		}

		public static bool SupportsBreakpoints(this MemoryType memType)
		{
			switch(memType) {
				case MemoryType.NesSecondarySpriteRam:
				case MemoryType.SpcDspRegisters:
					return false;
			}

			return true;
		}

		public static bool SupportsFreezeAddress(this MemoryType memType)
		{
			return memType.IsRelativeMemory() && !memType.IsPpuMemory();
		}

		public static string GetShortName(this MemoryType memType)
		{
			return memType switch {
				MemoryType.SnesMemory => "CPU",
				MemoryType.SpcMemory => "SPC",
				MemoryType.Sa1Memory => "SA1",
				MemoryType.GsuMemory => "GSU",
				MemoryType.NecDspMemory => "DSP",
				MemoryType.Cx4Memory => "CX4",

				MemoryType.SnesPrgRom => "PRG",
				MemoryType.SnesWorkRam => "WRAM",
				MemoryType.SnesSaveRam => "SRAM",
				MemoryType.SnesVideoRam => "VRAM",
				MemoryType.SnesSpriteRam => "OAM",
				MemoryType.SnesCgRam => "CGRAM",
				MemoryType.SnesRegister => "REG",

				MemoryType.SpcRam => "RAM",
				MemoryType.SpcRom => "ROM",
				MemoryType.SpcDspRegisters => "DSP",

				MemoryType.DspProgramRom => "PRG",
				MemoryType.DspDataRam => "RAM",
				MemoryType.DspDataRom => "ROM",

				MemoryType.Sa1InternalRam => "IRAM",
				MemoryType.Cx4DataRam => "DATA",
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

				MemoryType.PceMemory => "CPU",
				MemoryType.PcePrgRom => "PRG",
				MemoryType.PceWorkRam => "WRAM",
				MemoryType.PceSaveRam => "SRAM",
				MemoryType.PceCdromRam => "CDRAM",
				MemoryType.PceCardRam => "Card RAM",
				MemoryType.PceAdpcmRam => "ADPCM",
				MemoryType.PceArcadeCardRam => "ARC",
				MemoryType.PceVideoRam => "VRAM",
				MemoryType.PceVideoRamVdc2 => "VRAM2",
				MemoryType.PcePaletteRam => "PAL",
				MemoryType.PceSpriteRam => "SPR",
				MemoryType.PceSpriteRamVdc2 => "SPR2",

				MemoryType.None => "n/a",

				_ => throw new Exception("invalid type"),
			};
		}

		public static string GetFormatString(this MemoryType memType)
		{
			//TODO performance
			CpuType cpuType = memType.ToCpuType();
			if(memType == cpuType.ToMemoryType()) {
				return "X" + cpuType.GetAddressSize();
			}

			return memType switch {
				MemoryType.NesPpuMemory => "X4",
				MemoryType.NesSpriteRam => "X2",
				_ => "X" + (DebugApi.GetMemorySize(memType) - 1).ToString("X").Length
			};
		}

		public static bool IsWordAddressing(this MemoryType memType)
		{
			return memType switch {
				MemoryType.SnesVideoRam => true,
				MemoryType.SnesSpriteRam => true,
				MemoryType.SnesCgRam => true,
				MemoryType.PceVideoRam => true,
				MemoryType.PceSpriteRam => true,
				MemoryType.PceVideoRamVdc2 => true,
				MemoryType.PceSpriteRamVdc2 => true,
				_ => false
			};
		}
	}
}
