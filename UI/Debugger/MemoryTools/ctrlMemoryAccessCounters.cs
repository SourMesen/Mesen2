using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Controls;
using Mesen.GUI.Forms;
using Mesen.GUI.Config;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlMemoryAccessCounters : BaseControl
	{
		private bool _sorting = false;
		private UInt64 _masterClock = 0;
		private AddressCounters[] _counts = new AddressCounters[0];
		private AddressCounters[] _newCounts = new AddressCounters[0];
		private SnesMemoryType _memoryType = SnesMemoryType.PrgRom;
		private SortType _sortType = SortType.Address;

		public ctrlMemoryAccessCounters()
		{
			InitializeComponent();
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			if(!IsDesignMode) {
				InitMemoryTypeDropdown();
			}
		}

		public void InitMemoryTypeDropdown()
		{
			cboMemoryType.InitMemoryTypeDropdown(excludeCpuMemory: true);
			UpdateMemoryType();
		}

		public void RefreshData()
		{
			if(_sorting) {
				return;
			}
			
			DebugApi.GetMemoryAccessCounts(_memoryType, ref _newCounts);

			bool isGameboyMode = EmuApi.GetRomInfo().CoprocessorType == CoprocessorType.Gameboy;
			DebugState state = DebugApi.GetState();
			_masterClock = isGameboyMode ? state.Gameboy.MemoryManager.CycleCount : state.MasterClock;

			_sorting = true;
			Task.Run(() => {
				switch(_sortType) {
					case SortType.Address: break;
					case SortType.Value: break;
					case SortType.Read: Array.Sort(_newCounts, new SortReadComparer()); break;
					case SortType.ReadStamp: Array.Sort(_newCounts, new SortReadStampComparer()); break;
					case SortType.Write: Array.Sort(_newCounts, new SortWriteComparer()); break;
					case SortType.WriteStamp: Array.Sort(_newCounts, new SortWriteStampComparer()); break;
					case SortType.Exec: Array.Sort(_newCounts, new SortExecComparer()); break;
					case SortType.ExecStamp: Array.Sort(_newCounts, new SortExecStampComparer()); break;
					case SortType.UninitRead: Array.Sort(_newCounts, new SortUninitComparer()); break;
				}

				AddressCounters[] counts = _counts;
				_counts = _newCounts;
				_newCounts = counts;

				this.BeginInvoke((Action)(() => {
					_sorting = false;
					lstCounters.BeginUpdate();
					lstCounters.VirtualListSize = _counts.Length;
					lstCounters.EndUpdate();
				}));
			});
		}

		private void cboMemoryType_SelectedIndexChanged(object sender, EventArgs e)
		{
			UpdateMemoryType();
		}

		private void UpdateMemoryType()
		{
			_memoryType = cboMemoryType.GetEnumValue<SnesMemoryType>();
			RefreshData();
		}

		private void btnReset_Click(object sender, EventArgs e)
		{
			DebugApi.ResetMemoryAccessCounts();
			RefreshData();
		}

		private string FormatNumber(UInt64 value)
		{
			if(value >= 1000000000000) {
				return ((double)value / 1000000000000).ToString("0.00") + " T";
			} else if(value >= 1000000000) {
				return ((double)value / 1000000000).ToString("0.00") + " G";
			} else if(value >= 1000000) {
				return ((double)value / 1000000).ToString("0.00") + " M";
			} else if(value >= 1000) {
				return ((double)value / 1000).ToString("0.00") + " K";
			}
			return value.ToString();
		}

		private void lstCounters_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
		{
			AddressCounters counts = _counts[e.ItemIndex];
			ListViewItem item = new ListViewItem("$" + counts.Address.ToString("X4"));
			item.Selected = false;
			item.Focused = false;

			item.SubItems.Add("$" + DebugApi.GetMemoryValue(_memoryType, counts.Address).ToString("X2"));
			item.SubItems.Add(FormatNumber(counts.ReadCount));
			item.SubItems.Add(counts.ReadStamp > 0 ? FormatNumber(_masterClock - counts.ReadStamp) : "n/a");
			item.SubItems.Add(FormatNumber(counts.WriteCount));
			item.SubItems.Add(counts.WriteStamp > 0 ? FormatNumber(_masterClock - counts.WriteStamp) : "n/a");
			item.SubItems.Add(FormatNumber(counts.ExecCount));
			item.SubItems.Add(counts.ExecStamp > 0 ? FormatNumber(_masterClock - counts.ExecStamp) : "n/a");
			item.SubItems.Add(counts.UninitRead != 0 ? "☑" : "☐");

			if(counts.ReadCount == 0 && counts.WriteCount == 0 && counts.ExecCount == 0) {
				item.ForeColor = Color.Gray;
			}

			e.Item = item;
		}

		public void GoToAddress()
		{
			GoToAddress address = new GoToAddress();
			address.Address = 0;

			using(frmGoToLine frm = new frmGoToLine(address, 8)) {
				frm.StartPosition = FormStartPosition.Manual;
				Point topLeft = this.PointToScreen(new Point(0, 0));
				frm.Location = new Point(topLeft.X + (this.Width - frm.Width) / 2, topLeft.Y + (this.Height - frm.Height) / 2);
				if(frm.ShowDialog() == DialogResult.OK) {
					int index = -1;
					for(int i = 0; i < _counts.Length; i++) {
						if(_counts[i].Address == address.Address) {
							index = i;
							break;
						}
					}

					if(index >= 0) {
						lstCounters.EnsureVisible(index);
						lstCounters.SelectedIndices.Clear();
						lstCounters.SelectedIndices.Add(index);
					}
				}
			}
		}

		private void lstCounters_ColumnClick(object sender, ColumnClickEventArgs e)
		{
			_sortType = (SortType)e.Column;
			RefreshData();
		}

		private class SortReadComparer : IComparer<AddressCounters> { public int Compare(AddressCounters a, AddressCounters b) { return a.ReadCount.CompareTo(b.ReadCount) * -2 + a.Address.CompareTo(b.Address); } }
		private class SortReadStampComparer : IComparer<AddressCounters> { public int Compare(AddressCounters a, AddressCounters b) { return a.ReadStamp.CompareTo(b.ReadStamp) * -2 + a.Address.CompareTo(b.Address); } }
		private class SortWriteComparer : IComparer<AddressCounters> { public int Compare(AddressCounters a, AddressCounters b) { return a.WriteCount.CompareTo(b.WriteCount) * -2 + a.Address.CompareTo(b.Address); } }
		private class SortWriteStampComparer : IComparer<AddressCounters> { public int Compare(AddressCounters a, AddressCounters b) { return a.WriteStamp.CompareTo(b.WriteStamp) * -2 + a.Address.CompareTo(b.Address); } }
		private class SortExecComparer : IComparer<AddressCounters> { public int Compare(AddressCounters a, AddressCounters b) { return a.ExecCount.CompareTo(b.ExecCount) * -2 + a.Address.CompareTo(b.Address); } }
		private class SortExecStampComparer : IComparer<AddressCounters> { public int Compare(AddressCounters a, AddressCounters b) { return a.ExecStamp.CompareTo(b.ExecStamp) * -2 + a.Address.CompareTo(b.Address); } }
		private class SortUninitComparer : IComparer<AddressCounters> { public int Compare(AddressCounters a, AddressCounters b) { return a.UninitRead.CompareTo(b.UninitRead) * -2 + a.Address.CompareTo(b.Address); } }

		private enum SortType
		{
			Address = 0,
			Value,
			Read,
			ReadStamp,
			Write,
			WriteStamp,
			Exec,
			ExecStamp,
			UninitRead,
		}
	}
}
