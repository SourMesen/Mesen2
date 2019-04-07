namespace Mesen.GUI.Debugger
{
	partial class frmDebugger
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if(disposing && (components != null)) {
				components.Dispose();
			}
			if(this._notifListener != null) {
				this._notifListener.Dispose();
				this._notifListener = null;
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.ctrlDisassemblyView = new Mesen.GUI.Debugger.Controls.ctrlDisassemblyView();
			this.ctrlMesenMenuStrip1 = new Mesen.GUI.Controls.ctrlMesenMenuStrip();
			this.debugToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuContinue = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuBreak = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuStepInto = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuStepOver = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuStepOut = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuStepBack = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuReset = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPowerCycle = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem24 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuToggleBreakpoint = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEnableDisableBreakpoint = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRunPpuCycle = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRunScanline = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRunOneFrame = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem8 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuBreakIn = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuBreakOn = new System.Windows.Forms.ToolStripMenuItem();
			this.searchToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFind = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFindNext = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFindPrev = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem9 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuFindAllOccurrences = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGoTo = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGoToAddress = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem23 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuGoToProgramCounter = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem22 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuGoToResetHandler = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGoToIrqHandler = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGoToNmiHandler = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGoToBrkHandler = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGoToCopHandler = new System.Windows.Forms.ToolStripMenuItem();
			this.optionsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuShowByteCode = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripSeparator();
			this.fontSizeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuIncreaseFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuDecreaseFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuResetFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem21 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSelectFont = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuPreferences = new System.Windows.Forms.ToolStripMenuItem();
			this.ctrlSplitContainer = new Mesen.GUI.Controls.ctrlSplitContainer();
			this.panel1 = new System.Windows.Forms.Panel();
			this.ctrlPpuStatus = new Mesen.GUI.Debugger.Controls.ctrlPpuStatus();
			this.ctrlSpcStatus = new Mesen.GUI.Debugger.Controls.ctrlSpcStatus();
			this.ctrlCpuStatus = new Mesen.GUI.Debugger.Controls.ctrlCpuStatus();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.grpWatch = new System.Windows.Forms.GroupBox();
			this.picWatchHelp = new System.Windows.Forms.PictureBox();
			this.ctrlWatch = new Mesen.GUI.Debugger.ctrlWatch();
			this.grpBreakpoints = new System.Windows.Forms.GroupBox();
			this.ctrlBreakpoints = new Mesen.GUI.Debugger.Controls.ctrlBreakpoints();
			this.grpCallstack = new System.Windows.Forms.GroupBox();
			this.ctrlCallstack = new Mesen.GUI.Debugger.Controls.ctrlCallstack();
			this.tsToolbar = new Mesen.GUI.Controls.ctrlMesenToolStrip();
			this.ctrlMesenMenuStrip1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.ctrlSplitContainer)).BeginInit();
			this.ctrlSplitContainer.Panel1.SuspendLayout();
			this.ctrlSplitContainer.Panel2.SuspendLayout();
			this.ctrlSplitContainer.SuspendLayout();
			this.panel1.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.grpWatch.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picWatchHelp)).BeginInit();
			this.grpBreakpoints.SuspendLayout();
			this.grpCallstack.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlDisassemblyView
			// 
			this.ctrlDisassemblyView.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlDisassemblyView.Location = new System.Drawing.Point(0, 0);
			this.ctrlDisassemblyView.Name = "ctrlDisassemblyView";
			this.ctrlDisassemblyView.Size = new System.Drawing.Size(484, 421);
			this.ctrlDisassemblyView.TabIndex = 0;
			// 
			// ctrlMesenMenuStrip1
			// 
			this.ctrlMesenMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.debugToolStripMenuItem,
            this.searchToolStripMenuItem,
            this.optionsToolStripMenuItem});
			this.ctrlMesenMenuStrip1.Location = new System.Drawing.Point(0, 0);
			this.ctrlMesenMenuStrip1.Name = "ctrlMesenMenuStrip1";
			this.ctrlMesenMenuStrip1.Size = new System.Drawing.Size(832, 24);
			this.ctrlMesenMenuStrip1.TabIndex = 1;
			this.ctrlMesenMenuStrip1.Text = "ctrlMesenMenuStrip1";
			// 
			// debugToolStripMenuItem
			// 
			this.debugToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuContinue,
            this.mnuBreak,
            this.toolStripMenuItem3,
            this.mnuStepInto,
            this.mnuStepOver,
            this.mnuStepOut,
            this.mnuStepBack,
            this.toolStripMenuItem1,
            this.mnuReset,
            this.mnuPowerCycle,
            this.toolStripMenuItem24,
            this.mnuToggleBreakpoint,
            this.mnuEnableDisableBreakpoint,
            this.toolStripMenuItem2,
            this.mnuRunPpuCycle,
            this.mnuRunScanline,
            this.mnuRunOneFrame,
            this.toolStripMenuItem8,
            this.mnuBreakIn,
            this.mnuBreakOn});
			this.debugToolStripMenuItem.Name = "debugToolStripMenuItem";
			this.debugToolStripMenuItem.Size = new System.Drawing.Size(54, 20);
			this.debugToolStripMenuItem.Text = "Debug";
			// 
			// mnuContinue
			// 
			this.mnuContinue.Image = global::Mesen.GUI.Properties.Resources.MediaPlay;
			this.mnuContinue.Name = "mnuContinue";
			this.mnuContinue.Size = new System.Drawing.Size(212, 22);
			this.mnuContinue.Text = "Continue";
			// 
			// mnuBreak
			// 
			this.mnuBreak.Enabled = false;
			this.mnuBreak.Image = global::Mesen.GUI.Properties.Resources.MediaPause;
			this.mnuBreak.Name = "mnuBreak";
			this.mnuBreak.ShortcutKeyDisplayString = "";
			this.mnuBreak.Size = new System.Drawing.Size(212, 22);
			this.mnuBreak.Text = "Break";
			// 
			// toolStripMenuItem3
			// 
			this.toolStripMenuItem3.Name = "toolStripMenuItem3";
			this.toolStripMenuItem3.Size = new System.Drawing.Size(209, 6);
			// 
			// mnuStepInto
			// 
			this.mnuStepInto.Image = global::Mesen.GUI.Properties.Resources.StepInto;
			this.mnuStepInto.Name = "mnuStepInto";
			this.mnuStepInto.Size = new System.Drawing.Size(212, 22);
			this.mnuStepInto.Text = "Step Into";
			// 
			// mnuStepOver
			// 
			this.mnuStepOver.Image = global::Mesen.GUI.Properties.Resources.StepOver;
			this.mnuStepOver.Name = "mnuStepOver";
			this.mnuStepOver.Size = new System.Drawing.Size(212, 22);
			this.mnuStepOver.Text = "Step Over";
			// 
			// mnuStepOut
			// 
			this.mnuStepOut.Image = global::Mesen.GUI.Properties.Resources.StepOut;
			this.mnuStepOut.Name = "mnuStepOut";
			this.mnuStepOut.Size = new System.Drawing.Size(212, 22);
			this.mnuStepOut.Text = "Step Out";
			// 
			// mnuStepBack
			// 
			this.mnuStepBack.Image = global::Mesen.GUI.Properties.Resources.StepBack;
			this.mnuStepBack.Name = "mnuStepBack";
			this.mnuStepBack.Size = new System.Drawing.Size(212, 22);
			this.mnuStepBack.Text = "Step Back";
			this.mnuStepBack.Visible = false;
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(209, 6);
			// 
			// mnuReset
			// 
			this.mnuReset.Image = global::Mesen.GUI.Properties.Resources.Refresh;
			this.mnuReset.Name = "mnuReset";
			this.mnuReset.Size = new System.Drawing.Size(212, 22);
			this.mnuReset.Text = "Reset";
			// 
			// mnuPowerCycle
			// 
			this.mnuPowerCycle.Image = global::Mesen.GUI.Properties.Resources.PowerCycle;
			this.mnuPowerCycle.Name = "mnuPowerCycle";
			this.mnuPowerCycle.Size = new System.Drawing.Size(212, 22);
			this.mnuPowerCycle.Text = "Power Cycle";
			// 
			// toolStripMenuItem24
			// 
			this.toolStripMenuItem24.Name = "toolStripMenuItem24";
			this.toolStripMenuItem24.Size = new System.Drawing.Size(209, 6);
			// 
			// mnuToggleBreakpoint
			// 
			this.mnuToggleBreakpoint.Image = global::Mesen.GUI.Properties.Resources.Breakpoint;
			this.mnuToggleBreakpoint.Name = "mnuToggleBreakpoint";
			this.mnuToggleBreakpoint.Size = new System.Drawing.Size(212, 22);
			this.mnuToggleBreakpoint.Text = "Toggle Breakpoint";
			// 
			// mnuEnableDisableBreakpoint
			// 
			this.mnuEnableDisableBreakpoint.Image = global::Mesen.GUI.Properties.Resources.BreakpointDisabled;
			this.mnuEnableDisableBreakpoint.Name = "mnuEnableDisableBreakpoint";
			this.mnuEnableDisableBreakpoint.Size = new System.Drawing.Size(212, 22);
			this.mnuEnableDisableBreakpoint.Text = "Disable/Enable Breakpoint";
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(209, 6);
			// 
			// mnuRunPpuCycle
			// 
			this.mnuRunPpuCycle.Image = global::Mesen.GUI.Properties.Resources.RunPpuCycle;
			this.mnuRunPpuCycle.Name = "mnuRunPpuCycle";
			this.mnuRunPpuCycle.Size = new System.Drawing.Size(212, 22);
			this.mnuRunPpuCycle.Text = "Run one PPU cycle";
			// 
			// mnuRunScanline
			// 
			this.mnuRunScanline.Image = global::Mesen.GUI.Properties.Resources.RunPpuScanline;
			this.mnuRunScanline.Name = "mnuRunScanline";
			this.mnuRunScanline.Size = new System.Drawing.Size(212, 22);
			this.mnuRunScanline.Text = "Run one scanline";
			// 
			// mnuRunOneFrame
			// 
			this.mnuRunOneFrame.Image = global::Mesen.GUI.Properties.Resources.RunPpuFrame;
			this.mnuRunOneFrame.Name = "mnuRunOneFrame";
			this.mnuRunOneFrame.Size = new System.Drawing.Size(212, 22);
			this.mnuRunOneFrame.Text = "Run one frame";
			// 
			// toolStripMenuItem8
			// 
			this.toolStripMenuItem8.Name = "toolStripMenuItem8";
			this.toolStripMenuItem8.Size = new System.Drawing.Size(209, 6);
			// 
			// mnuBreakIn
			// 
			this.mnuBreakIn.Name = "mnuBreakIn";
			this.mnuBreakIn.Size = new System.Drawing.Size(212, 22);
			this.mnuBreakIn.Text = "Break in...";
			// 
			// mnuBreakOn
			// 
			this.mnuBreakOn.Name = "mnuBreakOn";
			this.mnuBreakOn.Size = new System.Drawing.Size(212, 22);
			this.mnuBreakOn.Text = "Break on...";
			// 
			// searchToolStripMenuItem
			// 
			this.searchToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFind,
            this.mnuFindNext,
            this.mnuFindPrev,
            this.toolStripMenuItem9,
            this.mnuFindAllOccurrences,
            this.mnuGoTo});
			this.searchToolStripMenuItem.Name = "searchToolStripMenuItem";
			this.searchToolStripMenuItem.Size = new System.Drawing.Size(54, 20);
			this.searchToolStripMenuItem.Text = "Search";
			// 
			// mnuFind
			// 
			this.mnuFind.Image = global::Mesen.GUI.Properties.Resources.Find;
			this.mnuFind.Name = "mnuFind";
			this.mnuFind.Size = new System.Drawing.Size(183, 22);
			this.mnuFind.Text = "Find...";
			// 
			// mnuFindNext
			// 
			this.mnuFindNext.Image = global::Mesen.GUI.Properties.Resources.NextArrow;
			this.mnuFindNext.Name = "mnuFindNext";
			this.mnuFindNext.Size = new System.Drawing.Size(183, 22);
			this.mnuFindNext.Text = "Find Next";
			// 
			// mnuFindPrev
			// 
			this.mnuFindPrev.Image = global::Mesen.GUI.Properties.Resources.PreviousArrow;
			this.mnuFindPrev.Name = "mnuFindPrev";
			this.mnuFindPrev.Size = new System.Drawing.Size(183, 22);
			this.mnuFindPrev.Text = "Find Previous";
			// 
			// toolStripMenuItem9
			// 
			this.toolStripMenuItem9.Name = "toolStripMenuItem9";
			this.toolStripMenuItem9.Size = new System.Drawing.Size(180, 6);
			// 
			// mnuFindAllOccurrences
			// 
			this.mnuFindAllOccurrences.Name = "mnuFindAllOccurrences";
			this.mnuFindAllOccurrences.Size = new System.Drawing.Size(183, 22);
			this.mnuFindAllOccurrences.Text = "Find All Occurrences";
			this.mnuFindAllOccurrences.Visible = false;
			// 
			// mnuGoTo
			// 
			this.mnuGoTo.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuGoToAddress,
            this.toolStripMenuItem23,
            this.mnuGoToProgramCounter,
            this.toolStripMenuItem22,
            this.mnuGoToResetHandler,
            this.mnuGoToIrqHandler,
            this.mnuGoToNmiHandler,
            this.mnuGoToBrkHandler,
            this.mnuGoToCopHandler});
			this.mnuGoTo.Name = "mnuGoTo";
			this.mnuGoTo.Size = new System.Drawing.Size(183, 22);
			this.mnuGoTo.Text = "Go To...";
			this.mnuGoTo.DropDownOpening += new System.EventHandler(this.mnuGoTo_DropDownOpening);
			// 
			// mnuGoToAddress
			// 
			this.mnuGoToAddress.Name = "mnuGoToAddress";
			this.mnuGoToAddress.Size = new System.Drawing.Size(166, 22);
			this.mnuGoToAddress.Text = "Address";
			// 
			// toolStripMenuItem23
			// 
			this.toolStripMenuItem23.Name = "toolStripMenuItem23";
			this.toolStripMenuItem23.Size = new System.Drawing.Size(163, 6);
			// 
			// mnuGoToProgramCounter
			// 
			this.mnuGoToProgramCounter.Name = "mnuGoToProgramCounter";
			this.mnuGoToProgramCounter.ShortcutKeyDisplayString = "";
			this.mnuGoToProgramCounter.Size = new System.Drawing.Size(166, 22);
			this.mnuGoToProgramCounter.Text = "Program Counter";
			// 
			// toolStripMenuItem22
			// 
			this.toolStripMenuItem22.Name = "toolStripMenuItem22";
			this.toolStripMenuItem22.Size = new System.Drawing.Size(163, 6);
			// 
			// mnuGoToResetHandler
			// 
			this.mnuGoToResetHandler.Name = "mnuGoToResetHandler";
			this.mnuGoToResetHandler.Size = new System.Drawing.Size(166, 22);
			this.mnuGoToResetHandler.Text = "Reset Handler";
			// 
			// mnuGoToIrqHandler
			// 
			this.mnuGoToIrqHandler.Name = "mnuGoToIrqHandler";
			this.mnuGoToIrqHandler.Size = new System.Drawing.Size(166, 22);
			this.mnuGoToIrqHandler.Text = "IRQ Handler";
			// 
			// mnuGoToNmiHandler
			// 
			this.mnuGoToNmiHandler.Name = "mnuGoToNmiHandler";
			this.mnuGoToNmiHandler.Size = new System.Drawing.Size(166, 22);
			this.mnuGoToNmiHandler.Text = "NMI Handler";
			// 
			// mnuGoToBrkHandler
			// 
			this.mnuGoToBrkHandler.Name = "mnuGoToBrkHandler";
			this.mnuGoToBrkHandler.Size = new System.Drawing.Size(166, 22);
			this.mnuGoToBrkHandler.Text = "BRK Handler";
			// 
			// mnuGoToCopHandler
			// 
			this.mnuGoToCopHandler.Name = "mnuGoToCopHandler";
			this.mnuGoToCopHandler.Size = new System.Drawing.Size(166, 22);
			this.mnuGoToCopHandler.Text = "COP Handler";
			// 
			// optionsToolStripMenuItem
			// 
			this.optionsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuShowByteCode,
            this.toolStripMenuItem5,
            this.fontSizeToolStripMenuItem,
            this.toolStripMenuItem4,
            this.mnuPreferences});
			this.optionsToolStripMenuItem.Name = "optionsToolStripMenuItem";
			this.optionsToolStripMenuItem.Size = new System.Drawing.Size(61, 20);
			this.optionsToolStripMenuItem.Text = "Options";
			// 
			// mnuShowByteCode
			// 
			this.mnuShowByteCode.CheckOnClick = true;
			this.mnuShowByteCode.Name = "mnuShowByteCode";
			this.mnuShowByteCode.Size = new System.Drawing.Size(209, 22);
			this.mnuShowByteCode.Text = "Show byte code";
			// 
			// toolStripMenuItem5
			// 
			this.toolStripMenuItem5.Name = "toolStripMenuItem5";
			this.toolStripMenuItem5.Size = new System.Drawing.Size(206, 6);
			// 
			// fontSizeToolStripMenuItem
			// 
			this.fontSizeToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuIncreaseFontSize,
            this.mnuDecreaseFontSize,
            this.mnuResetFontSize,
            this.toolStripMenuItem21,
            this.mnuSelectFont});
			this.fontSizeToolStripMenuItem.Image = global::Mesen.GUI.Properties.Resources.Font;
			this.fontSizeToolStripMenuItem.Name = "fontSizeToolStripMenuItem";
			this.fontSizeToolStripMenuItem.Size = new System.Drawing.Size(209, 22);
			this.fontSizeToolStripMenuItem.Text = "Font Options";
			// 
			// mnuIncreaseFontSize
			// 
			this.mnuIncreaseFontSize.Name = "mnuIncreaseFontSize";
			this.mnuIncreaseFontSize.ShortcutKeyDisplayString = "";
			this.mnuIncreaseFontSize.Size = new System.Drawing.Size(157, 22);
			this.mnuIncreaseFontSize.Text = "Increase Size";
			// 
			// mnuDecreaseFontSize
			// 
			this.mnuDecreaseFontSize.Name = "mnuDecreaseFontSize";
			this.mnuDecreaseFontSize.ShortcutKeyDisplayString = "";
			this.mnuDecreaseFontSize.Size = new System.Drawing.Size(157, 22);
			this.mnuDecreaseFontSize.Text = "Decrease Size";
			// 
			// mnuResetFontSize
			// 
			this.mnuResetFontSize.Name = "mnuResetFontSize";
			this.mnuResetFontSize.ShortcutKeyDisplayString = "";
			this.mnuResetFontSize.Size = new System.Drawing.Size(157, 22);
			this.mnuResetFontSize.Text = "Reset to Default";
			// 
			// toolStripMenuItem21
			// 
			this.toolStripMenuItem21.Name = "toolStripMenuItem21";
			this.toolStripMenuItem21.Size = new System.Drawing.Size(154, 6);
			// 
			// mnuSelectFont
			// 
			this.mnuSelectFont.Name = "mnuSelectFont";
			this.mnuSelectFont.Size = new System.Drawing.Size(157, 22);
			this.mnuSelectFont.Text = "Select Font...";
			this.mnuSelectFont.Click += new System.EventHandler(this.mnuSelectFont_Click);
			// 
			// toolStripMenuItem4
			// 
			this.toolStripMenuItem4.Name = "toolStripMenuItem4";
			this.toolStripMenuItem4.Size = new System.Drawing.Size(206, 6);
			// 
			// mnuPreferences
			// 
			this.mnuPreferences.Image = global::Mesen.GUI.Properties.Resources.Settings;
			this.mnuPreferences.Name = "mnuPreferences";
			this.mnuPreferences.Size = new System.Drawing.Size(209, 22);
			this.mnuPreferences.Text = "Configure shortcut keys...";
			this.mnuPreferences.Click += new System.EventHandler(this.mnuPreferences_Click);
			// 
			// ctrlSplitContainer
			// 
			this.ctrlSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlSplitContainer.HidePanel2 = false;
			this.ctrlSplitContainer.Location = new System.Drawing.Point(0, 49);
			this.ctrlSplitContainer.Name = "ctrlSplitContainer";
			this.ctrlSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// ctrlSplitContainer.Panel1
			// 
			this.ctrlSplitContainer.Panel1.Controls.Add(this.ctrlDisassemblyView);
			this.ctrlSplitContainer.Panel1.Controls.Add(this.panel1);
			// 
			// ctrlSplitContainer.Panel2
			// 
			this.ctrlSplitContainer.Panel2.Controls.Add(this.tableLayoutPanel1);
			this.ctrlSplitContainer.Size = new System.Drawing.Size(832, 595);
			this.ctrlSplitContainer.SplitterDistance = 421;
			this.ctrlSplitContainer.TabIndex = 2;
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.ctrlPpuStatus);
			this.panel1.Controls.Add(this.ctrlSpcStatus);
			this.panel1.Controls.Add(this.ctrlCpuStatus);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Right;
			this.panel1.Location = new System.Drawing.Point(484, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(348, 421);
			this.panel1.TabIndex = 2;
			// 
			// ctrlPpuStatus
			// 
			this.ctrlPpuStatus.Dock = System.Windows.Forms.DockStyle.Top;
			this.ctrlPpuStatus.Location = new System.Drawing.Point(0, 268);
			this.ctrlPpuStatus.Name = "ctrlPpuStatus";
			this.ctrlPpuStatus.Padding = new System.Windows.Forms.Padding(3, 0, 3, 0);
			this.ctrlPpuStatus.Size = new System.Drawing.Size(348, 47);
			this.ctrlPpuStatus.TabIndex = 3;
			// 
			// ctrlSpcStatus
			// 
			this.ctrlSpcStatus.Dock = System.Windows.Forms.DockStyle.Top;
			this.ctrlSpcStatus.Location = new System.Drawing.Point(0, 148);
			this.ctrlSpcStatus.Name = "ctrlSpcStatus";
			this.ctrlSpcStatus.Padding = new System.Windows.Forms.Padding(3, 0, 3, 0);
			this.ctrlSpcStatus.Size = new System.Drawing.Size(348, 120);
			this.ctrlSpcStatus.TabIndex = 2;
			// 
			// ctrlCpuStatus
			// 
			this.ctrlCpuStatus.Dock = System.Windows.Forms.DockStyle.Top;
			this.ctrlCpuStatus.Location = new System.Drawing.Point(0, 0);
			this.ctrlCpuStatus.Name = "ctrlCpuStatus";
			this.ctrlCpuStatus.Padding = new System.Windows.Forms.Padding(3, 0, 3, 0);
			this.ctrlCpuStatus.Size = new System.Drawing.Size(348, 148);
			this.ctrlCpuStatus.TabIndex = 1;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 3;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
			this.tableLayoutPanel1.Controls.Add(this.grpWatch, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.grpBreakpoints, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.grpCallstack, 2, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 1;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(832, 170);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// grpWatch
			// 
			this.grpWatch.Controls.Add(this.picWatchHelp);
			this.grpWatch.Controls.Add(this.ctrlWatch);
			this.grpWatch.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpWatch.Location = new System.Drawing.Point(3, 3);
			this.grpWatch.Name = "grpWatch";
			this.grpWatch.Size = new System.Drawing.Size(271, 164);
			this.grpWatch.TabIndex = 1;
			this.grpWatch.TabStop = false;
			this.grpWatch.Text = "Watch";
			// 
			// picWatchHelp
			// 
			this.picWatchHelp.Image = global::Mesen.GUI.Properties.Resources.Help;
			this.picWatchHelp.Location = new System.Drawing.Point(44, -1);
			this.picWatchHelp.Name = "picWatchHelp";
			this.picWatchHelp.Size = new System.Drawing.Size(16, 16);
			this.picWatchHelp.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.picWatchHelp.TabIndex = 4;
			this.picWatchHelp.TabStop = false;
			// 
			// ctrlWatch
			// 
			this.ctrlWatch.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlWatch.Location = new System.Drawing.Point(3, 16);
			this.ctrlWatch.Name = "ctrlWatch";
			this.ctrlWatch.Size = new System.Drawing.Size(265, 145);
			this.ctrlWatch.TabIndex = 0;
			// 
			// grpBreakpoints
			// 
			this.grpBreakpoints.Controls.Add(this.ctrlBreakpoints);
			this.grpBreakpoints.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpBreakpoints.Location = new System.Drawing.Point(280, 3);
			this.grpBreakpoints.Name = "grpBreakpoints";
			this.grpBreakpoints.Size = new System.Drawing.Size(271, 164);
			this.grpBreakpoints.TabIndex = 2;
			this.grpBreakpoints.TabStop = false;
			this.grpBreakpoints.Text = "Breakpoints";
			// 
			// ctrlBreakpoints
			// 
			this.ctrlBreakpoints.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlBreakpoints.Location = new System.Drawing.Point(3, 16);
			this.ctrlBreakpoints.Name = "ctrlBreakpoints";
			this.ctrlBreakpoints.Size = new System.Drawing.Size(265, 145);
			this.ctrlBreakpoints.TabIndex = 0;
			this.ctrlBreakpoints.BreakpointNavigation += new Mesen.GUI.Debugger.Controls.ctrlBreakpoints.BreakpointNavigationHandler(this.ctrlBreakpoints_BreakpointNavigation);
			// 
			// grpCallstack
			// 
			this.grpCallstack.Controls.Add(this.ctrlCallstack);
			this.grpCallstack.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpCallstack.Location = new System.Drawing.Point(557, 3);
			this.grpCallstack.Name = "grpCallstack";
			this.grpCallstack.Size = new System.Drawing.Size(272, 164);
			this.grpCallstack.TabIndex = 3;
			this.grpCallstack.TabStop = false;
			this.grpCallstack.Text = "Call Stack";
			// 
			// ctrlCallstack
			// 
			this.ctrlCallstack.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlCallstack.Location = new System.Drawing.Point(3, 16);
			this.ctrlCallstack.Name = "ctrlCallstack";
			this.ctrlCallstack.Size = new System.Drawing.Size(266, 145);
			this.ctrlCallstack.TabIndex = 0;
			this.ctrlCallstack.FunctionSelected += new Mesen.GUI.Debugger.Controls.ctrlCallstack.NavigateToAddressHandler(this.ctrlCallstack_FunctionSelected);
			// 
			// tsToolbar
			// 
			this.tsToolbar.Location = new System.Drawing.Point(0, 24);
			this.tsToolbar.Name = "tsToolbar";
			this.tsToolbar.Size = new System.Drawing.Size(832, 25);
			this.tsToolbar.TabIndex = 3;
			this.tsToolbar.Text = "ctrlMesenToolStrip1";
			// 
			// frmDebugger
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(832, 644);
			this.Controls.Add(this.ctrlSplitContainer);
			this.Controls.Add(this.tsToolbar);
			this.Controls.Add(this.ctrlMesenMenuStrip1);
			this.Name = "frmDebugger";
			this.Text = "Debugger";
			this.ctrlMesenMenuStrip1.ResumeLayout(false);
			this.ctrlMesenMenuStrip1.PerformLayout();
			this.ctrlSplitContainer.Panel1.ResumeLayout(false);
			this.ctrlSplitContainer.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.ctrlSplitContainer)).EndInit();
			this.ctrlSplitContainer.ResumeLayout(false);
			this.panel1.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.grpWatch.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.picWatchHelp)).EndInit();
			this.grpBreakpoints.ResumeLayout(false);
			this.grpCallstack.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private Controls.ctrlDisassemblyView ctrlDisassemblyView;
		private GUI.Controls.ctrlMesenMenuStrip ctrlMesenMenuStrip1;
		private System.Windows.Forms.ToolStripMenuItem debugToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuContinue;
		private System.Windows.Forms.ToolStripMenuItem mnuBreak;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
		private System.Windows.Forms.ToolStripMenuItem mnuStepInto;
		private System.Windows.Forms.ToolStripMenuItem mnuStepOver;
		private System.Windows.Forms.ToolStripMenuItem mnuStepOut;
		private System.Windows.Forms.ToolStripMenuItem mnuStepBack;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuReset;
		private System.Windows.Forms.ToolStripMenuItem mnuPowerCycle;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem24;
		private System.Windows.Forms.ToolStripMenuItem mnuToggleBreakpoint;
		private System.Windows.Forms.ToolStripMenuItem mnuEnableDisableBreakpoint;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuRunPpuCycle;
		private System.Windows.Forms.ToolStripMenuItem mnuRunScanline;
		private System.Windows.Forms.ToolStripMenuItem mnuRunOneFrame;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem8;
		private System.Windows.Forms.ToolStripMenuItem mnuBreakIn;
		private System.Windows.Forms.ToolStripMenuItem mnuBreakOn;
		private GUI.Controls.ctrlSplitContainer ctrlSplitContainer;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private ctrlWatch ctrlWatch;
		private System.Windows.Forms.GroupBox grpWatch;
		private System.Windows.Forms.GroupBox grpBreakpoints;
		private Controls.ctrlBreakpoints ctrlBreakpoints;
		private Controls.ctrlCpuStatus ctrlCpuStatus;
		private GUI.Controls.ctrlMesenToolStrip tsToolbar;
		private System.Windows.Forms.GroupBox grpCallstack;
		private Controls.ctrlCallstack ctrlCallstack;
		private System.Windows.Forms.ToolStripMenuItem searchToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuFind;
		private System.Windows.Forms.ToolStripMenuItem mnuFindNext;
		private System.Windows.Forms.ToolStripMenuItem mnuFindPrev;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem9;
		private System.Windows.Forms.ToolStripMenuItem mnuFindAllOccurrences;
		private System.Windows.Forms.ToolStripMenuItem mnuGoTo;
		private System.Windows.Forms.ToolStripMenuItem mnuGoToAddress;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem23;
		private System.Windows.Forms.ToolStripMenuItem mnuGoToProgramCounter;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem22;
		private System.Windows.Forms.ToolStripMenuItem mnuGoToResetHandler;
		private System.Windows.Forms.ToolStripMenuItem mnuGoToIrqHandler;
		private System.Windows.Forms.ToolStripMenuItem mnuGoToNmiHandler;
		private System.Windows.Forms.ToolStripMenuItem mnuGoToBrkHandler;
		private System.Windows.Forms.ToolStripMenuItem mnuGoToCopHandler;
		private System.Windows.Forms.ToolStripMenuItem optionsToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuPreferences;
		private System.Windows.Forms.PictureBox picWatchHelp;
		private System.Windows.Forms.ToolStripMenuItem fontSizeToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuIncreaseFontSize;
		private System.Windows.Forms.ToolStripMenuItem mnuDecreaseFontSize;
		private System.Windows.Forms.ToolStripMenuItem mnuResetFontSize;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem21;
		private System.Windows.Forms.ToolStripMenuItem mnuSelectFont;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
		private System.Windows.Forms.ToolStripMenuItem mnuShowByteCode;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem5;
		private System.Windows.Forms.Panel panel1;
		private Controls.ctrlPpuStatus ctrlPpuStatus;
		private Controls.ctrlSpcStatus ctrlSpcStatus;
	}
}