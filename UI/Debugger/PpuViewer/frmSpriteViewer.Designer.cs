namespace Mesen.GUI.Debugger
{
	partial class frmSpriteViewer
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
			this.ctrlSplitContainer = new Mesen.GUI.Controls.ctrlSplitContainer();
			this.ctrlImagePanel = new Mesen.GUI.Debugger.PpuViewer.ctrlImagePanel();
			this.ctrlSpriteList = new Mesen.GUI.Debugger.PpuViewer.ctrlSpriteList();
			this.ctrlMesenMenuStrip1 = new Mesen.GUI.Controls.ctrlMesenMenuStrip();
			this.mnuFile = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuClose = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuView = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefresh = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRefresh = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuZoomIn = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuZoomOut = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuCopyToClipboard = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuSaveAsPng = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
			((System.ComponentModel.ISupportInitialize)(this.ctrlSplitContainer)).BeginInit();
			this.ctrlSplitContainer.Panel1.SuspendLayout();
			this.ctrlSplitContainer.Panel2.SuspendLayout();
			this.ctrlSplitContainer.SuspendLayout();
			this.ctrlMesenMenuStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlScanlineCycleSelect
			// 
			this.ctrlScanlineCycleSelect.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.ctrlScanlineCycleSelect.Location = new System.Drawing.Point(0, 512);
			this.ctrlScanlineCycleSelect.Name = "ctrlScanlineCycleSelect";
			this.ctrlScanlineCycleSelect.Size = new System.Drawing.Size(842, 28);
			this.ctrlScanlineCycleSelect.TabIndex = 5;
			// 
			// ctrlSplitContainer
			// 
			this.ctrlSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlSplitContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
			this.ctrlSplitContainer.HidePanel2 = false;
			this.ctrlSplitContainer.Location = new System.Drawing.Point(0, 24);
			this.ctrlSplitContainer.Name = "ctrlSplitContainer";
			// 
			// ctrlSplitContainer.Panel1
			// 
			this.ctrlSplitContainer.Panel1.Controls.Add(this.ctrlImagePanel);
			this.ctrlSplitContainer.Panel1MinSize = 256;
			// 
			// ctrlSplitContainer.Panel2
			// 
			this.ctrlSplitContainer.Panel2.Controls.Add(this.ctrlSpriteList);
			this.ctrlSplitContainer.Panel2.Padding = new System.Windows.Forms.Padding(2, 0, 0, 0);
			this.ctrlSplitContainer.Panel2MinSize = 100;
			this.ctrlSplitContainer.Size = new System.Drawing.Size(842, 488);
			this.ctrlSplitContainer.SplitterDistance = 512;
			this.ctrlSplitContainer.TabIndex = 8;
			// 
			// ctrlImagePanel
			// 
			this.ctrlImagePanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlImagePanel.GridSizeX = 0;
			this.ctrlImagePanel.GridSizeY = 0;
			this.ctrlImagePanel.Image = null;
			this.ctrlImagePanel.ImageScale = 1;
			this.ctrlImagePanel.ImageSize = new System.Drawing.Size(0, 0);
			this.ctrlImagePanel.Location = new System.Drawing.Point(0, 0);
			this.ctrlImagePanel.Name = "ctrlImagePanel";
			this.ctrlImagePanel.Overlay = new System.Drawing.Rectangle(0, 0, 0, 0);
			this.ctrlImagePanel.Selection = new System.Drawing.Rectangle(0, 0, 0, 0);
			this.ctrlImagePanel.SelectionWrapPosition = 0;
			this.ctrlImagePanel.Size = new System.Drawing.Size(512, 488);
			this.ctrlImagePanel.TabIndex = 9;
			this.ctrlImagePanel.MouseClick += new System.Windows.Forms.MouseEventHandler(this.ctrlImagePanel_MouseClick);
			// 
			// ctrlSpriteList
			// 
			this.ctrlSpriteList.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlSpriteList.Location = new System.Drawing.Point(2, 0);
			this.ctrlSpriteList.Name = "ctrlSpriteList";
			this.ctrlSpriteList.Size = new System.Drawing.Size(324, 488);
			this.ctrlSpriteList.TabIndex = 0;
			this.ctrlSpriteList.SpriteSelected += new Mesen.GUI.Debugger.PpuViewer.ctrlSpriteList.SpriteSelectedHandler(this.ctrlSpriteList_SpriteSelected);
			// 
			// ctrlMesenMenuStrip1
			// 
			this.ctrlMesenMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFile,
            this.mnuView});
			this.ctrlMesenMenuStrip1.Location = new System.Drawing.Point(0, 0);
			this.ctrlMesenMenuStrip1.Name = "ctrlMesenMenuStrip1";
			this.ctrlMesenMenuStrip1.Size = new System.Drawing.Size(842, 24);
			this.ctrlMesenMenuStrip1.TabIndex = 8;
			this.ctrlMesenMenuStrip1.Text = "ctrlMesenMenuStrip1";
			// 
			// mnuFile
			// 
			this.mnuFile.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuCopyToClipboard,
            this.mnuSaveAsPng,
            this.toolStripMenuItem3,
            this.mnuClose});
			this.mnuFile.Name = "mnuFile";
			this.mnuFile.Size = new System.Drawing.Size(37, 20);
			this.mnuFile.Text = "File";
			// 
			// mnuClose
			// 
			this.mnuClose.Image = global::Mesen.GUI.Properties.Resources.Exit;
			this.mnuClose.Name = "mnuClose";
			this.mnuClose.Size = new System.Drawing.Size(169, 22);
			this.mnuClose.Text = "Close";
			this.mnuClose.Click += new System.EventHandler(this.mnuClose_Click);
			// 
			// mnuView
			// 
			this.mnuView.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuAutoRefresh,
            this.toolStripMenuItem2,
            this.mnuRefresh,
            this.toolStripMenuItem1,
            this.mnuZoomIn,
            this.mnuZoomOut});
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
			this.mnuAutoRefresh.CheckedChanged += new System.EventHandler(this.mnuAutoRefresh_CheckedChanged);
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
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(138, 6);
			// 
			// mnuZoomIn
			// 
			this.mnuZoomIn.Name = "mnuZoomIn";
			this.mnuZoomIn.Size = new System.Drawing.Size(141, 22);
			this.mnuZoomIn.Text = "Zoom In";
			this.mnuZoomIn.Click += new System.EventHandler(this.mnuZoomIn_Click);
			// 
			// mnuZoomOut
			// 
			this.mnuZoomOut.Name = "mnuZoomOut";
			this.mnuZoomOut.Size = new System.Drawing.Size(141, 22);
			this.mnuZoomOut.Text = "Zoom Out";
			this.mnuZoomOut.Click += new System.EventHandler(this.mnuZoomOut_Click);
			// 
			// mnuCopyToClipboard
			// 
			this.mnuCopyToClipboard.Image = global::Mesen.GUI.Properties.Resources.Copy;
			this.mnuCopyToClipboard.Name = "mnuCopyToClipboard";
			this.mnuCopyToClipboard.Size = new System.Drawing.Size(169, 22);
			this.mnuCopyToClipboard.Text = "Copy to clipboard";
			// 
			// mnuSaveAsPng
			// 
			this.mnuSaveAsPng.Image = global::Mesen.GUI.Properties.Resources.Export;
			this.mnuSaveAsPng.Name = "mnuSaveAsPng";
			this.mnuSaveAsPng.Size = new System.Drawing.Size(169, 22);
			this.mnuSaveAsPng.Text = "Save as PNG";
			// 
			// toolStripMenuItem3
			// 
			this.toolStripMenuItem3.Name = "toolStripMenuItem3";
			this.toolStripMenuItem3.Size = new System.Drawing.Size(166, 6);
			// 
			// frmSpriteViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(842, 540);
			this.Controls.Add(this.ctrlSplitContainer);
			this.Controls.Add(this.ctrlScanlineCycleSelect);
			this.Controls.Add(this.ctrlMesenMenuStrip1);
			this.MainMenuStrip = this.ctrlMesenMenuStrip1;
			this.Name = "frmSpriteViewer";
			this.Text = "Sprite Viewer";
			this.Controls.SetChildIndex(this.ctrlMesenMenuStrip1, 0);
			this.Controls.SetChildIndex(this.ctrlScanlineCycleSelect, 0);
			this.Controls.SetChildIndex(this.ctrlSplitContainer, 0);
			this.ctrlSplitContainer.Panel1.ResumeLayout(false);
			this.ctrlSplitContainer.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.ctrlSplitContainer)).EndInit();
			this.ctrlSplitContainer.ResumeLayout(false);
			this.ctrlMesenMenuStrip1.ResumeLayout(false);
			this.ctrlMesenMenuStrip1.PerformLayout();
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
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuZoomIn;
		private System.Windows.Forms.ToolStripMenuItem mnuZoomOut;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuRefresh;
		private PpuViewer.ctrlImagePanel ctrlImagePanel;
		private GUI.Controls.ctrlSplitContainer ctrlSplitContainer;
		private PpuViewer.ctrlSpriteList ctrlSpriteList;
		private System.Windows.Forms.ToolStripMenuItem mnuCopyToClipboard;
		private System.Windows.Forms.ToolStripMenuItem mnuSaveAsPng;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
	}
}