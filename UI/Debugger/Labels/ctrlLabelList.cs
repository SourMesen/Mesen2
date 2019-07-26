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
using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using Mesen.GUI.Debugger.Labels;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlLabelList : BaseControl
	{
		//public event EventHandler OnFindOccurrence;
		//public event GoToDestinationEventHandler OnLabelSelected;

		private List<ListViewItem> _listItems = new List<ListViewItem>();
		private int _sortColumn = 0;
		private bool _descSort = false;
		private CpuType _cpuType = CpuType.Cpu;

		public CpuType CpuType
		{
			get { return _cpuType; }
			set { _cpuType = value; }
		}

		public ctrlLabelList()
		{
			InitializeComponent();
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			if(!IsDesignMode) {
				mnuShowComments.Checked = ConfigManager.Config.Debug.Debugger.ShowCommentsInLabelList;
				InitShortcuts();
				UpdateLabelList();
				LabelManager.OnLabelUpdated += OnLabelUpdated;
			}
		}

		protected override void OnHandleDestroyed(EventArgs e)
		{
			base.OnHandleDestroyed(e);
			LabelManager.OnLabelUpdated -= OnLabelUpdated;
		}

		private void InitShortcuts()
		{
			mnuAdd.InitShortcut(this, nameof(DebuggerShortcutsConfig.LabelList_Add));
			mnuEdit.InitShortcut(this, nameof(DebuggerShortcutsConfig.LabelList_Edit));
			mnuDelete.InitShortcut(this, nameof(DebuggerShortcutsConfig.LabelList_Delete));

			mnuAddToWatch.InitShortcut(this, nameof(DebuggerShortcutsConfig.LabelList_AddToWatch));
			mnuAddBreakpoint.InitShortcut(this, nameof(DebuggerShortcutsConfig.LabelList_AddBreakpoint));
			mnuFindOccurrences.InitShortcut(this, nameof(DebuggerShortcutsConfig.LabelList_FindOccurrences));

			mnuViewInCpuMemory.InitShortcut(this, nameof(DebuggerShortcutsConfig.LabelList_ViewInCpuMemory));
			mnuViewInMemoryType.InitShortcut(this, nameof(DebuggerShortcutsConfig.LabelList_ViewInMemoryType));
		}
		
		private void OnLabelUpdated(object sender, EventArgs e)
		{
			UpdateLabelList();
		}

		public static void EditLabel(CodeLabel label)
		{
			using(frmEditLabel frm = new frmEditLabel(label)) {
				frm.ShowDialog();
			}
		}

		public int CompareLabels(ListViewItem x, ListViewItem y)
		{
			int result = String.Compare(((ListViewItem)x).SubItems[_sortColumn].Text, ((ListViewItem)y).SubItems[_sortColumn].Text);
			if(result == 0 && (_sortColumn == 0 || _sortColumn == 3)) {
				result = String.Compare(((ListViewItem)x).SubItems[2].Text, ((ListViewItem)y).SubItems[2].Text);
			}
			return result * (_descSort ? -1 : 1);
		}

		private void SortItems()
		{
			_listItems.Sort(CompareLabels);
		}

		public void UpdateLabelListAddresses()
		{
			bool needUpdate = false;

			foreach(ListViewItem item in _listItems) {
				CodeLabel label = (CodeLabel)item.Tag;

				Int32 relativeAddress = label.GetRelativeAddress().Address;
				if(relativeAddress != (Int32)item.SubItems[1].Tag) {
					needUpdate = true;
					if(relativeAddress >= 0) {
						item.SubItems[1].Text = "$" + relativeAddress.ToString("X4");
					} else {
						item.SubItems[1].Text = "[n/a]";
					}
					item.SubItems[1].Tag = relativeAddress;
				}
			}
			if(needUpdate) {
				SortItems();
			}
		}

		public void UpdateLabelList()
		{
			List<CodeLabel> labels = LabelManager.GetLabels(_cpuType);
			List<ListViewItem> items = new List<ListViewItem>(labels.Count);

			foreach(CodeLabel label in labels) {
				if((label.Label.Length > 0 || ConfigManager.Config.Debug.Debugger.ShowCommentsInLabelList)) {
					ListViewItem item = new ListViewItem(label.Label);

					string prefix = string.Empty;
					switch(label.MemoryType) {
						case SnesMemoryType.PrgRom: prefix = "PRG: $"; break;
						case SnesMemoryType.Register: prefix = "REG: $"; break;
						case SnesMemoryType.SaveRam: prefix = "SRAM: $"; break;
						case SnesMemoryType.WorkRam: prefix = "WRAM: $"; break;
						case SnesMemoryType.SpcRam: prefix = "RAM: $"; break;
						case SnesMemoryType.SpcRom: prefix = "ROM: $"; break;
						case SnesMemoryType.Sa1InternalRam: prefix = "IRAM: $"; break;
						default: throw new Exception("Unsupported type");
					}
					int relAddress = label.GetRelativeAddress().Address;
					item.SubItems.Add(relAddress >= 0 ? "$" + relAddress.ToString("X4") : "-");
					item.SubItems.Add(prefix + label.Address.ToString("X4"));
					item.SubItems.Add(ConfigManager.Config.Debug.Debugger.ShowCommentsInLabelList ? label.Comment : "");
					item.Tag = label;
					
					items.Add(item);
				}
			}

			_listItems = items;
			SortItems();

			lstLabels.BeginUpdate();
			lstLabels.VirtualMode = true;
			lstLabels.VirtualListSize = items.Count;
			lstLabels.EndUpdate();

			colComment.AutoResize(ColumnHeaderAutoResizeStyle.ColumnContent);
			if(!ConfigManager.Config.Debug.Debugger.ShowCommentsInLabelList) {
				colComment.Width = 0;
			}
		}

		private ListViewItem GetSelectedItem()
		{
			return _listItems[lstLabels.SelectedIndices[0]];
		}

		private CodeLabel GetSelectedLabel()
		{
			return _listItems[lstLabels.SelectedIndices[0]].Tag as CodeLabel;
		}

		private void lstLabels_DoubleClick(object sender, EventArgs e)
		{
			if(lstLabels.SelectedIndices.Count > 0) {
				CodeLabel label = (CodeLabel)GetSelectedLabel();
				int relAddress = label.GetRelativeAddress().Address;
				if(relAddress >= 0) {
					DebugWindowManager.OpenDebugger(_cpuType).GoToAddress(relAddress);
				}
			}
		}
		
		private void lstLabels_SelectedIndexChanged(object sender, EventArgs e)
		{
			mnuDelete.Enabled = lstLabels.SelectedIndices.Count > 0;
			mnuEdit.Enabled = lstLabels.SelectedIndices.Count == 1;
			mnuFindOccurrences.Enabled = lstLabels.SelectedIndices.Count == 1;
			mnuAddToWatch.Enabled = lstLabels.SelectedIndices.Count == 1;
			mnuAddBreakpoint.Enabled = lstLabels.SelectedIndices.Count == 1;

			if(lstLabels.SelectedIndices.Count == 1) {
				CodeLabel label = GetSelectedLabel();

				bool availableInCpuMemory = label.GetRelativeAddress().Address >= 0;
				mnuViewInCpuMemory.Enabled = availableInCpuMemory;

				bool showViewInMemoryType = label.MemoryType != SnesMemoryType.Register && label.MemoryType != SnesMemoryType.SpcRom && label.MemoryType != SnesMemoryType.SpcRam;
				if(showViewInMemoryType) {
					mnuViewInMemoryType.Text = "View in " + ResourceHelper.GetEnumText(label.MemoryType);
					mnuViewInMemoryType.Visible = true;
				} else {
					mnuViewInMemoryType.Visible = false;
				}
			} else {
				mnuViewInCpuMemory.Enabled = false;
				mnuViewInMemoryType.Visible = false;
			}
		}

		private void mnuDelete_Click(object sender, EventArgs e)
		{
			if(lstLabels.SelectedIndices.Count > 0) {
				int topIndex = lstLabels.TopItem.Index;
				int lastSelectedIndex = lstLabels.SelectedIndices[lstLabels.SelectedIndices.Count - 1];
				List<int> selectedIndexes = new List<int>(lstLabels.SelectedIndices.Cast<int>().ToList());
				for(int i = selectedIndexes.Count - 1; i >= 0; i--) {
					CodeLabel label = (CodeLabel)_listItems[selectedIndexes[i]].Tag;
					LabelManager.DeleteLabel(label, i == 0);
				}
				
				//Reposition scroll bar and selected/focused item
				if(lstLabels.Items.Count > topIndex) {
					lstLabels.TopItem = lstLabels.Items[topIndex];
				}
				if(lastSelectedIndex < lstLabels.Items.Count) {
					lstLabels.Items[lastSelectedIndex].Selected = true;
				} else if(lstLabels.Items.Count > 0) {
					lstLabels.Items[lstLabels.Items.Count - 1].Selected = true;
				}
				if(lstLabels.SelectedIndices.Count > 0) {
					GetSelectedItem().Focused = true;
				}
			}
		}

		private void mnuAdd_Click(object sender, EventArgs e)
		{
			CodeLabel newLabel = new CodeLabel() {
				Address = 0,
				MemoryType = _cpuType == CpuType.Spc ? SnesMemoryType.SpcRam : SnesMemoryType.PrgRom,
				Label = "",
				Comment = ""
			};

			EditLabel(newLabel);
		}

		private void mnuEdit_Click(object sender, EventArgs e)
		{
			if(lstLabels.SelectedIndices.Count > 0) {
				EditLabel(GetSelectedLabel());
			}
		}

		private void mnuFindOccurrences_Click(object sender, EventArgs e)
		{
			//TODO
			//OnFindOccurrence?.Invoke(GetSelectedItem().SubItems[1].Tag, null);
		}

		private void lstLabels_ColumnClick(object sender, ColumnClickEventArgs e)
		{
			lstLabels.BeginUpdate();
			if(_sortColumn == e.Column) {
				_descSort = !_descSort;
			}
			_sortColumn = e.Column;
			SortItems();
			lstLabels.EndUpdate();
		}

		private void mnuAddBreakpoint_Click(object sender, EventArgs e)
		{
			if(lstLabels.SelectedIndices.Count > 0) {
				CodeLabel label = GetSelectedLabel();
				BreakpointManager.AddBreakpoint(new Breakpoint() {
					//TODO LABELS, fix memory type for registers
					CpuType = _cpuType,
					MemoryType = label.MemoryType == SnesMemoryType.Register ? SnesMemoryType.CpuMemory : label.MemoryType,
					BreakOnExec = true,
					BreakOnRead = true,
					BreakOnWrite = true,
					Address = label.Address,
					StartAddress = label.Address,
					EndAddress = label.Address,
					AddressType = BreakpointAddressType.SingleAddress
				});
			}
		}

		private void mnuAddToWatch_Click(object sender, EventArgs e)
		{
			if(lstLabels.SelectedIndices.Count > 0) {
				WatchManager.GetWatchManager(_cpuType).AddWatch("[" + GetSelectedLabel().Label + "]");
			}			
		}

		private void mnuShowComments_Click(object sender, EventArgs e)
		{
			ConfigManager.Config.Debug.Debugger.ShowCommentsInLabelList = mnuShowComments.Checked;
			ConfigManager.ApplyChanges();
			this.UpdateLabelList();
		}

		private void lstLabels_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
		{
			e.Item = _listItems[e.ItemIndex];
		}

		private void lstLabels_SearchForVirtualItem(object sender, SearchForVirtualItemEventArgs e)
		{
			for(int i = 0; i < _listItems.Count; i++) {
				if(_listItems[i].Text.StartsWith(e.Text, StringComparison.InvariantCultureIgnoreCase)) {
					e.Index = i;
					return;
				}
			}
		}

		private void mnuViewInCpuMemory_Click(object sender, EventArgs e)
		{
			if(lstLabels.SelectedIndices.Count == 1) {
				CodeLabel label = GetSelectedLabel();
				AddressInfo relAddress = label.GetRelativeAddress();
				if(relAddress.Address >= 0) {
					DebugWindowManager.OpenMemoryViewer(relAddress);
				}
			}
		}

		private void mnuViewInMemoryType_Click(object sender, EventArgs e)
		{
			if(lstLabels.SelectedIndices.Count == 1) {
				CodeLabel label = GetSelectedLabel();
				if(label.MemoryType != SnesMemoryType.Register) {
					DebugWindowManager.OpenMemoryViewer(label.GetAbsoluteAddress());
				}
			}
		}
	}
}
