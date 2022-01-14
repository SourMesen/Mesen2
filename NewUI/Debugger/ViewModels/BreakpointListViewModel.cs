using Dock.Model.ReactiveUI.Controls;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Reactive.Linq;
using System.Linq;
using Mesen.Interop;
using Mesen.Debugger.Utilities;
using Mesen.Config;
using Mesen.Debugger.Windows;
using Avalonia.Controls;
using System.Collections;
using Mesen.ViewModels;

namespace Mesen.Debugger.ViewModels
{
	public class BreakpointListViewModel : ViewModelBase
	{
		[Reactive] public List<Breakpoint> Breakpoints { get; private set; } = new List<Breakpoint>();
		public CpuType CpuType { get; }
		public DisassemblyViewModel Disassembly { get; }

		[Obsolete("For designer only")]
		public BreakpointListViewModel() : this(CpuType.Cpu, new()) { }

		public BreakpointListViewModel(CpuType cpuType, DisassemblyViewModel disassembly)
		{
			CpuType = cpuType;
			Disassembly = disassembly;
		}

		public void UpdateBreakpoints()
		{
			Breakpoints = new List<Breakpoint>(BreakpointManager.Breakpoints);
		}

		public void InitContextMenu(Control parent, DataGrid grid)
		{
			DebugShortcutManager.CreateContextMenu(parent, new object[] {
				new ContextMenuAction() {
					ActionType = ActionType.Add,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_Add),
					OnClick = () => {
						Breakpoint bp = new Breakpoint() { BreakOnRead = true, BreakOnWrite = true, BreakOnExec = true, CpuType = CpuType };
						BreakpointEditWindow.EditBreakpoint(bp, parent);
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Edit,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_Edit),
					IsEnabled = () => grid.SelectedItem is Breakpoint,
					OnClick = () => {
						if(grid.SelectedItem is Breakpoint bp) {
							BreakpointEditWindow.EditBreakpoint(bp, parent);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_Delete),
					IsEnabled = () => grid.SelectedItems.Count > 0,
					OnClick = () => {
						foreach(object item in grid.SelectedItems.Cast<object>().ToList()) {
							if(item is Breakpoint bp) {
								BreakpointManager.RemoveBreakpoint(bp);
							}
						}
					}
				},

				new Separator(),

				new ContextMenuAction() {
					ActionType = ActionType.GoToLocation,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_GoToLocation),
					IsEnabled = () => grid.SelectedItem is Breakpoint bp && bp.IsCpuBreakpoint && bp.GetRelativeAddress() >= 0,
					OnClick = () => {
						if(grid.SelectedItem is Breakpoint bp && bp.IsCpuBreakpoint) {
							int addr = bp.GetRelativeAddress();
							if(addr >= 0) {
								Disassembly.ScrollToAddress((uint)addr);
							}
						}
					}
				}
			});
		}
	}
}
