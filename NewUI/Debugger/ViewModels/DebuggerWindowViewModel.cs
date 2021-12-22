using Avalonia.Controls;
using Dock.Model.Controls;
using Dock.Model.Core;
using Dock.Model.ReactiveUI.Controls;
using Mesen.Config;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive;

namespace Mesen.Debugger.ViewModels
{
	public class DebuggerWindowViewModel : ViewModelBase
	{
		[Reactive] public DebuggerConfig Config { get; private set; }

		[Reactive] public DisassemblyViewerViewModel Disassembly { get; private set; }
		[Reactive] public BreakpointListViewModel BreakpointList { get; private set; }
		[Reactive] public WatchListViewModel WatchList { get; private set; }
		[Reactive] public LabelListViewModel LabelList { get; private set; }
		[Reactive] public CallStackViewModel CallStack { get; private set; }

		[Reactive] public DebuggerDockFactory DockFactory { get; private set; }
		[Reactive] public IRootDock DockLayout { get; private set; }

		public ReactiveCommand<Unit, Unit> ShowBreakpointsCommand { get; }
		public ReactiveCommand<Unit, Unit> ShowCpuStatusCommand { get; }
		public ReactiveCommand<Unit, Unit> ShowPpuStatusCommand { get; }

		public CpuType CpuType { get; private set; }

		public DebuggerWindowViewModel()
		{
			Config = ConfigManager.Config.Debug.Debugger;

			ShowBreakpointsCommand = ReactiveCommand.Create(ShowBreakpoints);
			ShowCpuStatusCommand = ReactiveCommand.Create(ShowCpuStatus);
			ShowPpuStatusCommand = ReactiveCommand.Create(ShowPpuStatus);

			Disassembly = new DisassemblyViewerViewModel(Config);
			BreakpointList = new BreakpointListViewModel();
			
			DockFactory = new DebuggerDockFactory(this);

			if(Design.IsDesignMode) {
				return;
			}

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
					DockFactory.PpuStatusTool.StatusViewModel = new NesPpuViewModel();
					ConfigApi.SetDebuggerFlag(DebuggerFlags.NesDebuggerEnabled, true);
					break;

				case ConsoleType.Gameboy:
				case ConsoleType.GameboyColor:
					CpuType = CpuType.Gameboy;
					ConfigApi.SetDebuggerFlag(DebuggerFlags.GbDebuggerEnabled, true);
					break;
			}

			DefaultLabelHelper.SetDefaultLabels();
			LabelList = new LabelListViewModel(CpuType);

			CallStack = new CallStackViewModel(CpuType);
			WatchList = new WatchListViewModel(CpuType);

			BreakpointManager.AddCpuType(CpuType);

			DockLayout = DockFactory.CreateLayout();
			DockFactory.InitLayout(DockLayout);
		}

		internal void UpdateDisassembly()
		{
			//TODO
			Disassembly.DataProvider = new CodeDataProvider(CpuType);
			Disassembly.UpdateMaxScroll();
			Disassembly.ScrollPosition = (Disassembly.StyleProvider?.ActiveAddress ?? 0);
		}

		public void UpdateCpuPpuState()
		{
			switch(CpuType) {
				case CpuType.Cpu:
					if(DockFactory.CpuStatusTool.StatusViewModel is SnesCpuViewModel snesCpuModel) {
						CpuState state = DebugApi.GetCpuState<CpuState>(CpuType);
						snesCpuModel.UpdateState(state);
						if(Disassembly.StyleProvider != null) {
							Disassembly.StyleProvider.ActiveAddress = (state.K << 16) | state.PC;
						}
					}

					if(DockFactory.PpuStatusTool.StatusViewModel is SnesPpuViewModel snesPpuModel) {
						snesPpuModel.State = DebugApi.GetPpuState<PpuState>(CpuType);
					}
					break;

				case CpuType.Nes:
					if(DockFactory.CpuStatusTool.StatusViewModel is NesCpuViewModel nesCpuModel) {
						NesCpuState state = DebugApi.GetCpuState<NesCpuState>(CpuType);
						nesCpuModel.UpdateState(state);
						if(Disassembly.StyleProvider != null) {
							Disassembly.StyleProvider.ActiveAddress = state.PC;
						}
					}

					if(DockFactory.PpuStatusTool.StatusViewModel is NesPpuViewModel nesPpuModel) {
						nesPpuModel.State = DebugApi.GetPpuState<NesPpuState>(CpuType);
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
			ShowTool(BreakpointList);
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
