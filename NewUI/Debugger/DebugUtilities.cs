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
			//TODO move to C++ core
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

				case CpuType.NecDsp: {
					NecDspState state = DebugApi.GetCpuState<NecDspState>(cpuType);
					return (uint)(state.PC * 3);
				}

				case CpuType.Gsu: {
					GsuState state = DebugApi.GetCpuState<GsuState>(cpuType);
					return (uint)((state.ProgramBank << 16) | state.R[15]);
				}

				case CpuType.Cx4: {
					Cx4State state = DebugApi.GetCpuState<Cx4State>(cpuType);
					return (uint)(state.Cache.Address[state.Cache.Page] + (state.PC * 2)) & 0xFFFFFF;
				}

				default: throw new Exception("Invalid cpu type");
			}

		}
	}
}
