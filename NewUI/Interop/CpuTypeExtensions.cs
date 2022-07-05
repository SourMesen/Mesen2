using System;

namespace Mesen.Interop
{
	public static class CpuTypeExtensions
	{
		public static MemoryType ToMemoryType(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => MemoryType.SnesMemory,
				CpuType.Spc => MemoryType.SpcMemory,
				CpuType.NecDsp => MemoryType.NecDspMemory,
				CpuType.Sa1 => MemoryType.Sa1Memory,
				CpuType.Gsu => MemoryType.GsuMemory,
				CpuType.Cx4 => MemoryType.Cx4Memory,
				CpuType.Gameboy => MemoryType.GameboyMemory,
				CpuType.Nes => MemoryType.NesMemory,
				CpuType.Pce => MemoryType.PceMemory,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static MemoryType GetVramMemoryType(this CpuType cpuType, bool getExtendedRam = false)
		{
			return cpuType switch {
				CpuType.Snes => MemoryType.SnesVideoRam,
				CpuType.Gameboy => MemoryType.GbVideoRam,
				CpuType.Nes => MemoryType.NesPpuMemory,
				CpuType.Pce => getExtendedRam ? MemoryType.PceVideoRamVdc2 : MemoryType.PceVideoRam,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static MemoryType GetSpriteRamMemoryType(this CpuType cpuType, bool getExtendedRam = false)
		{
			return cpuType switch {
				CpuType.Snes => MemoryType.SnesSpriteRam,
				CpuType.Gameboy => MemoryType.GbSpriteRam,
				CpuType.Nes => MemoryType.NesSpriteRam,
				CpuType.Pce => getExtendedRam ? MemoryType.PceSpriteRamVdc2 : MemoryType.PceSpriteRam,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static MemoryType GetPrgRomMemoryType(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => MemoryType.SnesPrgRom,
				CpuType.NecDsp => MemoryType.DspProgramRom,
				CpuType.Sa1 => MemoryType.SnesPrgRom,
				CpuType.Gsu => MemoryType.SnesPrgRom,
				CpuType.Cx4 => MemoryType.SnesPrgRom,
				
				CpuType.Gameboy => MemoryType.GbPrgRom,
				CpuType.Nes => MemoryType.NesPrgRom,
				CpuType.Pce => MemoryType.PcePrgRom,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static int GetAddressSize(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => 6,
				CpuType.Spc => 4,
				CpuType.NecDsp => 4,
				CpuType.Sa1 => 6,
				CpuType.Gsu => 6,
				CpuType.Cx4 => 6,
				CpuType.Gameboy => 4,
				CpuType.Nes => 4,
				CpuType.Pce => 4,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static int GetByteCodeSize(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => 4,
				CpuType.Spc => 3,
				CpuType.NecDsp => 3,
				CpuType.Sa1 => 4,
				CpuType.Gsu => 3,
				CpuType.Cx4 => 4,
				CpuType.Gameboy => 3,
				CpuType.Nes => 3,
				CpuType.Pce => 4,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static DebuggerFlags GetDebuggerFlag(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => DebuggerFlags.SnesDebuggerEnabled,
				CpuType.Spc => DebuggerFlags.SpcDebuggerEnabled,
				CpuType.NecDsp => DebuggerFlags.NecDspDebuggerEnabled,
				CpuType.Sa1 => DebuggerFlags.Sa1DebuggerEnabled,
				CpuType.Gsu => DebuggerFlags.GsuDebuggerEnabled,
				CpuType.Cx4 => DebuggerFlags.Cx4DebuggerEnabled,
				CpuType.Gameboy => DebuggerFlags.GbDebuggerEnabled,
				CpuType.Nes => DebuggerFlags.NesDebuggerEnabled,
				CpuType.Pce => DebuggerFlags.PceDebuggerEnabled,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static ConsoleType GetConsoleType(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => ConsoleType.Snes,
				CpuType.Spc => ConsoleType.Snes,
				CpuType.NecDsp => ConsoleType.Snes,
				CpuType.Sa1 => ConsoleType.Snes,
				CpuType.Gsu => ConsoleType.Snes,
				CpuType.Cx4 => ConsoleType.Snes,
				CpuType.Gameboy => ConsoleType.Gameboy,
				CpuType.Nes => ConsoleType.Nes,
				CpuType.Pce => ConsoleType.PcEngine,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static bool SupportsAssembler(this CpuType cpuType)
		{
			switch(cpuType) {
				case CpuType.Snes:
				case CpuType.Gameboy:
				case CpuType.Nes:
				case CpuType.Pce:
					return true;
				
				default:
					return false;
			};
		}

		public static bool SupportsFunctionList(this CpuType cpuType)
		{
			switch(cpuType) {
				case CpuType.Snes:
				case CpuType.Sa1:
				case CpuType.Gameboy:
				case CpuType.Nes:
				case CpuType.Pce:
					return true;

				default:
					return false;
			};
		}

		public static bool SupportsCallStack(this CpuType cpuType)
		{
			switch(cpuType) {
				case CpuType.Gsu:
					return false;

				default:
					return true;
			};
		}

		public static bool SupportsMemoryMappings(this CpuType cpuType)
		{
			switch(cpuType) {
				case CpuType.Gameboy:
				case CpuType.Nes:
				case CpuType.Pce:
					return true;

				default:
					return false;
			};
		}

		public static byte GetNopOpCode(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => 0xEA,
				CpuType.Gameboy => 0x00,
				CpuType.Nes => 0xEA,
				CpuType.Pce => 0xEA,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static bool CanAccessMemoryType(this CpuType cpuType, MemoryType memType)
		{
			switch(memType) {
				case MemoryType.SnesPrgRom:
					return cpuType == CpuType.Snes || cpuType == CpuType.Sa1 || cpuType == CpuType.Gsu || cpuType == CpuType.Cx4;
				
				case MemoryType.SnesSaveRam:
					return cpuType == CpuType.Snes || cpuType == CpuType.Sa1 || cpuType == CpuType.Cx4;

				default:
					//All other types are specific to a single CPU type
					return memType.ToCpuType() == cpuType;
			}
		}
	}
}
