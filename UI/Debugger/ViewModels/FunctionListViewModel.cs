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
using System.Text;

namespace Mesen.Debugger.ViewModels
{
	public class FunctionListViewModel : DisposableViewModel
	{
		[Reactive] public MesenList<FunctionViewModel> Functions { get; private set; } = new();
		[Reactive] public SelectionModel<FunctionViewModel?> Selection { get; set; } = new() { SingleSelect = false };
		[Reactive] public SortState SortState { get; set; } = new();
		public List<int> ColumnWidths { get; } = ConfigManager.Config.Debug.Debugger.FunctionListColumnWidths;

		public CpuType CpuType { get; }
		public DebuggerWindowViewModel Debugger { get; }

		[Obsolete("For designer only")]
		public FunctionListViewModel() : this(CpuType.Snes, new()) { }

		public FunctionListViewModel(CpuType cpuType, DebuggerWindowViewModel debugger)
		{
			CpuType = cpuType;
			Debugger = debugger;

			SortState.SetColumnSort("AbsAddr", ListSortDirection.Ascending, true);
		}

		public void Sort(object? param)
		{
			UpdateFunctionList();
		}

		private Dictionary<string, Func<FunctionViewModel, FunctionViewModel, int>> _comparers = new() {
			{ "Function", (a, b) => string.Compare(a.LabelName, b.LabelName, StringComparison.OrdinalIgnoreCase) },
			{ "RelAddr", (a, b) => a.RelAddress.CompareTo(b.RelAddress) },
			{ "AbsAddr", (a, b) => a.AbsAddress.CompareTo(b.AbsAddress) },
		};

		public void UpdateFunctionList()
		{
			List<int> selectedIndexes = Selection.SelectedIndexes.ToList();

			MemoryType prgMemType = CpuType.GetPrgRomMemoryType();
			List<FunctionViewModel> sortedFunctions = DebugApi.GetCdlFunctions(CpuType.GetPrgRomMemoryType()).Select(f => new FunctionViewModel(new AddressInfo() { Address = (int)f, Type = prgMemType }, CpuType)).ToList();

			SortHelper.SortList(sortedFunctions, SortState.SortOrder, _comparers, "AbsAddr");

			Functions.Replace(sortedFunctions);
			Selection.SelectIndexes(selectedIndexes, Functions.Count);
		}

		public void InitContextMenu(Control parent)
		{
			AddDisposables(DebugShortcutManager.CreateContextMenu(parent, new object[] {
				new ContextMenuAction() {
					ActionType = ActionType.EditLabel,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FunctionList_EditLabel),
					IsEnabled = () => Selection.SelectedItems.Count == 1,
					OnClick = () => {
						if(Selection.SelectedItem is FunctionViewModel vm) {
							LabelEditWindow.EditLabel(CpuType, parent, vm.Label ?? new CodeLabel(vm.FuncAddr));
						}
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.ToggleBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FunctionList_ToggleBreakpoint),
					IsEnabled = () => Selection.SelectedItems.Count == 1,
					OnClick = () => {
						if(Selection.SelectedItem is FunctionViewModel vm) {
							BreakpointManager.ToggleBreakpoint(vm.FuncAddr, CpuType);
						}
					}
				},

				new ContextMenuSeparator(),
		
				new ContextMenuAction() {
					ActionType = ActionType.FindOccurrences,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FunctionList_FindOccurrences),
					IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is FunctionViewModel vm && vm.RelAddress >= 0,
					OnClick = () => {
						if(Selection.SelectedItem is FunctionViewModel vm && vm.RelAddress >= 0) {
							DisassemblySearchOptions options = new() { MatchCase = true, MatchWholeWord = true };
							Debugger.FindAllOccurrences(vm.Label?.Label ?? vm.RelAddressDisplay, options);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.GoToLocation,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FunctionList_GoToLocation),
					IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is FunctionViewModel vm && vm.RelAddress >= 0,
					OnClick = () => {
						if(Selection.SelectedItem is FunctionViewModel vm) {
							if(vm.RelAddress >= 0) {
								Debugger.ScrollToAddress(vm.RelAddress);
							}
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FunctionList_ViewInMemoryViewer),
					IsEnabled = () => Selection.SelectedItems.Count == 1,
					OnClick = () => {
						if(Selection.SelectedItem is FunctionViewModel vm) {
							AddressInfo addr = new AddressInfo() { Address = vm.RelAddress, Type = CpuType.ToMemoryType() };
							if(addr.Address < 0) {
								addr = vm.FuncAddr;
							}
							MemoryToolsWindow.ShowInMemoryTools(addr.Type, addr.Address);
						}
					}
				},
			}));
		}
	}

	public class FunctionViewModel : INotifyPropertyChanged
	{
		private string _format;

		public AddressInfo FuncAddr { get; private set; }
		public CpuType _cpuType;
			
		public string AbsAddressDisplay { get; }
		public int AbsAddress => FuncAddr.Address;
		public int RelAddress { get; private set; }
		public string RelAddressDisplay => RelAddress >= 0 ? ("$" + RelAddress.ToString(_format)) : "<unavailable>";
		public object RowBrush => RelAddress >= 0 ? AvaloniaProperty.UnsetValue : Brushes.Gray;
		public FontStyle RowStyle => RelAddress >= 0 ? FontStyle.Normal : FontStyle.Italic;

		public CodeLabel? Label => LabelManager.GetLabel(FuncAddr);
		public string LabelName => Label?.Label ?? "<no label>";

		public event PropertyChangedEventHandler? PropertyChanged;

		public void Refresh()
		{
			int addr = DebugApi.GetRelativeAddress(FuncAddr, _cpuType).Address;
			if(addr != RelAddress) {
				RelAddress = addr;
				PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(RowBrush)));
				PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(RowStyle)));
				PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(RelAddressDisplay)));
			}

			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(LabelName)));
		}

		public FunctionViewModel(AddressInfo funcAddr, CpuType cpuType)
		{
			FuncAddr = funcAddr;
			_cpuType = cpuType;
			RelAddress = DebugApi.GetRelativeAddress(FuncAddr, _cpuType).Address;
			_format = "X" + cpuType.GetAddressSize();

			AbsAddressDisplay = "$" + FuncAddr.Address.ToString(_format);
		}
	}
}
