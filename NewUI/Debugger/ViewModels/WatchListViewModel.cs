using Dock.Model.ReactiveUI.Controls;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Reactive.Linq;
using System.Linq;
using Mesen.Interop;
using Avalonia.Controls;

namespace Mesen.Debugger.ViewModels
{
	public class WatchListViewModel : Tool
	{
		[Reactive] public List<WatchValueInfo> WatchEntries { get; private set; } = new List<WatchValueInfo>();
		[Reactive] public int SelectedIndex { get; set; } = 0;

		private WatchManager _manager;

		public WatchListViewModel() : this(CpuType.Cpu) { }

		public WatchListViewModel(CpuType cpuType)
		{
			Id = "WatchList";
			Title = "Watch";
			_manager = WatchManager.GetWatchManager(cpuType);
			_manager.WatchChanged += WatchListViewModel_WatchChanged;
			UpdateWatch();
		}

		public void UpdateWatch()
		{
			WatchEntries = _manager.GetWatchContent(WatchEntries);
		}

		public void EditWatch(int index, string expression)
		{
			_manager.UpdateWatch(index, expression);
		}
		
		public void MoveUp(int index)
		{
			List<string> entries = _manager.WatchEntries;
			if(index > 0 && index < entries.Count) {
				string currentEntry = entries[index];
				string entryAbove = entries[index - 1];
				_manager.UpdateWatch(index - 1, currentEntry);
				_manager.UpdateWatch(index, entryAbove);
				SelectedIndex = index - 1;
			}
		}

		public void MoveDown(int index)
		{
			List<string> entries = _manager.WatchEntries;
			if(index < entries.Count - 1) {
				string currentEntry = entries[index];
				string entryBelow = entries[index + 1];
				_manager.UpdateWatch(index + 1, currentEntry);
				_manager.UpdateWatch(index, entryBelow);
				SelectedIndex = index + 1;
			}
		}

		private void WatchListViewModel_WatchChanged(object? sender, EventArgs e)
		{
			UpdateWatch();
		}

		internal void DeleteWatch(List<WatchValueInfo> items)
		{
			int[] indexes = items.Select(x => WatchEntries.IndexOf(x)).ToArray();
			_manager.RemoveWatch(indexes);
		}
	}
}
