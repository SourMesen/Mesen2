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
			this.mnuDisableEnableBreakpoint = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRunCpuCycle = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRun1000Cycles = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRunPpuCycle = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRunScanline = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRunOneFrame = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem8 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuBreakIn = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuBreakOn = new System.Windows.Forms.ToolStripMenuItem();
			this.ctrlSplitContainer = new Mesen.GUI.Controls.ctrlSplitContainer();
			this.ctrlStatus = new Mesen.GUI.Debugger.Controls.ctrlConsoleStatus();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.grpWatch = new System.Windows.Forms.GroupBox();
			this.ctrlWatch = new Mesen.GUI.Debugger.ctrlWatch();
			this.grpBreakpoints = new System.Windows.Forms.GroupBox();
			this.ctrlBreakpoints = new Mesen.GUI.Debugger.Controls.ctrlBreakpoints();
			this.tsToolbar = new Mesen.GUI.Controls.ctrlMesenToolStrip();
			this.ctrlMesenMenuStrip1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.ctrlSplitContainer)).BeginInit();
			this.ctrlSplitContainer.Panel1.SuspendLayout();
			this.ctrlSplitContainer.Panel2.SuspendLayout();
			this.ctrlSplitContainer.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.grpWatch.SuspendLayout();
			this.grpBreakpoints.SuspendLayout();
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
            this.debugToolStripMenuItem});
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
            this.mnuDisableEnableBreakpoint,
            this.toolStripMenuItem2,
            this.mnuRunCpuCycle,
            this.mnuRun1000Cycles,
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
			this.mnuContinue.Click += new System.EventHandler(this.mnuContinue_Click);
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
			this.mnuStepInto.Click += new System.EventHandler(this.mnuStepInto_Click);
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
			// mnuDisableEnableBreakpoint
			// 
			this.mnuDisableEnableBreakpoint.Image = global::Mesen.GUI.Properties.Resources.BreakpointDisabled;
			this.mnuDisableEnableBreakpoint.Name = "mnuDisableEnableBreakpoint";
			this.mnuDisableEnableBreakpoint.Size = new System.Drawing.Size(212, 22);
			this.mnuDisableEnableBreakpoint.Text = "Disable/Enable Breakpoint";
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(209, 6);
			// 
			// mnuRunCpuCycle
			// 
			this.mnuRunCpuCycle.Image = global::Mesen.GUI.Properties.Resources.JumpTarget;
			this.mnuRunCpuCycle.Name = "mnuRunCpuCycle";
			this.mnuRunCpuCycle.Size = new System.Drawing.Size(212, 22);
			this.mnuRunCpuCycle.Text = "Run one CPU cycle";
			// 
			// mnuRun1000Cycles
			// 
			this.mnuRun1000Cycles.Name = "mnuRun1000Cycles";
			this.mnuRun1000Cycles.Size = new System.Drawing.Size(212, 22);
			this.mnuRun1000Cycles.Text = "Run 1000 CPU cycles";
			this.mnuRun1000Cycles.Click += new System.EventHandler(this.mnuRun1000Cycles_Click);
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
			this.ctrlSplitContainer.Panel1.Controls.Add(this.ctrlStatus);
			// 
			// ctrlSplitContainer.Panel2
			// 
			this.ctrlSplitContainer.Panel2.Controls.Add(this.tableLayoutPanel1);
			this.ctrlSplitContainer.Size = new System.Drawing.Size(832, 595);
			this.ctrlSplitContainer.SplitterDistance = 421;
			this.ctrlSplitContainer.TabIndex = 2;
			// 
			// ctrlStatus
			// 
			this.ctrlStatus.Dock = System.Windows.Forms.DockStyle.Right;
			this.ctrlStatus.Location = new System.Drawing.Point(484, 0);
			this.ctrlStatus.Name = "ctrlStatus";
			this.ctrlStatus.Padding = new System.Windows.Forms.Padding(3, 0, 3, 0);
			this.ctrlStatus.Size = new System.Drawing.Size(348, 421);
			this.ctrlStatus.TabIndex = 1;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.Controls.Add(this.grpWatch, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.grpBreakpoints, 1, 0);
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
			this.grpWatch.Controls.Add(this.ctrlWatch);
			this.grpWatch.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpWatch.Location = new System.Drawing.Point(3, 3);
			this.grpWatch.Name = "grpWatch";
			this.grpWatch.Size = new System.Drawing.Size(410, 164);
			this.grpWatch.TabIndex = 1;
			this.grpWatch.TabStop = false;
			this.grpWatch.Text = "Watch";
			// 
			// ctrlWatch
			// 
			this.ctrlWatch.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlWatch.Location = new System.Drawing.Point(3, 16);
			this.ctrlWatch.Name = "ctrlWatch";
			this.ctrlWatch.Size = new System.Drawing.Size(404, 145);
			this.ctrlWatch.TabIndex = 0;
			// 
			// grpBreakpoints
			// 
			this.grpBreakpoints.Controls.Add(this.ctrlBreakpoints);
			this.grpBreakpoints.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpBreakpoints.Location = new System.Drawing.Point(419, 3);
			this.grpBreakpoints.Name = "grpBreakpoints";
			this.grpBreakpoints.Size = new System.Drawing.Size(410, 164);
			this.grpBreakpoints.TabIndex = 2;
			this.grpBreakpoints.TabStop = false;
			this.grpBreakpoints.Text = "Breakpoints";
			// 
			// ctrlBreakpoints
			// 
			this.ctrlBreakpoints.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlBreakpoints.Location = new System.Drawing.Point(3, 16);
			this.ctrlBreakpoints.Name = "ctrlBreakpoints";
			this.ctrlBreakpoints.Size = new System.Drawing.Size(404, 145);
			this.ctrlBreakpoints.TabIndex = 0;
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
			this.tableLayoutPanel1.ResumeLayout(false);
			this.grpWatch.ResumeLayout(false);
			this.grpBreakpoints.ResumeLayout(false);
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
		private System.Windows.Forms.ToolStripMenuItem mnuDisableEnableBreakpoint;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuRunCpuCycle;
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
		private Controls.ctrlConsoleStatus ctrlStatus;
		private System.Windows.Forms.ToolStripMenuItem mnuRun1000Cycles;
		private GUI.Controls.ctrlMesenToolStrip tsToolbar;
	}
}