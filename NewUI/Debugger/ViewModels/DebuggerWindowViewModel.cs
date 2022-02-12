using Avalonia.Controls;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using Dock.Model.Controls;
using Dock.Model.Core;
using Dock.Model.ReactiveUI.Controls;
using Mesen.Config;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Integration;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Reactive;

namespace Mesen.Debugger.ViewModels
{
	public class DebuggerWindowViewModel : DisposableViewModel
	{
		[Reactive] public string Title { get; private set; } = "Debugger";
		[Reactive] public WindowIcon? Icon { get; private set; } = null;
		[Reactive] public bool IsMainCpuDebugger { get; private set; } = true;

		[Reactive] public DebuggerConfig Config { get; private set; }

		[Reactive] public DebuggerOptionsViewModel Options { get; private set; }

		[Reactive] public DisassemblyViewModel Disassembly { get; private set; }
		[Reactive] public BreakpointListViewModel BreakpointList { get; private set; }
		[Reactive] public WatchListViewModel WatchList { get; private set; }
		[Reactive] public BaseConsoleStatusViewModel? ConsoleStatus { get; private set; }
		[Reactive] public LabelListViewModel LabelList { get; private set; }
		[Reactive] public CallStackViewModel CallStack { get; private set; }
		[Reactive] public SourceViewViewModel? SourceView { get; private set; }
		[Reactive] public MemoryMappingViewModel? MemoryMappings { get; private set; }

		[Reactive] public DebuggerDockFactory DockFactory { get; private set; }
		[Reactive] public IRootDock DockLayout { get; private set; }

		[Reactive] public string BreakReason { get; private set; } = "";
		[Reactive] public string BreakElapsedCycles { get; private set; } = "";

		[Reactive] public List<ContextMenuAction> ToolbarItems { get; private set; } = new();
		
		[Reactive] public List<ContextMenuAction> FileMenuItems { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> DebugMenuItems { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> SearchMenuItems { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> OptionMenuItems { get; private set; } = new();

		public CpuType CpuType { get; private set; }
		private UInt64 _masterClock = 0;

		[Obsolete("For designer only")]
		public DebuggerWindowViewModel() : this(null) { }

		public DebuggerWindowViewModel(CpuType? cpuType)
		{
			if(!Design.IsDesignMode) {
				DebugApi.InitializeDebugger();
			}

			if(Design.IsDesignMode) {
				CpuType = CpuType.Snes;
			} else {
				RomInfo romInfo = EmuApi.GetRomInfo();
				if(cpuType != null) {
					CpuType = cpuType.Value;
					if(romInfo.ConsoleType.GetMainCpuType() != CpuType) {
						Title = ResourceHelper.GetEnumText(CpuType) + " Debugger";
						Icon = new WindowIcon(ImageUtilities.BitmapFromAsset("Assets/" + CpuType.ToString() + "Debugger.png"));
						IsMainCpuDebugger = false;
					} else {
						Icon = new WindowIcon(ImageUtilities.BitmapFromAsset("Assets/Debugger.png"));
					}
				} else {
					CpuType = romInfo.ConsoleType.GetMainCpuType();
					Icon = new WindowIcon(ImageUtilities.BitmapFromAsset("Assets/Debugger.png"));

					//TODO temporary - try to load DBG file if it exists
					string dbgPath = Path.ChangeExtension(romInfo.RomPath, ".dbg");
					if(File.Exists(dbgPath)) {
						SourceView = new(DbgImporter.Import(CpuType, romInfo.Format, dbgPath, true, true), CpuType);
					}
				}
			}

			Config = ConfigManager.Config.Debug.Debugger;

			Options = new DebuggerOptionsViewModel(Config, CpuType);
			Disassembly = new DisassemblyViewModel(ConfigManager.Config.Debug, CpuType);
			BreakpointList = new BreakpointListViewModel(CpuType, Disassembly);
			LabelList = new LabelListViewModel(CpuType, Disassembly);
			CallStack = new CallStackViewModel(CpuType, Disassembly);
			WatchList = new WatchListViewModel(CpuType);
			ConsoleStatus = CpuType switch {
				CpuType.Snes => new SnesStatusViewModel(),
				CpuType.Nes => new NesStatusViewModel(),
				_ => null
			};

			DockFactory = new DebuggerDockFactory();

			DockFactory.BreakpointListTool.Model = BreakpointList;
			DockFactory.LabelListTool.Model = LabelList;
			DockFactory.CallStackTool.Model = CallStack;
			DockFactory.WatchListTool.Model = WatchList;
			DockFactory.DisassemblyTool.Model = Disassembly;
			DockFactory.SourceViewTool.Model = SourceView;
			DockFactory.StatusTool.Model = ConsoleStatus;
			
			DockLayout = DockFactory.CreateLayout();
			DockFactory.InitLayout(DockLayout);

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, Config_PropertyChanged));

			if(CpuType == CpuType.Nes || CpuType == CpuType.Gameboy) {
				MemoryMappings = new MemoryMappingViewModel(CpuType);
			}

			WatchList.Manager.WatchChanged += Manager_WatchChanged;
			LabelManager.OnLabelUpdated += LabelManager_OnLabelUpdated;
			BreakpointManager.BreakpointsChanged += BreakpointManager_BreakpointsChanged;
			BreakpointManager.AddCpuType(CpuType);
			ConfigApi.SetDebuggerFlag(CpuType.GetDebuggerFlag(), true);
		}

		protected override void DisposeView()
		{
			WatchList.Manager.WatchChanged -= Manager_WatchChanged;
			LabelManager.OnLabelUpdated -= LabelManager_OnLabelUpdated;
			BreakpointManager.BreakpointsChanged -= BreakpointManager_BreakpointsChanged;
			BreakpointManager.RemoveCpuType(CpuType);
			ConfigApi.SetDebuggerFlag(CpuType.GetDebuggerFlag(), false);
		}

		private void Config_PropertyChanged(object? sender, PropertyChangedEventArgs e)
		{
			Config.ApplyConfig();
			UpdateDebugger();
		}

		public void UpdateConsoleState()
		{
			ConsoleStatus?.UpdateConsoleState();
		}

		private void Manager_WatchChanged(object? sender, EventArgs e)
		{
			WatchList.UpdateWatch();
		}

		private void LabelManager_OnLabelUpdated(object? sender, EventArgs e)
		{
			LabelList.UpdateLabelList();
			BreakpointList.RefreshBreakpointList();
			CallStack.RefreshCallStack();
			Disassembly.Refresh();
		}

		private void BreakpointManager_BreakpointsChanged(object? sender, EventArgs e)
		{
			Disassembly.InvalidateVisual();
			BreakpointList.UpdateBreakpoints();
		}

		public void UpdateDebugger(bool forBreak = false, BreakEvent? evt = null)
		{
			if(forBreak) {
				if(ConsoleStatus != null) {
					ConsoleStatus.EditAllowed = true;
				}
				UpdateStatusBar(evt);
			}

			ConsoleStatus?.UpdateUiState();
			UpdateDisassembly(forBreak);
			SourceView?.Refresh(Disassembly.ActiveAddress);
			MemoryMappings?.Refresh();
			BreakpointList.RefreshBreakpointList();
			LabelList.RefreshLabelList();
			WatchList.UpdateWatch();
			CallStack.UpdateCallStack();
		}

		private void UpdateStatusBar(BreakEvent? evt)
		{
			UInt64 prevMasterClock = _masterClock;
			_masterClock = EmuApi.GetTimingInfo().MasterClock;
			if(prevMasterClock > 0) {
				BreakElapsedCycles = $"{_masterClock - prevMasterClock} cycles elapsed";
			}

			if(evt != null) {
				string breakReason = "";
				BreakEvent brkEvent = evt.Value;
				if(brkEvent.Source != BreakSource.Unspecified) {
					breakReason = ResourceHelper.GetEnumText(brkEvent.Source);
					if(brkEvent.Source == BreakSource.Breakpoint) {
						breakReason += (
							": " +
							ResourceHelper.GetEnumText(brkEvent.Operation.Type) + " " +
							brkEvent.Operation.MemType.GetShortName() +
							" ($" + brkEvent.Operation.Address.ToString("X4") +
							":$" + brkEvent.Operation.Value.ToString("X2") + ")"
						);
					}
				}
				BreakReason = breakReason;
			} else {
				BreakReason = "";
			}
		}

		private void UpdateDisassembly(bool scrollToActiveAddress)
		{
			if(scrollToActiveAddress) {
				//Scroll to the active address and highlight it
				Disassembly.SetActiveAddress((int)DebugUtilities.GetProgramCounter(CpuType));
				if(!EmuApi.IsPaused()) {
					//Clear the highlight if the emulation is still running
					Disassembly.SetActiveAddress(null);
				}
			}
			Disassembly.Refresh();
		}

		public void ProcessResumeEvent()
		{
			if(ConsoleStatus != null) {
				ConsoleStatus.EditAllowed = false;
			}
			Disassembly.SetActiveAddress(null);
			Disassembly.Refresh();
			SourceView?.Refresh(null);
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

		public void InitializeMenu(Window wnd)
		{
			DebuggerConfig cfg = ConfigManager.Config.Debug.Debugger;
			
			ToolbarItems = GetDebugMenu(wnd, true);
			DebugMenuItems = GetDebugMenu(wnd, false);

			FileMenuItems = new List<ContextMenuAction>() {
				SaveRomActionHelper.GetSaveRomAction(wnd),
				SaveRomActionHelper.GetSaveEditsAsIpsAction(wnd),
				new ContextMenuSeparator(),
				GetCdlActionMenu(wnd),
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd.Close()
				}
			};

			OptionMenuItems = new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.ShowSettingsPanel,
					IsSelected = () => cfg.ShowSettingsPanel,
					OnClick = () => cfg.ShowSettingsPanel = !cfg.ShowSettingsPanel
				},
				new ContextMenuAction() {
					ActionType = ActionType.ShowMemoryMappings,
					IsVisible = () => MemoryMappings != null,
					IsSelected = () => cfg.ShowMemoryMappings,
					OnClick = () => cfg.ShowMemoryMappings = !cfg.ShowMemoryMappings
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.ShowWatchList,
					IsSelected = () => IsToolVisible(DockFactory.WatchListTool),
					OnClick = () => ToggleTool(DockFactory.WatchListTool)
				},
				new ContextMenuAction() {
					ActionType = ActionType.ShowBreakpointList,
					IsSelected = () => IsToolVisible(DockFactory.BreakpointListTool),
					OnClick = () => ToggleTool(DockFactory.BreakpointListTool)
				},
				new ContextMenuAction() {
					ActionType = ActionType.ShowCallStack,
					IsSelected = () => IsToolVisible(DockFactory.CallStackTool),
					OnClick = () => ToggleTool(DockFactory.CallStackTool)
				},
				new ContextMenuAction() {
					ActionType = ActionType.ShowLabelList,
					IsSelected = () => IsToolVisible(DockFactory.LabelListTool),
					OnClick = () => ToggleTool(DockFactory.LabelListTool)
				},
				new ContextMenuAction() {
					ActionType = ActionType.ShowConsoleStatus,
					IsSelected = () => IsToolVisible(DockFactory.StatusTool),
					OnClick = () => ToggleTool(DockFactory.StatusTool)
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.OpenDebugSettings,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenDebugSettings),
					OnClick = () => DebuggerConfigWindow.Open(DebugConfigWindowTab.Debugger, wnd)
				},
			};

			SearchMenuItems = new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.GoToAddress,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.GoTo),
					OnClick = async () => {
						int? address = await new GoToWindow(DebugApi.GetMemorySize(CpuType.ToMemoryType()) - 1).ShowCenteredDialog<int?>(wnd);
						if(address != null) {
							Disassembly.ScrollToAddress((uint)address.Value);
							DockFactory.SetActiveDockable(DockFactory.DisassemblyTool);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.GoToAll,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.GoToAll),
					IsEnabled = () => false,
					OnClick = () => { }
				}
			};

			Dispatcher.UIThread.Post(() => {
				DebugShortcutManager.RegisterActions(wnd, FileMenuItems);
				DebugShortcutManager.RegisterActions(wnd, DebugMenuItems);
				DebugShortcutManager.RegisterActions(wnd, SearchMenuItems);
				DebugShortcutManager.RegisterActions(wnd, OptionMenuItems);
			});
		}

		private List<ContextMenuAction> GetDebugMenu(Control wnd, bool forToolbar)
		{
			Func<bool> isPaused = () => EmuApi.IsPaused();
			Func<bool> isRunning = () => !EmuApi.IsPaused();

			List<ContextMenuAction> debugMenu = new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.Continue,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Continue),
					IsEnabled = isPaused,
					OnClick = () => {
						DebugApi.ResumeExecution();
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.Break,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Break),
					IsEnabled = isRunning,
					OnClick = () => EmuApi.Pause()
				},
			};

			if(!forToolbar) {
				debugMenu.AddRange(new List<ContextMenuAction> {
					new ContextMenuSeparator(),

					new ContextMenuAction() {
						ActionType = ActionType.Reset,
						Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Reset),
						OnClick = () => EmuApi.Reset()
					},
					new ContextMenuAction() {
						ActionType = ActionType.PowerCycle,
						Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.PowerCycle),
						OnClick = () => EmuApi.PowerCycle()
					}
				});
			};

			debugMenu.AddRange(new List<ContextMenuAction> {
				new ContextMenuSeparator(),

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

				new ContextMenuSeparator(),

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

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.BreakIn,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakIn),
					OnClick = () => new BreakInWindow(CpuType).ShowCenteredDialog(wnd)
				},
				new ContextMenuAction() {
					ActionType = ActionType.BreakOn,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakOn),
					OnClick = () => new BreakOnWindow(CpuType).ShowCenteredDialog(wnd)
				},
			});

			return debugMenu;
		}

		private ContextMenuAction GetCdlActionMenu(Window wnd)
		{
			return new ContextMenuAction() {
				ActionType = ActionType.CodeDataLogger,
				IsEnabled = () => CpuType.ToMemoryType().SupportsCdl(),
				SubActions = new() {
					new ContextMenuAction() {
						ActionType = ActionType.ResetCdl,
						Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ResetCdl),
						OnClick = () => {
							DebugApi.ResetCdl(CpuType);
							Disassembly.Refresh();
						}
					},
					new ContextMenuSeparator(),
					new ContextMenuAction() {
						ActionType = ActionType.LoadCdl,
						Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LoadCdl),
						OnClick = async () => {
							string? filename = await FileDialogHelper.OpenFile(ConfigManager.DebuggerFolder, wnd, FileDialogHelper.CdlExt);
							if(filename != null) {
								byte[] data = File.ReadAllBytes(filename);
								DebugApi.SetCdlData(CpuType, data, data.Length);
								Disassembly.Refresh();
							}
						}
					},
					new ContextMenuAction() {
						ActionType = ActionType.SaveCdl,
						Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.SaveCdl),
						OnClick = async () => {
							string? filename = await FileDialogHelper.SaveFile(ConfigManager.DebuggerFolder, EmuApi.GetRomInfo().GetRomName() + ".cdl", wnd, FileDialogHelper.CdlExt);
							if(filename != null) {
								CdlFlags[] data = DebugApi.GetCdlData(0, (uint)DebugApi.GetMemorySize(CpuType.GetPrgRomMemoryType()), CpuType.GetPrgRomMemoryType());
								File.WriteAllBytes(filename, data.Cast<byte>().ToArray());
							}
						}
					},
				}
			};
		}

		private void Step(int instructionCount, StepType type)
		{
			switch(type) {
				case StepType.PpuStep:
				case StepType.PpuScanline:
				case StepType.PpuFrame:
					DebugApi.Step(CpuType.GetConsoleType().GetMainCpuType(), instructionCount, type);
					break;

				default:
					DebugApi.Step(CpuType, instructionCount, type);
					break;
			}
		}
	}
}
