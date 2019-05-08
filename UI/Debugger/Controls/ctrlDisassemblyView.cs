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
using static Mesen.GUI.Debugger.Integration.DbgImporter;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlDisassemblyView : BaseControl
	{
		private BaseStyleProvider _styleProvider;
		private IDisassemblyManager _manager;
		private DbgImporter _symbolProvider;
		private bool _inSourceView = false;

		public ctrlDisassemblyView()
		{
			InitializeComponent();
			if(IsDesignMode) {
				return;
			}

			InitShortcuts();

			DebugWorkspaceManager.SymbolProviderChanged += DebugWorkspaceManager_SymbolProviderChanged;
			BreakpointManager.BreakpointsChanged += BreakpointManager_BreakpointsChanged;
			LabelManager.OnLabelUpdated += OnLabelUpdated;
		}

		protected override void OnHandleDestroyed(EventArgs e)
		{
			base.OnHandleDestroyed(e);
			DebugWorkspaceManager.SymbolProviderChanged -= DebugWorkspaceManager_SymbolProviderChanged;
			LabelManager.OnLabelUpdated -= OnLabelUpdated;
			BreakpointManager.BreakpointsChanged -= BreakpointManager_BreakpointsChanged;
		}

		private void DebugWorkspaceManager_SymbolProviderChanged(DbgImporter symbolProvider)
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
			_manager.RefreshCode(_inSourceView ? _symbolProvider : null, _inSourceView ? cboSourceFile.SelectedItem as DbgImporter.FileInfo : null);
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
					foreach(DbgImporter.FileInfo file in _symbolProvider.Files.Values) {
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
			mnuSwitchView.InitShortcut(this, nameof(DebuggerShortcutsConfig.CodeWindow_SwitchView));

			mnuSwitchView.Click += (s, e) => { ToggleView(); };
		}

		public void ToggleView()
		{
			_inSourceView = !_inSourceView;
			UpdateSourceFileDropdown();
			UpdateCode();
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

		public void ScrollToAddress(uint address)
		{
			if(_inSourceView) {
				AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = (int)address, Type = SnesMemoryType.CpuMemory });
				if(absAddress.Address >= 0 && absAddress.Type == SnesMemoryType.PrgRom) {
					LineInfo line = _symbolProvider?.GetSourceCodeLineInfo(absAddress.Address);
					if(line != null) {
						foreach(DbgImporter.FileInfo fileInfo in cboSourceFile.Items) {
							if(fileInfo.ID == line.FileID) {
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

		public void UpdateCode()
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

			_manager.RefreshCode(_inSourceView ? _symbolProvider : null, _inSourceView ? cboSourceFile.SelectedItem as DbgImporter.FileInfo : null);
			ctrlCode.DataProvider = _manager.Provider;

			if(centerLineAddress >= 0) {
				//Scroll to the same address as before, to prevent the code view from changing due to setting or banking changes, etc.
				int lineIndex = _manager.Provider.GetLineIndex((UInt32)centerLineAddress) + scrollOffset;
				ctrlCode.ScrollToLineIndex(lineIndex, eHistoryType.None, false, true);
			}
		}

		public ctrlScrollableTextbox CodeViewer { get { return ctrlCode; } }

		public void GoToAddress(int address)
		{
			ScrollToAddress((uint)address);
		}

		public void GoToActiveAddress()
		{
			if(_styleProvider.ActiveAddress.HasValue) {
				ScrollToAddress((uint)_styleProvider.ActiveAddress.Value);
			}
		}

		public void ToggleBreakpoint()
		{
			_manager.ToggleBreakpoint(ctrlCode.SelectedLine);
		}

		public void EnableDisableBreakpoint()
		{
			_manager.EnableDisableBreakpoint(ctrlCode.SelectedLine);
		}

		private void mnuToggleBreakpoint_Click(object sender, EventArgs e)
		{
			ToggleBreakpoint();
		}

		private void ctrlCode_MouseDown(object sender, MouseEventArgs e)
		{
			if(e.X < 20) {
				int lineIndex = ctrlCode.GetLineIndexAtPosition(e.Y);
				_manager.ToggleBreakpoint(lineIndex);
			}
			BaseForm.GetPopupTooltip(this.FindForm()).Hide();
		}

		private string _lastWord = null;
		private void ctrlCode_MouseMove(object sender, MouseEventArgs e)
		{
			string word = ctrlCode.GetWordUnderLocation(e.Location).Trim();
			if(word == _lastWord) {
				return;
			}

			if(word.Length > 0) {
				Dictionary<string, string> values = _manager.GetTooltipData(word, ctrlCode.GetLineIndexAtPosition(e.Y));
				if(values != null) {
					Form form = this.FindForm();
					Point tooltipLocation = ctrlCode.GetWordEndPosition(e.Location);
					BaseForm.GetPopupTooltip(form).SetTooltip(form.PointToClient(ctrlCode.PointToScreen(tooltipLocation)), values);
				} else {
					BaseForm.GetPopupTooltip(this.FindForm()).Hide();
				}
			} else {
				BaseForm.GetPopupTooltip(this.FindForm()).Hide();
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
	}
}
