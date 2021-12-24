using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;

namespace Mesen.Debugger.ViewModels
{
	public class DebuggerOptionsViewModel : ViewModelBase
	{
		public DebuggerConfig Config { get; }

		public bool IsSnes { get; }
		public bool IsNes { get; }
		public bool IsGameboy { get; }

		public DebuggerOptionsViewModel() : this(new DebuggerConfig(), CpuType.Cpu) { }

		public DebuggerOptionsViewModel(DebuggerConfig config, CpuType cpuType)
		{
			Config = config;
			IsSnes = cpuType == CpuType.Cpu;
			IsNes = cpuType == CpuType.Nes;
			IsGameboy = cpuType == CpuType.Gameboy;
		}
	}
}
