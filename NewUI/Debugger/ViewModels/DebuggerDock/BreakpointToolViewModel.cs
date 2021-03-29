using Dock.Model.ReactiveUI.Controls;

namespace Mesen.Debugger.ViewModels.DebuggerDock
{
	public class BreakpointToolViewModel : Tool
	{
		public BreakpointListViewModel BreakpointListViewModel { get; set; }

		public BreakpointToolViewModel(BreakpointListViewModel breakpointListViewModel)
		{
			Id = "BreakpointTool";
			Title = "Breakpoints";
			BreakpointListViewModel = breakpointListViewModel;
		}
	}
}