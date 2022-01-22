using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger
{
	public static class DebugUtilities
	{
		public static uint GetProgramCounter(CpuType cpuType)
		{
			switch(cpuType) {
				case CpuType.Snes:
				case CpuType.Sa1: {
					SnesCpuState state = DebugApi.GetCpuState<SnesCpuState>(cpuType);
					return (uint)(state.K << 16) | state.PC;
				}

				case CpuType.Spc: {
					SpcState state = DebugApi.GetCpuState<SpcState>(cpuType);
					return (uint)state.PC;
				}

				case CpuType.Gameboy: {
					GbCpuState state = DebugApi.GetCpuState<GbCpuState>(cpuType);
					return (uint)state.PC;
				}

				case CpuType.Nes: {
					NesCpuState state = DebugApi.GetCpuState<NesCpuState>(cpuType);
					return (uint)state.PC;
				}

				default: throw new Exception("Invalid cpu type");
			}

		}
	}
}
