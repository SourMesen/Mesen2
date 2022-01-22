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
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static MemoryType GetVramMemoryType(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => MemoryType.SnesVideoRam,
				CpuType.Gameboy => MemoryType.GbVideoRam,
				CpuType.Nes => MemoryType.NesPpuMemory,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static MemoryType GetSpriteRamMemoryType(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => MemoryType.SnesSpriteRam,
				CpuType.Gameboy => MemoryType.GbSpriteRam,
				CpuType.Nes => MemoryType.NesSpriteRam,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static MemoryType GetPrgRomMemoryType(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => MemoryType.SnesPrgRom,
				CpuType.Gameboy => MemoryType.GbPrgRom,
				CpuType.Nes => MemoryType.NesPrgRom,
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
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static DebuggerFlags GetDebuggerFlag(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Snes => DebuggerFlags.CpuDebuggerEnabled,
				CpuType.Spc => DebuggerFlags.SpcDebuggerEnabled,
				CpuType.NecDsp => DebuggerFlags.NecDspDebuggerEnabled,
				CpuType.Sa1 => DebuggerFlags.Sa1DebuggerEnabled,
				CpuType.Gsu => DebuggerFlags.GsuDebuggerEnabled,
				CpuType.Cx4 => DebuggerFlags.Cx4DebuggerEnabled,
				CpuType.Gameboy => DebuggerFlags.GbDebuggerEnabled,
				CpuType.Nes => DebuggerFlags.NesDebuggerEnabled,
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
				_ => throw new Exception("Invalid CPU type"),
			};
		}
	}
}
