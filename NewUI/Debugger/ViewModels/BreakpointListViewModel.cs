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
using Mesen.ViewModels;
using System.ComponentModel;
using Mesen.Utilities;

namespace Mesen.Debugger.ViewModels
{
	public class BreakpointListViewModel : ViewModelBase
	{
		[Reactive] public SwappableList<BreakpointViewModel> Breakpoints { get; private set; } = new();
		[Reactive] public int SelectedIndex { get; set; } = -1;

		public CpuType CpuType { get; }
		public DisassemblyViewModel Disassembly { get; }

		[Obsolete("For designer only")]
		public BreakpointListViewModel() : this(CpuType.Snes, new()) { }

		public BreakpointListViewModel(CpuType cpuType, DisassemblyViewModel disassembly)
		{
			CpuType = cpuType;
			Disassembly = disassembly;
			UpdateBreakpoints();
		}

		public void UpdateBreakpoints()
		{
			int selection = SelectedIndex;
			Breakpoints.Swap(BreakpointManager.GetBreakpoints(CpuType).Select(bp => new BreakpointViewModel(bp)));
			if(selection >= 0) {
				if(selection < Breakpoints.Count) {
					SelectedIndex = selection;
				} else {
					SelectedIndex = Breakpoints.Count - 1;
				}
			}
		}

		public void RefreshBreakpointList()
		{
			foreach(BreakpointViewModel bp in Breakpoints) {
				bp.Refresh();
			}
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
						if(grid.SelectedItem is BreakpointViewModel vm) {
							BreakpointEditWindow.EditBreakpoint(vm.Breakpoint, parent);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_Delete),
					IsEnabled = () => grid.SelectedItems.Count > 0,
					OnClick = () => {
						foreach(object item in grid.SelectedItems.Cast<object>().ToList()) {
							if(item is BreakpointViewModel vm) {
								BreakpointManager.RemoveBreakpoint(vm.Breakpoint);
							}
						}
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.GoToLocation,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_GoToLocation),
					IsEnabled = () => grid.SelectedItem is BreakpointViewModel vm && vm.Breakpoint.IsCpuBreakpoint && vm.Breakpoint.GetRelativeAddress() >= 0,
					OnClick = () => {
						if(grid.SelectedItem is BreakpointViewModel vm && vm.Breakpoint.IsCpuBreakpoint) {
							int addr = vm.Breakpoint.GetRelativeAddress();
							if(addr >= 0) {
								Disassembly.ScrollToAddress((uint)addr);
							}
						}
					}
				}
			});
		}

		public class BreakpointViewModel : INotifyPropertyChanged
		{
			public Breakpoint Breakpoint { get; set; }
			public string TypeDisplay => Breakpoint.ToReadableType();
			public string AddressDisplay => Breakpoint.GetAddressString(true);

			public event PropertyChangedEventHandler? PropertyChanged;

			public void Refresh()
			{
				PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(TypeDisplay)));
				PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(AddressDisplay)));
			}

			public BreakpointViewModel(Breakpoint bp)
			{
				Breakpoint = bp;
			}
		}
	}
}
