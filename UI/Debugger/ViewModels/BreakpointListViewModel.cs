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
using System.Collections;
using DataBoxControl;
using Avalonia.Collections;
using Avalonia.Controls.Selection;

namespace Mesen.Debugger.ViewModels
{
	public class BreakpointListViewModel : DisposableViewModel
	{
		[Reactive] public MesenList<BreakpointViewModel> Breakpoints { get; private set; } = new();
		[Reactive] public SelectionModel<BreakpointViewModel?> Selection { get; set; } = new() { SingleSelect = false };
		[Reactive] public SortState SortState { get; set; } = new();
		public List<int> ColumnWidths { get; } = ConfigManager.Config.Debug.Debugger.BreakpointListColumnWidths;

		public CpuType CpuType { get; }
		public DebuggerWindowViewModel Debugger { get; }

		[Obsolete("For designer only")]
		public BreakpointListViewModel() : this(CpuType.Snes, new()) { }

		public BreakpointListViewModel(CpuType cpuType, DebuggerWindowViewModel debugger)
		{
			CpuType = cpuType;
			Debugger = debugger;
		}

		public void Sort(object? param)
		{
			UpdateBreakpoints();
		}

		private Dictionary<string, Func<BreakpointViewModel, BreakpointViewModel, int>> _comparers = new() {
			{ "Enabled", (a, b) => a.Breakpoint.Enabled.CompareTo(b.Breakpoint.Enabled) },
			{ "Marked", (a, b) => a.Breakpoint.MarkEvent.CompareTo(b.Breakpoint.MarkEvent) },
			{ "Type", (a, b) => string.Compare(a.Breakpoint.ToReadableType(), b.Breakpoint.ToReadableType(), StringComparison.OrdinalIgnoreCase) },
			{ "Address", (a, b) => string.Compare(a.Breakpoint.GetAddressString(true), b.Breakpoint.GetAddressString(true), StringComparison.OrdinalIgnoreCase) },
			{ "Condition", (a, b) => string.Compare(a.Breakpoint.Condition, b.Breakpoint.Condition, StringComparison.OrdinalIgnoreCase) },
		};

		public void UpdateBreakpoints()
		{
			List<int> selectedIndexes = Selection.SelectedIndexes.ToList();

			List<BreakpointViewModel> sortedBreakpoints = BreakpointManager.GetBreakpoints(CpuType).Select(bp => new BreakpointViewModel(bp)).ToList();
			
			if(SortState.SortOrder.Count > 0) {
				SortHelper.SortList(sortedBreakpoints, SortState.SortOrder, _comparers, "Address");
			}

			Breakpoints.Replace(sortedBreakpoints);
			Selection.SelectIndexes(selectedIndexes, Breakpoints.Count);
		}

		public void RefreshBreakpointList()
		{
			foreach(BreakpointViewModel bp in Breakpoints) {
				bp.Refresh();
			}
		}

		public void InitContextMenu(Control parent)
		{
			AddDisposables(DebugShortcutManager.CreateContextMenu(parent, new object[] {
				new ContextMenuAction() {
					ActionType = ActionType.AddBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_Add),
					OnClick = () => {
						Breakpoint bp = new Breakpoint() { BreakOnRead = true, BreakOnWrite = true, BreakOnExec = true, CpuType = CpuType };
						BreakpointEditWindow.EditBreakpoint(bp, parent);
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.AddForbidBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_AddForbid),
					OnClick = () => {
						Breakpoint bp = new Breakpoint() { Forbid = true, CpuType = CpuType };
						BreakpointEditWindow.EditBreakpoint(bp, parent);
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.Edit,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_Edit),
					IsEnabled = () => Selection.SelectedItems.Count == 1,
					OnClick = () => {
						if(Selection.SelectedItem is BreakpointViewModel vm) {
							BreakpointEditWindow.EditBreakpoint(vm.Breakpoint, parent);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_Delete),
					IsEnabled = () => Selection.SelectedItems.Count > 0,
					OnClick = () => {
						List<Breakpoint> selectedBps = Selection.SelectedItems.Cast<BreakpointViewModel>().Select(vm => vm.Breakpoint).ToList();
						BreakpointManager.RemoveBreakpoints(selectedBps);
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.EnableBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_EnableBreakpoint),
					IsEnabled = () => Selection.SelectedItems.Any(bp => bp?.Breakpoint.Enabled == false),
					OnClick = () => {
						List<Breakpoint> selectedBps = Selection.SelectedItems.Cast<BreakpointViewModel>().Select(vm => vm.Breakpoint).ToList();
						foreach(Breakpoint bp in selectedBps) {
							bp.Enabled = true;
						}
						BreakpointManager.RefreshBreakpoints();
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.DisableBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_DisableBreakpoint),
					IsEnabled = () => Selection.SelectedItems.Any(bp => bp?.Breakpoint.Enabled == true),
					OnClick = () => {
						List<Breakpoint> selectedBps = Selection.SelectedItems.Cast<BreakpointViewModel>().Select(vm => vm.Breakpoint).ToList();
						foreach(Breakpoint bp in selectedBps) {
							bp.Enabled = false;
						}
						BreakpointManager.RefreshBreakpoints();
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.GoToLocation,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_GoToLocation),
					IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is BreakpointViewModel vm && vm.Breakpoint.SupportsExec && vm.Breakpoint.GetRelativeAddress() >= 0,
					OnClick = () => {
						if(Selection.SelectedItem is BreakpointViewModel vm && vm.Breakpoint.SupportsExec) {
							int addr = vm.Breakpoint.GetRelativeAddress();
							if(addr >= 0) {
								Debugger.ScrollToAddress(addr);
							}
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakpointList_ViewInMemoryViewer),
					IsEnabled = () => Selection.SelectedItems.Count == 1,
					OnClick = () => {
						if(Selection.SelectedItem is BreakpointViewModel vm) {
							MemoryToolsWindow.ShowInMemoryTools(vm.Breakpoint.MemoryType, (int)vm.Breakpoint.StartAddress);
						}
					}
				}
			}));
		}
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
