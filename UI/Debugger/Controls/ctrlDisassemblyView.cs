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
using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Code;
using Mesen.GUI.Debugger.Labels;
using Mesen.GUI.Debugger.Integration;
using Mesen.GUI.Debugger.Workspace;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlDisassemblyView : BaseControl
	{
		private BaseStyleProvider _styleProvider;
		private IDisassemblyManager _manager;
		private ISymbolProvider _symbolProvider;
		private bool _inSourceView = false;

		public ctrlDisassemblyView()
		{
			InitializeComponent();
			if(IsDesignMode) {
				return;
			}

			DebugWorkspaceManager.SymbolProviderChanged += DebugWorkspaceManager_SymbolProviderChanged;
			BreakpointManager.BreakpointsChanged += BreakpointManager_BreakpointsChanged;
			LabelManager.OnLabelUpdated += OnLabelUpdated;
		}

		protected override void OnHandleCreated(EventArgs e)
		{
			base.OnHandleCreated(e);
			if(IsDesignMode) {
				return;
			}

			InitShortcuts();
		}

		protected override void OnHandleDestroyed(EventArgs e)
		{
			base.OnHandleDestroyed(e);
			DebugWorkspaceManager.SymbolProviderChanged -= DebugWorkspaceManager_SymbolProviderChanged;
			LabelManager.OnLabelUpdated -= OnLabelUpdated;
			BreakpointManager.BreakpointsChanged -= BreakpointManager_BreakpointsChanged;
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			UpdateContextMenu();
			return base.ProcessCmdKey(ref msg, keyData);
		}

		private void DebugWorkspaceManager_SymbolProviderChanged(ISymbolProvider symbolProvider)
		{
			_symbolProvider = symbolProvider;

			if(_symbolProvider == null && _inSourceView) {
				ToggleView();
			}

			if(_manager?.Provider != null) {
				UpdateSourceFileDropdown();
				UpdateCode();
			}
		}

		protected override void OnInvalidated(InvalidateEventArgs e)
		{
			base.OnInvalidated(e);
			ctrlCode.Invalidate();
		}

		private void OnLabelUpdated(object sender, EventArgs e)
		{
			ctrlCode.Invalidate();
		}

		private void BreakpointManager_BreakpointsChanged(object sender, EventArgs e)
		{
			ctrlCode.Invalidate();
		}

		public void Initialize(IDisassemblyManager manager, BaseStyleProvider styleProvider)
		{
			_manager = manager;
			_styleProvider = styleProvider;
			_symbolProvider = DebugWorkspaceManager.GetSymbolProvider();

			ctrlCode.StyleProvider = _styleProvider;
			ctrlCode.ShowContentNotes = false;
			ctrlCode.ShowMemoryValues = true;
			ctrlCode.ExtendedMarginWidth = manager.ByteCodeSize * 4;
			ctrlCode.AddressSize = manager.AddressSize;

			UpdateSourceFileDropdown();
			_manager.RefreshCode(_inSourceView ? _symbolProvider : null, _inSourceView ? cboSourceFile.SelectedItem as SourceFileInfo : null);
		}

		private void UpdateSourceFileDropdown()
		{
			mnuSwitchView.Enabled = false;
			mnuSwitchView.Visible = false;
			sepSwitchView.Visible = false;
			cboSourceFile.Visible = false;
			lblSourceFile.Visible = false;

			if(_manager.AllowSourceView && _symbolProvider != null) {
				cboSourceFile.BeginUpdate();
				cboSourceFile.Items.Clear();
				cboSourceFile.Sorted = false;
				if(_symbolProvider != null) {
					foreach(SourceFileInfo file in _symbolProvider.SourceFiles) {
						if(file.Data != null && file.Data.Length > 0 && !file.Name.ToLower().EndsWith(".chr")) {
							cboSourceFile.Items.Add(file);
						}
					}
				}
				cboSourceFile.Sorted = true;
				cboSourceFile.EndUpdate();

				if(cboSourceFile.Items.Count > 0) {
					cboSourceFile.SelectedIndex = 0;
				}

				mnuSwitchView.Enabled = true;
				mnuSwitchView.Visible = true;
				sepSwitchView.Visible = true;

				if(_inSourceView) {
					lblSourceFile.Visible = true;
					cboSourceFile.Visible = true;
				}
			}
		}

		private void InitShortcuts()
		{
			mnuToggleBreakpoint.InitShortcut(this, nameof(DebuggerShortcutsConfig.CodeWindow_ToggleBreakpoint));
			mnuToggleBreakpoint.Click += (s, e) => ToggleBreakpoint();

			mnuEditLabel.InitShortcut(this, nameof(DebuggerShortcutsConfig.CodeWindow_EditLabel));
			mnuEditLabel.Click += (s, e) => EditLabel(GetActionTarget());

			mnuEditInMemoryTools.InitShortcut(this, nameof(DebuggerShortcutsConfig.CodeWindow_EditInMemoryViewer));
			mnuEditInMemoryTools.Click += (s, e) => EditInMemoryTools(GetActionTarget());

			mnuEditSelectedCode.InitShortcut(this, nameof(DebuggerShortcutsConfig.CodeWindow_EditSelectedCode));
			mnuEditSelectedCode.Click += (s, e) => EditSelectedCode();

			mnuAddToWatch.InitShortcut(this, nameof(DebuggerShortcutsConfig.LabelList_AddToWatch));

			mnuMarkAsCode.InitShortcut(this, nameof(DebuggerShortcutsConfig.MarkAsCode));
			mnuMarkAsCode.Click += (s, e) => MarkSelectionAs(CdlFlags.Code);
			mnuMarkAsData.InitShortcut(this, nameof(DebuggerShortcutsConfig.MarkAsData));
			mnuMarkAsData.Click += (s, e) => MarkSelectionAs(CdlFlags.Data);
			mnuMarkAsUnidentifiedData.InitShortcut(this, nameof(DebuggerShortcutsConfig.MarkAsUnidentified));
			mnuMarkAsUnidentifiedData.Click += (s, e) => MarkSelectionAs(CdlFlags.None);

			mnuSwitchView.InitShortcut(this, nameof(DebuggerShortcutsConfig.CodeWindow_SwitchView));
			mnuSwitchView.Click += (s, e) => { ToggleView(); };
		}

		private void MarkSelectionAs(CdlFlags type)
		{
			SelectedAddressRange range = GetSelectedAddressRange();
			if(!_inSourceView && range != null && range.Start.Type == SnesMemoryType.PrgRom && range.End.Type == SnesMemoryType.PrgRom) {
				DebugApi.MarkBytesAs((UInt32)range.Start.Address, (UInt32)range.End.Address, type);
				DebugWindowManager.OpenDebugger(CpuType.Cpu)?.RefreshDisassembly();
			}
		}

		class SelectedAddressRange
		{
			public AddressInfo Start;
			public AddressInfo End;
			public string Display;
		}

		private SelectedAddressRange GetSelectedAddressRange()
		{
			SelectedAddressRange range = GetSelectedRelativeAddressRange();

			if(range != null && range.Start.Address >= 0 && range.End.Address >= 0) {
				return new SelectedAddressRange() {
					Start = DebugApi.GetAbsoluteAddress(range.Start),
					End = DebugApi.GetAbsoluteAddress(range.End),
					Display = $"${range.Start.Address.ToString("X4")} - ${range.End.Address.ToString("X4")}"
				};
			}

			return null;
		}

		private SelectedAddressRange GetSelectedRelativeAddressRange()
		{
			int firstLineOfSelection = ctrlCode.SelectionStart;
			int lineCount = ctrlCode.LineCount;

			while(firstLineOfSelection < lineCount && _manager.Provider.GetLineAddress(firstLineOfSelection) < 0) {
				firstLineOfSelection++;
			}
			int firstLineAfterSelection = ctrlCode.SelectionStart + ctrlCode.SelectionLength + 1;
			while(firstLineOfSelection < lineCount && _manager.Provider.GetLineAddress(firstLineAfterSelection) < 0) {
				firstLineAfterSelection++;
			}

			int start = _manager.Provider.GetLineAddress(firstLineOfSelection);
			int end = _manager.Provider.GetLineAddress(firstLineAfterSelection) - 1;

			if(start >= 0 && end >= 0) {
				return new SelectedAddressRange() {
					Start = new AddressInfo() { Address = start, Type = _manager.RelativeMemoryType },
					End = new AddressInfo() { Address = end, Type = _manager.RelativeMemoryType },
					Display = $"${start.ToString("X4")} - ${end.ToString("X4")}"
				};
			}

			return null;
		}

		public void ToggleView()
		{
			_inSourceView = !_inSourceView;
			int prgAddress = ctrlCode.CurrentLine;
			UpdateSourceFileDropdown();
			UpdateCode();
			if(prgAddress >= 0) {
				ScrollToAddress((uint)prgAddress);
			}
		}

		public void SetActiveAddress(int? address)
		{
			if(_styleProvider.ActiveAddress == null && address == null) {
				return;
			}

			_styleProvider.ActiveAddress = address;
			if(address.HasValue && address.Value >= 0) {
				ScrollToAddress((uint)address.Value);
			}

			ctrlCode.Invalidate();
		}

		public void GoToDestination(GoToDestination dest)
		{
			bool bringToFront = true;
			if(dest.RelativeAddress?.Type == _manager.CpuType.ToMemoryType()) {
				ScrollToAddress((uint)dest.RelativeAddress.Value.Address);
			} else if(dest.RelativeAddress != null) {
				DebugWindowManager.OpenDebugger(dest.RelativeAddress.Value.Type.ToCpuType())?.GoToAddress(dest.RelativeAddress.Value.Address);
				bringToFront = false;
			} else if(dest.SourceLocation != null) {
				ScrollToSourceLocation(dest.SourceLocation);
			} else if(dest.File != null) {
				SelectFile(dest.File);
			}

			if(bringToFront) {
				Form parent = this.ParentForm;
				if(parent.WindowState == FormWindowState.Minimized) {
					//Unminimize window if it was minimized
					parent.WindowState = FormWindowState.Normal;
				}
				parent.BringToFront();
			}
		}

		public bool SelectFile(SourceFileInfo file)
		{
			if(!_inSourceView) {
				ToggleView();
			}

			foreach(SourceFileInfo fileInfo in cboSourceFile.Items) {
				if(fileInfo == file) {
					cboSourceFile.SelectedItem = fileInfo;
					return true;
				}
			}

			return false;
		}

		public void ScrollToSourceLocation(SourceCodeLocation location)
		{
			if(SelectFile(location.File)) {
				ctrlCode.ScrollToLineIndex(location.LineNumber);
			}
		}

		public void ScrollToSymbol(SourceSymbol symbol)
		{
			SourceCodeLocation definition = _symbolProvider?.GetSymbolDefinition(symbol);
			if(definition != null) {
				ScrollToSourceLocation(definition);
			}
		}

		public void ScrollToAddress(uint address)
		{
			if(_inSourceView) {
				AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = (int)address, Type = SnesMemoryType.CpuMemory });
				if(absAddress.Address >= 0) {
					SourceCodeLocation line = _symbolProvider?.GetSourceCodeLineInfo(absAddress);
					if(line != null) {
						foreach(SourceFileInfo fileInfo in cboSourceFile.Items) {
							if(line.File == fileInfo) {
								cboSourceFile.SelectedItem = fileInfo;
								ctrlCode.ScrollToLineIndex(line.LineNumber);
								return;
							}
						}
					}
				}
				ToggleView();
			}
			ctrlCode.ScrollToAddress((int)address);
		}

		public void UpdateCode(bool refreshDisassembly = false)
		{
			int centerLineIndex = ctrlCode.GetLineIndexAtPosition(0) + ctrlCode.GetNumberVisibleLines() / 2;
			int centerLineAddress;
			int scrollOffset = -1;
			do {
				//Save the address at the center of the debug view
				centerLineAddress = _manager.Provider.GetLineAddress(centerLineIndex);
				centerLineIndex--;
				scrollOffset++;
			} while(centerLineAddress < 0 && centerLineIndex > 0);

			if(refreshDisassembly) {
				DebugApi.RefreshDisassembly(CpuType.Cpu);
				DebugApi.RefreshDisassembly(CpuType.Spc);
				DebugApi.RefreshDisassembly(CpuType.Sa1);
				DebugApi.RefreshDisassembly(CpuType.Gsu);
				DebugApi.RefreshDisassembly(CpuType.NecDsp);
				DebugApi.RefreshDisassembly(CpuType.Cx4);
			}

			_manager.RefreshCode(_inSourceView ? _symbolProvider : null, _inSourceView ? cboSourceFile.SelectedItem as SourceFileInfo : null);
			ctrlCode.DataProvider = _manager.Provider;

			if(centerLineAddress >= 0) {
				//Scroll to the same address as before, to prevent the code view from changing due to setting or banking changes, etc.
				int lineIndex = _manager.Provider.GetLineIndex((UInt32)centerLineAddress) + scrollOffset;
				ctrlCode.ScrollToLineIndex(lineIndex, eHistoryType.None, false, true);
			}
		}

		public ctrlScrollableTextbox CodeViewer { get { return ctrlCode; } }

		public void GoToActiveAddress()
		{
			if(_styleProvider.ActiveAddress.HasValue) {
				ScrollToAddress((uint)_styleProvider.ActiveAddress.Value);
			}
		}

		public void ToggleBreakpoint()
		{
			ToggleBreakpoint(_manager.Provider.GetLineAddress(ctrlCode.SelectedLine));
		}

		public void EnableDisableBreakpoint()
		{
			EnableDisableBreakpoint(_manager.Provider.GetLineAddress(ctrlCode.SelectedLine));
		}
		
		private void ToggleBreakpoint(int address)
		{
			if(address >= 0) {
				AddressInfo relAddress = new AddressInfo() {
					Address = address,
					Type = _manager.RelativeMemoryType
				};

				AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
				if(absAddress.Address < 0) {
					BreakpointManager.ToggleBreakpoint(relAddress, _manager.CpuType);
				} else {
					BreakpointManager.ToggleBreakpoint(absAddress, _manager.CpuType);
				}
			}
		}

		private void EnableDisableBreakpoint(int address)
		{
			if(address >= 0) {
				AddressInfo relAddress = new AddressInfo() {
					Address = address,
					Type = _manager.RelativeMemoryType
				};

				if(!BreakpointManager.EnableDisableBreakpoint(relAddress, _manager.CpuType)) {
					AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
					BreakpointManager.EnableDisableBreakpoint(absAddress, _manager.CpuType);
				}
			}
		}

		private void EditLabel(LocationInfo location)
		{
			if(location.Symbol != null) {
				//Don't allow edit label on symbols
				return;
			}

			if(location.Label != null) {
				using(frmEditLabel frm = new frmEditLabel(location.Label)) {
					frm.ShowDialog();
				}
			} else if(location.Address >= 0) {
				AddressInfo relAddress = new AddressInfo() {
					Address = location.Address,
					Type = _manager.RelativeMemoryType
				};

				AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
				if(absAddress.Address >= 0) {
					CodeLabel label = LabelManager.GetLabel((uint)absAddress.Address, absAddress.Type);
					if(label == null) {
						label = new CodeLabel() {
							Address = (uint)absAddress.Address,
							MemoryType = absAddress.Type
						};
					}

					using(frmEditLabel frm = new frmEditLabel(label)) {
						frm.ShowDialog();
					}
				}
			}
		}
		
		private void EditInMemoryTools(LocationInfo location)
		{
			if(location.Symbol != null) {
				AddressInfo? address = _symbolProvider.GetSymbolAddressInfo(location.Symbol);
				if(address.HasValue) {
					DebugWindowManager.OpenMemoryViewer(address.Value);
				}
			} else if(location.Address >= 0) {
				AddressInfo relAddress = new AddressInfo() {
					Address = location.Address,
					Type = _manager.RelativeMemoryType
				};

				DebugWindowManager.OpenMemoryViewer(relAddress);
			} else if(location.Label != null) {
				DebugWindowManager.OpenMemoryViewer(location.Label.GetAbsoluteAddress());
			}
		}

		private void GoToLocation(LocationInfo location)
		{
			if(location.Symbol != null) {
				ScrollToSymbol(location.Symbol);
			} else if(location.Address >= 0) {
				ScrollToAddress((uint)location.Address);
			}
		}

		private string GetSelectedCode()
		{
			StringBuilder sb = new StringBuilder();

			bool prevSeparator = false;
			for(int i = ctrlCode.SelectionStart, end = ctrlCode.SelectionStart + ctrlCode.SelectionLength; i <= end; i++) {
				CodeLineData line = _manager.Provider.GetCodeLineData(i);
				if(line.Flags.HasFlag(LineFlags.BlockStart) || line.Flags.HasFlag(LineFlags.BlockEnd)|| line.Flags.HasFlag(LineFlags.SubStart)) {
					if(!prevSeparator) {
						prevSeparator = true;
						sb.AppendLine(";-------");
					}
				} else {
					prevSeparator = false;

					sb.Append(line.Text.PadLeft(line.Indentation / 7 + line.Text.Length, ' '));
					if(line.Comment.Length > 0) {
						sb.Append(" " + line.Comment);
					}
					if(i < end) {
						sb.AppendLine();
					}
				}
			}

			return sb.ToString();
		}

		private void EditSelectedCode()
		{
			SelectedAddressRange range = GetSelectedRelativeAddressRange();

			if(range.Start.Address >= 0 && range.End.Address >= 0 && range.Start.Address <= range.End.Address) {
				int length = range.End.Address - range.Start.Address + 1;
				DebugWindowManager.OpenAssembler(GetSelectedCode(), range.Start.Address, length);
			}
		}

		private LocationInfo GetActionTarget()
		{
			string word = _lastWord;
			int lineIndex;
			if(string.IsNullOrWhiteSpace(word)) {
				word = "$" + _manager.Provider.GetLineAddress(ctrlCode.SelectedLine).ToString("X6");
				lineIndex = ctrlCode.SelectedLine;
			} else {
				lineIndex = ctrlCode.GetLineIndexAtPosition(_lastLocation.Y);
			}

			return _manager.GetLocationInfo(word, lineIndex);
		}

		private void mnuGoToLocation_Click(object sender, EventArgs e)
		{
			GoToLocation(GetActionTarget());
		}

		private void mnuAddToWatch_Click(object sender, EventArgs e)
		{
			LocationInfo location = GetActionTarget();

			if(location.Symbol != null) {
				WatchManager.GetWatchManager(_manager.CpuType).AddWatch("[" + location.Symbol.Name + "]");
			} else if(location.Label != null) {
				string label = location.Label.Label;
				if(location.ArrayIndex.HasValue) {
					label += "+" + location.ArrayIndex.Value.ToString();
				}
				WatchManager.GetWatchManager(_manager.CpuType).AddWatch("[" + label + "]");
			} else if(location.Address >= 0) {
				WatchManager.GetWatchManager(_manager.CpuType).AddWatch("[$" + location.Address.ToString("X" + _manager.AddressSize.ToString()) + "]");
			}
		}

		private void ctrlCode_MouseDown(object sender, MouseEventArgs e)
		{
			if(e.X < 20 && e.Button == MouseButtons.Left) {
				int lineIndex = ctrlCode.GetLineIndexAtPosition(e.Y);
				ToggleBreakpoint(_manager.Provider.GetLineAddress(lineIndex));
			}
			BaseForm.GetPopupTooltip(this.FindForm()).Hide();
		}

		private string _lastWord = null;
		private Point _lastLocation = Point.Empty;
		private void ctrlCode_MouseMove(object sender, MouseEventArgs e)
		{
			_lastLocation = e.Location;

			string word = ctrlCode.GetWordUnderLocation(e.Location).Trim();
			if(word == _lastWord) {
				return;
			}

			Form form = this.FindForm();
			if(word.Length > 0) {
				Dictionary<string, string> values = _manager.GetTooltipData(word, ctrlCode.GetLineIndexAtPosition(e.Y));
				if(values != null) {
					Point tooltipLocation = ctrlCode.GetWordEndPosition(e.Location);
					BaseForm.GetPopupTooltip(form).SetTooltip(form.PointToClient(ctrlCode.PointToScreen(tooltipLocation)), values);
				} else {
					BaseForm.GetPopupTooltip(form).Hide();
				}
			} else {
				BaseForm.GetPopupTooltip(form).Hide();
			}

			_lastWord = word;
		}

		private void ctrlCode_MouseLeave(object sender, EventArgs e)
		{
			BaseForm.GetPopupTooltip(this.FindForm()).Hide();
		}

		private void ctrlCode_TextZoomChanged(object sender, EventArgs e)
		{
			ConfigManager.Config.Debug.Debugger.TextZoom = ctrlCode.TextZoom;
			ConfigManager.ApplyChanges();
		}

		private void cboSourceFile_SelectedIndexChanged(object sender, EventArgs e)
		{
			if(_manager?.Provider != null) {
				UpdateCode();
			}
		}

		public void SetMessage(TextboxMessageInfo msg)
		{
			ctrlCode.SetMessage(msg);
		}

		private void UpdateContextMenu()
		{
			LocationInfo location = GetActionTarget();
			bool active = location.Address >= 0 || location.Label != null || location.Symbol != null;

			string suffix = location.Symbol?.Name ?? location.Label?.Label;
			if(string.IsNullOrWhiteSpace(suffix)) {
				suffix = (location.Address >= 0 ? ("$" + location.Address.ToString("X6")) : "");
			}

			string labelName = "";
			if(suffix.Length > 0) {
				labelName = " (" + suffix + ")";
				if(location.ArrayIndex.HasValue) {
					suffix += "+" + location.ArrayIndex.Value.ToString();
				}
				suffix = " (" + suffix + ")";
			} else if(!active || location.Symbol != null) {
				suffix = "";
				labelName = "";
			}

			bool enableGoToLocation = (location.Address != _manager.Provider.GetLineAddress(ctrlCode.SelectedLine));

			mnuAddToWatch.Text = "Add to Watch" + suffix;
			mnuEditInMemoryTools.Text = "Edit in Memory Tools" + suffix;
			mnuEditLabel.Text = "Edit Label" + labelName;
			mnuGoToLocation.Text = "Go to Location" + (enableGoToLocation ? suffix : "");

			SelectedAddressRange range = GetSelectedAddressRange();
			if(range != null && range.Start.Type == SnesMemoryType.PrgRom && range.End.Type == SnesMemoryType.PrgRom) {
				mnuMarkSelectionAs.Enabled = true;
				mnuMarkSelectionAs.Text = "Mark selection as... (" + range.Display + ")";
			} else {
				mnuMarkSelectionAs.Enabled = false;
				mnuMarkSelectionAs.Text = "Mark selection as...";
			}

			bool showMarkAs = !_inSourceView && (_manager.CpuType == CpuType.Cpu || _manager.CpuType == CpuType.Sa1);
			mnuMarkSelectionAs.Visible = showMarkAs;
			sepMarkSelectionAs.Visible = showMarkAs;
			mnuEditSelectedCode.Visible = showMarkAs;

			mnuAddToWatch.Enabled = active;
			mnuEditInMemoryTools.Enabled = active;
			mnuEditLabel.Enabled = active && location.Symbol == null;
			mnuGoToLocation.Enabled = active && enableGoToLocation;
		}

		private void ctxMenu_Opening(object sender, CancelEventArgs e)
		{
			UpdateContextMenu();
		}

		private void ctxMenu_Closing(object sender, ToolStripDropDownClosingEventArgs e)
		{
			mnuAddToWatch.Enabled = true;
			mnuEditInMemoryTools.Enabled = true;
			mnuEditLabel.Enabled = true;
			mnuGoToLocation.Enabled = true;
		}

		private void ctrlCode_MouseDoubleClick(object sender, MouseEventArgs e)
		{
			GoToLocation(GetActionTarget());
		}
	}
}
