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

		public LocationInfo? GetLocation()
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
			
			return new LocationInfo() { RelAddress = new() { Address = entry.NumericValue, Type = CpuType.ToMemoryType() } };
		}
	}
}
