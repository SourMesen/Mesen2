using Dock.Model.Controls;
using Dock.Model.Core;
using Dock.Model.ReactiveUI.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive;

namespace Mesen.Debugger.ViewModels
{
	public class DebuggerWindowViewModel : ViewModelBase
	{
		[Reactive] public DisassemblyViewerViewModel Disassembly { get; private set; }
		[Reactive] public BreakpointListViewModel BreakpointList { get; private set; }

		[Reactive] public DebuggerDockFactory DockFactory { get; private set; }
		[Reactive] public IRootDock DockLayout { get; private set; }

		public ReactiveCommand<Unit, Unit> ShowBreakpointsCommand { get; }
		public ReactiveCommand<Unit, Unit> ShowCpuStatusCommand { get; }
		public ReactiveCommand<Unit, Unit> ShowPpuStatusCommand { get; }

		public CpuType CpuType { get; private set; }

		public DebuggerWindowViewModel()
		{
			ShowBreakpointsCommand = ReactiveCommand.Create(ShowBreakpoints);
			ShowCpuStatusCommand = ReactiveCommand.Create(ShowCpuStatus);
			ShowPpuStatusCommand = ReactiveCommand.Create(ShowPpuStatus);

			Disassembly = new DisassemblyViewerViewModel();
			BreakpointList = new BreakpointListViewModel();

			var factory = new DebuggerDockFactory(this);
			var layout = factory.CreateLayout();
			factory.InitLayout(layout);

			DockFactory = factory;
			DockLayout = layout;

			RomInfo romInfo = EmuApi.GetRomInfo();

			switch(romInfo.ConsoleType) {
				case ConsoleType.Snes:
					CpuType = CpuType.Cpu;
					DockFactory.CpuStatusTool.StatusViewModel = new SnesCpuViewModel();
					DockFactory.PpuStatusTool.StatusViewModel = new SnesPpuViewModel();
					ConfigApi.SetDebuggerFlag(DebuggerFlags.CpuDebuggerEnabled, true);
					break;

				case ConsoleType.Nes:
					CpuType = CpuType.Nes;
					DockFactory.CpuStatusTool.StatusViewModel = new NesCpuViewModel();
					//DockFactory.PpuStatusTool.StatusViewModel = new NesPpuViewModel();
					ConfigApi.SetDebuggerFlag(DebuggerFlags.NesDebuggerEnabled, true);
					break;

				case ConsoleType.Gameboy:
				case ConsoleType.GameboyColor:
					CpuType = CpuType.Gameboy;
					ConfigApi.SetDebuggerFlag(DebuggerFlags.GbDebuggerEnabled, true);
					break;
			}
		}

		internal void UpdateDisassembly()
		{
			//TODO
			Disassembly.DataProvider = new CodeDataProvider(CpuType);
			Disassembly.UpdateMaxScroll();
			Disassembly.ScrollPosition = (Disassembly.StyleProvider?.ActiveAddress ?? 0);
		}

		public void UpdateCpuState()
		{
			switch(CpuType) {
				case CpuType.Cpu:
					if(DockFactory.CpuStatusTool.StatusViewModel is SnesCpuViewModel snesModel) {
						CpuState state = DebugApi.GetState<CpuState>(CpuType);
						snesModel.UpdateState(state);
						if(Disassembly.StyleProvider != null) {
							Disassembly.StyleProvider.ActiveAddress = (state.K << 16) | state.PC;
						}
					}
					break;

				case CpuType.Nes:
					if(DockFactory.CpuStatusTool.StatusViewModel is NesCpuViewModel nesModel) {
						NesCpuState state = DebugApi.GetState<NesCpuState>(CpuType);
						nesModel.UpdateState(state);
						if(Disassembly.StyleProvider != null) {
							Disassembly.StyleProvider.ActiveAddress = state.PC;
						}
					}
					break;
			}
		}

		public void UpdatePpuState()
		{
			/*switch(CpuType) {
				case CpuType.Cpu:
					if(DockFactory.CpuStatusTool.StatusViewModel is NesCpuViewModel snesModel) {
						snesModel.State = DebugApi.GetState<CpuState>(CpuType);
					}
					break;

				case CpuType.Nes:
					if(DockFactory.CpuStatusTool.StatusViewModel is NesCpuViewModel nesModel) {
						nesModel.State = DebugApi.GetState<NesCpuState>(CpuType);
					}
					break;
			}*/
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
