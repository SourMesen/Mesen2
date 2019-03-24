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

			mnuRunCpuCycle.InitShortcut(this, nameof(DebuggerShortcutsConfig.RunCpuCycle));
			mnuRunPpuCycle.InitShortcut(this, nameof(DebuggerShortcutsConfig.RunPpuCycle));
			mnuRunScanline.InitShortcut(this, nameof(DebuggerShortcutsConfig.RunPpuScanline));
			mnuRunOneFrame.InitShortcut(this, nameof(DebuggerShortcutsConfig.RunPpuFrame));
		}

		private void InitToolbar()
		{
			tsToolbar.AddItemsToToolbar(
				mnuContinue, mnuBreak, null,
				mnuStepInto, mnuStepOver, mnuStepOut, mnuStepBack, null,
				mnuRunCpuCycle, null,
				mnuRunPpuCycle, mnuRunScanline, mnuRunOneFrame, null,
				mnuToggleBreakpoint, mnuDisableEnableBreakpoint, null,
				mnuBreakIn, null, mnuBreakOn
			);
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.CodeBreak:
					DebugState state = DebugApi.GetState();
					int activeAddress = (int)((state.Cpu.K << 16) | state.Cpu.PC);

					this.BeginInvoke((MethodInvoker)(() => {
						ctrlStatus.UpdateStatus(state);
						ctrlDisassemblyView.SetActiveAddress(activeAddress);
						ctrlWatch.UpdateWatch(true);
					}));
					break;
			}
		}
		
		private void mnuStepInto_Click(object sender, EventArgs e)
		{
			DebugApi.Step(1);
		}

		private void mnuContinue_Click(object sender, EventArgs e)
		{
			DebugApi.ResumeExecution();
		}

		private void mnuRun1000Cycles_Click(object sender, EventArgs e)
		{
			DebugApi.Step(1000);
		}
	}
}
