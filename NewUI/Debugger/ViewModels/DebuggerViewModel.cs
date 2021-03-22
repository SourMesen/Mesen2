using Mesen.ViewModels;
using ReactiveUI;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels
{
	public class DebuggerViewModel : ViewModelBase
	{
		private DisassemblyViewerViewModel _disassembly;
		public DisassemblyViewerViewModel Disassembly
		{
			get => _disassembly;
			set => this.RaiseAndSetIfChanged(ref _disassembly, value);
		}

		private SnesCpuViewModel _snesCpu;
		public SnesCpuViewModel SnesCpu
		{
			get => _snesCpu;
			set => this.RaiseAndSetIfChanged(ref _snesCpu, value);
		}

		private BreakpointListViewModel _breakpointList;
		public BreakpointListViewModel BreakpointList
		{
			get => _breakpointList;
			set => this.RaiseAndSetIfChanged(ref _breakpointList, value);
		}


		public DebuggerViewModel()
		{
			_disassembly = new DisassemblyViewerViewModel();
			_snesCpu = new SnesCpuViewModel();
			_breakpointList = new BreakpointListViewModel();
		}
   }
}
