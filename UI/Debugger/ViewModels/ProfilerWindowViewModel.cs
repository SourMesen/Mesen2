using Avalonia.Collections;
using Avalonia.Controls;
using Avalonia.Controls.Selection;
using Avalonia.VisualTree;
using Avalonia.Threading;
using DataBoxControl;
using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Utilities;
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
using System.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class ProfilerWindowViewModel : DisposableViewModel
	{
		[Reactive] public List<ProfilerTab> ProfilerTabs { get; set; } = new List<ProfilerTab>();
		[Reactive] public ProfilerTab? SelectedTab { get; set; } = null;
		
		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		public ProfilerConfig Config { get; }

		public ProfilerWindowViewModel(Window? wnd)
		{
			Config = ConfigManager.Config.Debug.Profiler;

			if(Design.IsDesignMode) {
				return;
			}

			UpdateAvailableTabs();

			AddDisposable(this.WhenAnyValue(x => x.SelectedTab).Subscribe(x => {
				if(SelectedTab != null && EmuApi.IsPaused()) {
					RefreshData();
				}
			}));

			FileMenuActions = AddDisposables(new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.ResetProfilerData,
					OnClick = () => SelectedTab?.ResetData()
				},
				new ContextMenuAction() {
					ActionType = ActionType.CopyToClipboard,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Copy),
					OnClick = () => wnd?.GetVisualDescendants().Where(a => a is DataBox).Cast<DataBox>().FirstOrDefault()?.CopyToClipboard()
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd?.Close()
				}
			});

			ViewMenuActions = AddDisposables(new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.Refresh,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Refresh),
					OnClick = () => RefreshData()
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.EnableAutoRefresh,
					IsSelected = () => Config.AutoRefresh,
					OnClick = () => Config.AutoRefresh = !Config.AutoRefresh
				},
				new ContextMenuAction() {
					ActionType = ActionType.RefreshOnBreakPause,
					IsSelected = () => Config.RefreshOnBreakPause,
					OnClick = () => Config.RefreshOnBreakPause = !Config.RefreshOnBreakPause
				}
			});

			if(Design.IsDesignMode || wnd == null) {
				return;
			}

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);

			LabelManager.OnLabelUpdated += LabelManager_OnLabelUpdated;
		}

		protected override void DisposeView()
		{
			LabelManager.OnLabelUpdated -= LabelManager_OnLabelUpdated;
		}

		private void LabelManager_OnLabelUpdated(object? sender, EventArgs e)
		{
			ProfilerTab tab = (SelectedTab ?? ProfilerTabs[0]);
			Dispatcher.UIThread.Post(() => {
				tab?.RefreshGrid();
			});
		}

		public void UpdateAvailableTabs()
		{
			List<ProfilerTab> tabs = new();
			foreach(CpuType type in EmuApi.GetRomInfo().CpuTypes) {
				if(type.SupportsCallStack()) {
					tabs.Add(new ProfilerTab() {
						TabName = ResourceHelper.GetEnumText(type),
						CpuType = type
					});
				}
			}

			ProfilerTabs = tabs;
			SelectedTab = tabs[0];
		}

		public void RefreshData()
		{
			ProfilerTab tab = (SelectedTab ?? ProfilerTabs[0]);
			tab.RefreshData();
			Dispatcher.UIThread.Post(() => {
				tab.RefreshGrid();
			});
		}
	}

	public class ProfilerTab : ReactiveObject
	{
		[Reactive] public string TabName { get; set; } = "";
		[Reactive] public CpuType CpuType { get; set; } = CpuType.Snes;
		[Reactive] public MesenList<ProfiledFunctionViewModel> GridData { get; private set; } = new();
		[Reactive] public SelectionModel<ProfiledFunctionViewModel> Selection { get; set; } = new();
		[Reactive] public SortState SortState { get; set; } = new();
		public ProfilerConfig Config => ConfigManager.Config.Debug.Profiler;
		public List<int> ColumnWidths { get; } = ConfigManager.Config.Debug.Profiler.ColumnWidths;

		private object _updateLock = new();		
		private int _dataSize = 0;
		private ProfiledFunction[] _coreProfilerData = new ProfiledFunction[100000];
		private ProfiledFunction[] _profilerData = Array.Empty<ProfiledFunction>();

		private UInt64 _totalCycles;

		public ProfilerTab()
		{
			SortState.SetColumnSort("InclusiveTime", ListSortDirection.Descending, false);
		}

		public ProfiledFunction? GetRawData(int index)
		{
			ProfiledFunction[] data = _profilerData;
			if(index < data.Length) {
				return data[index];
			}
			return null;
		}

		public void ResetData()
		{
			DebugApi.ResetProfiler(CpuType);
			GridData.Clear();
			RefreshData();
			RefreshGrid();
		}

		public void RefreshData()
		{
			lock(_updateLock) {
				_dataSize = DebugApi.GetProfilerData(CpuType, ref _coreProfilerData);
			}
		}

		public void RefreshGrid()
		{
			lock(_updateLock) {
				Array.Resize(ref _profilerData, _dataSize);
				Array.Copy(_coreProfilerData, _profilerData, _dataSize);
			}

			Sort();

			UInt64 totalCycles = 0;
			ProfiledFunction[] profilerData = _profilerData;
			foreach(ProfiledFunction f in profilerData) {
				totalCycles += f.ExclusiveCycles;
			}
			_totalCycles = totalCycles;

			while(GridData.Count < profilerData.Length) {
				GridData.Add(new ProfiledFunctionViewModel());
			}

			for(int i = 0; i < profilerData.Length; i++) {
				GridData[i].Update(profilerData[i], CpuType, _totalCycles);
			}
		}

		public void SortCommand(object? param)
		{
			RefreshGrid();
		}

		public void Sort()
		{
			CpuType cpuType = CpuType;

			Dictionary<string, Func<ProfiledFunction, ProfiledFunction, int>> comparers = new() {
				{ "FunctionName", (a, b) => string.Compare(a.GetFunctionName(cpuType), b.GetFunctionName(cpuType), StringComparison.OrdinalIgnoreCase) },
				{ "CallCount", (a, b) => a.CallCount.CompareTo(b.CallCount) },
				{ "InclusiveTime", (a, b) => a.InclusiveCycles.CompareTo(b.InclusiveCycles) },
				{ "InclusiveTimePercent", (a, b) => a.InclusiveCycles.CompareTo(b.InclusiveCycles) },
				{ "ExclusiveTime", (a, b) => a.ExclusiveCycles.CompareTo(b.ExclusiveCycles) },
				{ "ExclusiveTimePercent", (a, b) => a.ExclusiveCycles.CompareTo(b.ExclusiveCycles) },
				{ "AvgCycles", (a, b) => a.GetAvgCycles().CompareTo(b.GetAvgCycles()) },
				{ "MinCycles", (a, b) => a.MinCycles.CompareTo(b.MinCycles) },
				{ "MaxCycles", (a, b) => a.MaxCycles.CompareTo(b.MaxCycles) },
			};

			SortHelper.SortArray(_profilerData, SortState.SortOrder, comparers, "InclusiveTime");
		}
	}

	public static class ProfiledFunctionExtensions
	{
		public static string GetFunctionName(this ProfiledFunction func, CpuType cpuType)
		{
			string functionName;

			if(func.Address.Address == -1) {
				functionName = "[Reset]";
			} else {
				CodeLabel? label = LabelManager.GetLabel((UInt32)func.Address.Address, func.Address.Type);

				int hexCount = cpuType.GetAddressSize();
				functionName = func.Address.Type.GetShortName() + ": $" + func.Address.Address.ToString("X" + hexCount.ToString());
				if(label != null) {
					functionName = label.Label + " (" + functionName + ")";
				}
			}

			if(func.Flags.HasFlag(StackFrameFlags.Irq)) {
				functionName = "[irq] " + functionName;
			} else if(func.Flags.HasFlag(StackFrameFlags.Nmi)) {
				functionName = "[nmi] " + functionName;
			}

			return functionName;
		}
	}

	public class ProfiledFunctionViewModel : INotifyPropertyChanged
	{
		public event PropertyChangedEventHandler? PropertyChanged;

		private string _functionName = "";
		public string FunctionName
		{
			get
			{
				UpdateFields();
				return _functionName;
			}
		}

		public string ExclusiveCycles { get; set; } = "";
		public string InclusiveCycles { get; set; } = "";
		public string CallCount { get; set; } = "";
		public string MinCycles { get; set; } = "";
		public string MaxCycles { get; set; } = "";

		public string ExclusivePercent { get; set; } = "";
		public string InclusivePercent { get; set; } = "";
		public string AvgCycles { get; set; } = "";

		private ProfiledFunction _funcData;
		private CpuType _cpuType;
		private UInt64 _totalCycles;

		public void Update(ProfiledFunction func, CpuType cpuType, UInt64 totalCycles)
		{
			_funcData = func;
			_cpuType = cpuType;
			_totalCycles = totalCycles;

			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ProfiledFunctionViewModel.FunctionName)));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ProfiledFunctionViewModel.ExclusiveCycles)));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ProfiledFunctionViewModel.InclusiveCycles)));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ProfiledFunctionViewModel.CallCount)));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ProfiledFunctionViewModel.MinCycles)));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ProfiledFunctionViewModel.MaxCycles)));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ProfiledFunctionViewModel.ExclusivePercent)));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ProfiledFunctionViewModel.InclusivePercent)));
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ProfiledFunctionViewModel.AvgCycles)));
		}

		private void UpdateFields()
		{
			_functionName = _funcData.GetFunctionName(_cpuType);
			ExclusiveCycles = _funcData.ExclusiveCycles.ToString();
			InclusiveCycles = _funcData.InclusiveCycles.ToString();
			CallCount = _funcData.CallCount.ToString();
			MinCycles = _funcData.MinCycles == UInt64.MaxValue ? "n/a" : _funcData.MinCycles.ToString();
			MaxCycles = _funcData.MaxCycles == 0 ? "n/a" : _funcData.MaxCycles.ToString();

			AvgCycles = (_funcData.CallCount == 0 ? 0 : (_funcData.InclusiveCycles / _funcData.CallCount)).ToString();
			ExclusivePercent = ((double)_funcData.ExclusiveCycles / _totalCycles * 100).ToString("0.00");
			InclusivePercent = ((double)_funcData.InclusiveCycles / _totalCycles * 100).ToString("0.00");
		}
	}
}
