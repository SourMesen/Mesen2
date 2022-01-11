using System;

namespace Mesen.Interop
{
	public static class CpuTypeExtensions
	{
		public static SnesMemoryType ToMemoryType(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Cpu => SnesMemoryType.CpuMemory,
				CpuType.Spc => SnesMemoryType.SpcMemory,
				CpuType.NecDsp => SnesMemoryType.NecDspMemory,
				CpuType.Sa1 => SnesMemoryType.Sa1Memory,
				CpuType.Gsu => SnesMemoryType.GsuMemory,
				CpuType.Cx4 => SnesMemoryType.Cx4Memory,
				CpuType.Gameboy => SnesMemoryType.GameboyMemory,
				CpuType.Nes => SnesMemoryType.NesMemory,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static SnesMemoryType GetVramMemoryType(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Cpu => SnesMemoryType.VideoRam,
				CpuType.Gameboy => SnesMemoryType.GbVideoRam,
				CpuType.Nes => SnesMemoryType.NesPpuMemory,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static SnesMemoryType GetSpriteRamMemoryType(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Cpu => SnesMemoryType.SpriteRam,
				CpuType.Gameboy => SnesMemoryType.GbSpriteRam,
				CpuType.Nes => SnesMemoryType.NesSpriteRam,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static SnesMemoryType GetPrgRomMemoryType(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Cpu => SnesMemoryType.PrgRom,
				CpuType.Gameboy => SnesMemoryType.GbPrgRom,
				CpuType.Nes => SnesMemoryType.NesPrgRom,
				_ => throw new Exception("Invalid CPU type"),
			};
		}

		public static int GetAddressSize(this CpuType cpuType)
		{
			return cpuType switch {
				CpuType.Cpu => 6,
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
				CpuType.Cpu => 4,
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
				CpuType.Cpu => DebuggerFlags.CpuDebuggerEnabled,
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
				CpuType.Cpu => ConsoleType.Snes,
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
