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
using System.Collections;
using DataBoxControl;
using Avalonia.Collections;
using Avalonia.Controls.Selection;
using Mesen.Debugger.Views.DebuggerDock;
using Mesen.Debugger.ViewModels.DebuggerDock;
using Dock.Model.Core;

namespace Mesen.Debugger.ViewModels;

public class FindResultListViewModel : ViewModelBase
{
	[Reactive] public MesenList<FindResultViewModel> FindResults { get; private set; } = new();
	[Reactive] public SelectionModel<FindResultViewModel?> Selection { get; set; } = new() { SingleSelect = false };
	[Reactive] public SortState SortState { get; set; } = new();

	public DebuggerWindowViewModel Debugger { get; }

	private CodeLineData[] _results = Array.Empty<CodeLineData>();
	private string _format;

	[Obsolete("For designer only")]
	public FindResultListViewModel() : this(new()) { }

	public FindResultListViewModel(DebuggerWindowViewModel debugger)
	{
		Debugger = debugger;
		
		_format = "X" + debugger.CpuType.GetAddressSize();
		SortState.SetColumnSort("Address", ListSortDirection.Ascending, true);
	}

	public void Sort(object? param)
	{
		UpdateResults();
	}

	public void SetResults(CodeLineData[] results)
	{
		_results = results;
		Selection.Clear();
		UpdateResults();
		Selection.SelectedIndex = 0;
		Debugger.OpenTool(Debugger.DockFactory.FindResultListTool);
	}

	private void UpdateResults()
	{
		int selection = Selection.SelectedIndex;

		List<FindResultViewModel> sortedResults = _results.Select(result => new FindResultViewModel(result, _format)).ToList();

		Dictionary<string, Func<FindResultViewModel, FindResultViewModel, int>> comparers = new() {
			{ "Address", (a, b) => string.Compare(a.Address, b.Address, StringComparison.OrdinalIgnoreCase) },
			{ "Text", (a, b) => string.Compare(a.Text, b.Text, StringComparison.OrdinalIgnoreCase) },
		};

		sortedResults.Sort((a, b) => {
			foreach((string column, ListSortDirection order) in SortState.SortOrder) {
				int result = comparers[column](a, b);
				if(result != 0) {
					return result * (order == ListSortDirection.Ascending ? 1 : -1);
				}
			}
			return comparers["Address"](a, b);
		});

		FindResults.Replace(sortedResults);

		if(selection >= 0) {
			if(selection < FindResults.Count) {
				Selection.SelectedIndex = selection;
			} else {
				Selection.SelectedIndex = FindResults.Count - 1;
			}
		}
	}

	public void GoToResult(FindResultViewModel result)
	{
		int addr = result.Result.Address;
		if(addr >= 0) {
			Debugger.Disassembly.SetSelectedRow(addr, true);
		}
	}

	public void InitContextMenu(Control parent)
	{
		DebugShortcutManager.CreateContextMenu(parent, new object[] {
			new ContextMenuAction() {
				ActionType = ActionType.AddWatch,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FindResultList_AddWatch),
				IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is FindResultViewModel vm && vm.Result.Address >= 0,
				OnClick = () => {
					if(Selection.SelectedItem is FindResultViewModel vm) {
						if(vm.Result.Address > 0) {
							WatchManager.GetWatchManager(Debugger.CpuType).AddWatch("[$" + vm.Result.Address.ToString(_format) + "]");
						}
					}
				}
			},
			new ContextMenuAction() {
				ActionType = ActionType.ToggleBreakpoint,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FindResultList_ToggleBreakpoint),
				IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is FindResultViewModel vm && vm.Result.Address >= 0,
				OnClick = () => {
					if(Selection.SelectedItem is FindResultViewModel vm) {
						if(vm.Result.Address > 0) {
							AddressInfo relAddress = new AddressInfo() { Address = vm.Result.Address, Type = Debugger.CpuType.ToMemoryType() };
							AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
							if(absAddress.Address >= 0) {
								BreakpointManager.ToggleBreakpoint(absAddress, Debugger.CpuType);
							} else if(relAddress.Address >= 0) {
								BreakpointManager.ToggleBreakpoint(relAddress, Debugger.CpuType);
							}
						}
					}
				}
			},
			new ContextMenuSeparator(),
			new ContextMenuAction() {
				ActionType = ActionType.GoToLocation,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FindResultList_GoToLocation),
				IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is FindResultViewModel vm && vm.Result.Address >= 0,
				OnClick = () => {
					if(Selection.SelectedItem is FindResultViewModel vm) {
						GoToResult(vm);
					}
				}
			}
		});
	}
}

public class FindResultViewModel
{
	public CodeLineData Result { get; init; }
	public string Address { get; init; }
	public string Text { get; init; }

	public FindResultViewModel(CodeLineData result, string format)
	{
		Result = result;
		string text = result.Text;
		if(result.EffectiveAddress >= 0) {
			text += " " + result.GetEffectiveAddressString(format, out _);
		}
		Text = text;
		Address = "$" + result.Address.ToString(format);
	}
}
