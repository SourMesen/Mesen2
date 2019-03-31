using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmDebugger : BaseForm
	{
		private NotificationListener _notifListener;

		public frmDebugger()
		{
			InitializeComponent();
			if(DesignMode) {
				return;
			}
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;

			InitShortcuts();
			InitToolbar();

			DebugApi.Step(10000);
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);

			if(this._notifListener != null) {
				this._notifListener.Dispose();
				this._notifListener = null;
			}
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			if(keyData == ConfigManager.Config.Debug.Shortcuts.ToggleBreakContinue) {
				if(EmuApi.IsPaused()) {
					DebugApi.ResumeExecution();
				} else {
					DebugApi.Step(1);
				}
				return true;
			}

			return base.ProcessCmdKey(ref msg, keyData);
		}

		private void InitShortcuts()
		{
			mnuReset.InitShortcut(this, nameof(DebuggerShortcutsConfig.Reset));
			mnuPowerCycle.InitShortcut(this, nameof(DebuggerShortcutsConfig.PowerCycle));

			mnuContinue.InitShortcut(this, nameof(DebuggerShortcutsConfig.Continue));
			mnuBreak.InitShortcut(this, nameof(DebuggerShortcutsConfig.Break));
			mnuBreakIn.InitShortcut(this, nameof(DebuggerShortcutsConfig.BreakIn));
			mnuBreakOn.InitShortcut(this, nameof(DebuggerShortcutsConfig.BreakOn));

			mnuStepBack.InitShortcut(this, nameof(DebuggerShortcutsConfig.StepBack));
			mnuStepOut.InitShortcut(this, nameof(DebuggerShortcutsConfig.StepOut));
			mnuStepInto.InitShortcut(this, nameof(DebuggerShortcutsConfig.StepInto));
			mnuStepOver.InitShortcut(this, nameof(DebuggerShortcutsConfig.StepOver));

			mnuRunPpuCycle.InitShortcut(this, nameof(DebuggerShortcutsConfig.RunPpuCycle));
			mnuRunScanline.InitShortcut(this, nameof(DebuggerShortcutsConfig.RunPpuScanline));
			mnuRunOneFrame.InitShortcut(this, nameof(DebuggerShortcutsConfig.RunPpuFrame));

			mnuToggleBreakpoint.InitShortcut(this, nameof(DebuggerShortcutsConfig.CodeWindow_ToggleBreakpoint));
			mnuEnableDisableBreakpoint.InitShortcut(this, nameof(DebuggerShortcutsConfig.CodeWindow_DisableEnableBreakpoint));

			mnuReset.InitShortcut(this, nameof(DebuggerShortcutsConfig.Reset));
			mnuPowerCycle.InitShortcut(this, nameof(DebuggerShortcutsConfig.PowerCycle));

			mnuGoToAddress.InitShortcut(this, nameof(DebuggerShortcutsConfig.GoTo));
			mnuGoToProgramCounter.InitShortcut(this, nameof(DebuggerShortcutsConfig.GoToProgramCounter));

			mnuFind.InitShortcut(this, nameof(DebuggerShortcutsConfig.Find));
			mnuFindNext.InitShortcut(this, nameof(DebuggerShortcutsConfig.FindNext));
			mnuFindPrev.InitShortcut(this, nameof(DebuggerShortcutsConfig.FindPrev));

			mnuStepInto.Click += (s, e) => { DebugApi.Step(1); };
			mnuStepOver.Click += (s, e) => { DebugApi.Step(1, StepType.CpuStepOver); };
			mnuStepOut.Click += (s, e) => { DebugApi.Step(1, StepType.CpuStepOut); };
			mnuRunPpuCycle.Click += (s, e) => { DebugApi.Step(1, StepType.PpuStep); };
			mnuRunScanline.Click += (s, e) => { DebugApi.Step(341, StepType.PpuStep); };
			mnuRunOneFrame.Click += (s, e) => { DebugApi.Step(341*262, StepType.PpuStep); }; //TODO ntsc/pal
			mnuContinue.Click += (s, e) => { DebugApi.ResumeExecution(); };
			mnuBreak.Click += (s, e) => { DebugApi.Step(1); };

			mnuReset.Click += (s, e) => { EmuApi.Reset(); };
			mnuPowerCycle.Click += (s, e) => { EmuApi.PowerCycle(); };

			mnuToggleBreakpoint.Click += (s, e) => { ctrlDisassemblyView.ToggleBreakpoint(); };
			mnuEnableDisableBreakpoint.Click += (s, e) => { ctrlDisassemblyView.EnableDisableBreakpoint(); };

			mnuGoToAddress.Click += (s, e) => { GoToAddress(); };
			mnuGoToNmiHandler.Click += (s, e) => { GoToVector(CpuVector.Nmi); };
			mnuGoToIrqHandler.Click += (s, e) => { GoToVector(CpuVector.Irq); };
			mnuGoToResetHandler.Click += (s, e) => { GoToVector(CpuVector.Reset); };
			mnuGoToBrkHandler.Click += (s, e) => { GoToVector(CpuVector.Brk); };
			mnuGoToCopHandler.Click += (s, e) => { GoToVector(CpuVector.Cop); };
			mnuGoToProgramCounter.Click += (s, e) => { ctrlDisassemblyView.GoToActiveAddress();	};

			mnuFind.Click += (s, e) => { ctrlDisassemblyView.CodeViewer.OpenSearchBox(); };
			mnuFindNext.Click += (s, e) => { ctrlDisassemblyView.CodeViewer.FindNext(); };
			mnuFindPrev.Click += (s, e) => { ctrlDisassemblyView.CodeViewer.FindPrevious(); };
		}

		private void InitToolbar()
		{
			tsToolbar.AddItemsToToolbar(
				mnuContinue, mnuBreak, null,
				mnuStepInto, mnuStepOver, mnuStepOut, null,
				mnuRunPpuCycle, mnuRunScanline, mnuRunOneFrame, null,
				mnuToggleBreakpoint, mnuEnableDisableBreakpoint, null,
				mnuBreakIn, null, mnuBreakOn
			);
		}

		private void UpdateContinueAction()
		{
			bool paused = EmuApi.IsPaused();
			mnuContinue.Enabled = paused;
			mnuBreak.Enabled = !paused;

			if(!paused) {
				ctrlDisassemblyView.SetActiveAddress(null);
			}
		}

		private void GoToAddress()
		{
			GoToAddress address = new GoToAddress();
			using(frmGoToLine frm = new frmGoToLine(address, 6)) {
				frm.StartPosition = FormStartPosition.CenterParent;
				if(frm.ShowDialog(ctrlDisassemblyView) == DialogResult.OK) {
					ctrlDisassemblyView.GoToAddress((int)address.Address);
				}
			}
		}

		private int GetVectorAddress(CpuVector vector)
		{
			uint address = (uint)vector;
			byte lsb = DebugApi.GetMemoryValue(SnesMemoryType.CpuMemory, address);
			byte msb = DebugApi.GetMemoryValue(SnesMemoryType.CpuMemory, address + 1);
			return (msb << 8) | lsb;
		}

		private void GoToVector(CpuVector vector)
		{
			ctrlDisassemblyView.GoToAddress(GetVectorAddress(vector));
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.PpuFrameDone:
					this.BeginInvoke((MethodInvoker)(() => {
						UpdateContinueAction();
					}));
					break;

				case ConsoleNotificationType.CodeBreak:
					DebugState state = DebugApi.GetState();
					int activeAddress = (int)((state.Cpu.K << 16) | state.Cpu.PC);

					this.BeginInvoke((MethodInvoker)(() => {
						UpdateContinueAction();

						ctrlStatus.UpdateStatus(state);
						ctrlDisassemblyView.SetActiveAddress(activeAddress);
						ctrlWatch.UpdateWatch(true);
						ctrlCallstack.UpdateCallstack();
					}));
					break;
			}
		}
		
		private void ctrlCallstack_FunctionSelected(uint address)
		{
			ctrlDisassemblyView.ScrollToAddress(address);
		}

		private void mnuPreferences_Click(object sender, EventArgs e)
		{
			using(frmDbgPreferences frm = new frmDbgPreferences()) {
				frm.ShowDialog(sender, this);
			}
		}

		private void ctrlBreakpoints_BreakpointNavigation(Breakpoint bp)
		{
			ctrlDisassemblyView.GoToAddress(bp.GetRelativeAddress());
		}

		private void mnuGoTo_DropDownOpening(object sender, EventArgs e)
		{
			mnuGoToNmiHandler.Text = "NMI Handler ($" + GetVectorAddress(CpuVector.Nmi).ToString("X4") + ")";
			mnuGoToIrqHandler.Text = "IRQ Handler ($" + GetVectorAddress(CpuVector.Irq).ToString("X4") + ")";
			mnuGoToResetHandler.Text = "Reset Handler ($" + GetVectorAddress(CpuVector.Reset).ToString("X4") + ")";
			mnuGoToBrkHandler.Text = "BRK Handler ($" + GetVectorAddress(CpuVector.Brk).ToString("X4") + ")";
			mnuGoToCopHandler.Text = "COP Handler ($" + GetVectorAddress(CpuVector.Cop).ToString("X4") + ")";
		}
	}

	public enum CpuVector
	{
		Reset = 0xFFFC,
		Nmi = 0xFFEA,
		Irq = 0xFFEE,
		Brk = 0xFFE6,
		Cop = 0xFFE4,
	}
}
