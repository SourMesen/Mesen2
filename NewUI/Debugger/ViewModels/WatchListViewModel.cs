using ReactiveUI.Fody.Helpers;
using System.Collections.Generic;
using System.Reactive.Linq;
using System.Linq;
using Mesen.Interop;
using Mesen.ViewModels;
using Mesen.Utilities;
using System.Text.RegularExpressions;
using Mesen.Debugger.Labels;
using System;
using System.Globalization;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Utilities;
using Avalonia.Controls;
using Mesen.Config;
using Avalonia.Threading;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Windows;

namespace Mesen.Debugger.ViewModels
{
	public class WatchListViewModel : ViewModelBase
	{
		private static Regex _watchAddressOrLabel = new Regex(@"^(\[|{)(\s*((\$[0-9A-Fa-f]+)|(\d+)|([@_a-zA-Z0-9]+)))\s*[,]{0,1}\d*\s*(\]|})$", RegexOptions.Compiled);

		[Reactive] public SwappableList<WatchValueInfo> WatchEntries { get; private set; } = new();
		[Reactive] public int SelectedIndex { get; set; } = -1;

		public WatchManager Manager { get; }
		public CpuType CpuType { get; }

		public WatchListViewModel() : this(CpuType.Snes) { }

		public WatchListViewModel(CpuType cpuType)
		{
			CpuType = cpuType;
			Manager = WatchManager.GetWatchManager(cpuType);
			UpdateWatch();
		}

		public void UpdateWatch()
		{
			int selection = SelectedIndex;
			WatchEntries.Swap(Manager.GetWatchContent(WatchEntries));
			if(selection >= 0) {
				if(selection < WatchEntries.Count) {
					SelectedIndex = selection;
				} else {
					SelectedIndex = WatchEntries.Count - 1;
				}
			}
		}

		public void EditWatch(int index, string expression)
		{
			Manager.UpdateWatch(index, expression);
		}
		
		public void MoveUp(int index)
		{
			List<string> entries = Manager.WatchEntries;
			if(index > 0 && index < entries.Count) {
				string currentEntry = entries[index];
				string entryAbove = entries[index - 1];
				Manager.UpdateWatch(index - 1, currentEntry);
				Manager.UpdateWatch(index, entryAbove);
				SelectedIndex = index - 1;
			}
		}

		public void MoveDown(int index)
		{
			List<string> entries = Manager.WatchEntries;
			if(index < entries.Count - 1) {
				string currentEntry = entries[index];
				string entryBelow = entries[index + 1];
				Manager.UpdateWatch(index + 1, currentEntry);
				Manager.UpdateWatch(index, entryBelow);
				SelectedIndex = index + 1;
			}
		}

		private int[] GetIndexes(List<WatchValueInfo> items)
		{
			return items.Select(x => WatchEntries.IndexOf(x)).ToArray();
		}

		internal void DeleteWatch(List<WatchValueInfo> items)
		{
			Manager.RemoveWatch(GetIndexes(items));
		}

		internal void SetSelectionFormat(WatchFormatStyle format, int byteLength, List<WatchValueInfo> items)
		{
			Manager.SetSelectionFormat(format, byteLength, GetIndexes(items));
		}

		internal void ClearSelectionFormat(List<WatchValueInfo> items)
		{
			Manager.ClearSelectionFormat(GetIndexes(items));
		}

		private LocationInfo? GetLocation()
		{
			if(SelectedIndex < 0 || SelectedIndex >= WatchEntries.Count) {
				return null;
			}

			WatchValueInfo entry = WatchEntries[SelectedIndex];

			Match match = _watchAddressOrLabel.Match(entry.Expression);
			if(match.Success) {
				string address = match.Groups[3].Value;

				if(address[0] >= '0' && address[0] <= '9' || address[0] == '$') {
					//CPU Address
					bool isHex = address[0] == '$';
					string addrString = isHex ? address.Substring(1) : address;
					if(Int32.TryParse(addrString, isHex ? NumberStyles.AllowHexSpecifier : NumberStyles.None, null, out int parsedAddress)) {
						return new LocationInfo() { RelAddress = new() { Address = parsedAddress, Type = CpuType.ToMemoryType() } };
					}
				} else {
					//Label
					CodeLabel? label = LabelManager.GetLabel(address);
					if(label != null) {
						if(label.Matches(CpuType)) {
							return new LocationInfo() { Label = label, RelAddress = label.GetRelativeAddress(CpuType) };
						}
						return null;
					}
				}
			}

			if(entry.NumericValue >= 0) {
				return new LocationInfo() { RelAddress = new() { Address = entry.NumericValue, Type = CpuType.ToMemoryType() } };
			} else {
				return null;
			}
		}

		public void InitContextMenu(Control ctrl, MesenDataGrid grid)
		{
			DebugShortcutManager.CreateContextMenu(ctrl, new object[] {
				new ContextMenuAction() {
					ActionType = ActionType.Add,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.WatchList_Add),
					OnClick = () => {
						grid.SelectedIndex = WatchEntries.Count - 1;
						grid.ScrollIntoView(grid.SelectedItem, grid.Columns[0]);
						Dispatcher.UIThread.Post(() => {
							//This needs to be done as a post, otherwise the edit operation doesn't
							//start properly when the user right-clicks outside the grid
							grid.CurrentColumn = grid.Columns[0];
							grid.BeginEdit();
						});
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Edit,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.WatchList_Edit),
					IsEnabled = () => grid.SelectedItems.Count == 1,
					OnClick = () => {
						grid.CurrentColumn = grid.Columns[0];
						grid.BeginEdit();
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.WatchList_Delete),
					IsEnabled = () => grid.SelectedItems.Count > 0,
					OnClick = () => {
						DeleteWatch(grid.SelectedItems.Cast<WatchValueInfo>().ToList());
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.MoveUp,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.WatchList_MoveUp),
					IsEnabled = () => grid.SelectedItems.Count == 1 && grid.SelectedIndex > 0,
					OnClick = () => {
						MoveUp(grid.SelectedIndex);
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.MoveDown,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.WatchList_MoveDown),
					IsEnabled = () => grid.SelectedItems.Count == 1 && grid.SelectedIndex < WatchEntries.Count - 2,
					OnClick = () => {
						MoveDown(grid.SelectedIndex);
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.RowDisplayFormat,
					SubActions = new() {
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatBinary,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Binary, 1, grid.SelectedItems.Cast<WatchValueInfo>().ToList())
						},
						new ContextMenuSeparator(),
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatHex8Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Hex, 1, grid.SelectedItems.Cast<WatchValueInfo>().ToList())
						},
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatHex16Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Hex, 2, grid.SelectedItems.Cast<WatchValueInfo>().ToList())
						},
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatHex24Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Hex, 3, grid.SelectedItems.Cast<WatchValueInfo>().ToList())
						},
						new ContextMenuSeparator(),
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatSigned8Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Signed, 1, grid.SelectedItems.Cast<WatchValueInfo>().ToList())
						},
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatSigned16Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Signed, 2, grid.SelectedItems.Cast<WatchValueInfo>().ToList())
						},
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatSigned24Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Signed, 3, grid.SelectedItems.Cast<WatchValueInfo>().ToList())
						},
						new ContextMenuSeparator(),
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatUnsigned,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Unsigned, 1, grid.SelectedItems.Cast<WatchValueInfo>().ToList())
						},
						new ContextMenuSeparator(),
						new ContextMenuAction() {
							ActionType = ActionType.ClearFormat,
							OnClick = () => ClearSelectionFormat(grid.SelectedItems.Cast<WatchValueInfo>().ToList())
						}
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.WatchDecimalDisplay,
					IsSelected = () => ConfigManager.Config.Debug.Debugger.WatchFormat == WatchFormatStyle.Unsigned,
					OnClick = () => {
						ConfigManager.Config.Debug.Debugger.WatchFormat = WatchFormatStyle.Unsigned;
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.WatchHexDisplay,
					IsSelected = () => ConfigManager.Config.Debug.Debugger.WatchFormat == WatchFormatStyle.Hex,
					OnClick = () => {
						ConfigManager.Config.Debug.Debugger.WatchFormat = WatchFormatStyle.Hex;
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.WatchBinaryDisplay,
					IsSelected = () => ConfigManager.Config.Debug.Debugger.WatchFormat == WatchFormatStyle.Binary,
					OnClick = () => {
						ConfigManager.Config.Debug.Debugger.WatchFormat = WatchFormatStyle.Binary;
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					IsEnabled = () => GetLocation() != null,
					HintText = GetLocationHint,
					OnClick = () => {
						LocationInfo? location = GetLocation();
						if(location != null && location.RelAddress != null) {
							MemoryToolsWindow.ShowInMemoryTools(CpuType.ToMemoryType(), location.RelAddress.Value.Address);
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.GoToLocation,
					IsEnabled = () => GetLocation() != null,
					HintText = GetLocationHint,
					OnClick = () => {
						LocationInfo? location = GetLocation();
						if(location != null && location.RelAddress != null) {
							DebuggerWindow.OpenWindowAtAddress(CpuType, location.RelAddress.Value.Address);
						}
					}
				}
			});
		}

		private string GetLocationHint()
		{
			LocationInfo? location = GetLocation();
			if(location?.Label != null) {
				return location.Label.Label;
			} else if(location?.RelAddress != null) {
				return "$" + location.RelAddress.Value.Address.ToString("X" + CpuType.GetAddressSize());
			}
			return "";
		}
	}
}
