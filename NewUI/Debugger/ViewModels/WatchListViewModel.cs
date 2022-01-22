using ReactiveUI.Fody.Helpers;
using System.Collections.Generic;
using System.Reactive.Linq;
using System.Linq;
using Mesen.Interop;
using Mesen.ViewModels;

namespace Mesen.Debugger.ViewModels
{
	public class WatchListViewModel : ViewModelBase
	{
		[Reactive] public List<WatchValueInfo> WatchEntries { get; private set; } = new List<WatchValueInfo>();
		[Reactive] public int SelectedIndex { get; set; } = -1;

		public WatchManager Manager { get; }

		public WatchListViewModel() : this(CpuType.Snes) { }

		public WatchListViewModel(CpuType cpuType)
		{
			Manager = WatchManager.GetWatchManager(cpuType);
			UpdateWatch();
		}

		public void UpdateWatch()
		{
			int selection = SelectedIndex;
			WatchEntries = Manager.GetWatchContent(WatchEntries);
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
	}
}
