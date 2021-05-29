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
				case CpuType.Cpu:
				case CpuType.Sa1: {
					CpuState state = DebugApi.GetState<CpuState>(cpuType);
					return (uint)(state.K << 16) | state.PC;
				}

				case CpuType.Spc: {
					SpcState state = DebugApi.GetState<SpcState>(cpuType);
					return (uint)state.PC;
				}

				case CpuType.Gameboy: {
					GbCpuState state = DebugApi.GetState<GbCpuState>(cpuType);
					return (uint)state.PC;
				}

				case CpuType.Nes: {
					NesCpuState state = DebugApi.GetState<NesCpuState>(cpuType);
					return (uint)state.PC;
				}

				default: throw new Exception("Invalid cpu type");
			}

		}
	}
}
