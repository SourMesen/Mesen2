using Avalonia.Controls;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace Mesen.Debugger.ViewModels
{
	public class ProfilerWindowViewModel : ViewModelBase
	{
		[Reactive] public List<ProfilerTab> ProfilerTabs { get; set; } = new List<ProfilerTab>();
		[Reactive] public ProfilerTab SelectedTab { get; set; } = null!;

		public ProfilerConfig Config { get; }

		public ProfilerWindowViewModel()
		{
			Config = ConfigManager.Config.Debug.Profiler;

			if(Design.IsDesignMode) {
				return;
			}

			UpdateAvailableTabs();
		}

		public void UpdateAvailableTabs()
		{
			List<ProfilerTab> tabs = new();
			foreach(CpuType type in EmuApi.GetRomInfo().CpuTypes) {
				tabs.Add(new ProfilerTab() {
					TabName = ResourceHelper.GetEnumText(type),
					CpuType = type
				});
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
		[Reactive] public ObservableCollection<ProfiledFunctionViewModel> GridData { get; private set; } = new ObservableCollection<ProfiledFunctionViewModel>();
		
		private ProfiledFunction[] _profilerData = new ProfiledFunction[0];
		private UInt64 _totalCycles;
		public HashSet<int> LoadedRows { get; } = new HashSet<int>();
		private int _sortColumn = 5;
		private bool _sortOrder = true;

		public ProfilerTab()
		{
		}

		public void ResetData()
		{
			DebugApi.ResetProfiler(CpuType);
			GridData.Clear();
			LoadedRows.Clear();
			RefreshData();
			RefreshGrid();
		}

		public void RefreshData()
		{
			_profilerData = DebugApi.GetProfilerData(CpuType);
		}

		public void RefreshGrid()
		{
			Sort();

			UInt64 totalCycles = 0;
			foreach(ProfiledFunction f in _profilerData) {
				totalCycles += f.ExclusiveCycles;
			}
			_totalCycles = totalCycles;

			while(GridData.Count < _profilerData.Length) {
				GridData.Add(new ProfiledFunctionViewModel());
			}

			UpdateLoadedRows();
		}

		public void UpdateRow(int index, ProfiledFunctionViewModel row)
		{
			if(index >= _profilerData.Length) {
				return;
			}

			ProfiledFunction func = _profilerData[index];

			row.FunctionName = GetFunctionName(func);
			row.ExclusiveCycles = func.ExclusiveCycles.ToString();
			row.InclusiveCycles = func.InclusiveCycles.ToString();
			row.CallCount = func.CallCount.ToString();
			row.MinCycles = func.MinCycles == UInt64.MaxValue ? "n/a" : func.MinCycles.ToString();
			row.MaxCycles = func.MaxCycles == 0 ? "n/a" : func.MaxCycles.ToString();

			row.AvgCycles = (func.CallCount == 0 ? 0 : (func.InclusiveCycles / func.CallCount)).ToString();
			row.ExclusivePercent = ((double)func.ExclusiveCycles / _totalCycles * 100).ToString("0.00");
			row.InclusivePercent = ((double)func.InclusiveCycles / _totalCycles * 100).ToString("0.00");

			LoadedRows.Add(index);
		}

		public void UnloadRow(int index)
		{
			LoadedRows.Remove(index);
		}

		public void UpdateLoadedRows()
		{
			foreach(int i in LoadedRows) {
				UpdateRow(i, GridData[i]);
			}
		}

		private string GetFunctionName(ProfiledFunction func)
		{
			int hexCount = this.CpuType == CpuType.Spc ? 4 : 6;
			string functionName;

			if(func.Address.Address == -1) {
				functionName = "[Reset]";
			} else {
				CodeLabel? label = LabelManager.GetLabel((UInt32)func.Address.Address, func.Address.Type);

				functionName = func.Address.Type.GetShortName() + ": $" + func.Address.Address.ToString("X" + hexCount.ToString());
				if(label != null) {
					functionName = label.Label + " (" + functionName + ")";
				}
			}

			return functionName;
		}

		private object GetColumnContent(ProfiledFunction func, int columnIndex)
		{
			switch(columnIndex) {
				case 0: return GetFunctionName(func);
				case 1: return func.CallCount;
				case 2: return func.InclusiveCycles;
				case 3: return (double)func.InclusiveCycles / _totalCycles * 100;
				case 4: return func.ExclusiveCycles;
				case 5: return (double)func.ExclusiveCycles / _totalCycles * 100;
				case 6: return func.CallCount == 0 ? 0 : (func.InclusiveCycles / func.CallCount);
				case 7: return func.MinCycles;
				case 8: return func.MaxCycles;
			}
			throw new Exception("Invalid column index");
		}

		public void Sort()
		{
			Array.Sort(_profilerData, new ListComparer(this, _sortColumn, _sortOrder));
		}

		public void Sort(int column)
		{
			if(_sortColumn == column) {
				_sortOrder = !_sortOrder;
			} else {
				_sortColumn = column;
				_sortOrder = column == 0 ? false : true;
			}

			Array.Sort(_profilerData, new ListComparer(this, _sortColumn, _sortOrder));

			UpdateLoadedRows();
		}

		private class ListComparer : IComparer<ProfiledFunction>
		{
			private ProfilerTab _profiler;
			private int _columnIndex;
			private bool _sortOrder;

			public ListComparer(ProfilerTab profiler, int columnIndex, bool sortOrder)
			{
				_profiler = profiler;
				_columnIndex = columnIndex;
				_sortOrder = sortOrder;
			}

			public int Compare(ProfiledFunction x, ProfiledFunction y)
			{
				if(_columnIndex == 0) {
					if(_sortOrder) {
						return String.Compare(_profiler.GetFunctionName(y), _profiler.GetFunctionName(x));
					} else {
						return String.Compare(_profiler.GetFunctionName(x), _profiler.GetFunctionName(y));
					}
				} else {
					IComparable columnValueY = (IComparable)_profiler.GetColumnContent(x, _columnIndex);
					IComparable columnValueX = (IComparable)_profiler.GetColumnContent(y, _columnIndex);
					if(_sortOrder) {
						return columnValueX.CompareTo(columnValueY);
					} else {
						return columnValueY.CompareTo(columnValueX);
					}
				}
			}
		}
	}

	public class ProfiledFunctionViewModel : ReactiveObject
	{
		[Reactive] public string ExclusiveCycles { get; set; } = "";
		[Reactive] public string InclusiveCycles { get; set; } = "";
		[Reactive] public string CallCount { get; set; } = "";
		[Reactive] public string MinCycles { get; set; } = "";
		[Reactive] public string MaxCycles { get; set; } = "";

		[Reactive] public string FunctionName { get; set; } = "";
		[Reactive] public string ExclusivePercent { get; set; } = "";
		[Reactive] public string InclusivePercent { get; set; } = "";
		[Reactive] public string AvgCycles { get; set; } = "";
	}
}
