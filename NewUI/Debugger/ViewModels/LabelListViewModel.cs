using Avalonia.Controls;
using Dock.Model.ReactiveUI.Controls;
using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class LabelListViewModel : Tool
	{
		public CpuType CpuType { get; }
		public DisassemblyViewModel Disassembly { get; }

		[Reactive] public List<CodeLabel> Labels { get; private set; } = new List<CodeLabel>();

		[Obsolete("For designer only")]
		public LabelListViewModel() : this(CpuType.Cpu, new()) { }

		public LabelListViewModel(CpuType cpuType, DisassemblyViewModel disassembly)
		{
			CpuType = cpuType;
			Id = "Labels";
			Title = "Labels";
			CanPin = false;
			Disassembly = disassembly;
			UpdateLabelList();
		}

		public void UpdateLabelList()
		{
  			Labels = LabelManager.GetLabels(CpuType);
		}

		public void InitContextMenu(Control parent, DataGrid grid)
		{
			DebugShortcutManager.CreateContextMenu(parent, new object[] {
				new ContextMenuAction() {
					ActionType = ActionType.Add,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_Add),
					OnClick = () => LabelEditWindow.EditLabel(parent, new CodeLabel())
				},

				new ContextMenuAction() {
					ActionType = ActionType.Edit,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_Edit),
					IsEnabled = () => grid.SelectedItem is CodeLabel,
					OnClick = () => {
						CodeLabel? label = grid.SelectedItem as CodeLabel;
						if(label != null) {
							LabelEditWindow.EditLabel(parent, label);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_Delete),
					IsEnabled = () => grid.SelectedItems.Count > 0,
					OnClick = () => {
						foreach(object item in grid.SelectedItems.Cast<object>().ToList()) {
							if(item is CodeLabel label) {
								LabelManager.DeleteLabel(label, true);
							}
						}
					}
				},

				new Separator(),

				new ContextMenuAction() {
					ActionType = ActionType.AddBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_AddBreakpoint),
					IsEnabled = () => grid.SelectedItem is CodeLabel,
					OnClick = () => {
						if(grid.SelectedItem is CodeLabel label) {
							AddressInfo addr = label.GetAbsoluteAddress();
							BreakpointManager.AddBreakpoint(addr, CpuType);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.AddWatch,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_AddToWatch),
					IsEnabled = () => grid.SelectedItem is CodeLabel,
					OnClick = () => {
						if(grid.SelectedItem is CodeLabel label) {
							AddressInfo addr = label.GetRelativeAddress(CpuType);
							if(addr.Address >= 0) {
								WatchManager.GetWatchManager(CpuType).AddWatch("[$" + addr.Address.ToString("X2") + "]");
							}
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.GoToLocation,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.LabelList_GoToLocation),
					IsEnabled = () => grid.SelectedItem is CodeLabel label && label.GetRelativeAddress(CpuType).Address >= 0,
					OnClick = () => {
						if(grid.SelectedItem is CodeLabel label) {
							AddressInfo addr = label.GetRelativeAddress(CpuType);
							if(addr.Address >= 0) {
								Disassembly.ScrollToAddress((uint)addr.Address);
							}
						}
					}
				},
			});
		}
	}
}
