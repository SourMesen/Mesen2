namespace Mesen.GUI.Debugger
{
	partial class frmEventViewer
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
			this.ctrlPpuView = new Mesen.GUI.Debugger.ctrlEventViewerPpuView();
			this.mnuMain = new Mesen.GUI.Controls.ctrlMesenMenuStrip();
			this.mnuFile = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuClose = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuView = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRefresh = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefreshSpeed = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefreshLow = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefreshNormal = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefreshHigh = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuAutoRefresh = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRefreshOnBreakPause = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuZoomIn = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuZoomOut = new System.Windows.Forms.ToolStripMenuItem();
			this.tabMain = new System.Windows.Forms.TabControl();
			this.tpgPpuView = new System.Windows.Forms.TabPage();
			this.tpgListView = new System.Windows.Forms.TabPage();
			this.ctrlListView = new Mesen.GUI.Debugger.ctrlEventViewerListView();
			this.ctrlFilters = new Mesen.GUI.Debugger.ctrlEventViewerFilters();
			this.mnuMain.SuspendLayout();
			this.tabMain.SuspendLayout();
			this.tpgPpuView.SuspendLayout();
			this.tpgListView.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlPpuView
			// 
			this.ctrlPpuView.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlPpuView.ImageScale = 1;
			this.ctrlPpuView.Location = new System.Drawing.Point(0, 0);
			this.ctrlPpuView.Margin = new System.Windows.Forms.Padding(0);
			this.ctrlPpuView.Name = "ctrlPpuView";
			this.ctrlPpuView.ScanlineCount = ((uint)(262u));
			this.ctrlPpuView.Size = new System.Drawing.Size(661, 541);
			this.ctrlPpuView.TabIndex = 0;
			// 
			// mnuMain
			// 
			this.mnuMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFile,
            this.mnuView});
			this.mnuMain.Location = new System.Drawing.Point(0, 0);
			this.mnuMain.Name = "mnuMain";
			this.mnuMain.Size = new System.Drawing.Size(946, 24);
			this.mnuMain.TabIndex = 10;
			this.mnuMain.Text = "ctrlMesenMenuStrip1";
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
			// 
			// mnuView
			// 
			this.mnuView.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuRefresh,
            this.mnuAutoRefreshSpeed,
            this.toolStripMenuItem1,
            this.mnuAutoRefresh,
            this.mnuRefreshOnBreakPause,
            this.toolStripMenuItem2,
            this.mnuZoomIn,
            this.mnuZoomOut});
			this.mnuView.Name = "mnuView";
			this.mnuView.Size = new System.Drawing.Size(44, 20);
			this.mnuView.Text = "View";
			// 
			// mnuRefresh
			// 
			this.mnuRefresh.Image = global::Mesen.GUI.Properties.Resources.Refresh;
			this.mnuRefresh.Name = "mnuRefresh";
			this.mnuRefresh.Size = new System.Drawing.Size(198, 22);
			this.mnuRefresh.Text = "Refresh";
			this.mnuRefresh.Click += new System.EventHandler(this.mnuRefresh_Click);
			// 
			// mnuAutoRefreshSpeed
			// 
			this.mnuAutoRefreshSpeed.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuAutoRefreshLow,
            this.mnuAutoRefreshNormal,
            this.mnuAutoRefreshHigh});
			this.mnuAutoRefreshSpeed.Image = global::Mesen.GUI.Properties.Resources.Speed;
			this.mnuAutoRefreshSpeed.Name = "mnuAutoRefreshSpeed";
			this.mnuAutoRefreshSpeed.Size = new System.Drawing.Size(198, 22);
			this.mnuAutoRefreshSpeed.Text = "Auto-refresh Speed";
			// 
			// mnuAutoRefreshLow
			// 
			this.mnuAutoRefreshLow.Name = "mnuAutoRefreshLow";
			this.mnuAutoRefreshLow.Size = new System.Drawing.Size(159, 22);
			this.mnuAutoRefreshLow.Text = "Low (15 FPS)";
			// 
			// mnuAutoRefreshNormal
			// 
			this.mnuAutoRefreshNormal.Name = "mnuAutoRefreshNormal";
			this.mnuAutoRefreshNormal.Size = new System.Drawing.Size(159, 22);
			this.mnuAutoRefreshNormal.Text = "Normal (30 FPS)";
			// 
			// mnuAutoRefreshHigh
			// 
			this.mnuAutoRefreshHigh.Name = "mnuAutoRefreshHigh";
			this.mnuAutoRefreshHigh.Size = new System.Drawing.Size(159, 22);
			this.mnuAutoRefreshHigh.Text = "High (60 FPS)";
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(195, 6);
			// 
			// mnuAutoRefresh
			// 
			this.mnuAutoRefresh.Checked = true;
			this.mnuAutoRefresh.CheckOnClick = true;
			this.mnuAutoRefresh.CheckState = System.Windows.Forms.CheckState.Checked;
			this.mnuAutoRefresh.Name = "mnuAutoRefresh";
			this.mnuAutoRefresh.Size = new System.Drawing.Size(198, 22);
			this.mnuAutoRefresh.Text = "Auto-refresh";
			this.mnuAutoRefresh.CheckedChanged += new System.EventHandler(this.mnuAutoRefresh_CheckedChanged);
			// 
			// mnuRefreshOnBreakPause
			// 
			this.mnuRefreshOnBreakPause.CheckOnClick = true;
			this.mnuRefreshOnBreakPause.Name = "mnuRefreshOnBreakPause";
			this.mnuRefreshOnBreakPause.Size = new System.Drawing.Size(198, 22);
			this.mnuRefreshOnBreakPause.Text = "Refresh on break/pause";
			this.mnuRefreshOnBreakPause.Click += new System.EventHandler(this.mnuRefreshOnBreakPause_Click);
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(195, 6);
			// 
			// mnuZoomIn
			// 
			this.mnuZoomIn.Name = "mnuZoomIn";
			this.mnuZoomIn.Size = new System.Drawing.Size(198, 22);
			this.mnuZoomIn.Text = "Zoom In";
			// 
			// mnuZoomOut
			// 
			this.mnuZoomOut.Name = "mnuZoomOut";
			this.mnuZoomOut.Size = new System.Drawing.Size(198, 22);
			this.mnuZoomOut.Text = "Zoom Out";
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tpgPpuView);
			this.tabMain.Controls.Add(this.tpgListView);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 24);
			this.tabMain.Margin = new System.Windows.Forms.Padding(0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(669, 567);
			this.tabMain.TabIndex = 11;
			this.tabMain.SelectedIndexChanged += new System.EventHandler(this.tabMain_SelectedIndexChanged);
			// 
			// tpgPpuView
			// 
			this.tpgPpuView.Controls.Add(this.ctrlPpuView);
			this.tpgPpuView.Location = new System.Drawing.Point(4, 22);
			this.tpgPpuView.Margin = new System.Windows.Forms.Padding(0);
			this.tpgPpuView.Name = "tpgPpuView";
			this.tpgPpuView.Size = new System.Drawing.Size(661, 541);
			this.tpgPpuView.TabIndex = 0;
			this.tpgPpuView.Text = "PPU View";
			this.tpgPpuView.UseVisualStyleBackColor = true;
			// 
			// tpgListView
			// 
			this.tpgListView.Controls.Add(this.ctrlListView);
			this.tpgListView.Location = new System.Drawing.Point(4, 22);
			this.tpgListView.Margin = new System.Windows.Forms.Padding(0);
			this.tpgListView.Name = "tpgListView";
			this.tpgListView.Size = new System.Drawing.Size(675, 541);
			this.tpgListView.TabIndex = 1;
			this.tpgListView.Text = "List View";
			this.tpgListView.UseVisualStyleBackColor = true;
			// 
			// ctrlListView
			// 
			this.ctrlListView.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlListView.Location = new System.Drawing.Point(0, 0);
			this.ctrlListView.Margin = new System.Windows.Forms.Padding(0);
			this.ctrlListView.Name = "ctrlListView";
			this.ctrlListView.Size = new System.Drawing.Size(675, 541);
			this.ctrlListView.TabIndex = 2;
			// 
			// ctrlFilters
			// 
			this.ctrlFilters.Dock = System.Windows.Forms.DockStyle.Right;
			this.ctrlFilters.Location = new System.Drawing.Point(669, 24);
			this.ctrlFilters.Name = "ctrlFilters";
			this.ctrlFilters.Padding = new System.Windows.Forms.Padding(0, 0, 2, 0);
			this.ctrlFilters.Size = new System.Drawing.Size(277, 567);
			this.ctrlFilters.TabIndex = 12;
			this.ctrlFilters.OptionsChanged += new System.EventHandler(this.ctrlFilters_OptionsChanged);
			// 
			// frmEventViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(946, 591);
			this.Controls.Add(this.tabMain);
			this.Controls.Add(this.ctrlFilters);
			this.Controls.Add(this.mnuMain);
			this.Name = "frmEventViewer";
			this.Text = "Event Viewer";
			this.Controls.SetChildIndex(this.mnuMain, 0);
			this.Controls.SetChildIndex(this.ctrlFilters, 0);
			this.Controls.SetChildIndex(this.tabMain, 0);
			this.mnuMain.ResumeLayout(false);
			this.mnuMain.PerformLayout();
			this.tabMain.ResumeLayout(false);
			this.tpgPpuView.ResumeLayout(false);
			this.tpgListView.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private ctrlEventViewerPpuView ctrlPpuView;
		private GUI.Controls.ctrlMesenMenuStrip mnuMain;
		private System.Windows.Forms.ToolStripMenuItem mnuFile;
		private System.Windows.Forms.ToolStripMenuItem mnuClose;
		private System.Windows.Forms.ToolStripMenuItem mnuView;
		private System.Windows.Forms.ToolStripMenuItem mnuRefreshOnBreakPause;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuZoomIn;
		private System.Windows.Forms.ToolStripMenuItem mnuZoomOut;
		private System.Windows.Forms.ToolStripMenuItem mnuRefresh;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoRefresh;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoRefreshSpeed;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoRefreshLow;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoRefreshNormal;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoRefreshHigh;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tpgPpuView;
		private System.Windows.Forms.TabPage tpgListView;
		private ctrlEventViewerFilters ctrlFilters;
		private ctrlEventViewerListView ctrlListView;
	}
}