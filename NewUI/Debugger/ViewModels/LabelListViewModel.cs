using Avalonia;
using Avalonia.Collections;
using Avalonia.Controls;
using Avalonia.Controls.Selection;
using Avalonia.Media;
using Dock.Model.ReactiveUI.Controls;
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
	public class LabelListViewModel : ViewModelBase
	{
		[Reactive] public AvaloniaList<LabelViewModel> Labels { get; private set; } = new();
		[Reactive] public SelectionModel<LabelViewModel?> Selection { get; set; } = new() { SingleSelect = false };

		public CpuType CpuType { get; }
		public DisassemblyViewModel Disassembly { get; }

		[Obsolete("For designer only")]
		public LabelListViewModel() : this(CpuType.Snes, new()) { }

		public LabelListViewModel(CpuType cpuType, DisassemblyViewModel disassembly)
		{
			CpuType = cpuType;
			Disassembly = disassembly;
			UpdateLabelList();
		}

		public void UpdateLabelList()
		{
			int selection = Selection.SelectedIndex;

			Labels.Clear();
			Labels.AddRange(LabelManager.GetLabels(CpuType).Select(l => new LabelViewModel(l, CpuType)));

			if(selection >= 0) {
				if(selection < Labels.Count) {
					Selection.SelectedIndex = selection;
				} else {
					Selection.SelectedIndex = Labels.Count - 1;
				}
			}
		}

		public void RefreshLabelList()
		{
			foreach(LabelViewModel label in Labels) {
				label.Refresh();
			}
		}

		public void InitContextMenu(Control parent)
		{
			DebugShortcutManager.CreateContextMenu(parent, new object[] {
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
					ActionType = ActionType.AddBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_AddBreakpoint),
					IsEnabled = () => Selection.SelectedItems.Count == 1,
					OnClick = () => {
						if(Selection.SelectedItem is LabelViewModel vm) {
							AddressInfo addr = vm.Label.GetAbsoluteAddress();
							BreakpointManager.AddBreakpoint(addr, CpuType);
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
					ActionType = ActionType.GoToLocation,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_GoToLocation),
					IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedItem is LabelViewModel vm && vm.Label.GetRelativeAddress(CpuType).Address >= 0,
					OnClick = () => {
						if(Selection.SelectedItem is LabelViewModel vm) {
							AddressInfo addr = vm.Label.GetRelativeAddress(CpuType);
							if(addr.Address >= 0) {
								Disassembly.SetSelectedRow(addr.Address, true);
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
			});
		}

		public class LabelViewModel : INotifyPropertyChanged
		{
			private string _format;

			public CodeLabel Label { get; set; }
			public CpuType CpuType { get; }
			public string AbsAddressDisplay { get; }

			public int RelAddress { get; private set; }
			public string RelAddressDisplay => RelAddress >= 0 ? ("$" + RelAddress.ToString(_format)) : "<unavailable>";
			public object RowBrush => RelAddress >= 0 ? AvaloniaProperty.UnsetValue : Brushes.Gray;
			public FontStyle RowStyle => RelAddress >= 0 ? FontStyle.Normal : FontStyle.Italic;

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
				CpuType = cpuType;
				RelAddress = Label.GetRelativeAddress(CpuType).Address;
				_format = "X" + cpuType.GetAddressSize();

				if(Label.MemoryType.IsRelativeMemory()) {
					AbsAddressDisplay = "";
				} else {
					AbsAddressDisplay = "$" + label.Address.ToString(_format) + " [" + label.MemoryType.GetShortName() + "]";
				}
			}
		}
	}
}
