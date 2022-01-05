using Avalonia.Controls;
using Dock.Model.Controls;
using Dock.Model.Core;
using Dock.Model.ReactiveUI.Controls;
using Mesen.Config;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Reactive;

namespace Mesen.Debugger.ViewModels
{
	public class DebuggerWindowViewModel : ViewModelBase
	{
		[Reactive] public DebuggerConfig Config { get; private set; }

		[Reactive] public DebuggerOptionsViewModel Options { get; private set; }

		[Reactive] public DisassemblyViewModel Disassembly { get; private set; }
		[Reactive] public BreakpointListViewModel BreakpointList { get; private set; }
		[Reactive] public WatchListViewModel WatchList { get; private set; }
		[Reactive] public LabelListViewModel LabelList { get; private set; }
		[Reactive] public CallStackViewModel CallStack { get; private set; }

		[Reactive] public DebuggerDockFactory DockFactory { get; private set; }
		[Reactive] public IRootDock DockLayout { get; private set; }

		[Reactive] public List<object> ToolbarItems { get; private set; } = new();
		[Reactive] public List<object> DebugMenuItems { get; private set; } = new();
		[Reactive] public List<object> OptionMenuItems { get; private set; } = new();

		public CpuType CpuType { get; private set; }

		//For designer
		public DebuggerWindowViewModel() : this(null) { }

		public DebuggerWindowViewModel(CpuType? cpuType = null)
		{
			Config = ConfigManager.Config.Debug.Debugger;

			Options = new DebuggerOptionsViewModel(Config, CpuType);

			Disassembly = new DisassemblyViewModel(ConfigManager.Config.Debug);
			BreakpointList = new BreakpointListViewModel();
			
			DockFactory = new DebuggerDockFactory(this);

			if(Design.IsDesignMode) {
				CpuType = CpuType.Cpu;
			} else if(cpuType != null) {
				CpuType = cpuType.Value;
			} else {
				RomInfo romInfo = EmuApi.GetRomInfo();
				CpuType = romInfo.ConsoleType.GetMainCpuType();
			}

			switch(CpuType) {
				case CpuType.Cpu:
					DockFactory.CpuStatusTool.StatusViewModel = new SnesCpuViewModel();
					DockFactory.PpuStatusTool.StatusViewModel = new SnesPpuViewModel();
					break;

				case CpuType.Nes:
					DockFactory.CpuStatusTool.StatusViewModel = new NesCpuViewModel();
					DockFactory.PpuStatusTool.StatusViewModel = new NesPpuViewModel();
					break;

				case CpuType.Gameboy:
					break;
			}

			DefaultLabelHelper.SetDefaultLabels();
			LabelList = new LabelListViewModel(CpuType);
			CallStack = new CallStackViewModel(CpuType);
			WatchList = new WatchListViewModel(CpuType);

			DockLayout = DockFactory.CreateLayout();
			DockFactory.InitLayout(DockLayout);

			if(Design.IsDesignMode) {
				return;
			}

			BreakpointManager.AddCpuType(CpuType);
			ConfigApi.SetDebuggerFlag(CpuType.GetDebuggerFlag(), true);
		}

		internal void Cleanup()
		{
			BreakpointManager.RemoveCpuType(CpuType);
			ConfigApi.SetDebuggerFlag(CpuType.GetDebuggerFlag(), false);
		}

		internal void UpdateDisassembly()
		{
			Disassembly.DataProvider = new CodeDataProvider(CpuType);
			Disassembly.SetActiveAddress(DebugUtilities.GetProgramCounter(CpuType));
		}

		public void UpdateCpuPpuState()
		{
			switch(CpuType) {
				case CpuType.Cpu:
					if(DockFactory.CpuStatusTool.StatusViewModel is SnesCpuViewModel snesCpuModel) {
						CpuState state = DebugApi.GetCpuState<CpuState>(CpuType);
						snesCpuModel.UpdateState(state);
					}

					if(DockFactory.PpuStatusTool.StatusViewModel is SnesPpuViewModel snesPpuModel) {
						snesPpuModel.State = DebugApi.GetPpuState<PpuState>(CpuType);
					}
					break;

				case CpuType.Nes:
					if(DockFactory.CpuStatusTool.StatusViewModel is NesCpuViewModel nesCpuModel) {
						NesCpuState state = DebugApi.GetCpuState<NesCpuState>(CpuType);
						nesCpuModel.UpdateState(state);
					}

					if(DockFactory.PpuStatusTool.StatusViewModel is NesPpuViewModel nesPpuModel) {
						nesPpuModel.State = DebugApi.GetPpuState<NesPpuState>(CpuType);
					}
					break;
			}
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

		private bool IsToolVisible(Tool tool)
		{
			return (tool.Owner as IDock)?.VisibleDockables?.Contains(tool) == true;
		}

		private void ToggleTool(Tool tool)
		{
			if(IsToolVisible(tool)) {
				DockFactory.CloseDockable(tool);
			} else {
				if(DockLayout.VisibleDockables?.Count > 0 && DockLayout.VisibleDockables[0] is IDock dock) {
					DockFactory.SplitToDock(dock, new ToolDock {
						Proportion = 0.33,
						VisibleDockables = DockFactory.CreateList<IDockable>(tool)
					}, DockOperation.Bottom);
				}
			}
		}

		private void ClearActiveAddress()
		{
			if(Disassembly.StyleProvider is BaseStyleProvider p) {
				p.ActiveAddress = null;
				Disassembly.DataProvider = new CodeDataProvider(CpuType);
			}
		}

		public void InitializeMenu(Window wnd)
		{
			DebuggerConfig cfg = ConfigManager.Config.Debug.Debugger;
			
			ToolbarItems = GetDebugMenu();
			DebugMenuItems = GetDebugMenu();

			OptionMenuItems = new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.ShowSettingsPanel,
					IsSelected = () => cfg.ShowSettingsPanel,
					OnClick = () => cfg.ShowSettingsPanel = !cfg.ShowSettingsPanel
				},
				new ContextMenuAction() {
					ActionType = ActionType.ShowMemoryMappings,
					IsSelected = () => cfg.ShowMemoryMappings,
					OnClick = () => cfg.ShowMemoryMappings = !cfg.ShowMemoryMappings
				},
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.ShowWatchList,
					IsSelected = () => IsToolVisible(WatchList),
					OnClick = () => ToggleTool(WatchList)
				},
				new ContextMenuAction() {
					ActionType = ActionType.ShowBreakpointList,
					IsSelected = () => IsToolVisible(BreakpointList),
					OnClick = () => ToggleTool(BreakpointList)
				},
				new ContextMenuAction() {
					ActionType = ActionType.ShowCallStack,
					IsSelected = () => IsToolVisible(CallStack),
					OnClick = () => ToggleTool(CallStack)
				},
				new ContextMenuAction() {
					ActionType = ActionType.ShowLabelList,
					IsSelected = () => IsToolVisible(LabelList),
					OnClick = () => ToggleTool(LabelList)
				},
				new ContextMenuAction() {
					ActionType = ActionType.ShowConsoleStatus,
					IsSelected = () => IsToolVisible(DockFactory.CpuStatusTool),
					OnClick = () => ToggleTool(DockFactory.CpuStatusTool)
				},
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.Preferences,
					OnClick = () => DebuggerConfigWindow.Open(DebugConfigWindowTab.Debugger, wnd)
				},
			};

			DebugShortcutManager.RegisterActions(wnd, OptionMenuItems);
			DebugShortcutManager.RegisterActions(wnd, DebugMenuItems);
		}

		private List<object> GetDebugMenu()
		{
			Func<bool> isPaused = () => EmuApi.IsPaused();
			Func<bool> isRunning = () => !EmuApi.IsPaused();

			return new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.Continue,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Continue),
					IsEnabled = isPaused,
					OnClick = () => {
						DebugApi.ResumeExecution();
						ClearActiveAddress();
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.Break,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Break),
					IsEnabled = isRunning,
					OnClick = () => EmuApi.Pause()
				},

				new Separator(),

				new ContextMenuAction() {
					ActionType = ActionType.Reset,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Reset),
					OnClick = () => EmuApi.Reset()
				},
				new ContextMenuAction() {
					ActionType = ActionType.PowerCycle,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.PowerCycle),
					OnClick = () => EmuApi.PowerCycle()
				},

				new Separator(),

				new ContextMenuAction() {
					ActionType = ActionType.StepInto,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.StepInto),
					OnClick = () => Step(1, StepType.Step)
				},
				new ContextMenuAction() {
					ActionType = ActionType.StepOver,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.StepOver),
					OnClick = () => Step(1, StepType.StepOver)
				},
				new ContextMenuAction() {
					ActionType = ActionType.StepOut,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.StepOut),
					OnClick = () => Step(1, StepType.StepOut)
				},
				new ContextMenuAction() {
					ActionType = ActionType.StepBack,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.StepBack),
					IsEnabled = () => false,
					OnClick = () => { } //TODO
				},

				new Separator(),

				new ContextMenuAction() {
					ActionType = ActionType.RunPpuCycle,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.RunPpuCycle),
					OnClick = () => Step(1, StepType.PpuStep)
				},
				new ContextMenuAction() {
					ActionType = ActionType.RunPpuScanline,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.RunPpuScanline),
					OnClick = () => Step(1, StepType.PpuScanline)
				},
				new ContextMenuAction() {
					ActionType = ActionType.RunPpuFrame,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.RunPpuFrame),
					OnClick = () => Step(1, StepType.PpuFrame)
				},

				new Separator(),

				new ContextMenuAction() {
					ActionType = ActionType.BreakIn,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakIn),
					IsEnabled = () => false,
					OnClick = () => { } //TODO
				},
				new ContextMenuAction() {
					ActionType = ActionType.BreakOn,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakOn),
					IsEnabled = () => false,
					OnClick = () => { } //TODO
				},
			};
		}

		private void Step(int instructionCount, StepType type)
		{
			DebugApi.Step(CpuType, instructionCount, type);
			ClearActiveAddress();
		}
	}
}
