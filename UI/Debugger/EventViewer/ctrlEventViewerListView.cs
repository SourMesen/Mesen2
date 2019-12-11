using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using Mesen.GUI.Controls;
using Mesen.GUI.Forms;
using Mesen.GUI.Config;
using System.Collections.ObjectModel;

namespace Mesen.GUI.Debugger
{
	public partial class ctrlEventViewerListView : BaseControl
	{
		private List<DebugEventInfo> _debugEvents = new List<DebugEventInfo>();
		private ReadOnlyCollection<Breakpoint> _breakpoints = null;
		private eSortColumn _sortColumn = eSortColumn.Scanline;
		private bool _sortAscending = true;

		public ctrlEventViewerListView()
		{
			InitializeComponent();
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			if(!IsDesignMode) {
				mnuCopy.InitShortcut(this, nameof(DebuggerShortcutsConfig.Copy));
			}
		}

		public void RefreshViewer()
		{
			_breakpoints = BreakpointManager.Breakpoints;
			EventViewerDisplayOptions options = ConfigManager.Config.Debug.EventViewer.GetInteropOptions();
			DebugEventInfo[] eventInfoArray = DebugApi.GetDebugEvents(options);

			lstEvents.BeginUpdate();
			_debugEvents = new List<DebugEventInfo>(eventInfoArray);
			SortData();
			lstEvents.VirtualListSize = _debugEvents.Count;
			lstEvents.EndUpdate();
		}

		private ListViewItem CreateListViewItem(int index)
		{
			DebugEventInfo evt = _debugEvents[index];
			bool isDma = evt.Operation.Type == MemoryOperationType.DmaWrite || evt.Operation.Type == MemoryOperationType.DmaRead;

			string details = "";
			if(evt.Type == DebugEventType.Breakpoint) {
				if(evt.BreakpointId >= 0 && evt.BreakpointId < _breakpoints.Count) {
					Breakpoint bp = _breakpoints[evt.BreakpointId];
					details += " Type: " + bp.ToReadableType();
					details += " Addresses: " + bp.GetAddressString(true);
					if(bp.Condition.Length > 0) {
						details += " Condition: " + bp.Condition;
					}
				}
			}

			if(isDma) {
				bool indirectHdma = false;
				if((evt.DmaChannel & ctrlEventViewerPpuView.HdmaChannelFlag) != 0) {
					indirectHdma = evt.DmaChannelInfo.HdmaIndirectAddressing;
					details += "HDMA #" + (evt.DmaChannel & 0x07).ToString();
					details += indirectHdma ? " (indirect)" : "";
				} else {
					details += "DMA #" + (evt.DmaChannel & 0x07).ToString();
				}

				int aBusAddress;
				if(indirectHdma) {
					aBusAddress = (evt.DmaChannelInfo.SrcBank << 16) | evt.DmaChannelInfo.TransferSize;
				} else {
					aBusAddress = (evt.DmaChannelInfo.SrcBank << 16) | evt.DmaChannelInfo.SrcAddress;
				}

				if(!evt.DmaChannelInfo.InvertDirection) {
					details += " - $" + aBusAddress.ToString("X4") + " -> $" + (0x2100 | evt.DmaChannelInfo.DestAddress).ToString("X4");
				} else {
					details += " - $" + aBusAddress.ToString("X4") + " <- $" + (0x2100 | evt.DmaChannelInfo.DestAddress).ToString("X4");
				}
			}

			return new ListViewItem(new string[] {
				evt.ProgramCounter.ToString("X4"),
				evt.Scanline.ToString(),
				evt.Cycle.ToString(),
				evt.Type.ToString(),
				evt.Type == DebugEventType.Register ? ("$" + evt.Operation.Address.ToString("X4")) : "",
				evt.Type == DebugEventType.Register ? ("$" + evt.Operation.Value.ToString("X2")) : "",
				details
			});
		}

		private void lstEvents_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
		{
			e.Item = CreateListViewItem(e.ItemIndex);	
		}

		private void lstEvents_ColumnClick(object sender, ColumnClickEventArgs e)
		{
			eSortColumn sortColumn = (eSortColumn)e.Column;
			if(sortColumn == _sortColumn) {
				_sortAscending = !_sortAscending;
			} else {
				_sortColumn = sortColumn;
				_sortAscending = true;
			}

			lstEvents.BeginUpdate();
			SortData();
			lstEvents.VirtualListSize = _debugEvents.Count;
			lstEvents.EndUpdate();
		}

		private void SortData()
		{
			_debugEvents.Sort((DebugEventInfo a, DebugEventInfo b) => {
				int result = 0;
				switch(_sortColumn) {
					case eSortColumn.PC: result = ((int)a.ProgramCounter - (int)b.ProgramCounter); break;
					case eSortColumn.Scanline: result = ((int)a.Scanline - (int)b.Scanline); break;
					case eSortColumn.Cycle: result = ((int)a.Cycle - (int)b.Cycle); break;
					case eSortColumn.Type: result = ((int)a.Type - (int)b.Type); break;
					case eSortColumn.Address: result = ((int)a.Operation.Address - (int)b.Operation.Address); break;
					case eSortColumn.Value: result = ((int)a.Operation.Value - (int)b.Operation.Value); break;
				}

				if(result == 0) {
					result = ((int)a.Cycle - (int)b.Cycle);
				}
				return _sortAscending ? result : -result;
			});
		}

		private void CopyList()
		{
			StringBuilder sb = new StringBuilder();
			for(int i = 0; i < _debugEvents.Count; i++) {
				foreach(ListViewItem.ListViewSubItem subItem in CreateListViewItem(i).SubItems) {
					sb.Append(subItem.Text);
					sb.Append("\t");
				}
				sb.AppendLine();
			}
			Clipboard.SetText(sb.ToString());
		}
		
		private void mnuCopy_Click(object sender, EventArgs e)
		{
			CopyList();
		}
		
		private enum eSortColumn
		{
			PC,
			Scanline,
			Cycle,
			Type,
			Address,
			Value
		}
	}
}
