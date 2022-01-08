using System;

namespace Mesen.Interop
{
	public static class ConsoleTypeExtensions
	{
		public static CpuType GetMainCpuType(this ConsoleType type)
		{
			return type switch {
				ConsoleType.Snes => CpuType.Cpu,
				ConsoleType.Nes => CpuType.Nes,
				ConsoleType.Gameboy or ConsoleType.GameboyColor => CpuType.Gameboy,
				_ => throw new Exception("Invalid type")
			};
		}
	}
}
