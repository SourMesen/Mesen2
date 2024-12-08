using Avalonia;
using Avalonia.Collections;
using Avalonia.Controls;
using Avalonia.Controls.Selection;
using Avalonia.Media;
using DataBoxControl;
using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class LabelListViewModel : DisposableViewModel
	{
		[Reactive] public MesenList<LabelViewModel> Labels { get; private set; } = new();
		[Reactive] public SelectionModel<LabelViewModel?> Selection { get; set; } = new() { SingleSelect = false };
		[Reactive] public SortState SortState { get; set; } = new();
		public List<int> ColumnWidths { get; } = ConfigManager.Config.Debug.Debugger.LabelListColumnWidths;

		public CpuType CpuType { get; }
		public DebuggerWindowViewModel Debugger { get; }

		[Obsolete("For designer only")]
		public LabelListViewModel() : this(CpuType.Snes, new()) { }

		public LabelListViewModel(CpuType cpuType, DebuggerWindowViewModel debugger)
		{
			CpuType = cpuType;
			Debugger = debugger;

			SortState.SetColumnSort("Label", ListSortDirection.Ascending, true);
		}

		public void Sort(object? param)
		{
			UpdateLabelList();
		}

		private Dictionary<string, Func<LabelViewModel, LabelViewModel, int>> _comparers = new() {
			{ "Label", (a, b) => string.Compare(a.Label.Label, b.Label.Label, StringComparison.OrdinalIgnoreCase) },
			{ "RelAddr", (a, b) => a.RelAddress.CompareTo(b.RelAddress) },
			{ "AbsAddr", (a, b) => a.Label.Address.CompareTo(b.Label.Address) },
			{ "Comment", (a, b) => string.Compare(a.Label.Comment, b.Label.Comment, StringComparison.OrdinalIgnoreCase) },
		};

		public void UpdateLabelList()
		{
			List<int> selectedIndexes = Selection.SelectedIndexes.ToList();

			List<LabelViewModel> sortedLabels = LabelManager.GetLabels(CpuType).Select(l => new LabelViewModel(l, CpuType)).ToList();

			SortHelper.SortList(sortedLabels, SortState.SortOrder, _comparers, "Label");

			Labels.Replace(sortedLabels);

			Selection.SelectIndexes(selectedIndexes, Labels.Count);
		}

		public void RefreshLabelList()
		{
			foreach(LabelViewModel label in Labels) {
				label.Refresh();
			}
		}

		public void InitContextMenu(Control parent)
		{
			AddDisposables(DebugShortcutManager.CreateContextMenu(parent, new object[] {
				new ContextMenuAction() {
					ActionType = ActionType.Add,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_Add),
					OnClick = () => LabelEditWindow.EditLabel(CpuType, parent, new CodeLabel())
				},

				new ContextMenuAction() {
					ActionType = ActionType.Edit,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_Edit),
					IsEnabled = () => Selection.SelectedItems.Count == 1,
					OnClick = () => {
						if(Selection.SelectedItem is LabelViewModel vm) {
							LabelEditWindow.EditLabel(CpuType, parent, vm.Label);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_Delete),
					IsEnabled = () => Selection.SelectedItems.Count > 0,
					OnClick = () => {
						IEnumerable<CodeLabel> labels = Selection.SelectedItems.Cast<LabelViewModel>().Select(vm => vm.Label).ToList();
						LabelManager.DeleteLabels(labels);
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.ToggleBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_ToggleBreakpoint),
					IsEnabled = () => Selection.SelectedItems.Count == 1,
					OnClick = () => {
						if(Selection.SelectedItem is LabelViewModel vm) {
							AddressInfo addr = vm.Label.GetAbsoluteAddress();
							BreakpointManager.ToggleBreakpoint(addr, CpuType);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.AddWatch,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_AddToWatch),
					IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is LabelViewModel vm && vm.RelAddress >= 0,
					OnClick = () => {
						if(Selection.SelectedItem is LabelViewModel vm) {
							if(vm.Label?.Label.Length > 0) {
								WatchManager.GetWatchManager(CpuType).AddWatch("[" + vm.Label.Label + "]");
							} else {
								WatchManager.GetWatchManager(CpuType).AddWatch("[" + vm.RelAddressDisplay + "]");
							}
						}
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.FindOccurrences,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_FindOccurrences),
					IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is LabelViewModel vm && vm.Label.GetRelativeAddress(CpuType).Address >= 0,
					OnClick = () => {
						if(Selection.SelectedItem is LabelViewModel vm) {
							DisassemblySearchOptions options = new() { MatchCase = true, MatchWholeWord = true };
							Debugger.FindAllOccurrences(vm.Label.Label, options);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.GoToLocation,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_GoToLocation),
					IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is LabelViewModel vm && vm.Label.GetRelativeAddress(CpuType).Address >= 0,
					OnClick = () => {
						if(Selection.SelectedItem is LabelViewModel vm) {
							AddressInfo addr = vm.Label.GetRelativeAddress(CpuType);
							if(addr.Address >= 0) {
								Debugger.ScrollToAddress(addr.Address);
							}
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_ViewInMemoryViewer),
					IsEnabled = () => Selection.SelectedItems.Count == 1,
					OnClick = () => {
						if(Selection.SelectedItem is LabelViewModel vm) {
							AddressInfo addr = vm.Label.GetRelativeAddress(CpuType);
							if(addr.Address < 0) {
								addr = vm.Label.GetAbsoluteAddress();
							}
							MemoryToolsWindow.ShowInMemoryTools(addr.Type, addr.Address);
						}
					}
				},
			}));
		}
	}

	public class LabelViewModel : INotifyPropertyChanged
	{
		private string _format;
		private bool _isUnmappedType;

		public CodeLabel Label { get; set; }
		public CpuType CpuType { get; }
		public string AbsAddressDisplay { get; }

		public string LabelText { get; private set; }
		public string LabelComment { get; private set; }
		public int RelAddress { get; private set; }
		public string RelAddressDisplay => RelAddress >= 0 ? ("$" + RelAddress.ToString(_format)) : (_isUnmappedType ? "" : "<unavailable>");
		public object RowBrush => RelAddress >= 0 || _isUnmappedType ? AvaloniaProperty.UnsetValue : Brushes.Gray;
		public FontStyle RowStyle => RelAddress >= 0 || _isUnmappedType ? FontStyle.Normal : FontStyle.Italic;

		public event PropertyChangedEventHandler? PropertyChanged;

		public void Refresh()
		{
			int addr = Label.GetRelativeAddress(CpuType).Address;
			if(addr != RelAddress) {
				RelAddress = addr;
				PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(RowBrush)));
				PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(RowStyle)));
				PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(RelAddressDisplay)));
			}
		}

		public LabelViewModel(CodeLabel label, CpuType cpuType)
		{
			Label = label;
			LabelText = label.Label;
			LabelComment = label.Comment;
			CpuType = cpuType;
			RelAddress = Label.GetRelativeAddress(CpuType).Address;
			_format = "X" + cpuType.GetAddressSize();
			_isUnmappedType = Label.MemoryType.IsUnmapped();

			if(Label.MemoryType.IsRelativeMemory()) {
				AbsAddressDisplay = "";
			} else {
				AbsAddressDisplay = "$" + label.Address.ToString(label.MemoryType.GetFormatString()) + " [" + label.MemoryType.GetShortName() + "]";
			}
		}
	}
}
