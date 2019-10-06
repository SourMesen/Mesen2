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
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuAutoRefresh = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefreshSpeed = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefreshLow = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefreshNormal = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefreshHigh = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRefreshOnBreakPause = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuZoomIn = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuZoomOut = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuMain.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlPpuView
			// 
			this.ctrlPpuView.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlPpuView.ImageScale = 1;
			this.ctrlPpuView.Location = new System.Drawing.Point(0, 24);
			this.ctrlPpuView.Name = "ctrlPpuView";
			this.ctrlPpuView.Size = new System.Drawing.Size(945, 531);
			this.ctrlPpuView.TabIndex = 0;
			// 
			// mnuMain
			// 
			this.mnuMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFile,
            this.mnuView});
			this.mnuMain.Location = new System.Drawing.Point(0, 0);
			this.mnuMain.Name = "mnuMain";
			this.mnuMain.Size = new System.Drawing.Size(945, 24);
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
			// frmEventViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(945, 555);
			this.Controls.Add(this.ctrlPpuView);
			this.Controls.Add(this.mnuMain);
			this.Name = "frmEventViewer";
			this.Text = "Event Viewer";
			this.Controls.SetChildIndex(this.mnuMain, 0);
			this.Controls.SetChildIndex(this.ctrlPpuView, 0);
			this.mnuMain.ResumeLayout(false);
			this.mnuMain.PerformLayout();
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
	}
}