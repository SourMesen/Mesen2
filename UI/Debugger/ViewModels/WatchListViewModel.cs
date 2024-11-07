using ReactiveUI.Fody.Helpers;
using System.Collections.Generic;
using System.Reactive.Linq;
using System.Linq;
using Mesen.Interop;
using Mesen.ViewModels;
using System.Text.RegularExpressions;
using Mesen.Debugger.Labels;
using System;
using System.Globalization;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Utilities;
using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Windows;
using Avalonia.Collections;
using Avalonia.Controls.Selection;
using Avalonia.Interactivity;
using Mesen.Utilities;

namespace Mesen.Debugger.ViewModels
{
	public class WatchListViewModel : DisposableViewModel, IToolHelpTooltip
	{
		private static Regex _watchAddressOrLabel = new Regex(@"^(\[|{)(\s*((\$[0-9A-Fa-f]+)|(\d+)|([@_a-zA-Z0-9]+)))\s*[,]{0,1}\d*\s*(\]|})$", RegexOptions.Compiled);

		[Reactive] public MesenList<WatchValueInfo> WatchEntries { get; private set; } = new();
		[Reactive] public SelectionModel<WatchValueInfo> Selection { get; set; } = new() { SingleSelect = false };
		public List<int> ColumnWidths { get; } = ConfigManager.Config.Debug.Debugger.WatchListColumnWidths;

		public WatchManager Manager { get; }
		public CpuType CpuType { get; }
		public object HelpTooltip => ExpressionTooltipHelper.GetHelpTooltip(CpuType, true);

		public WatchListViewModel() : this(CpuType.Snes) { }

		public WatchListViewModel(CpuType cpuType)
		{
			CpuType = cpuType;
			Manager = WatchManager.GetWatchManager(cpuType);

			Manager.WatchChanged += Manager_WatchChanged;
		}

		protected override void DisposeView()
		{
			base.DisposeView();
			Manager.WatchChanged -= Manager_WatchChanged;
		}

		private void Manager_WatchChanged(bool resetSelection)
		{
			if(resetSelection) {
				Selection.Clear();
			}

			UpdateWatch();
		}

		public void UpdateWatch()
		{
			List<WatchValueInfo> newEntries = Manager.GetWatchContent(WatchEntries);

			if(newEntries.Count != WatchEntries.Count) {
				List<int> selectedIndexes = Selection.SelectedIndexes.ToList();
				WatchEntries.Replace(newEntries);
				Selection.SelectIndexes(selectedIndexes, WatchEntries.Count);
			} else {
				for(int i = 0; i < newEntries.Count; i++) {
					WatchEntries[i].Expression = newEntries[i].Expression;
					WatchEntries[i].Value = newEntries[i].Value;
					WatchEntries[i].NumericValue = newEntries[i].NumericValue;
					WatchEntries[i].IsChanged = newEntries[i].IsChanged;
				}
			}
		}

		public void EditWatch(int index, string expression)
		{
			if(index < 0) {
				return;
			}

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
			}
		}

		private int[] GetIndexes(IEnumerable<WatchValueInfo?> items)
		{
			return items.Cast<WatchValueInfo>().Select(x => WatchEntries.IndexOf(x)).ToArray();
		}

		internal void DeleteWatch(List<WatchValueInfo?> items)
		{
			Manager.RemoveWatch(GetIndexes(items));
		}

		internal void SetSelectionFormat(WatchFormatStyle format, int byteLength, IEnumerable<WatchValueInfo?> items)
		{
			Manager.SetSelectionFormat(format, byteLength, GetIndexes(items));
		}

		internal void ClearSelectionFormat(IEnumerable<WatchValueInfo?> items)
		{
			Manager.ClearSelectionFormat(GetIndexes(items));
		}

		private LocationInfo? GetLocation()
		{
			if(Selection.SelectedIndex < 0 || Selection.SelectedIndex >= WatchEntries.Count) {
				return null;
			}

			WatchValueInfo entry = WatchEntries[Selection.SelectedIndex];

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

			if(entry.NumericValue >= 0 && entry.NumericValue < UInt32.MaxValue) {
				return new LocationInfo() { RelAddress = new() { Address = (int)entry.NumericValue, Type = CpuType.ToMemoryType() } };
			} else {
				return null;
			}
		}

		public void InitContextMenu(Control ctrl)
		{
			AddDisposables(DebugShortcutManager.CreateContextMenu(ctrl, new object[] {
				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.WatchList_Delete),
					IsEnabled = () => Selection.SelectedItems.Count > 0,
					OnClick = () => {
						DeleteWatch(Selection.SelectedItems.ToList());
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.MoveUp,
					RoutingStrategy = RoutingStrategies.Tunnel,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.WatchList_MoveUp),
					IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedIndex > 0,
					OnClick = () => {
						MoveUp(Selection.SelectedIndex);
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.MoveDown,
					RoutingStrategy = RoutingStrategies.Tunnel,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.WatchList_MoveDown),
					IsEnabled = () => Selection.SelectedItems.Count == 1 && Selection.SelectedIndex < WatchEntries.Count - 2,
					OnClick = () => {
						MoveDown(Selection.SelectedIndex);
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.RowDisplayFormat,
					SubActions = new() {
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatBinary,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Binary, 1, Selection.SelectedItems)
						},
						new ContextMenuSeparator(),
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatHex8Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Hex, 1, Selection.SelectedItems)
						},
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatHex16Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Hex, 2, Selection.SelectedItems)
						},
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatHex24Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Hex, 3, Selection.SelectedItems)
						},
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatHex32Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Hex, 4, Selection.SelectedItems)
						},
						new ContextMenuSeparator(),
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatSigned8Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Signed, 1, Selection.SelectedItems)
						},
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatSigned16Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Signed, 2, Selection.SelectedItems)
						},
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatSigned24Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Signed, 3, Selection.SelectedItems)
						},
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatSigned32Bits,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Signed, 4, Selection.SelectedItems)
						},
						new ContextMenuSeparator(),
						new ContextMenuAction() {
							ActionType = ActionType.RowFormatUnsigned,
							OnClick = () => SetSelectionFormat(WatchFormatStyle.Unsigned, 1, Selection.SelectedItems)
						},
						new ContextMenuSeparator(),
						new ContextMenuAction() {
							ActionType = ActionType.ClearFormat,
							OnClick = () => ClearSelectionFormat(Selection.SelectedItems)
						}
					}
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.WatchDecimalDisplay,
					IsSelected = () => ConfigManager.Config.Debug.Debugger.WatchFormat == WatchFormatStyle.Signed,
					OnClick = () => {
						ConfigManager.Config.Debug.Debugger.WatchFormat = WatchFormatStyle.Signed;
						UpdateWatch();
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.WatchHexDisplay,
					IsSelected = () => ConfigManager.Config.Debug.Debugger.WatchFormat == WatchFormatStyle.Hex,
					OnClick = () => {
						ConfigManager.Config.Debug.Debugger.WatchFormat = WatchFormatStyle.Hex;
						UpdateWatch();
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.WatchBinaryDisplay,
					IsSelected = () => ConfigManager.Config.Debug.Debugger.WatchFormat == WatchFormatStyle.Binary,
					OnClick = () => {
						ConfigManager.Config.Debug.Debugger.WatchFormat = WatchFormatStyle.Binary;
						UpdateWatch();
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
			}));
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
