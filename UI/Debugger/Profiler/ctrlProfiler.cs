using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Collections;
using Mesen.GUI.Controls;
using Mesen.GUI.Debugger.Labels;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlProfiler : BaseControl
	{
		private ProfiledFunction[] _newData = new ProfiledFunction[0];
		private ProfiledFunction[] _functions = new ProfiledFunction[0];
		UInt64 _exclusiveTotal = 0;
		private int _sortColumn = 5;
		private bool _sortOrder = true;

		public CpuType CpuType { get; internal set; }

		public ctrlProfiler()
		{
			InitializeComponent();
		}

		public void RefreshData()
		{
			_newData = DebugApi.GetProfilerData(this.CpuType);
		}

		public void RefreshList()
		{
			lstFunctions.BeginUpdate();

			_functions = _newData;
			_exclusiveTotal = 0;
			foreach(ProfiledFunction func in _functions) {
				_exclusiveTotal += func.ExclusiveCycles;
			}

			Array.Sort(_functions, new ListComparer(this, _sortColumn, _sortOrder));
			lstFunctions.VirtualListSize = _functions.Length;

			lstFunctions.EndUpdate();
		}

		private void btnReset_Click(object sender, EventArgs e)
		{
			DebugApi.ResetProfiler(this.CpuType);
			lstFunctions.Items.Clear();
			RefreshData();
		}
		
		private void lstFunctions_ColumnClick(object sender, ColumnClickEventArgs e)
		{
			if(_sortColumn == e.Column) {
				_sortOrder = !_sortOrder;
			} else {
				_sortColumn = e.Column;
				_sortOrder = e.Column == 0 ? false : true;
			}

			RefreshList();
		}
		
		private void lstFunctions_DoubleClick(object sender, EventArgs e)
		{
			if(lstFunctions.SelectedIndices.Count > 0) {
				AddressInfo relativeAddress = DebugApi.GetRelativeAddress(_functions[lstFunctions.SelectedIndices[0]].Address, this.CpuType);
				if(relativeAddress.Address >= 0) {
					frmDebugger debugger = DebugWindowManager.OpenDebugger(this.CpuType);
					debugger.GoToAddress(relativeAddress.Address);
				}
			}
		}

		private void lstFunctions_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
		{
			e.Item = GetListItem(_functions[e.ItemIndex]);
		}

		private ListViewItem GetListItem(ProfiledFunction func)
		{
			ListViewItem item = new ListViewItem(GetFunctionName(func));
			item.Selected = false;
			item.Focused = false;

			item.SubItems.Add(GetColumnContent(func, 1).ToString());
			item.SubItems.Add(GetColumnContent(func, 2).ToString());
			item.SubItems.Add(((double)GetColumnContent(func, 3)).ToString("0.00"));
			item.SubItems.Add(GetColumnContent(func, 4).ToString());
			item.SubItems.Add(((double)GetColumnContent(func, 5)).ToString("0.00"));
			item.SubItems.Add(GetColumnContent(func, 6).ToString());
			item.SubItems.Add((UInt64)GetColumnContent(func, 7) == UInt64.MaxValue ? "n/a" : GetColumnContent(func, 7).ToString());
			item.SubItems.Add((UInt64)GetColumnContent(func, 8) == 0 ? "n/a" : GetColumnContent(func, 8).ToString());

			return item;
		}

		private string GetFunctionName(ProfiledFunction func)
		{
			int hexCount = this.CpuType == CpuType.Spc ? 4 : 6;
			string functionName;

			if(func.Address.Address == -1) {
				functionName = "[Reset]";
			} else {
				CodeLabel label = LabelManager.GetLabel((UInt32)func.Address.Address, func.Address.Type);

				switch(func.Address.Type) {
					case SnesMemoryType.PrgRom: functionName = "PRG: $"; break;
					case SnesMemoryType.Register: functionName = "REG: $"; break;
					case SnesMemoryType.SaveRam: functionName = "SRAM: $"; break;
					case SnesMemoryType.WorkRam: functionName = "WRAM: $"; break;
					case SnesMemoryType.SpcRam: functionName = "SPC: $"; break;
					case SnesMemoryType.SpcRom: functionName = "SPC ROM: $"; break;
					case SnesMemoryType.Sa1InternalRam: functionName = "IRAM: $"; break;
					case SnesMemoryType.GbPrgRom: functionName = "PRG: $"; break;
					case SnesMemoryType.GbWorkRam: functionName = "WRAM: $"; break;
					case SnesMemoryType.GbCartRam: functionName = "SRAM: $"; break;
					case SnesMemoryType.GbHighRam: functionName = "HRAM: $"; break;
					default: throw new Exception("Unsupported type");
				}

				functionName += func.Address.Address.ToString("X" + hexCount.ToString());
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
				case 3: return (double)func.InclusiveCycles / _exclusiveTotal * 100;
				case 4: return func.ExclusiveCycles;
				case 5: return (double)func.ExclusiveCycles / _exclusiveTotal * 100;
				case 6: return func.CallCount == 0 ? 0 : (func.InclusiveCycles / func.CallCount);
				case 7: return func.MinCycles;
				case 8: return func.MaxCycles;
			}
			throw new Exception("Invalid column index");
		}

		private class ListComparer : IComparer<ProfiledFunction>
		{
			private int _columnIndex;
			private bool _sortOrder;
			private ctrlProfiler _profiler;

			public ListComparer(ctrlProfiler profiler, int columnIndex, bool sortOrder)
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
}
