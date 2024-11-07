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
using Mesen.Debugger.Disassembly;
using Avalonia.Threading;

namespace Mesen.Debugger.ViewModels;

public class FindResultListViewModel : DisposableViewModel
{
	[Reactive] public MesenList<FindResultViewModel> FindResults { get; private set; } = new();
	[Reactive] public SelectionModel<FindResultViewModel?> Selection { get; set; } = new() { SingleSelect = false };
	[Reactive] public SortState SortState { get; set; } = new();
	public List<int> ColumnWidths { get; } = ConfigManager.Config.Debug.Debugger.FindResultColumnWidths;

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
		UpdateResults(FindResults);
	}

	public void SetResults(IEnumerable<FindResultViewModel> results)
	{
		Selection.Clear();
		UpdateResults(results);
		Selection.SelectedIndex = 0;
		Dispatcher.UIThread.Post(() => {
			//TODOv2 - run this in a post to bypass a bug that might be fixed in the latest Avalonia preview, to re-test after upgrading
			Debugger.OpenTool(Debugger.DockFactory.FindResultListTool);
		});
	}

	private Dictionary<string, Func<FindResultViewModel, FindResultViewModel, int>> _comparers = new() {
		{ "Address", (a, b) => string.Compare(a.Address, b.Address, StringComparison.OrdinalIgnoreCase) },
		{ "Result", (a, b) => string.Compare(a.Text, b.Text, StringComparison.OrdinalIgnoreCase) },
	};

	private void UpdateResults(IEnumerable<FindResultViewModel> results)
	{
		List<int> selectedIndexes = Selection.SelectedIndexes.ToList();
		List<FindResultViewModel> sortedResults = results.ToList();

		SortHelper.SortList(sortedResults, SortState.SortOrder, _comparers, "Address");

		FindResults.Replace(sortedResults);
		Selection.SelectIndexes(selectedIndexes, FindResults.Count);
	}

	public void GoToResult(FindResultViewModel result)
	{
		if(result.Location.RelAddress?.Address >= 0) {
			Debugger.ScrollToAddress(result.Location.RelAddress.Value.Address);
		} else if(result.Location.SourceLocation != null) {
			Debugger.SourceView?.ScrollToLocation(result.Location.SourceLocation.Value);
		}
	}

	public void InitContextMenu(Control parent)
	{
		AddDisposables(DebugShortcutManager.CreateContextMenu(parent, new object[] {
			new ContextMenuAction() {
				ActionType = ActionType.AddWatch,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FindResultList_AddWatch),
				IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is FindResultViewModel vm && vm.Location.RelAddress?.Address >= 0,
				OnClick = () => {
					if(Selection.SelectedItem is FindResultViewModel vm && vm.Location.RelAddress?.Address > 0) {
						WatchManager.GetWatchManager(Debugger.CpuType).AddWatch("[$" + vm.Location.RelAddress?.Address.ToString(_format) + "]");
					}
				}
			},
			new ContextMenuAction() {
				ActionType = ActionType.ToggleBreakpoint,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FindResultList_ToggleBreakpoint),
				IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is FindResultViewModel vm && (vm.Location.RelAddress?.Address >= 0 || vm.Location.AbsAddress?.Address >= 0),
				OnClick = () => {
					if(Selection.SelectedItem is FindResultViewModel vm) {
						if(vm.Location.AbsAddress?.Address > 0) {
							BreakpointManager.ToggleBreakpoint(vm.Location.AbsAddress.Value, Debugger.CpuType);
						} else if(vm.Location.RelAddress?.Address > 0) {
							AddressInfo relAddress = vm.Location.RelAddress.Value;
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
				IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is FindResultViewModel vm,
				OnClick = () => {
					if(Selection.SelectedItem is FindResultViewModel vm) {
						GoToResult(vm);
					}
				}
			}
		}));
	}
}

public class FindResultViewModel
{
	public LocationInfo Location { get; }
	public string Address { get; }
	public string Text { get; }

	public FindResultViewModel(LocationInfo location, string loc, string text)
	{
		Location = location;
		Address = loc;
		Text = text;
	}

	public FindResultViewModel(CodeLineData line)
	{
		Location = new LocationInfo() {
			RelAddress = new AddressInfo() { Address = line.Address, Type = line.CpuType.ToMemoryType() },
			AbsAddress = line.AbsoluteAddress
		};

		string format = "X" + line.CpuType.GetAddressSize();
		Address = "$" + line.Address.ToString(format);
		Text = line.Text;
		if(line.EffectiveAddress >= 0) {
			Text += " " + line.GetEffectiveAddressString(format, out _);
		}
	}
}
