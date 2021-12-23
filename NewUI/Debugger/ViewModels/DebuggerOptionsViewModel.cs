using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

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
