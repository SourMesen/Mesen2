using Dock.Model.ReactiveUI.Controls;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Reactive.Linq;
using System.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class BreakpointListViewModel : Tool
	{
		[Reactive] public List<Breakpoint> Breakpoints { get; private set; } = new List<Breakpoint>();

		public BreakpointListViewModel()
		{
			Id = "BreakpointList";
			Title = "Breakpoints";
			CanPin = false;
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
