namespace Mesen.GUI.Forms
{
	partial class frmMain
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
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.ctrlRenderer = new Mesen.GUI.Controls.ctrlRenderer();
			this.mnuMain = new Mesen.GUI.Controls.ctrlMesenMenuStrip();
			this.mnuFile = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuOpen = new System.Windows.Forms.ToolStripMenuItem();
			this.debugToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRun = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuStep = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRun100Instructions = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuDebugger = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuTraceLogger = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuMemoryTools = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuMain.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlRenderer
			// 
			this.ctrlRenderer.Location = new System.Drawing.Point(0, 27);
			this.ctrlRenderer.Name = "ctrlRenderer";
			this.ctrlRenderer.Size = new System.Drawing.Size(512, 448);
			this.ctrlRenderer.TabIndex = 0;
			// 
			// mnuMain
			// 
			this.mnuMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFile,
            this.debugToolStripMenuItem});
			this.mnuMain.Location = new System.Drawing.Point(0, 0);
			this.mnuMain.Name = "mnuMain";
			this.mnuMain.Size = new System.Drawing.Size(512, 24);
			this.mnuMain.TabIndex = 1;
			this.mnuMain.Text = "ctrlMesenMenuStrip1";
			// 
			// mnuFile
			// 
			this.mnuFile.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuOpen});
			this.mnuFile.Name = "mnuFile";
			this.mnuFile.Size = new System.Drawing.Size(37, 20);
			this.mnuFile.Text = "File";
			// 
			// mnuOpen
			// 
			this.mnuOpen.Image = global::Mesen.GUI.Properties.Resources.Folder;
			this.mnuOpen.Name = "mnuOpen";
			this.mnuOpen.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
			this.mnuOpen.Size = new System.Drawing.Size(152, 22);
			this.mnuOpen.Text = "Open";
			this.mnuOpen.Click += new System.EventHandler(this.mnuOpen_Click);
			// 
			// debugToolStripMenuItem
			// 
			this.debugToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuRun,
            this.mnuStep,
            this.mnuRun100Instructions,
            this.toolStripMenuItem1,
            this.mnuDebugger,
            this.mnuMemoryTools,
            this.mnuTraceLogger});
			this.debugToolStripMenuItem.Name = "debugToolStripMenuItem";
			this.debugToolStripMenuItem.Size = new System.Drawing.Size(54, 20);
			this.debugToolStripMenuItem.Text = "Debug";
			// 
			// mnuRun
			// 
			this.mnuRun.Name = "mnuRun";
			this.mnuRun.ShortcutKeys = System.Windows.Forms.Keys.F5;
			this.mnuRun.Size = new System.Drawing.Size(163, 22);
			this.mnuRun.Text = "Run";
			this.mnuRun.Click += new System.EventHandler(this.mnuRun_Click);
			// 
			// mnuStep
			// 
			this.mnuStep.Name = "mnuStep";
			this.mnuStep.ShortcutKeys = System.Windows.Forms.Keys.F11;
			this.mnuStep.Size = new System.Drawing.Size(163, 22);
			this.mnuStep.Text = "Step";
			this.mnuStep.Click += new System.EventHandler(this.mnuStep_Click);
			// 
			// mnuRun100Instructions
			// 
			this.mnuRun100Instructions.Name = "mnuRun100Instructions";
			this.mnuRun100Instructions.ShortcutKeys = System.Windows.Forms.Keys.F6;
			this.mnuRun100Instructions.Size = new System.Drawing.Size(163, 22);
			this.mnuRun100Instructions.Text = "Run 1000 ops";
			this.mnuRun100Instructions.Click += new System.EventHandler(this.mnuRun100Instructions_Click);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(160, 6);
			// 
			// mnuDebugger
			// 
			this.mnuDebugger.Enabled = false;
			this.mnuDebugger.Image = global::Mesen.GUI.Properties.Resources.Debugger;
			this.mnuDebugger.Name = "mnuDebugger";
			this.mnuDebugger.Size = new System.Drawing.Size(163, 22);
			this.mnuDebugger.Text = "Debugger";
			// 
			// mnuTraceLogger
			// 
			this.mnuTraceLogger.Image = global::Mesen.GUI.Properties.Resources.LogWindow;
			this.mnuTraceLogger.Name = "mnuTraceLogger";
			this.mnuTraceLogger.Size = new System.Drawing.Size(163, 22);
			this.mnuTraceLogger.Text = "Trace Logger";
			this.mnuTraceLogger.Click += new System.EventHandler(this.mnuTraceLogger_Click);
			// 
			// mnuMemoryTools
			// 
			this.mnuMemoryTools.Image = global::Mesen.GUI.Properties.Resources.CheatCode;
			this.mnuMemoryTools.Name = "mnuMemoryTools";
			this.mnuMemoryTools.Size = new System.Drawing.Size(163, 22);
			this.mnuMemoryTools.Text = "Memory Tools";
			this.mnuMemoryTools.Click += new System.EventHandler(this.mnuMemoryTools_Click);
			// 
			// frmMain
			// 
			this.AllowDrop = true;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(512, 475);
			this.Controls.Add(this.ctrlRenderer);
			this.Controls.Add(this.mnuMain);
			this.MainMenuStrip = this.mnuMain;
			this.Name = "frmMain";
			this.Text = "frmMain";
			this.mnuMain.ResumeLayout(false);
			this.mnuMain.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private Controls.ctrlRenderer ctrlRenderer;
		private Controls.ctrlMesenMenuStrip mnuMain;
		private System.Windows.Forms.ToolStripMenuItem debugToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuDebugger;
		private System.Windows.Forms.ToolStripMenuItem mnuTraceLogger;
		private System.Windows.Forms.ToolStripMenuItem mnuStep;
		private System.Windows.Forms.ToolStripMenuItem mnuFile;
		private System.Windows.Forms.ToolStripMenuItem mnuOpen;
		private System.Windows.Forms.ToolStripMenuItem mnuRun;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuRun100Instructions;
		private System.Windows.Forms.ToolStripMenuItem mnuMemoryTools;
	}
}