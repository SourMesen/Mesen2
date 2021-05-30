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

		private WatchManager _manager;

		public WatchListViewModel(CpuType cpuType)
		{
			Id = "WatchList";
			Title = "Watch";
			_manager = WatchManager.GetWatchManager(cpuType);
			_manager.WatchChanged += WatchListViewModel_WatchChanged;

			WatchEntries = _manager.GetWatchContent(new List<WatchValueInfo>());
		}

		public void UpdateWatch(int index, string expression)
		{
			_manager.UpdateWatch(index, expression);
		}

		private void WatchListViewModel_WatchChanged(object? sender, EventArgs e)
		{
			WatchEntries = _manager.GetWatchContent(WatchEntries);
		}
	}
}
