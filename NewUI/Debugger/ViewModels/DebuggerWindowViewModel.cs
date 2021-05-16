using Dock.Model;
using Dock.Model.Controls;
using Dock.Model.Core;
using Dock.Model.ReactiveUI.Controls;
using Mesen.GUI;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels
{
	public class DebuggerWindowViewModel : ViewModelBase
	{
		[Reactive] public DisassemblyViewerViewModel Disassembly { get; set; }
		[Reactive] public SnesCpuViewModel SnesCpu { get; set; }
		[Reactive] public SnesPpuViewModel SnesPpu { get; set; }
		[Reactive] public BreakpointListViewModel BreakpointList { get; set; }

		[Reactive] public DebuggerDockFactory DockFactory { get; set; }
		[Reactive] public IRootDock DockLayout { get; set; }

		public ReactiveCommand<Unit, Unit> ShowBreakpointsCommand { get; }
		public ReactiveCommand<Unit, Unit> ShowCpuStatusCommand { get; }
		public ReactiveCommand<Unit, Unit> ShowPpuStatusCommand { get; }

		public DebuggerWindowViewModel()
		{
			ShowBreakpointsCommand = ReactiveCommand.Create(ShowBreakpoints);
			ShowCpuStatusCommand = ReactiveCommand.Create(ShowCpuStatus);
			ShowPpuStatusCommand = ReactiveCommand.Create(ShowPpuStatus);

			NesCpuState state = DebugApi.GetState<NesCpuState>(CpuType.Nes);

			Disassembly = new DisassemblyViewerViewModel();
			SnesCpu = new SnesCpuViewModel();
			SnesPpu = new SnesPpuViewModel();
			BreakpointList = new BreakpointListViewModel();

			var factory = new DebuggerDockFactory(this);
			var layout = factory.CreateLayout();
			factory.InitLayout(layout);

			DockFactory = factory;
			DockLayout = layout;
		}

		private ToolDock? FindToolDock(IDock dock)
		{
			if(dock is ToolDock) {
				return (ToolDock)dock;
			}

			if(dock.VisibleDockables == null) {
				return null;
			}

			foreach(IDockable dockable in dock.VisibleDockables) {
				if(dockable is IDock) {
					ToolDock? result = FindToolDock((IDock)dockable);
					if(result != null) {
						return result;
					}
				}
			}

			return null;
		}

		private void ShowTool(Tool tool)
		{
			if((tool.Owner as IDock)?.VisibleDockables?.Contains(tool) == true) {
				return;
			}

			ToolDock? dock = FindToolDock(DockLayout);
			if(dock != null) {
				tool.Owner = dock;
				dock.VisibleDockables?.Add(tool);
			}
		}

		public void ShowBreakpoints()
		{
			ShowTool(DockFactory.BreakpointListTool);
		}

		public void ShowCpuStatus()
		{
			ShowTool(DockFactory.CpuStatusTool);
		}

		public void ShowPpuStatus()
		{
			ShowTool(DockFactory.PpuStatusTool);
		}
	}
}
