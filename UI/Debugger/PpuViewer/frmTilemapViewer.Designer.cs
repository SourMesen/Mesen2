namespace Mesen.GUI.Debugger
{
	partial class frmTilemapViewer
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
			this.picTilemap = new Mesen.GUI.Controls.ctrlMesenPictureBox();
			this.btnLayer1 = new System.Windows.Forms.Button();
			this.btnLayer2 = new System.Windows.Forms.Button();
			this.btnLayer3 = new System.Windows.Forms.Button();
			this.btnLayer4 = new System.Windows.Forms.Button();
			this.ctrlScanlineCycleSelect = new Mesen.GUI.Debugger.Controls.ctrlScanlineCycleSelect();
			this.pnlTilemap = new System.Windows.Forms.Panel();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.chkShowScrollOverlay = new System.Windows.Forms.CheckBox();
			this.chkShowTileGrid = new System.Windows.Forms.CheckBox();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.lblLayer = new System.Windows.Forms.Label();
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
			((System.ComponentModel.ISupportInitialize)(this.picTilemap)).BeginInit();
			this.pnlTilemap.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.tableLayoutPanel3.SuspendLayout();
			this.ctrlMesenMenuStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// picTilemap
			// 
			this.picTilemap.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
			this.picTilemap.Location = new System.Drawing.Point(0, 0);
			this.picTilemap.Name = "picTilemap";
			this.picTilemap.Size = new System.Drawing.Size(512, 512);
			this.picTilemap.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
			this.picTilemap.TabIndex = 0;
			this.picTilemap.TabStop = false;
			// 
			// btnLayer1
			// 
			this.btnLayer1.BackColor = System.Drawing.SystemColors.ControlLight;
			this.btnLayer1.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btnLayer1.Location = new System.Drawing.Point(45, 3);
			this.btnLayer1.Name = "btnLayer1";
			this.btnLayer1.Size = new System.Drawing.Size(32, 22);
			this.btnLayer1.TabIndex = 1;
			this.btnLayer1.Text = "1";
			this.btnLayer1.UseVisualStyleBackColor = false;
			this.btnLayer1.Click += new System.EventHandler(this.btnLayer1_Click);
			// 
			// btnLayer2
			// 
			this.btnLayer2.BackColor = System.Drawing.SystemColors.ControlLight;
			this.btnLayer2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btnLayer2.Location = new System.Drawing.Point(83, 3);
			this.btnLayer2.Name = "btnLayer2";
			this.btnLayer2.Size = new System.Drawing.Size(32, 22);
			this.btnLayer2.TabIndex = 2;
			this.btnLayer2.Text = "2";
			this.btnLayer2.UseVisualStyleBackColor = false;
			this.btnLayer2.Click += new System.EventHandler(this.btnLayer2_Click);
			// 
			// btnLayer3
			// 
			this.btnLayer3.BackColor = System.Drawing.SystemColors.ControlLight;
			this.btnLayer3.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btnLayer3.Location = new System.Drawing.Point(121, 3);
			this.btnLayer3.Name = "btnLayer3";
			this.btnLayer3.Size = new System.Drawing.Size(32, 22);
			this.btnLayer3.TabIndex = 3;
			this.btnLayer3.Text = "3";
			this.btnLayer3.UseVisualStyleBackColor = false;
			this.btnLayer3.Click += new System.EventHandler(this.btnLayer3_Click);
			// 
			// btnLayer4
			// 
			this.btnLayer4.BackColor = System.Drawing.SystemColors.ControlLight;
			this.btnLayer4.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btnLayer4.Location = new System.Drawing.Point(159, 3);
			this.btnLayer4.Name = "btnLayer4";
			this.btnLayer4.Size = new System.Drawing.Size(32, 22);
			this.btnLayer4.TabIndex = 4;
			this.btnLayer4.Text = "4";
			this.btnLayer4.UseVisualStyleBackColor = false;
			this.btnLayer4.Click += new System.EventHandler(this.btnLayer4_Click);
			// 
			// ctrlScanlineCycleSelect
			// 
			this.ctrlScanlineCycleSelect.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.ctrlScanlineCycleSelect.Location = new System.Drawing.Point(0, 546);
			this.ctrlScanlineCycleSelect.Name = "ctrlScanlineCycleSelect";
			this.ctrlScanlineCycleSelect.Size = new System.Drawing.Size(667, 28);
			this.ctrlScanlineCycleSelect.TabIndex = 5;
			// 
			// pnlTilemap
			// 
			this.pnlTilemap.AutoScroll = true;
			this.pnlTilemap.Controls.Add(this.picTilemap);
			this.pnlTilemap.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pnlTilemap.Location = new System.Drawing.Point(3, 31);
			this.pnlTilemap.Name = "pnlTilemap";
			this.pnlTilemap.Size = new System.Drawing.Size(512, 488);
			this.pnlTilemap.TabIndex = 6;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.pnlTilemap, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel3, 0, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 24);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(667, 522);
			this.tableLayoutPanel1.TabIndex = 7;
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 1;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Controls.Add(this.chkShowScrollOverlay, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkShowTileGrid, 0, 0);
			this.tableLayoutPanel2.Location = new System.Drawing.Point(521, 3);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 3;
			this.tableLayoutPanel1.SetRowSpan(this.tableLayoutPanel2, 2);
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(143, 320);
			this.tableLayoutPanel2.TabIndex = 7;
			// 
			// chkShowScrollOverlay
			// 
			this.chkShowScrollOverlay.AutoSize = true;
			this.chkShowScrollOverlay.Location = new System.Drawing.Point(3, 26);
			this.chkShowScrollOverlay.Name = "chkShowScrollOverlay";
			this.chkShowScrollOverlay.Size = new System.Drawing.Size(117, 17);
			this.chkShowScrollOverlay.TabIndex = 1;
			this.chkShowScrollOverlay.Text = "Show scroll overlay";
			this.chkShowScrollOverlay.UseVisualStyleBackColor = true;
			this.chkShowScrollOverlay.Click += new System.EventHandler(this.chkShowScrollOverlay_Click);
			// 
			// chkShowTileGrid
			// 
			this.chkShowTileGrid.AutoSize = true;
			this.chkShowTileGrid.Location = new System.Drawing.Point(3, 3);
			this.chkShowTileGrid.Name = "chkShowTileGrid";
			this.chkShowTileGrid.Size = new System.Drawing.Size(89, 17);
			this.chkShowTileGrid.TabIndex = 0;
			this.chkShowTileGrid.Text = "Show tile grid";
			this.chkShowTileGrid.UseVisualStyleBackColor = true;
			this.chkShowTileGrid.Click += new System.EventHandler(this.chkShowTileGrid_Click);
			// 
			// tableLayoutPanel3
			// 
			this.tableLayoutPanel3.ColumnCount = 6;
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Controls.Add(this.btnLayer4, 4, 0);
			this.tableLayoutPanel3.Controls.Add(this.btnLayer2, 2, 0);
			this.tableLayoutPanel3.Controls.Add(this.btnLayer3, 3, 0);
			this.tableLayoutPanel3.Controls.Add(this.btnLayer1, 1, 0);
			this.tableLayoutPanel3.Controls.Add(this.lblLayer, 0, 0);
			this.tableLayoutPanel3.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel3.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel3.Name = "tableLayoutPanel3";
			this.tableLayoutPanel3.RowCount = 1;
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Size = new System.Drawing.Size(205, 28);
			this.tableLayoutPanel3.TabIndex = 8;
			// 
			// lblLayer
			// 
			this.lblLayer.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblLayer.AutoSize = true;
			this.lblLayer.Location = new System.Drawing.Point(3, 7);
			this.lblLayer.Name = "lblLayer";
			this.lblLayer.Size = new System.Drawing.Size(36, 13);
			this.lblLayer.TabIndex = 5;
			this.lblLayer.Text = "Layer:";
			// 
			// ctrlMesenMenuStrip1
			// 
			this.ctrlMesenMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFile,
            this.mnuView});
			this.ctrlMesenMenuStrip1.Location = new System.Drawing.Point(0, 0);
			this.ctrlMesenMenuStrip1.Name = "ctrlMesenMenuStrip1";
			this.ctrlMesenMenuStrip1.Size = new System.Drawing.Size(667, 24);
			this.ctrlMesenMenuStrip1.TabIndex = 8;
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
			this.mnuClose.Size = new System.Drawing.Size(152, 22);
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
			this.mnuAutoRefresh.Size = new System.Drawing.Size(152, 22);
			this.mnuAutoRefresh.Text = "Auto-refresh";
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(149, 6);
			// 
			// mnuRefresh
			// 
			this.mnuRefresh.Image = global::Mesen.GUI.Properties.Resources.Refresh;
			this.mnuRefresh.Name = "mnuRefresh";
			this.mnuRefresh.Size = new System.Drawing.Size(152, 22);
			this.mnuRefresh.Text = "Refresh";
			this.mnuRefresh.Click += new System.EventHandler(this.mnuRefresh_Click);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(149, 6);
			// 
			// mnuZoomIn
			// 
			this.mnuZoomIn.Name = "mnuZoomIn";
			this.mnuZoomIn.Size = new System.Drawing.Size(152, 22);
			this.mnuZoomIn.Text = "Zoom In";
			this.mnuZoomIn.Click += new System.EventHandler(this.mnuZoomIn_Click);
			// 
			// mnuZoomOut
			// 
			this.mnuZoomOut.Name = "mnuZoomOut";
			this.mnuZoomOut.Size = new System.Drawing.Size(152, 22);
			this.mnuZoomOut.Text = "Zoom Out";
			this.mnuZoomOut.Click += new System.EventHandler(this.mnuZoomOut_Click);
			// 
			// frmTilemapViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(667, 574);
			this.Controls.Add(this.tableLayoutPanel1);
			this.Controls.Add(this.ctrlScanlineCycleSelect);
			this.Controls.Add(this.ctrlMesenMenuStrip1);
			this.MainMenuStrip = this.ctrlMesenMenuStrip1;
			this.Name = "frmTilemapViewer";
			this.Text = "Tilemap Viewer";
			((System.ComponentModel.ISupportInitialize)(this.picTilemap)).EndInit();
			this.pnlTilemap.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel3.PerformLayout();
			this.ctrlMesenMenuStrip1.ResumeLayout(false);
			this.ctrlMesenMenuStrip1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private GUI.Controls.ctrlMesenPictureBox picTilemap;
		private System.Windows.Forms.Button btnLayer1;
		private System.Windows.Forms.Button btnLayer2;
		private System.Windows.Forms.Button btnLayer3;
		private System.Windows.Forms.Button btnLayer4;
		private Controls.ctrlScanlineCycleSelect ctrlScanlineCycleSelect;
		private System.Windows.Forms.Panel pnlTilemap;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.CheckBox chkShowTileGrid;
		private System.Windows.Forms.CheckBox chkShowScrollOverlay;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
		private System.Windows.Forms.Label lblLayer;
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
	}
}