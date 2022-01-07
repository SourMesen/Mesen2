using Dock.Model.ReactiveUI.Controls;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Reactive.Linq;
using System.Linq;
using Mesen.Interop;

namespace Mesen.Debugger.ViewModels
{
	public class BreakpointListViewModel : Tool
	{
		[Reactive] public List<Breakpoint> Breakpoints { get; private set; } = new List<Breakpoint>();
		public CpuType CpuType { get; }

		[Obsolete("For designer only")]
		public BreakpointListViewModel() : this(CpuType.Cpu) { }

		public BreakpointListViewModel(CpuType cpuType)
		{
			Id = "BreakpointList";
			Title = "Breakpoints";
			CanPin = false;
			CpuType = cpuType;
			BreakpointManager.BreakpointsChanged += BreakpointManager_BreakpointsChanged;
		}

		public void UpdateBreakpoints()
		{
			Breakpoints = new List<Breakpoint>(BreakpointManager.Breakpoints);
		}

		private void BreakpointManager_BreakpointsChanged(object? sender, EventArgs e)
		{
			UpdateBreakpoints();
		}
	}
}
