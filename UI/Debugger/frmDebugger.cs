using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Code;
using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Debugger.Labels;
using Mesen.GUI.Debugger.Workspace;
using Mesen.GUI.Forms;
using System;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices;
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
		private int _destAddress = -1;

		private ctrlCpuStatus ctrlCpuStatus;
		private ctrlSpcStatus ctrlSpcStatus;
		private ctrlGsuStatus ctrlGsuStatus;
		private ctrlNecDspStatus ctrlNecDspStatus;
		private ctrlCx4Status ctrlCx4Status;
		private ctrlGameboyStatus ctrlGameboyStatus;

		private ctrlMemoryMapping ctrlMemoryMapping;

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
			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;

			bool isPaused = EmuApi.IsPaused();

			ConfigManager.Config.Debug.Debugger.ApplyConfig();

			mnuUseAltSpcOpNames.Visible = false;

			switch(_cpuType) {
				case CpuType.Cpu:
					ctrlDisassemblyView.Initialize(new CpuDisassemblyManager(), new CpuLineStyleProvider());
					ConfigApi.SetDebuggerFlag(DebuggerFlags.CpuDebuggerEnabled, true);
					this.Text = "CPU Debugger";

					this.ctrlCpuStatus = new ctrlCpuStatus();
					this.ctrlCpuStatus.Padding = new Padding(3, 0, 3, 0);
					this.ctrlCpuStatus.Dock = DockStyle.Top;
					pnlStatus.Controls.Add(this.ctrlCpuStatus);
					break;

				case CpuType.Spc:
					ctrlDisassemblyView.Initialize(new SpcDisassemblyManager(), new SpcLineStyleProvider());
					ConfigApi.SetDebuggerFlag(DebuggerFlags.SpcDebuggerEnabled, true);
					mnuBreakOnWdm.Visible = false;
					mnuBreakOnCop.Visible = false;
					mnuBreakOnUnitRead.Visible = false;
					sepBreakOnUnitRead.Visible = false;
					mnuUseAltSpcOpNames.Visible = true;
					this.Text = "SPC Debugger";

					this.ctrlSpcStatus = new ctrlSpcStatus();
					this.ctrlSpcStatus.Padding = new Padding(3, 0, 3, 0);
					this.ctrlSpcStatus.Dock = DockStyle.Top;
					pnlStatus.Controls.Add(this.ctrlSpcStatus);
					break;

				case CpuType.Sa1:
					ctrlDisassemblyView.Initialize(new Sa1DisassemblyManager(), new Sa1LineStyleProvider());
					ConfigApi.SetDebuggerFlag(DebuggerFlags.Sa1DebuggerEnabled, true);
					this.Text = "SA-1 Debugger";

					this.ctrlCpuStatus = new ctrlCpuStatus();
					this.ctrlCpuStatus.Padding = new Padding(3, 0, 3, 0);
					this.ctrlCpuStatus.Dock = DockStyle.Top;
					pnlStatus.Controls.Add(this.ctrlCpuStatus);
					break;

				case CpuType.Gsu:
					ctrlDisassemblyView.Initialize(new GsuDisassemblyManager(), new GsuLineStyleProvider());
					ConfigApi.SetDebuggerFlag(DebuggerFlags.GsuDebuggerEnabled, true);
					this.Text = "GSU Debugger";
					HideDebuggerElements();
					
					this.ctrlGsuStatus = new ctrlGsuStatus();
					this.ctrlGsuStatus.Padding = new Padding(3, 0, 3, 0);
					this.ctrlGsuStatus.Dock = DockStyle.Top;
					pnlStatus.Controls.Add(this.ctrlGsuStatus);
					break;
				
				case CpuType.NecDsp:
					ctrlDisassemblyView.Initialize(new NecDspDisassemblyManager(), new NecDspLineStyleProvider());
					ConfigApi.SetDebuggerFlag(DebuggerFlags.NecDspDebuggerEnabled, true);
					this.Text = "DSP Debugger";
					HideDebuggerElements();
					
					this.ctrlNecDspStatus = new ctrlNecDspStatus();
					this.ctrlNecDspStatus.Padding = new Padding(3, 0, 3, 0);
					this.ctrlNecDspStatus.Dock = DockStyle.Top;
					pnlStatus.Controls.Add(this.ctrlNecDspStatus);
					break;

				case CpuType.Cx4:
					ctrlDisassemblyView.Initialize(new Cx4DisassemblyManager(), new Cx4LineStyleProvider());
					ConfigApi.SetDebuggerFlag(DebuggerFlags.Cx4DebuggerEnabled, true);
					this.Text = "CX4 Debugger";

					ctrlLabelList.Visible = false;
					HideDebuggerElements();

					this.ctrlCx4Status = new ctrlCx4Status();
					this.ctrlCx4Status.Padding = new Padding(3, 0, 3, 0);
					this.ctrlCx4Status.Dock = DockStyle.Top;
					pnlStatus.Controls.Add(this.ctrlCx4Status);
					break;

				case CpuType.Gameboy:
					ctrlDisassemblyView.Initialize(new GbDisassemblyManager(), new GbLineStyleProvider());
					ConfigApi.SetDebuggerFlag(DebuggerFlags.GbDebuggerEnabled, true);
					this.Text = "Game Boy Debugger";

					ctrlMemoryMapping = new ctrlMemoryMapping();
					ctrlMemoryMapping.Size = new Size(this.ClientSize.Width, 33);
					ctrlMemoryMapping.Margin = new Padding(3, 0, 3, 3);
					ctrlMemoryMapping.Dock = DockStyle.Bottom;
					ctrlMemoryMapping.Visible = ConfigManager.Config.Debug.Debugger.ShowMemoryMappings;
					this.Controls.Add(ctrlMemoryMapping);
					ctrlMemoryMapping.SendToBack();

					sepBrkCopStpWdm.Visible = false;
					mnuBreakOnBrk.Visible = false;
					mnuBreakOnWdm.Visible = false;
					mnuBreakOnCop.Visible = false;
					mnuBreakOnStp.Visible = false;
					ctrlPpuStatus.Visible = false;

					this.ctrlGameboyStatus = new ctrlGameboyStatus();
					this.ctrlGameboyStatus.Padding = new Padding(3, 0, 3, 0);
					this.ctrlGameboyStatus.Dock = DockStyle.Top;
					pnlStatus.Controls.Add(this.ctrlGameboyStatus);
					break;
			}

			ctrlBreakpoints.CpuType = _cpuType;
			ctrlWatch.CpuType = _cpuType;

			InitShortcuts();
			InitToolbar();
			LoadConfig();

			toolTip.SetToolTip(picWatchHelp, ctrlWatch.GetTooltipText());

			BreakpointManager.AddCpuType(_cpuType);

			if(!isPaused) {
				DebugApi.Step(_cpuType, 10000, StepType.Step);
			} else {
				BreakEvent evt = new BreakEvent() { BreakpointId = -1, Source = BreakSource.Unspecified };
				RefreshDebugger(evt);
			}

			base.OnLoad(e);
		}

		private void HideDebuggerElements()
		{
			grpCallstack.Visible = false;

			//Needed for Mono
			tlpBottomPanel.Controls.Remove(grpCallstack);
			tlpBottomPanel.ColumnCount = 2;
			tlpBottomPanel.ColumnStyles.RemoveAt(2);

			mnuStepOver.Visible = false;
			mnuStepOut.Visible = false;
			mnuStepInto.Text = "Step";

			sepBrkCopStpWdm.Visible = false;
			mnuBreakOnWdm.Visible = false;
			mnuBreakOnCop.Visible = false;
			mnuBreakOnStp.Visible = false;
			mnuBreakOnBrk.Visible = false;
			sepBreakOnUnitRead.Visible = false;
			mnuBreakOnUnitRead.Visible = false;
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

			switch(_cpuType) {
				case CpuType.Cpu: ConfigApi.SetDebuggerFlag(DebuggerFlags.CpuDebuggerEnabled, false); break;
				case CpuType.Spc: ConfigApi.SetDebuggerFlag(DebuggerFlags.SpcDebuggerEnabled, false); break;
				case CpuType.Sa1: ConfigApi.SetDebuggerFlag(DebuggerFlags.Sa1DebuggerEnabled, false); break;
				case CpuType.Gsu: ConfigApi.SetDebuggerFlag(DebuggerFlags.GsuDebuggerEnabled, false); break;
				case CpuType.NecDsp: ConfigApi.SetDebuggerFlag(DebuggerFlags.NecDspDebuggerEnabled, false); break;
			}

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
					DebugApi.Step(_cpuType, 1, StepType.Step);
				}
				return true;
			}

			return base.ProcessCmdKey(ref msg, keyData);
		}

		private void InitShortcuts()
		{
			mnuReloadRom.InitShortcut(this, nameof(DebuggerShortcutsConfig.ReloadRom));
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

			mnuGoToAll.InitShortcut(this, nameof(DebuggerShortcutsConfig.GoToAll));
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
			
			mnuSaveRomAs.InitShortcut(this, nameof(DebuggerShortcutsConfig.SaveRomAs));
			mnuSaveAsIps.InitShortcut(this, nameof(DebuggerShortcutsConfig.SaveEditAsIps));

			mnuSaveRomAs.Click += (s, e) => { SaveRomAs(false, CdlStripOption.StripNone); };
			mnuSaveAsIps.Click += (s, e) => { SaveRomAs(true, CdlStripOption.StripNone); };
			mnuCdlStripUnusedData.Click += (s, e) => { SaveRomAs(false, CdlStripOption.StripUnused); };
			mnuCdlStripUsedData.Click += (s, e) => { SaveRomAs(false, CdlStripOption.StripUsed); };

			mnuStepInto.Click += (s, e) => { DebugApi.Step(_cpuType, 1, StepType.Step); };
			mnuStepOver.Click += (s, e) => { DebugApi.Step(_cpuType, 1, StepType.StepOver); };
			mnuStepOut.Click += (s, e) => { DebugApi.Step(_cpuType, 1, StepType.StepOut); };
			mnuRunPpuCycle.Click += (s, e) => { DebugApi.Step(_cpuType, 1, StepType.PpuStep); };
			mnuRunScanline.Click += (s, e) => {
				if(_cpuType == CpuType.Gameboy) {
					DebugApi.Step(_cpuType, 456, StepType.PpuStep);
				} else {
					DebugApi.Step(_cpuType, 341, StepType.PpuStep);
				}
			};
			mnuRunOneFrame.Click += (s, e) => {
				if(_cpuType == CpuType.Gameboy) {
					DebugApi.Step(_cpuType, 456*154, StepType.PpuStep);
				} else {
					//TODO ntsc/pal
					DebugApi.Step(_cpuType, 341 * 262, StepType.PpuStep);
				}
			};
			mnuContinue.Click += (s, e) => { DebugApi.ResumeExecution(); };
			mnuBreak.Click += (s, e) => { DebugApi.Step(_cpuType, 1, StepType.Step); };

			mnuReloadRom.Click += (s, e) => { Task.Run(() => { EmuApi.ReloadRom(); }); };
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
			mnuAutoResetCdl.Click += (s, e) => { InvertFlag(ref ConfigManager.Config.Debug.Debugger.AutoResetCdl); };

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

		public void RefreshDisassembly()
		{
			ctrlDisassemblyView.UpdateCode(true);
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

		private void mnuCodeDataLogger_DropDownOpening(object sender, EventArgs e)
		{
			DebuggerInfo cfg = ConfigManager.Config.Debug.Debugger;
			mnuAutoResetCdl.Checked = cfg.AutoResetCdl;
		}

		private void InitToolbar()
		{
			tsToolbar.AddItemsToToolbar(mnuContinue, mnuBreak, null);

			if(_cpuType != CpuType.Gsu && _cpuType != CpuType.NecDsp) {
				tsToolbar.AddItemsToToolbar(mnuStepInto, mnuStepOver, mnuStepOut, null);
			} else {
				tsToolbar.AddItemsToToolbar(mnuStepInto, null);
			}

			tsToolbar.AddItemsToToolbar(
				mnuRunPpuCycle, mnuRunScanline, mnuRunOneFrame, null,
				mnuToggleBreakpoint, mnuEnableDisableBreakpoint, null,
				mnuBreakIn, null, mnuBreakOn
			);
		}

		private void UpdateFlags(object sender, EventArgs e)
		{
			_entityBinder.UpdateObject();
			ConfigManager.Config.Debug.Debugger.ApplyConfig();
			RefreshDisassembly();
		}

		private void LoadConfig()
		{
			DebuggerInfo cfg = ConfigManager.Config.Debug.Debugger;
			_entityBinder.Entity = cfg;
			_entityBinder.AddBinding(nameof(cfg.ShowByteCode), mnuShowByteCode);
			_entityBinder.AddBinding(nameof(cfg.ShowMemoryMappings), mnuShowMemoryMappings);
			_entityBinder.AddBinding(nameof(cfg.UseLowerCaseDisassembly), mnuUseLowerCaseDisassembly);
			_entityBinder.AddBinding(nameof(cfg.UseAltSpcOpNames), mnuUseAltSpcOpNames);

			mnuShowByteCode.CheckedChanged += (s, e) => { ctrlDisassemblyView.CodeViewer.ShowContentNotes = mnuShowByteCode.Checked; };
			mnuShowMemoryMappings.CheckedChanged += (s, e) => { 
				if(_cpuType == CpuType.Gameboy) {
					ctrlMemoryMapping.Visible = mnuShowMemoryMappings.Checked;
				}
			};
			mnuUseLowerCaseDisassembly.CheckedChanged += this.UpdateFlags;
			mnuUseAltSpcOpNames.CheckedChanged += this.UpdateFlags;

			_entityBinder.UpdateUI();

			Font font = new Font(cfg.FontFamily, cfg.FontSize, cfg.FontStyle);
			ctrlDisassemblyView.CodeViewer.BaseFont = font;
			ctrlDisassemblyView.CodeViewer.TextZoom = cfg.TextZoom;

			RestoreLocation(cfg.WindowLocation, cfg.WindowSize);

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
			if(_firstBreak) {
				_destAddress = address;
			} else {
				ctrlDisassemblyView.ScrollToAddress((uint)address);
			}
		}

		private void GoToAddress()
		{
			GoToAddress address = new GoToAddress();
			using(frmGoToLine frm = new frmGoToLine(address, _cpuType.GetAddressSize())) {
				frm.StartPosition = FormStartPosition.CenterParent;
				if(frm.ShowDialog(ctrlDisassemblyView) == DialogResult.OK) {
					ctrlDisassemblyView.ScrollToAddress(address.Address);
				}
			}
		}

		private int GetVectorAddress(CpuVector vector)
		{
			uint address = (uint)vector;
			byte lsb = DebugApi.GetMemoryValue(_cpuType.ToMemoryType(), address);
			byte msb = DebugApi.GetMemoryValue(_cpuType.ToMemoryType(), address + 1);
			return (msb << 8) | lsb;
		}

		private void GoToVector(CpuVector vector)
		{
			ctrlDisassemblyView.ScrollToAddress((uint)GetVectorAddress(vector));
		}

		private void UpdateDebugger(DebugState state, int? activeAddress)
		{
			switch(_cpuType) {
				case CpuType.Cpu: ctrlCpuStatus.UpdateStatus(state.Cpu); break;
				case CpuType.Spc: ctrlSpcStatus.UpdateStatus(state.Spc); break;
				case CpuType.NecDsp: ctrlNecDspStatus.UpdateStatus(state.NecDsp); break;
				case CpuType.Sa1: ctrlCpuStatus.UpdateStatus(state.Sa1.Cpu); break;
				case CpuType.Gsu: ctrlGsuStatus.UpdateStatus(state.Gsu); break;
				case CpuType.Cx4: ctrlCx4Status.UpdateStatus(state.Cx4); break;
				case CpuType.Gameboy: 
					ctrlGameboyStatus.UpdateStatus(state.Gameboy);
					ctrlMemoryMapping.UpdateCpuRegions(state.Gameboy);
					break;
				default: throw new Exception("Unsupported CPU type");
			}

			ctrlPpuStatus.UpdateStatus(state);
			ctrlDisassemblyView.UpdateCode();
			ctrlDisassemblyView.SetActiveAddress(activeAddress);
			ctrlWatch.UpdateWatch(true);

			if(_cpuType != CpuType.Gsu && _cpuType != CpuType.NecDsp && _cpuType != CpuType.Cx4) {
				ctrlCallstack.UpdateCallstack(_cpuType);
			}
		}

		void ProcessBreakEvent(BreakEvent evt, DebugState state, int activeAddress)
		{
			Breakpoint bp = null;
			if(ConfigManager.Config.Debug.Debugger.BringToFrontOnBreak) {
				bp = BreakpointManager.GetBreakpointById(evt.BreakpointId);
				if(bp?.CpuType == _cpuType || evt.Source > BreakSource.PpuStep) {
					DebugWindowManager.BringToFront(this);
				}
			}

			UpdateContinueAction();
			UpdateDebugger(state, activeAddress);

			if(evt.Source == BreakSource.Breakpoint || evt.Source > BreakSource.PpuStep) {
				string message = ResourceHelper.GetEnumText(evt.Source);
				if(evt.Source == BreakSource.Breakpoint) {
					if(bp != null && bp.IsAssert) {
						message = "Assert failed: " + bp.Condition.Substring(2, bp.Condition.Length - 3);
					} else {
						message += ": " + ResourceHelper.GetEnumText(evt.Operation.Type) + " ($" + evt.Operation.Address.ToString("X4") + ":$" + evt.Operation.Value.ToString("X2") + ")";
					}
				}
				ctrlDisassemblyView.SetMessage(new TextboxMessageInfo() { Message = message });
			}
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded: {
					if(_cpuType == CpuType.Sa1) {
						CoprocessorType coprocessor = EmuApi.GetRomInfo().CoprocessorType;
						if(coprocessor != CoprocessorType.SA1) {
							this.Invoke((MethodInvoker)(() => {
								this.Close();
							}));
							return;
						}
					}

					if(ConfigManager.Config.Debug.Debugger.BreakOnPowerCycleReset) {
						DebugApi.Step(_cpuType, 1, StepType.PpuStep);
					}

					BreakpointManager.SetBreakpoints();

					DebugState state = DebugApi.GetState();
					this.BeginInvoke((MethodInvoker)(() => {
						//Refresh workspace here as well as frmMain to ensure workspace
						//is up-to-date no matter which form is notified first.
						DebugWorkspaceManager.GetWorkspace();

						bool isPowerCycle = e.Parameter.ToInt32() != 0;
						if(!isPowerCycle) {
							DebugWorkspaceManager.AutoImportSymbols();
						}
						LabelManager.RefreshLabels();
						DebugApi.RefreshDisassembly(_cpuType);
						UpdateDebugger(state, null);
					}));
					break;
				}

				case ConsoleNotificationType.GameReset:
					if(ConfigManager.Config.Debug.Debugger.BreakOnPowerCycleReset) {
						DebugApi.Step(_cpuType, 1, StepType.PpuStep);
					}
					break;

				case ConsoleNotificationType.PpuFrameDone:
					this.BeginInvoke((MethodInvoker)(() => {
						UpdateContinueAction();
					}));
					break;

				case ConsoleNotificationType.CodeBreak: {
					BreakEvent evt = (BreakEvent)Marshal.PtrToStructure(e.Parameter, typeof(BreakEvent));
					RefreshDebugger(evt);
					break;
				}
			}
		}

		private void RefreshDebugger(BreakEvent evt)
		{
			DebugState state = DebugApi.GetState();
			int activeAddress;
			switch(_cpuType) {
				case CpuType.Cpu: activeAddress = (int)((state.Cpu.K << 16) | state.Cpu.PC); break;
				case CpuType.Spc: activeAddress = (int)state.Spc.PC; break;
				case CpuType.NecDsp: activeAddress = (int)(state.NecDsp.PC * 3); break;
				case CpuType.Sa1: activeAddress = (int)((state.Sa1.Cpu.K << 16) | state.Sa1.Cpu.PC); break;
				case CpuType.Gsu: activeAddress = (int)((state.Gsu.ProgramBank << 16) | state.Gsu.R[15]); break;
				case CpuType.Cx4: activeAddress = (int)((state.Cx4.Cache.Address[state.Cx4.Cache.Page] + (state.Cx4.PC * 2)) & 0xFFFFFF); break;
				case CpuType.Gameboy: activeAddress = (int)(state.Gameboy.Cpu.PC & 0xFFFF); break;
				default: throw new Exception("Unsupported cpu type");
			}

			this.BeginInvoke((MethodInvoker)(() => {
				ProcessBreakEvent(evt, state, activeAddress);

				if(_firstBreak) {
					_firstBreak = false;
					if(!ConfigManager.Config.Debug.Debugger.BreakOnOpen) {
						DebugApi.ResumeExecution();
					}
					if(_destAddress >= 0) {
						GoToAddress(_destAddress);
					}
				}
			}));
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

		private void mnuConfigureColors_Click(object sender, EventArgs e)
		{
			using(frmDebuggerColors frm = new frmDebuggerColors()) {
				frm.ShowDialog(sender, this);
				ctrlDisassemblyView.Invalidate();
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

		private void mnuExit_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		private void mnuResetCdlLog_Click(object sender, EventArgs e)
		{
			byte[] emptyCdlLog = new byte[DebugApi.GetMemorySize(SnesMemoryType.PrgRom)];
			DebugApi.SetCdlData(emptyCdlLog, emptyCdlLog.Length);
			RefreshDisassembly();
		}

		private void mnuGoToAll_Click(object sender, EventArgs e)
		{
			using(frmGoToAll frm = new frmGoToAll(false, true)) {
				if(frm.ShowDialog() == DialogResult.OK) {
					GoToDestination(frm.Destination);
				}
			}
		}

		private void GoToDestination(GoToDestination dest)
		{
			ctrlDisassemblyView.GoToDestination(dest);
		}

		private void SaveRomAs(bool saveAsIps, CdlStripOption cdlStripOption)
		{
			using(SaveFileDialog sfd = new SaveFileDialog()) {
				if(saveAsIps) {
					sfd.SetFilter("IPS files (*.ips)|*.ips");
					sfd.FileName = EmuApi.GetRomInfo().GetRomName() + ".ips";
				} else if(_cpuType == CpuType.Gameboy) {
					sfd.SetFilter("GB files (*.gb,*.gbc)|*.gb;*.gbc");
					sfd.FileName = EmuApi.GetRomInfo().GetRomName() + "_Modified.gb";
				} else {
					sfd.SetFilter("SFC files (*.sfc)|*.sfc");
					sfd.FileName = EmuApi.GetRomInfo().GetRomName() + "_Modified.sfc";
				}

				sfd.InitialDirectory = ConfigManager.DebuggerFolder;
				if(sfd.ShowDialog() == DialogResult.OK) {
					DebugApi.SaveRomToDisk(sfd.FileName, saveAsIps, cdlStripOption);
				}
			}
		}

		protected override void OnDragEnter(DragEventArgs e)
		{
			base.OnDragEnter(e);

			try {
				if(e.Data != null && e.Data.GetDataPresent(DataFormats.FileDrop)) {
					string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
					if(files != null && files.Length > 0) {
						string ext = Path.GetExtension(files[0]).ToLower();
						if(ext == ".dbg" || ext == ".msl" || ext == ".sym") {
							e.Effect = DragDropEffects.Copy;
						}
					}
				}
			} catch(Exception ex) {
				MesenMsgBox.Show("UnexpectedError", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
			}
		}

		protected override void OnDragDrop(DragEventArgs e)
		{
			base.OnDragDrop(e);

			try {
				string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
				if(files != null && File.Exists(files[0])) {
					ImportLabelFile(files[0]);
				}
			} catch(Exception ex) {
				MesenMsgBox.Show("UnexpectedError", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
			}
		}

		private void mnuImportLabels_Click(object sender, EventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.SetFilter("All supported files (*.dbg, *.msl, *.sym)|*.dbg;*.msl;*.sym");
			if(ofd.ShowDialog() == DialogResult.OK) {
				ImportLabelFile(ofd.FileName);
				RefreshDisassembly();
			}
		}

		private static void ImportLabelFile(string path)
		{
			string ext = Path.GetExtension(path).ToLower();
			if(ext == ".msl") {
				DebugWorkspaceManager.ImportMslFile(path);
			} else if(ext == ".sym") {
				DebugWorkspaceManager.ImportSymFile(path);
			} else {
				DebugWorkspaceManager.ImportDbgFile(path);
			}
		}

		private void mnuExportLabels_Click(object sender, EventArgs e)
		{
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.SetFilter("All supported files (*.msl)|*.msl");
			if(sfd.ShowDialog() == DialogResult.OK) {
				MslLabelFile.Export(sfd.FileName);
			}
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
