﻿using System;

namespace Mesen.Interop
{
	public static class ConsoleTypeExtensions
	{
		public static CpuType GetMainCpuType(this ConsoleType type)
		{
			return type switch {
				ConsoleType.Snes => CpuType.Snes,
				ConsoleType.Nes => CpuType.Nes,
				ConsoleType.Gameboy => CpuType.Gameboy,
				ConsoleType.PcEngine => CpuType.Pce,
				ConsoleType.Sms => CpuType.Sms,
				ConsoleType.Gba => CpuType.Gba,
				_ => throw new Exception("Invalid type")
			};
		}

		public static bool SupportsCheats(this ConsoleType type)
		{
			return type switch {
				ConsoleType.Gba => false,
				_ => true
			};
		}
	}
}
