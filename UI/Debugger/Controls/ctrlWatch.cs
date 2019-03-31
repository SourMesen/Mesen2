using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;
using Mesen.GUI.Controls;
using System.Text.RegularExpressions;
using System.Globalization;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class ctrlWatch : BaseControl
	{
		private Color _updatedColor = Color.Red;
		private Color _normalColor = SystemColors.ControlText;

		private static Regex _watchAddressOrLabel = new Regex(@"^(\[|{)(\s*((\$[0-9A-Fa-f]+)|(\d+)|([@_a-zA-Z0-9]+)))\s*[,]{0,1}\d*\s*(\]|})$", RegexOptions.Compiled);

		private int _previousMaxLength = -1;
		private int _selectedAddress = -1;
		//private CodeLabel _selectedLabel = null;
		private List<WatchValueInfo> _previousValues = new List<WatchValueInfo>();

		private bool _isEditing = false;
		ListViewItem _keyDownItem = null;

		public ctrlWatch()
		{
			InitializeComponent();

			this.DoubleBuffered = true;
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			if(!IsDesignMode) {
				WatchManager.WatchChanged += WatchManager_WatchChanged;
				mnuRemoveWatch.InitShortcut(this, nameof(DebuggerShortcutsConfig.WatchList_Delete));
				mnuEditInMemoryViewer.InitShortcut(this, nameof(DebuggerShortcutsConfig.CodeWindow_EditInMemoryViewer));
				mnuViewInDisassembly.InitShortcut(this, nameof(DebuggerShortcutsConfig.MemoryViewer_ViewInDisassembly));
				mnuMoveUp.InitShortcut(this, nameof(DebuggerShortcutsConfig.WatchList_MoveUp));
				mnuMoveDown.InitShortcut(this, nameof(DebuggerShortcutsConfig.WatchList_MoveDown));
			}
		}

		public string GetTooltipText()
		{
			return (
				frmBreakpoint.GetConditionTooltip(true) + Environment.NewLine + Environment.NewLine +
				"Additionally, the watch window supports a syntax to display X bytes starting from a specific address. e.g:" + Environment.NewLine +
				"[$10, 16]: Display 16 bytes starting from address $10" + Environment.NewLine +
				"[MyLabel, 4]: Display 4 bytes starting from the address the specified label (MyLabel) refers to"
			);
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			if(lstWatch.SelectedItems.Count > 0) {
				//Used to prevent a Mono issue where pressing a key will change the selected item before we get a chance to edit it
				_keyDownItem = lstWatch.SelectedItems[0];

				if((_isEditing && keyData == Keys.Escape) || keyData == Keys.Enter) {
					if(keyData == Keys.Enter) {
						if(_isEditing) {
							ApplyEdit();
						} else {
							StartEdit(lstWatch.SelectedItems[0].Text);
						}
					} else if(keyData == Keys.Escape) {
						CancelEdit();
					}
					return true;
				}
			} else {
				_keyDownItem = null;
			}

			UpdateActions();
			return base.ProcessCmdKey(ref msg, keyData);
		}

		private void contextMenuWatch_Opening(object sender, CancelEventArgs e)
		{
			UpdateActions();
		}

		private void WatchManager_WatchChanged(object sender, EventArgs e)
		{
			if(this.InvokeRequired) {
				this.BeginInvoke((Action)(() => this.UpdateWatch()));
			} else {
				this.UpdateWatch();
			}
		}

		public void UpdateWatch(bool autoResizeColumns = true)
		{
			List<WatchValueInfo> watchContent = WatchManager.GetWatchContent(_previousValues);
			_previousValues = watchContent;

			bool updating = false;
			if(watchContent.Count != lstWatch.Items.Count - 1) {
				int currentFocus = lstWatch.FocusedItem?.Selected == true ? (lstWatch.FocusedItem?.Index ?? -1) : -1;
				lstWatch.BeginUpdate();
				lstWatch.Items.Clear();

				List<ListViewItem> itemsToAdd = new List<ListViewItem>();
				foreach(WatchValueInfo watch in watchContent) {
					ListViewItem item = new ListViewItem(watch.Expression);
					item.UseItemStyleForSubItems = false;
					item.SubItems.Add(watch.Value).ForeColor = watch.HasChanged ? _updatedColor : _normalColor;
					itemsToAdd.Add(item);
				}
				var lastItem = new ListViewItem("");
				lastItem.SubItems.Add("");
				itemsToAdd.Add(lastItem);
				lstWatch.Items.AddRange(itemsToAdd.ToArray());
				if(currentFocus >= 0 && currentFocus < lstWatch.Items.Count) {
					SetSelectedItem(currentFocus);
				}
				updating = true;
			} else {
				for(int i = 0; i < watchContent.Count; i++) {
					ListViewItem item = lstWatch.Items[i];
					bool needUpdate = (
						item.SubItems[0].Text != watchContent[i].Expression ||
						item.SubItems[1].Text != watchContent[i].Value ||
						item.SubItems[1].ForeColor != (watchContent[i].HasChanged ? _updatedColor : _normalColor)
					);
					if(needUpdate) {
						updating = true;
						item.SubItems[0].Text = watchContent[i].Expression;
						item.SubItems[1].Text = watchContent[i].Value;
						item.SubItems[1].ForeColor = watchContent[i].HasChanged ? _updatedColor : _normalColor;
					}
				}
			}

			if(updating) {
				if(watchContent.Count > 0) {
					int maxLength = watchContent.Select(info => info.Value.Length).Max();
					if(_previousMaxLength != maxLength) {
						if(autoResizeColumns) {
							lstWatch.AutoResizeColumn(1, ColumnHeaderAutoResizeStyle.ColumnContent);
						}
						if(colValue.Width < 100) {
							colValue.Width = 100;
						}
						_previousMaxLength = maxLength;
					}
				}
				lstWatch.EndUpdate();
			}
		}
				
		private void lstWatch_SelectedIndexChanged(object sender, EventArgs e)
		{
			mnuRemoveWatch.Enabled = lstWatch.SelectedItems.Count >= 1;
			UpdateActions();
		}

		private void UpdateActions()
		{
			mnuHexDisplay.Checked = ConfigManager.Config.Debug.Debugger.WatchFormat == WatchFormatStyle.Hex;
			mnuDecimalDisplay.Checked = ConfigManager.Config.Debug.Debugger.WatchFormat == WatchFormatStyle.Signed;
			mnuBinaryDisplay.Checked = ConfigManager.Config.Debug.Debugger.WatchFormat == WatchFormatStyle.Binary;
			mnuRowDisplayFormat.Enabled = lstWatch.SelectedItems.Count > 0;

			mnuEditInMemoryViewer.Enabled = false;
			mnuViewInDisassembly.Enabled = false;
			mnuMoveUp.Enabled = false;
			mnuMoveDown.Enabled = false;

			if(lstWatch.SelectedItems.Count == 1) {
				Match match = _watchAddressOrLabel.Match(lstWatch.SelectedItems[0].Text);
				if(match.Success) {
					string address = match.Groups[3].Value;

					if(address[0] >= '0' && address[0] <= '9' || address[0] == '$') {
						//CPU Address
						_selectedAddress = Int32.Parse(address[0] == '$' ? address.Substring(1) : address, address[0] == '$' ? NumberStyles.AllowHexSpecifier : NumberStyles.None);
						//TODO
						//_selectedLabel = null;
						mnuEditInMemoryViewer.Enabled = true;
						mnuViewInDisassembly.Enabled = true;
					} else {
						//Label
						_selectedAddress = -1;
						//TODO
						/*_selectedLabel = LabelManager.GetLabel(address);
						if(_selectedLabel != null) {
							mnuEditInMemoryViewer.Enabled = true;
							mnuViewInDisassembly.Enabled = true;
						}*/
					}
				}

				mnuMoveUp.Enabled = lstWatch.SelectedIndices[0] > 0 && lstWatch.SelectedIndices[0] < lstWatch.Items.Count - 1;
				mnuMoveDown.Enabled = lstWatch.SelectedIndices[0] < lstWatch.Items.Count - 2;
			}
		}

		private void mnuRemoveWatch_Click(object sender, EventArgs e)
		{
			if(lstWatch.SelectedItems.Count >= 1) {
				var itemsToRemove = new List<int>();
				foreach(ListViewItem item in lstWatch.SelectedItems) {
					itemsToRemove.Add(item.Index);
				}
				WatchManager.RemoveWatch(itemsToRemove.ToArray());
			}
		}

		private void mnuViewInDisassembly_Click(object sender, EventArgs e)
		{
			//TODO
			/*if(lstWatch.SelectedItems.Count != 1) {
				return;
			}

			if(_selectedAddress >= 0) {
				DebugWindowManager.GetDebugger().ScrollToAddress(_selectedAddress);
			} else if(_selectedLabel != null) {
				int relAddress = _selectedLabel.GetRelativeAddress();
				if(relAddress >= 0) {
					DebugWindowManager.GetDebugger().ScrollToAddress(relAddress);
				}
			}*/
		}

		private void mnuEditInMemoryViewer_Click(object sender, EventArgs e)
		{
			//TODO
			/*if(lstWatch.SelectedItems.Count != 1) {
				return;
			}

			if(_selectedAddress >= 0) {
				DebugWindowManager.OpenMemoryViewer(_selectedAddress, DebugMemoryType.CpuMemory);
			} else if(_selectedLabel != null) {
				DebugWindowManager.OpenMemoryViewer((int)_selectedLabel.Address, _selectedLabel.AddressType.ToMemoryType());
			}*/
		}

		private void StartEdit(string text, ListViewItem selectedItem = null)
		{
			if(selectedItem == null) {
				selectedItem = lstWatch.SelectedItems[0];
			}

			SetSelectedItem(selectedItem.Index);

			txtEdit.Location = selectedItem.Position;
			txtEdit.Width = selectedItem.Bounds.Width;
			txtEdit.Text = text;
			txtEdit.SelectionLength = 0;
			txtEdit.SelectionStart = text.Length;
			txtEdit.Visible = true;
			txtEdit.Focus();
			_isEditing = true;
		}

		private void lstWatch_Click(object sender, EventArgs e)
		{
			if(lstWatch.SelectedItems.Count == 1 && string.IsNullOrWhiteSpace(lstWatch.SelectedItems[0].Text)) {
				StartEdit("");
			}
		}

		private void lstWatch_DoubleClick(object sender, EventArgs e)
		{
			if(lstWatch.SelectedItems.Count == 1) {
				StartEdit(lstWatch.SelectedItems[0].Text);
			}
		}

		private void ApplyEdit()
		{
			if(lstWatch.SelectedItems.Count > 0) {
				lstWatch.SelectedItems[0].Text = txtEdit.Text;
				WatchManager.UpdateWatch(lstWatch.SelectedIndices[0], txtEdit.Text);
			}
			lstWatch.Focus();
		}

		private void CancelEdit()
		{
			if(lstWatch.SelectedItems.Count > 0) {
				txtEdit.Text = lstWatch.SelectedItems[0].Text;
			}
			lstWatch.Focus();
		}

		private void lstWatch_KeyPress(object sender, KeyPressEventArgs e)
		{
			if(lstWatch.SelectedItems.Count > 0) {
				if(e.KeyChar >= ' ' && e.KeyChar < 128) {
					e.Handled = true;
					StartEdit(e.KeyChar.ToString(), _keyDownItem);
					_keyDownItem = null;
				}
			}
		}

		private void txtEdit_Leave(object sender, EventArgs e)
		{
			_isEditing = false;
			txtEdit.Visible = false;
			lstWatch.Focus();
			ApplyEdit();
		}

		private void mnuMoveUp_Click(object sender, EventArgs e)
		{
			MoveUp(false);
		}

		private void mnuMoveDown_Click(object sender, EventArgs e)
		{
			MoveDown();
		}

		private void SetSelectedItem(int index)
		{
			if(index < lstWatch.Items.Count) {
				lstWatch.FocusedItem = lstWatch.Items[index];
				foreach(ListViewItem item in lstWatch.Items) {
					item.Selected = lstWatch.FocusedItem == item;
				}
			}
		}

		private void MoveUp(bool fromUpDownArrow)
		{
			if(lstWatch.SelectedIndices.Count == 0) {
				return;
			}

			int index = lstWatch.SelectedIndices[0];
			if(Program.IsMono && fromUpDownArrow) {
				//Mono appears to move the selection up before processing this
				index++;
			}

			if(index > 0 && index < lstWatch.Items.Count - 1) {
				string currentEntry = lstWatch.Items[index].SubItems[0].Text;
				string entryAbove = lstWatch.Items[index - 1].SubItems[0].Text;
				SetSelectedItem(index - 1);
				WatchManager.UpdateWatch(index - 1, currentEntry);
				WatchManager.UpdateWatch(index, entryAbove);
			} else {
				SetSelectedItem(index);
			}
		}

		private void MoveDown()
		{
			if(lstWatch.SelectedIndices.Count == 0) {
				return;
			}

			int index = lstWatch.SelectedIndices[0];
			if(index < lstWatch.Items.Count - 2) {
				string currentEntry = lstWatch.Items[index].SubItems[0].Text;
				string entryBelow = lstWatch.Items[index + 1].SubItems[0].Text;
				SetSelectedItem(index + 1);
				WatchManager.UpdateWatch(index + 1, currentEntry);
				WatchManager.UpdateWatch(index, entryBelow);
			} else {
				SetSelectedItem(index);
			}
		}

		private void lstWatch_OnMoveUpDown(Keys keyData, ref bool processed)
		{
			if(keyData == ConfigManager.Config.Debug.Shortcuts.WatchList_MoveUp) {
				MoveUp(true);
				processed = true;
			} else if(keyData == ConfigManager.Config.Debug.Shortcuts.WatchList_MoveDown) {
				MoveDown();
				processed = true;
			}
		}

		private void mnuImport_Click(object sender, EventArgs e)
		{
			using(OpenFileDialog ofd = new OpenFileDialog()) {
				ofd.SetFilter("Watch files (*.mwf)|*.mwf");
				if(ofd.ShowDialog() == DialogResult.OK) {
					WatchManager.Import(ofd.FileName);
				}
			}
		}

		private void mnuExport_Click(object sender, EventArgs e)
		{
			using(SaveFileDialog sfd = new SaveFileDialog()) {
				sfd.SetFilter("Watch files (*.mwf)|*.mwf");
				if(sfd.ShowDialog() == DialogResult.OK) {
					WatchManager.Export(sfd.FileName);
				}
			}
		}

		private void mnuHexDisplay_Click(object sender, EventArgs e)
		{
			ConfigManager.Config.Debug.Debugger.WatchFormat = WatchFormatStyle.Hex;
			ConfigManager.ApplyChanges();
			UpdateWatch();
		}

		private void mnuDecimalDisplay_Click(object sender, EventArgs e)
		{
			ConfigManager.Config.Debug.Debugger.WatchFormat = WatchFormatStyle.Signed;
			ConfigManager.ApplyChanges();
			UpdateWatch();
		}

		private void mnuBinaryDisplay_Click(object sender, EventArgs e)
		{
			ConfigManager.Config.Debug.Debugger.WatchFormat = WatchFormatStyle.Binary;
			ConfigManager.ApplyChanges();
			UpdateWatch();
		}

		private string GetFormatString(WatchFormatStyle format, int byteLength)
		{
			string formatString = ", ";
			switch(format) {
				case WatchFormatStyle.Binary: formatString += "B"; break;
				case WatchFormatStyle.Hex: formatString += "H"; break;
				case WatchFormatStyle.Signed: formatString += "S"; break;
				case WatchFormatStyle.Unsigned: formatString += "U"; break;
				default: throw new Exception("Unsupported type");
			}
			if(byteLength > 1) {
				formatString += byteLength.ToString();
			}
			return formatString;
		}

		private void SetSelectionFormat(WatchFormatStyle format, int byteLength)
		{
			SetSelectionFormat(GetFormatString(format, byteLength));
		}

		private void SetSelectionFormat(string formatString)
		{
			List<string> entries = WatchManager.WatchEntries;
			foreach(int i in lstWatch.SelectedIndices) {
				if(i < entries.Count) {
					Match match = WatchManager.FormatSuffixRegex.Match(entries[i]);
					if(match.Success) {
						WatchManager.UpdateWatch(i, match.Groups[1].Value + formatString);
					} else {
						WatchManager.UpdateWatch(i, entries[i] + formatString);
					}					
				}
			}
		}

		private void mnuRowBinary_Click(object sender, EventArgs e)
		{
			SetSelectionFormat(WatchFormatStyle.Binary, 1);
		}

		private void mnuRowHex1_Click(object sender, EventArgs e)
		{
			SetSelectionFormat(WatchFormatStyle.Hex, 1);
		}

		private void mnuRowHex2_Click(object sender, EventArgs e)
		{
			SetSelectionFormat(WatchFormatStyle.Hex, 2);
		}

		private void mnuRowHex3_Click(object sender, EventArgs e)
		{
			SetSelectionFormat(WatchFormatStyle.Hex, 3);
		}

		private void mnuRowSigned1_Click(object sender, EventArgs e)
		{
			SetSelectionFormat(WatchFormatStyle.Signed, 1);
		}

		private void mnuRowSigned2_Click(object sender, EventArgs e)
		{
			SetSelectionFormat(WatchFormatStyle.Signed, 2);
		}

		private void mnuRowSigned3_Click(object sender, EventArgs e)
		{
			SetSelectionFormat(WatchFormatStyle.Unsigned, 1);
		}

		private void mnuRowUnsigned_Click(object sender, EventArgs e)
		{
			SetSelectionFormat(WatchFormatStyle.Unsigned, 1);
		}

		private void mnuRowClearFormat_Click(object sender, EventArgs e)
		{
			SetSelectionFormat("");
		}
	}
}
