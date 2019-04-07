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

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlDisassemblyView : BaseControl
	{
		private BaseStyleProvider _styleProvider;
		private IDisassemblyManager _manager;

		public ctrlDisassemblyView()
		{
			InitializeComponent();
			if(IsDesignMode) {
				return;
			}

			InitShortcuts();

			BreakpointManager.BreakpointsChanged += BreakpointManager_BreakpointsChanged;
		}

		protected override void OnHandleDestroyed(EventArgs e)
		{
			base.OnHandleDestroyed(e);

			BreakpointManager.BreakpointsChanged -= BreakpointManager_BreakpointsChanged;
		}

		private void BreakpointManager_BreakpointsChanged(object sender, EventArgs e)
		{
			ctrlCode.Invalidate();
		}

		public void Initialize(IDisassemblyManager manager, BaseStyleProvider styleProvider)
		{
			_manager = manager;
			_styleProvider = styleProvider;

			ctrlCode.StyleProvider = _styleProvider;
			ctrlCode.ShowContentNotes = false;
			ctrlCode.ShowMemoryValues = true;
			ctrlCode.ExtendedMarginWidth = manager.ByteCodeSize * 4;
			ctrlCode.AddressSize = manager.AddressSize;

			_manager.RefreshCode();
		}

		private void InitShortcuts()
		{
			mnuToggleBreakpoint.InitShortcut(this, nameof(DebuggerShortcutsConfig.CodeWindow_ToggleBreakpoint));
		}

		public void SetActiveAddress(int? address)
		{
			if(_styleProvider.ActiveAddress == address) {
				return;
			}

			_styleProvider.ActiveAddress = address;
			if(address.HasValue && address.Value >= 0) {
				ctrlCode.ScrollToAddress(address.Value);
			}

			ctrlCode.Invalidate();
		}

		public void ScrollToAddress(uint address)
		{
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

			_manager.RefreshCode();
			ctrlCode.DataProvider = _manager.Provider;

			if(centerLineAddress >= 0) {
				//Scroll to the same address as before, to prevent the code view from changing due to setting or banking changes, etc.
				int lineIndex = _manager.Provider.GetLineIndex((UInt32)centerLineAddress) + scrollOffset;
				ctrlCode.ScrollToLineIndex(lineIndex, eHistoryType.None, false, true);
			}
			GoToActiveAddress();
		}

		public ctrlScrollableTextbox CodeViewer { get { return ctrlCode; } }

		public void GoToAddress(int address)
		{
			ctrlCode.ScrollToAddress(address);
		}

		public void GoToActiveAddress()
		{
			if(_styleProvider.ActiveAddress.HasValue) {
				ctrlCode.ScrollToAddress(_styleProvider.ActiveAddress.Value);
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
		}

		private void ctrlCode_TextZoomChanged(object sender, EventArgs e)
		{
			ConfigManager.Config.Debug.Debugger.TextZoom = ctrlCode.TextZoom;
			ConfigManager.ApplyChanges();
		}
	}
}
