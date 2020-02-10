namespace Mesen.GUI.Debugger
{
	partial class frmRegisterViewer
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
			this.ctrlScanlineCycleSelect = new Mesen.GUI.Debugger.Controls.ctrlScanlineCycleSelect();
			this.ctrlMesenMenuStrip1 = new Mesen.GUI.Controls.ctrlMesenMenuStrip();
			this.mnuFile = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuClose = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuView = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefresh = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRefresh = new System.Windows.Forms.ToolStripMenuItem();
			this.tabMain = new System.Windows.Forms.TabControl();
			this.tpgCpu = new System.Windows.Forms.TabPage();
			this.ctrlPropertyCpu = new Mesen.GUI.Debugger.ctrlPropertyList();
			this.tpgDma = new System.Windows.Forms.TabPage();
			this.ctrlPropertyDma = new Mesen.GUI.Debugger.ctrlPropertyList();
			this.tpgPpu = new System.Windows.Forms.TabPage();
			this.ctrlPropertyPpu = new Mesen.GUI.Debugger.ctrlPropertyList();
			this.tpgSpc = new System.Windows.Forms.TabPage();
			this.ctrlPropertySpc = new Mesen.GUI.Debugger.ctrlPropertyList();
			this.tpgDsp = new System.Windows.Forms.TabPage();
			this.ctrlPropertyDsp = new Mesen.GUI.Debugger.ctrlPropertyList();
			this.ctrlMesenMenuStrip1.SuspendLayout();
			this.tabMain.SuspendLayout();
			this.tpgCpu.SuspendLayout();
			this.tpgDma.SuspendLayout();
			this.tpgPpu.SuspendLayout();
			this.tpgSpc.SuspendLayout();
			this.tpgDsp.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlScanlineCycleSelect
			// 
			this.ctrlScanlineCycleSelect.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.ctrlScanlineCycleSelect.Location = new System.Drawing.Point(0, 411);
			this.ctrlScanlineCycleSelect.Name = "ctrlScanlineCycleSelect";
			this.ctrlScanlineCycleSelect.Size = new System.Drawing.Size(384, 28);
			this.ctrlScanlineCycleSelect.TabIndex = 5;
			// 
			// ctrlMesenMenuStrip1
			// 
			this.ctrlMesenMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFile,
            this.mnuView});
			this.ctrlMesenMenuStrip1.Location = new System.Drawing.Point(0, 0);
			this.ctrlMesenMenuStrip1.Name = "ctrlMesenMenuStrip1";
			this.ctrlMesenMenuStrip1.Size = new System.Drawing.Size(384, 24);
			this.ctrlMesenMenuStrip1.TabIndex = 9;
			this.ctrlMesenMenuStrip1.Text = "ctrlMesenMenuStrip1";
			// 
			// mnuFile
			// 
			this.mnuFile.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuClose});
			this.mnuFile.Name = "mnuFile";
			this.mnuFile.Size = new System.Drawing.Size(37, 20);
			this.mnuFile.Text = "File";
			// 
			// mnuClose
			// 
			this.mnuClose.Image = global::Mesen.GUI.Properties.Resources.Exit;
			this.mnuClose.Name = "mnuClose";
			this.mnuClose.Size = new System.Drawing.Size(103, 22);
			this.mnuClose.Text = "Close";
			this.mnuClose.Click += new System.EventHandler(this.mnuClose_Click);
			// 
			// mnuView
			// 
			this.mnuView.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuAutoRefresh,
            this.toolStripMenuItem2,
            this.mnuRefresh});
			this.mnuView.Name = "mnuView";
			this.mnuView.Size = new System.Drawing.Size(44, 20);
			this.mnuView.Text = "View";
			// 
			// mnuAutoRefresh
			// 
			this.mnuAutoRefresh.Checked = true;
			this.mnuAutoRefresh.CheckOnClick = true;
			this.mnuAutoRefresh.CheckState = System.Windows.Forms.CheckState.Checked;
			this.mnuAutoRefresh.Name = "mnuAutoRefresh";
			this.mnuAutoRefresh.Size = new System.Drawing.Size(141, 22);
			this.mnuAutoRefresh.Text = "Auto-refresh";
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(138, 6);
			// 
			// mnuRefresh
			// 
			this.mnuRefresh.Image = global::Mesen.GUI.Properties.Resources.Refresh;
			this.mnuRefresh.Name = "mnuRefresh";
			this.mnuRefresh.Size = new System.Drawing.Size(141, 22);
			this.mnuRefresh.Text = "Refresh";
			this.mnuRefresh.Click += new System.EventHandler(this.mnuRefresh_Click);
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tpgCpu);
			this.tabMain.Controls.Add(this.tpgDma);
			this.tabMain.Controls.Add(this.tpgPpu);
			this.tabMain.Controls.Add(this.tpgSpc);
			this.tabMain.Controls.Add(this.tpgDsp);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 24);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(384, 387);
			this.tabMain.TabIndex = 10;
			this.tabMain.SelectedIndexChanged += new System.EventHandler(this.tabMain_SelectedIndexChanged);
			// 
			// tpgCpu
			// 
			this.tpgCpu.Controls.Add(this.ctrlPropertyCpu);
			this.tpgCpu.Location = new System.Drawing.Point(4, 22);
			this.tpgCpu.Name = "tpgCpu";
			this.tpgCpu.Size = new System.Drawing.Size(376, 361);
			this.tpgCpu.TabIndex = 0;
			this.tpgCpu.Text = "CPU";
			this.tpgCpu.UseVisualStyleBackColor = true;
			// 
			// ctrlPropertyCpu
			// 
			this.ctrlPropertyCpu.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlPropertyCpu.Location = new System.Drawing.Point(0, 0);
			this.ctrlPropertyCpu.Name = "ctrlPropertyCpu";
			this.ctrlPropertyCpu.Size = new System.Drawing.Size(376, 361);
			this.ctrlPropertyCpu.TabIndex = 0;
			// 
			// tpgDma
			// 
			this.tpgDma.Controls.Add(this.ctrlPropertyDma);
			this.tpgDma.Location = new System.Drawing.Point(4, 22);
			this.tpgDma.Name = "tpgDma";
			this.tpgDma.Size = new System.Drawing.Size(376, 361);
			this.tpgDma.TabIndex = 2;
			this.tpgDma.Text = "DMA";
			this.tpgDma.UseVisualStyleBackColor = true;
			// 
			// ctrlPropertyDma
			// 
			this.ctrlPropertyDma.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlPropertyDma.Location = new System.Drawing.Point(0, 0);
			this.ctrlPropertyDma.Name = "ctrlPropertyDma";
			this.ctrlPropertyDma.Size = new System.Drawing.Size(376, 361);
			this.ctrlPropertyDma.TabIndex = 1;
			// 
			// tpgPpu
			// 
			this.tpgPpu.Controls.Add(this.ctrlPropertyPpu);
			this.tpgPpu.Location = new System.Drawing.Point(4, 22);
			this.tpgPpu.Name = "tpgPpu";
			this.tpgPpu.Size = new System.Drawing.Size(376, 361);
			this.tpgPpu.TabIndex = 1;
			this.tpgPpu.Text = "PPU";
			this.tpgPpu.UseVisualStyleBackColor = true;
			// 
			// ctrlPropertyPpu
			// 
			this.ctrlPropertyPpu.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlPropertyPpu.Location = new System.Drawing.Point(0, 0);
			this.ctrlPropertyPpu.Name = "ctrlPropertyPpu";
			this.ctrlPropertyPpu.Size = new System.Drawing.Size(376, 361);
			this.ctrlPropertyPpu.TabIndex = 2;
			// 
			// tpgSpc
			// 
			this.tpgSpc.Controls.Add(this.ctrlPropertySpc);
			this.tpgSpc.Location = new System.Drawing.Point(4, 22);
			this.tpgSpc.Name = "tpgSpc";
			this.tpgSpc.Size = new System.Drawing.Size(376, 361);
			this.tpgSpc.TabIndex = 5;
			this.tpgSpc.Text = "SPC";
			this.tpgSpc.UseVisualStyleBackColor = true;
			// 
			// ctrlPropertySpc
			// 
			this.ctrlPropertySpc.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlPropertySpc.Location = new System.Drawing.Point(0, 0);
			this.ctrlPropertySpc.Name = "ctrlPropertySpc";
			this.ctrlPropertySpc.Size = new System.Drawing.Size(376, 361);
			this.ctrlPropertySpc.TabIndex = 2;
			// 
			// tpgDsp
			// 
			this.tpgDsp.Controls.Add(this.ctrlPropertyDsp);
			this.tpgDsp.Location = new System.Drawing.Point(4, 22);
			this.tpgDsp.Name = "tpgDsp";
			this.tpgDsp.Size = new System.Drawing.Size(376, 361);
			this.tpgDsp.TabIndex = 6;
			this.tpgDsp.Text = "DSP";
			this.tpgDsp.UseVisualStyleBackColor = true;
			// 
			// ctrlPropertyDsp
			// 
			this.ctrlPropertyDsp.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlPropertyDsp.Location = new System.Drawing.Point(0, 0);
			this.ctrlPropertyDsp.Name = "ctrlPropertyDsp";
			this.ctrlPropertyDsp.Size = new System.Drawing.Size(376, 361);
			this.ctrlPropertyDsp.TabIndex = 3;
			// 
			// frmRegisterViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(384, 439);
			this.Controls.Add(this.tabMain);
			this.Controls.Add(this.ctrlScanlineCycleSelect);
			this.Controls.Add(this.ctrlMesenMenuStrip1);
			this.Name = "frmRegisterViewer";
			this.Text = "Register Viewer";
			this.Controls.SetChildIndex(this.ctrlMesenMenuStrip1, 0);
			this.Controls.SetChildIndex(this.ctrlScanlineCycleSelect, 0);
			this.Controls.SetChildIndex(this.tabMain, 0);
			this.ctrlMesenMenuStrip1.ResumeLayout(false);
			this.ctrlMesenMenuStrip1.PerformLayout();
			this.tabMain.ResumeLayout(false);
			this.tpgCpu.ResumeLayout(false);
			this.tpgDma.ResumeLayout(false);
			this.tpgPpu.ResumeLayout(false);
			this.tpgSpc.ResumeLayout(false);
			this.tpgDsp.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private Controls.ctrlScanlineCycleSelect ctrlScanlineCycleSelect;
		private GUI.Controls.ctrlMesenMenuStrip ctrlMesenMenuStrip1;
		private System.Windows.Forms.ToolStripMenuItem mnuFile;
		private System.Windows.Forms.ToolStripMenuItem mnuClose;
		private System.Windows.Forms.ToolStripMenuItem mnuView;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoRefresh;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuRefresh;
		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tpgCpu;
		private System.Windows.Forms.TabPage tpgPpu;
		private System.Windows.Forms.TabPage tpgDma;
		private System.Windows.Forms.TabPage tpgSpc;
		private ctrlPropertyList ctrlPropertyCpu;
		private ctrlPropertyList ctrlPropertyDma;
		private ctrlPropertyList ctrlPropertyPpu;
		private ctrlPropertyList ctrlPropertySpc;
	  private System.Windows.Forms.TabPage tpgDsp;
	  private ctrlPropertyList ctrlPropertyDsp;
   }
}