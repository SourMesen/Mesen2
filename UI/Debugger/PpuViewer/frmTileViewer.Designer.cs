namespace Mesen.GUI.Debugger
{
	partial class frmTileViewer
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
			this.ctrlScanlineCycleSelect = new Mesen.GUI.Debugger.Controls.ctrlScanlineCycleSelect();
			this.pnlTilemap = new System.Windows.Forms.Panel();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.cboMemoryType = new Mesen.GUI.Debugger.Controls.ComboBoxWithSeparator();
			this.nudBank = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblBank = new System.Windows.Forms.Label();
			this.lblSource = new System.Windows.Forms.Label();
			this.lblColumns = new System.Windows.Forms.Label();
			this.chkShowTileGrid = new System.Windows.Forms.CheckBox();
			this.lblBpp = new System.Windows.Forms.Label();
			this.cboFormat = new System.Windows.Forms.ComboBox();
			this.nudColumns = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblOffset = new System.Windows.Forms.Label();
			this.nudOffset = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.ctrlPaletteViewer = new Mesen.GUI.Debugger.ctrlPaletteViewer();
			((System.ComponentModel.ISupportInitialize)(this.picTilemap)).BeginInit();
			this.pnlTilemap.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
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
			this.picTilemap.DoubleClick += new System.EventHandler(this.picTilemap_DoubleClick);
			// 
			// ctrlScanlineCycleSelect
			// 
			this.ctrlScanlineCycleSelect.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.ctrlScanlineCycleSelect.Location = new System.Drawing.Point(0, 546);
			this.ctrlScanlineCycleSelect.Name = "ctrlScanlineCycleSelect";
			this.ctrlScanlineCycleSelect.Size = new System.Drawing.Size(737, 28);
			this.ctrlScanlineCycleSelect.TabIndex = 5;
			// 
			// pnlTilemap
			// 
			this.pnlTilemap.AutoScroll = true;
			this.pnlTilemap.Controls.Add(this.picTilemap);
			this.pnlTilemap.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pnlTilemap.Location = new System.Drawing.Point(3, 3);
			this.pnlTilemap.MinimumSize = new System.Drawing.Size(512, 512);
			this.pnlTilemap.Name = "pnlTilemap";
			this.pnlTilemap.Size = new System.Drawing.Size(531, 540);
			this.pnlTilemap.TabIndex = 6;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.pnlTilemap, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 1, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 1;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(737, 546);
			this.tableLayoutPanel1.TabIndex = 7;
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 2;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.Controls.Add(this.cboMemoryType, 1, 0);
			this.tableLayoutPanel2.Controls.Add(this.nudBank, 1, 3);
			this.tableLayoutPanel2.Controls.Add(this.lblBank, 0, 3);
			this.tableLayoutPanel2.Controls.Add(this.lblSource, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.lblColumns, 0, 2);
			this.tableLayoutPanel2.Controls.Add(this.chkShowTileGrid, 0, 5);
			this.tableLayoutPanel2.Controls.Add(this.lblBpp, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.cboFormat, 1, 1);
			this.tableLayoutPanel2.Controls.Add(this.nudColumns, 1, 2);
			this.tableLayoutPanel2.Controls.Add(this.lblOffset, 0, 4);
			this.tableLayoutPanel2.Controls.Add(this.nudOffset, 1, 4);
			this.tableLayoutPanel2.Controls.Add(this.ctrlPaletteViewer, 0, 6);
			this.tableLayoutPanel2.Location = new System.Drawing.Point(540, 3);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 7;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(194, 457);
			this.tableLayoutPanel2.TabIndex = 7;
			// 
			// cboMemoryType
			// 
			this.cboMemoryType.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
			this.cboMemoryType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboMemoryType.FormattingEnabled = true;
			this.cboMemoryType.Location = new System.Drawing.Point(59, 3);
			this.cboMemoryType.Name = "cboMemoryType";
			this.cboMemoryType.Size = new System.Drawing.Size(132, 21);
			this.cboMemoryType.TabIndex = 11;
			this.cboMemoryType.SelectedIndexChanged += new System.EventHandler(this.cboMemoryType_SelectedIndexChanged);
			// 
			// nudBank
			// 
			this.nudBank.DecimalPlaces = 0;
			this.nudBank.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudBank.Location = new System.Drawing.Point(59, 84);
			this.nudBank.Maximum = new decimal(new int[] {
            63,
            0,
            0,
            0});
			this.nudBank.MaximumSize = new System.Drawing.Size(10000, 21);
			this.nudBank.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudBank.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudBank.Name = "nudBank";
			this.nudBank.Size = new System.Drawing.Size(43, 21);
			this.nudBank.TabIndex = 10;
			this.nudBank.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudBank.ValueChanged += new System.EventHandler(this.nudBank_ValueChanged);
			// 
			// lblBank
			// 
			this.lblBank.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblBank.AutoSize = true;
			this.lblBank.Location = new System.Drawing.Point(3, 88);
			this.lblBank.Name = "lblBank";
			this.lblBank.Size = new System.Drawing.Size(35, 13);
			this.lblBank.TabIndex = 9;
			this.lblBank.Text = "Bank:";
			// 
			// lblSource
			// 
			this.lblSource.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblSource.AutoSize = true;
			this.lblSource.Location = new System.Drawing.Point(3, 7);
			this.lblSource.Name = "lblSource";
			this.lblSource.Size = new System.Drawing.Size(44, 13);
			this.lblSource.TabIndex = 7;
			this.lblSource.Text = "Source:";
			// 
			// lblColumns
			// 
			this.lblColumns.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblColumns.AutoSize = true;
			this.lblColumns.Location = new System.Drawing.Point(3, 61);
			this.lblColumns.Name = "lblColumns";
			this.lblColumns.Size = new System.Drawing.Size(50, 13);
			this.lblColumns.TabIndex = 3;
			this.lblColumns.Text = "Columns:";
			// 
			// chkShowTileGrid
			// 
			this.chkShowTileGrid.AutoSize = true;
			this.tableLayoutPanel2.SetColumnSpan(this.chkShowTileGrid, 2);
			this.chkShowTileGrid.Location = new System.Drawing.Point(3, 138);
			this.chkShowTileGrid.Name = "chkShowTileGrid";
			this.chkShowTileGrid.Size = new System.Drawing.Size(89, 17);
			this.chkShowTileGrid.TabIndex = 0;
			this.chkShowTileGrid.Text = "Show tile grid";
			this.chkShowTileGrid.UseVisualStyleBackColor = true;
			this.chkShowTileGrid.Click += new System.EventHandler(this.chkShowTileGrid_Click);
			// 
			// lblBpp
			// 
			this.lblBpp.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblBpp.AutoSize = true;
			this.lblBpp.Location = new System.Drawing.Point(3, 34);
			this.lblBpp.Name = "lblBpp";
			this.lblBpp.Size = new System.Drawing.Size(42, 13);
			this.lblBpp.TabIndex = 1;
			this.lblBpp.Text = "Format:";
			// 
			// cboFormat
			// 
			this.cboFormat.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboFormat.FormattingEnabled = true;
			this.cboFormat.Items.AddRange(new object[] {
            "2 BPP",
            "4 BPP",
            "8 BPP",
            "8 BPP - Direct Color Mode",
            "Mode 7",
            "Mode 7 - Direct Color Mode"});
			this.cboFormat.Location = new System.Drawing.Point(59, 30);
			this.cboFormat.Name = "cboFormat";
			this.cboFormat.Size = new System.Drawing.Size(132, 21);
			this.cboFormat.TabIndex = 2;
			this.cboFormat.SelectedIndexChanged += new System.EventHandler(this.cboBpp_SelectedIndexChanged);
			// 
			// nudColumns
			// 
			this.nudColumns.DecimalPlaces = 0;
			this.nudColumns.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudColumns.Location = new System.Drawing.Point(59, 57);
			this.nudColumns.Maximum = new decimal(new int[] {
            64,
            0,
            0,
            0});
			this.nudColumns.MaximumSize = new System.Drawing.Size(10000, 21);
			this.nudColumns.Minimum = new decimal(new int[] {
            8,
            0,
            0,
            0});
			this.nudColumns.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudColumns.Name = "nudColumns";
			this.nudColumns.Size = new System.Drawing.Size(43, 21);
			this.nudColumns.TabIndex = 4;
			this.nudColumns.Value = new decimal(new int[] {
            32,
            0,
            0,
            0});
			this.nudColumns.ValueChanged += new System.EventHandler(this.nudColumns_ValueChanged);
			// 
			// lblOffset
			// 
			this.lblOffset.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblOffset.AutoSize = true;
			this.lblOffset.Location = new System.Drawing.Point(3, 115);
			this.lblOffset.Name = "lblOffset";
			this.lblOffset.Size = new System.Drawing.Size(38, 13);
			this.lblOffset.TabIndex = 5;
			this.lblOffset.Text = "Offset:";
			// 
			// nudOffset
			// 
			this.nudOffset.DecimalPlaces = 0;
			this.nudOffset.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudOffset.Location = new System.Drawing.Point(59, 111);
			this.nudOffset.Maximum = new decimal(new int[] {
            63,
            0,
            0,
            0});
			this.nudOffset.MaximumSize = new System.Drawing.Size(10000, 21);
			this.nudOffset.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudOffset.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudOffset.Name = "nudOffset";
			this.nudOffset.Size = new System.Drawing.Size(43, 21);
			this.nudOffset.TabIndex = 6;
			this.nudOffset.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudOffset.ValueChanged += new System.EventHandler(this.nudOffset_ValueChanged);
			// 
			// ctrlPaletteViewer
			// 
			this.tableLayoutPanel2.SetColumnSpan(this.ctrlPaletteViewer, 2);
			this.ctrlPaletteViewer.Location = new System.Drawing.Point(3, 161);
			this.ctrlPaletteViewer.Name = "ctrlPaletteViewer";
			this.ctrlPaletteViewer.PaletteScale = 11;
			this.ctrlPaletteViewer.SelectionMode = Mesen.GUI.Debugger.PaletteSelectionMode.None;
			this.ctrlPaletteViewer.Size = new System.Drawing.Size(176, 176);
			this.ctrlPaletteViewer.TabIndex = 12;
			// 
			// frmTileViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(737, 574);
			this.Controls.Add(this.tableLayoutPanel1);
			this.Controls.Add(this.ctrlScanlineCycleSelect);
			this.Name = "frmTileViewer";
			this.Text = "Tile Viewer";
			((System.ComponentModel.ISupportInitialize)(this.picTilemap)).EndInit();
			this.pnlTilemap.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private GUI.Controls.ctrlMesenPictureBox picTilemap;
		private Controls.ctrlScanlineCycleSelect ctrlScanlineCycleSelect;
		private System.Windows.Forms.Panel pnlTilemap;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.CheckBox chkShowTileGrid;
		private GUI.Controls.MesenNumericUpDown nudBank;
		private System.Windows.Forms.Label lblBank;
		private System.Windows.Forms.Label lblSource;
		private System.Windows.Forms.Label lblColumns;
		private System.Windows.Forms.Label lblBpp;
		private System.Windows.Forms.ComboBox cboFormat;
		private GUI.Controls.MesenNumericUpDown nudColumns;
		private System.Windows.Forms.Label lblOffset;
		private GUI.Controls.MesenNumericUpDown nudOffset;
		private Controls.ComboBoxWithSeparator cboMemoryType;
		private ctrlPaletteViewer ctrlPaletteViewer;
	}
}