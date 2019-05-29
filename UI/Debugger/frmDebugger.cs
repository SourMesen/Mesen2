using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Code;
using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Debugger.Workspace;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmDebugger : BaseForm
	{
		private EntityBinder _entityBinder = new EntityBinder();
		private NotificationListener _notifListener;
		private CpuType _cpuType;
		private bool _firstBreak = true;

		public CpuType CpuType { get { return _cpuType; } }

		public frmDebugger(CpuType cpuType)
		{
			InitializeComponent();

			_cpuType = cpuType;

			ctrlLabelList.CpuType = cpuType;

			if(DesignMode) {
				return;
			}
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			this.Text = _cpuType == CpuType.Cpu ? "CPU Debugger" : "SPC Debugger";
			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;

			switch(_cpuType) {
				case CpuType.Cpu: ctrlDisassemblyView.Initialize(new CpuDisassemblyManager(), new CpuLineStyleProvider()); break;
				case CpuType.Spc: ctrlDisassemblyView.Initialize(new SpcDisassemblyManager(), new SpcLineStyleProvider()); break;
			}

			ctrlBreakpoints.CpuType = _cpuType;
			ctrlWatch.CpuType = _cpuType;

			InitShortcuts();
			InitToolbar();
			LoadConfig();

			toolTip.SetToolTip(picWatchHelp, ctrlWatch.GetTooltipText());

			ConfigApi.SetDebuggerFlag(_cpuType == CpuType.Cpu ? DebuggerFlags.CpuDebuggerEnabled : DebuggerFlags.SpcDebuggerEnabled, true);
			BreakpointManager.AddCpuType(_cpuType);
			DebugApi.Step(10000, StepType.CpuStep);
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			
			DebuggerInfo cfg = ConfigManager.Config.Debug.Debugger;
			cfg.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			cfg.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			cfg.SplitterDistance = ctrlSplitContainer.SplitterDistance;

			_entityBinder.UpdateObject();
			ConfigManager.ApplyChanges();

			ConfigApi.SetDebuggerFlag(_cpuType == CpuType.Cpu ? DebuggerFlags.CpuDebuggerEnabled : DebuggerFlags.SpcDebuggerEnabled, false);
			BreakpointManager.RemoveCpuType(_cpuType);

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
					DebugApi.Step(1, _cpuType == CpuType.Cpu ? StepType.CpuStep : StepType.SpcStep);
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

			mnuBreakIn.InitShortcut(this, nameof(DebuggerShortcutsConfig.BreakIn));
			mnuBreakOn.InitShortcut(this, nameof(DebuggerShortcutsConfig.BreakOn));

			mnuIncreaseFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.IncreaseFontSize));
			mnuDecreaseFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.DecreaseFontSize));
			mnuResetFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.ResetFontSize));

			mnuStepInto.Click += (s, e) => { DebugApi.Step(1, _cpuType == CpuType.Cpu ? StepType.CpuStep : StepType.SpcStep); };
			mnuStepOver.Click += (s, e) => { DebugApi.Step(1, _cpuType == CpuType.Cpu ? StepType.CpuStepOver : StepType.SpcStepOver); };
			mnuStepOut.Click += (s, e) => { DebugApi.Step(1, _cpuType == CpuType.Cpu ? StepType.CpuStepOut : StepType.SpcStepOut); };
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

			mnuBreakIn.Click += (s, e) => { using(frmBreakIn frm = new frmBreakIn(_cpuType)) { frm.ShowDialog(); } };
			mnuBreakOn.Click += (s, e) => { using(frmBreakOn frm = new frmBreakOn()) { frm.ShowDialog(); } };

			mnuIncreaseFontSize.Click += (s, e) => { ctrlDisassemblyView.CodeViewer.TextZoom += 10; };
			mnuDecreaseFontSize.Click += (s, e) => { ctrlDisassemblyView.CodeViewer.TextZoom -= 10; };
			mnuResetFontSize.Click += (s, e) => { ctrlDisassemblyView.CodeViewer.TextZoom = 100; };

			mnuBreakOnBrk.Click += (s, e) => { InvertFlag(ref ConfigManager.Config.Debug.Debugger.BreakOnBrk); };
			mnuBreakOnCop.Click += (s, e) => { InvertFlag(ref ConfigManager.Config.Debug.Debugger.BreakOnCop); };
			mnuBreakOnStp.Click += (s, e) => { InvertFlag(ref ConfigManager.Config.Debug.Debugger.BreakOnStp); };
			mnuBreakOnWdm.Click += (s, e) => { InvertFlag(ref ConfigManager.Config.Debug.Debugger.BreakOnWdm); };
			mnuBreakOnOpen.Click += (s, e) => { InvertFlag(ref ConfigManager.Config.Debug.Debugger.BreakOnOpen); };
			mnuBreakOnPowerCycleReset.Click += (s, e) => { InvertFlag(ref ConfigManager.Config.Debug.Debugger.BreakOnPowerCycleReset); };
			mnuBreakOnUnitRead.Click += (s, e) => { InvertFlag(ref ConfigManager.Config.Debug.Debugger.BreakOnUninitRead); };
			mnuBringToFrontOnBreak.Click += (s, e) => { InvertFlag(ref ConfigManager.Config.Debug.Debugger.BringToFrontOnBreak); };
			mnuBringToFrontOnPause.Click += (s, e) => { InvertFlag(ref ConfigManager.Config.Debug.Debugger.BringToFrontOnPause); };

			mnuHideUnident.Click += (s, e) => { SetValue(ref ConfigManager.Config.Debug.Debugger.UnidentifiedBlockDisplay, CodeDisplayMode.Hide);  RefreshDisassembly(); };
			mnuDisassembleUnident.Click += (s, e) => { SetValue(ref ConfigManager.Config.Debug.Debugger.UnidentifiedBlockDisplay, CodeDisplayMode.Disassemble); RefreshDisassembly(); };
			mnuShowUnident.Click += (s, e) => { SetValue(ref ConfigManager.Config.Debug.Debugger.UnidentifiedBlockDisplay, CodeDisplayMode.Show); RefreshDisassembly(); };

			mnuHideData.Click += (s, e) => { SetValue(ref ConfigManager.Config.Debug.Debugger.VerifiedDataDisplay, CodeDisplayMode.Hide); RefreshDisassembly(); };
			mnuDisassembleData.Click += (s, e) => { SetValue(ref ConfigManager.Config.Debug.Debugger.VerifiedDataDisplay, CodeDisplayMode.Disassemble); RefreshDisassembly(); };
			mnuShowData.Click += (s, e) => { SetValue(ref ConfigManager.Config.Debug.Debugger.VerifiedDataDisplay, CodeDisplayMode.Show); RefreshDisassembly(); };
		}

		private void SetValue<T>(ref T setting, T value)
		{
			setting = value;
			ConfigManager.ApplyChanges();
			ConfigManager.Config.Debug.Debugger.ApplyConfig();
		}

		private void InvertFlag(ref bool flag)
		{
			flag = !flag;
			ConfigManager.ApplyChanges();
			ConfigManager.Config.Debug.Debugger.ApplyConfig();
		}

		private void RefreshDisassembly()
		{
			DebugApi.RefreshDisassembly(CpuType.Cpu);
			DebugApi.RefreshDisassembly(CpuType.Spc);
			ctrlDisassemblyView.UpdateCode();
		}

		private void mnuBreakOptions_DropDownOpening(object sender, EventArgs e)
		{
			DebuggerInfo cfg = ConfigManager.Config.Debug.Debugger;
			mnuBreakOnBrk.Checked = cfg.BreakOnBrk;
			mnuBreakOnCop.Checked = cfg.BreakOnCop;
			mnuBreakOnStp.Checked = cfg.BreakOnStp;
			mnuBreakOnWdm.Checked = cfg.BreakOnWdm;
			mnuBreakOnOpen.Checked = cfg.BreakOnOpen;
			mnuBreakOnPowerCycleReset.Checked = cfg.BreakOnPowerCycleReset;
			mnuBreakOnUnitRead.Checked = cfg.BreakOnUninitRead;
			mnuBringToFrontOnBreak.Checked = cfg.BringToFrontOnBreak;
			mnuBringToFrontOnPause.Checked = cfg.BringToFrontOnPause;
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

		private void LoadConfig()
		{
			DebuggerInfo cfg = ConfigManager.Config.Debug.Debugger;
			_entityBinder.Entity = cfg;
			_entityBinder.AddBinding(nameof(cfg.ShowByteCode), mnuShowByteCode);

			mnuShowByteCode.CheckedChanged += (s, e) => { ctrlDisassemblyView.CodeViewer.ShowContentNotes = mnuShowByteCode.Checked; };

			_entityBinder.UpdateUI();

			Font font = new Font(cfg.FontFamily, cfg.FontSize, cfg.FontStyle);
			ctrlDisassemblyView.CodeViewer.BaseFont = font;
			ctrlDisassemblyView.CodeViewer.TextZoom = cfg.TextZoom;

			if(!cfg.WindowSize.IsEmpty) {
				this.StartPosition = FormStartPosition.Manual;
				this.Size = cfg.WindowSize;
				this.Location = cfg.WindowLocation;
			}

			if(cfg.SplitterDistance.HasValue) {
				ctrlSplitContainer.SplitterDistance = cfg.SplitterDistance.Value;
			}
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

		public void GoToAddress(int address)
		{
			ctrlDisassemblyView.ScrollToAddress((uint)address);
		}

		private void GoToAddress()
		{
			GoToAddress address = new GoToAddress();
			using(frmGoToLine frm = new frmGoToLine(address, _cpuType == CpuType.Spc ? 4 : 6)) {
				frm.StartPosition = FormStartPosition.CenterParent;
				if(frm.ShowDialog(ctrlDisassemblyView) == DialogResult.OK) {
					ctrlDisassemblyView.ScrollToAddress(address.Address);
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
			ctrlDisassemblyView.ScrollToAddress((uint)GetVectorAddress(vector));
		}

		private void UpdateDebugger(DebugState state, int? activeAddress)
		{
			if(_cpuType == CpuType.Cpu) {
				ctrlCpuStatus.UpdateStatus(state);
			} else {
				ctrlCpuStatus.Visible = false;
			}

			if(_cpuType == CpuType.Spc) {
				ctrlSpcStatus.UpdateStatus(state);
			} else {
				ctrlSpcStatus.Visible = false;
			}

			ctrlPpuStatus.UpdateStatus(state);
			ctrlDisassemblyView.UpdateCode();
			ctrlDisassemblyView.SetActiveAddress(activeAddress);
			ctrlWatch.UpdateWatch(true);
			ctrlCallstack.UpdateCallstack(_cpuType);
		}

		void ProcessBreakEvent(BreakEvent evt, DebugState state, int activeAddress)
		{
			if(ConfigManager.Config.Debug.Debugger.BringToFrontOnBreak) {
				Breakpoint bp = BreakpointManager.GetBreakpointById(evt.BreakpointId);
				if(bp?.MemoryType.ToCpuType() == _cpuType || evt.Source > BreakSource.PpuStep) {
					DebugWindowManager.BringToFront(this);
				}
			}

			UpdateContinueAction();
			UpdateDebugger(state, activeAddress);

			if(evt.Source == BreakSource.Breakpoint || evt.Source > BreakSource.PpuStep) {
				string message = ResourceHelper.GetEnumText(evt.Source);
				if(evt.Source == BreakSource.Breakpoint) {
					message += ": " + ResourceHelper.GetEnumText(evt.Operation.Type) + " ($" + evt.Operation.Address.ToString("X4") + ":$" + evt.Operation.Value.ToString("X2") + ")";
				}
				ctrlDisassemblyView.SetMessage(new TextboxMessageInfo() { Message = message });
			}
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded: {
					if(ConfigManager.Config.Debug.Debugger.BreakOnPowerCycleReset) {
						DebugApi.Step(1, StepType.PpuStep);
					}

					DebugState state = DebugApi.GetState();
					this.BeginInvoke((MethodInvoker)(() => {
						DebugWorkspaceManager.ImportDbgFile();
						DebugApi.RefreshDisassembly(_cpuType);
						UpdateDebugger(state, null);
						BreakpointManager.SetBreakpoints();
					}));
					break;
				}

				case ConsoleNotificationType.GameReset:
					if(ConfigManager.Config.Debug.Debugger.BreakOnPowerCycleReset) {
						DebugApi.Step(1, StepType.PpuStep);
					}
					break;

				case ConsoleNotificationType.PpuFrameDone:
					this.BeginInvoke((MethodInvoker)(() => {
						UpdateContinueAction();
					}));
					break;

				case ConsoleNotificationType.CodeBreak: {
					BreakEvent evt = (BreakEvent)Marshal.PtrToStructure(e.Parameter, typeof(BreakEvent));
					DebugState state = DebugApi.GetState();
					int activeAddress = _cpuType == CpuType.Cpu ? (int)((state.Cpu.K << 16) | state.Cpu.PC) : (int)state.Spc.PC;

					this.BeginInvoke((MethodInvoker)(() => {
						ProcessBreakEvent(evt, state, activeAddress);

						if(_firstBreak && !ConfigManager.Config.Debug.Debugger.BreakOnOpen) {
							DebugApi.ResumeExecution();
						}
						_firstBreak = false;
					}));
					break;
				}
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
			ctrlDisassemblyView.ScrollToAddress((uint)bp.GetRelativeAddress());
		}

		private void mnuGoTo_DropDownOpening(object sender, EventArgs e)
		{
			mnuGoToNmiHandler.Text = "NMI Handler ($" + GetVectorAddress(CpuVector.Nmi).ToString("X4") + ")";
			mnuGoToIrqHandler.Text = "IRQ Handler ($" + GetVectorAddress(CpuVector.Irq).ToString("X4") + ")";
			mnuGoToResetHandler.Text = "Reset Handler ($" + GetVectorAddress(CpuVector.Reset).ToString("X4") + ")";
			mnuGoToBrkHandler.Text = "BRK Handler ($" + GetVectorAddress(CpuVector.Brk).ToString("X4") + ")";
			mnuGoToCopHandler.Text = "COP Handler ($" + GetVectorAddress(CpuVector.Cop).ToString("X4") + ")";
		}

		private void mnuSelectFont_Click(object sender, EventArgs e)
		{
			Font newFont = FontDialogHelper.SelectFont(ctrlDisassemblyView.CodeViewer.BaseFont);

			DebuggerInfo cfg = ConfigManager.Config.Debug.Debugger;
			cfg.FontFamily = newFont.FontFamily.Name;
			cfg.FontStyle = newFont.Style;
			cfg.FontSize = newFont.Size;
			ConfigManager.ApplyChanges();

			ctrlDisassemblyView.CodeViewer.BaseFont = newFont;
		}

		private void mnuDbgIntegrationSettings_Click(object sender, EventArgs e)
		{
			using(frmIntegrationSettings frm = new frmIntegrationSettings()) {
				frm.ShowDialog();
			}
		}

		private void mnuUnidentifiedData_DropDownOpening(object sender, EventArgs e)
		{
			CodeDisplayMode mode = ConfigManager.Config.Debug.Debugger.UnidentifiedBlockDisplay;
			mnuShowUnident.Checked = mode == CodeDisplayMode.Show;
			mnuHideUnident.Checked = mode == CodeDisplayMode.Hide;
			mnuDisassembleUnident.Checked = mode == CodeDisplayMode.Disassemble;
		}

		private void mnuVerifiedData_DropDownOpening(object sender, EventArgs e)
		{
			CodeDisplayMode mode = ConfigManager.Config.Debug.Debugger.VerifiedDataDisplay;
			mnuShowData.Checked = mode == CodeDisplayMode.Show;
			mnuHideData.Checked = mode == CodeDisplayMode.Hide;
			mnuDisassembleData.Checked = mode == CodeDisplayMode.Disassemble;
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
