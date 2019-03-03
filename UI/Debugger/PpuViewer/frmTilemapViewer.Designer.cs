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
			((System.ComponentModel.ISupportInitialize)(this.picTilemap)).BeginInit();
			this.pnlTilemap.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.tableLayoutPanel3.SuspendLayout();
			this.SuspendLayout();
			// 
			// picTilemap
			// 
			this.picTilemap.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
			this.picTilemap.Location = new System.Drawing.Point(0, 0);
			this.picTilemap.MinimumSize = new System.Drawing.Size(256, 256);
			this.picTilemap.Name = "picTilemap";
			this.picTilemap.Size = new System.Drawing.Size(512, 512);
			this.picTilemap.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
			this.picTilemap.TabIndex = 0;
			this.picTilemap.TabStop = false;
			this.picTilemap.DoubleClick += new System.EventHandler(this.picTilemap_DoubleClick);
			// 
			// btnLayer1
			// 
			this.btnLayer1.Location = new System.Drawing.Point(45, 3);
			this.btnLayer1.Name = "btnLayer1";
			this.btnLayer1.Size = new System.Drawing.Size(32, 22);
			this.btnLayer1.TabIndex = 1;
			this.btnLayer1.Text = "1";
			this.btnLayer1.UseVisualStyleBackColor = true;
			this.btnLayer1.Click += new System.EventHandler(this.btnLayer1_Click);
			// 
			// btnLayer2
			// 
			this.btnLayer2.Location = new System.Drawing.Point(83, 3);
			this.btnLayer2.Name = "btnLayer2";
			this.btnLayer2.Size = new System.Drawing.Size(32, 22);
			this.btnLayer2.TabIndex = 2;
			this.btnLayer2.Text = "2";
			this.btnLayer2.UseVisualStyleBackColor = true;
			this.btnLayer2.Click += new System.EventHandler(this.btnLayer2_Click);
			// 
			// btnLayer3
			// 
			this.btnLayer3.Location = new System.Drawing.Point(121, 3);
			this.btnLayer3.Name = "btnLayer3";
			this.btnLayer3.Size = new System.Drawing.Size(32, 22);
			this.btnLayer3.TabIndex = 3;
			this.btnLayer3.Text = "3";
			this.btnLayer3.UseVisualStyleBackColor = true;
			this.btnLayer3.Click += new System.EventHandler(this.btnLayer3_Click);
			// 
			// btnLayer4
			// 
			this.btnLayer4.Location = new System.Drawing.Point(159, 3);
			this.btnLayer4.Name = "btnLayer4";
			this.btnLayer4.Size = new System.Drawing.Size(32, 22);
			this.btnLayer4.TabIndex = 4;
			this.btnLayer4.Text = "4";
			this.btnLayer4.UseVisualStyleBackColor = true;
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
			this.pnlTilemap.MinimumSize = new System.Drawing.Size(512, 512);
			this.pnlTilemap.Name = "pnlTilemap";
			this.pnlTilemap.Size = new System.Drawing.Size(512, 512);
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
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(667, 546);
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
			// frmTilemapViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(667, 574);
			this.Controls.Add(this.tableLayoutPanel1);
			this.Controls.Add(this.ctrlScanlineCycleSelect);
			this.Name = "frmTilemapViewer";
			this.Text = "Tilemap Viewer";
			((System.ComponentModel.ISupportInitialize)(this.picTilemap)).EndInit();
			this.pnlTilemap.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel3.PerformLayout();
			this.ResumeLayout(false);

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
	}
}