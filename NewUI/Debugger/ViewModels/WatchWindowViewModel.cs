using Avalonia.Collections;
using Avalonia.Controls;
using Avalonia.Controls.Selection;
using Avalonia.Threading;
using DataBoxControl;
using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;

namespace Mesen.Debugger.ViewModels
{
	public class WatchWindowViewModel : ViewModelBase
	{
		[Reactive] public List<WatchTab> WatchTabs { get; set; } = new List<WatchTab>();
		[Reactive] public WatchTab SelectedTab { get; set; } = null!;

		public WatchWindowConfig Config { get; }

		public WatchWindowViewModel()
		{
			Config = ConfigManager.Config.Debug.WatchWindow;

			if(Design.IsDesignMode) {
				return;
			}

			UpdateAvailableTabs();
		}

		public void UpdateAvailableTabs()
		{
			foreach(WatchTab tab in WatchTabs) {
				tab.Dispose();
			}

			List<WatchTab> tabs = new();
			foreach(CpuType type in EmuApi.GetRomInfo().CpuTypes) {
				tabs.Add(new WatchTab(type));
			}

			foreach(WatchTab tab in tabs) {
				tab.WatchList.UpdateWatch();
			}

			WatchTabs = tabs;
			SelectedTab = tabs[0];
		}

		public void RefreshData()
		{
			Dispatcher.UIThread.Post(() => {
				WatchTab tab = (SelectedTab ?? WatchTabs[0]);
				tab.WatchList.UpdateWatch();
			});
		}
	}

	public class WatchTab : DisposableViewModel
	{
		public string TabName { get; }
		public CpuType CpuType { get; }
		public WatchListViewModel WatchList { get; }

		public WatchTab(CpuType cpuType)
		{
			CpuType = cpuType;
			TabName = ResourceHelper.GetEnumText(cpuType);
			WatchList = new WatchListViewModel(cpuType);
		}

		protected override void DisposeView()
		{
			WatchList.Dispose();
		}
	}
}
