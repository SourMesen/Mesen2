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
			this.ctrlScanlineCycleSelect = new Mesen.GUI.Debugger.Controls.ctrlScanlineCycleSelect();
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
			this.lblPresets = new System.Windows.Forms.Label();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.btnPresetBg1 = new System.Windows.Forms.Button();
			this.btnPresetBg2 = new System.Windows.Forms.Button();
			this.btnPresetBg3 = new System.Windows.Forms.Button();
			this.btnPresetBg4 = new System.Windows.Forms.Button();
			this.tableLayoutPanel4 = new System.Windows.Forms.TableLayoutPanel();
			this.btnPresetOam1 = new System.Windows.Forms.Button();
			this.btnPresetOam2 = new System.Windows.Forms.Button();
			this.lblTileAddress = new System.Windows.Forms.Label();
			this.txtTileAddress = new System.Windows.Forms.TextBox();
			this.lblTileLayout = new System.Windows.Forms.Label();
			this.cboLayout = new System.Windows.Forms.ComboBox();
			this.ctrlImagePanel = new Mesen.GUI.Debugger.PpuViewer.ctrlImagePanel();
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
			this.tableLayoutPanel1.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.ctrlPaletteViewer)).BeginInit();
			this.tableLayoutPanel3.SuspendLayout();
			this.tableLayoutPanel4.SuspendLayout();
			this.ctrlMesenMenuStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlScanlineCycleSelect
			// 
			this.ctrlScanlineCycleSelect.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.ctrlScanlineCycleSelect.Location = new System.Drawing.Point(0, 546);
			this.ctrlScanlineCycleSelect.Name = "ctrlScanlineCycleSelect";
			this.ctrlScanlineCycleSelect.Size = new System.Drawing.Size(737, 28);
			this.ctrlScanlineCycleSelect.TabIndex = 5;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.ctrlImagePanel, 0, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 24);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 1;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 522F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(737, 522);
			this.tableLayoutPanel1.TabIndex = 7;
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 2;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.Controls.Add(this.cboMemoryType, 1, 0);
			this.tableLayoutPanel2.Controls.Add(this.nudBank, 1, 4);
			this.tableLayoutPanel2.Controls.Add(this.lblBank, 0, 4);
			this.tableLayoutPanel2.Controls.Add(this.lblSource, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.lblColumns, 0, 3);
			this.tableLayoutPanel2.Controls.Add(this.chkShowTileGrid, 0, 6);
			this.tableLayoutPanel2.Controls.Add(this.lblBpp, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.cboFormat, 1, 1);
			this.tableLayoutPanel2.Controls.Add(this.nudColumns, 1, 3);
			this.tableLayoutPanel2.Controls.Add(this.lblOffset, 0, 5);
			this.tableLayoutPanel2.Controls.Add(this.nudOffset, 1, 5);
			this.tableLayoutPanel2.Controls.Add(this.ctrlPaletteViewer, 0, 9);
			this.tableLayoutPanel2.Controls.Add(this.lblPresets, 0, 7);
			this.tableLayoutPanel2.Controls.Add(this.tableLayoutPanel3, 1, 7);
			this.tableLayoutPanel2.Controls.Add(this.tableLayoutPanel4, 1, 8);
			this.tableLayoutPanel2.Controls.Add(this.lblTileAddress, 0, 10);
			this.tableLayoutPanel2.Controls.Add(this.txtTileAddress, 1, 10);
			this.tableLayoutPanel2.Controls.Add(this.lblTileLayout, 0, 2);
			this.tableLayoutPanel2.Controls.Add(this.cboLayout, 1, 2);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(540, 3);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 12;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(194, 516);
			this.tableLayoutPanel2.TabIndex = 7;
			// 
			// cboMemoryType
			// 
			this.cboMemoryType.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboMemoryType.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
			this.cboMemoryType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboMemoryType.FormattingEnabled = true;
			this.cboMemoryType.Location = new System.Drawing.Point(77, 3);
			this.cboMemoryType.Name = "cboMemoryType";
			this.cboMemoryType.Size = new System.Drawing.Size(114, 21);
			this.cboMemoryType.TabIndex = 11;
			// 
			// nudBank
			// 
			this.nudBank.DecimalPlaces = 0;
			this.nudBank.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudBank.Location = new System.Drawing.Point(77, 111);
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
			// 
			// lblBank
			// 
			this.lblBank.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblBank.AutoSize = true;
			this.lblBank.Location = new System.Drawing.Point(3, 115);
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
			this.lblColumns.Location = new System.Drawing.Point(3, 88);
			this.lblColumns.Name = "lblColumns";
			this.lblColumns.Size = new System.Drawing.Size(50, 13);
			this.lblColumns.TabIndex = 3;
			this.lblColumns.Text = "Columns:";
			// 
			// chkShowTileGrid
			// 
			this.chkShowTileGrid.AutoSize = true;
			this.tableLayoutPanel2.SetColumnSpan(this.chkShowTileGrid, 2);
			this.chkShowTileGrid.Location = new System.Drawing.Point(3, 165);
			this.chkShowTileGrid.Name = "chkShowTileGrid";
			this.chkShowTileGrid.Size = new System.Drawing.Size(89, 17);
			this.chkShowTileGrid.TabIndex = 0;
			this.chkShowTileGrid.Text = "Show tile grid";
			this.chkShowTileGrid.UseVisualStyleBackColor = true;
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
			this.cboFormat.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboFormat.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboFormat.FormattingEnabled = true;
			this.cboFormat.Items.AddRange(new object[] {
            "2 BPP",
            "4 BPP",
            "8 BPP",
            "8 BPP - Direct Color Mode",
            "Mode 7",
            "Mode 7 - Direct Color Mode"});
			this.cboFormat.Location = new System.Drawing.Point(77, 30);
			this.cboFormat.Name = "cboFormat";
			this.cboFormat.Size = new System.Drawing.Size(114, 21);
			this.cboFormat.TabIndex = 2;
			// 
			// nudColumns
			// 
			this.nudColumns.DecimalPlaces = 0;
			this.nudColumns.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudColumns.Location = new System.Drawing.Point(77, 84);
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
			// 
			// lblOffset
			// 
			this.lblOffset.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblOffset.AutoSize = true;
			this.lblOffset.Location = new System.Drawing.Point(3, 142);
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
			this.nudOffset.Location = new System.Drawing.Point(77, 138);
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
			// 
			// ctrlPaletteViewer
			// 
			this.tableLayoutPanel2.SetColumnSpan(this.ctrlPaletteViewer, 2);
			this.ctrlPaletteViewer.Location = new System.Drawing.Point(3, 244);
			this.ctrlPaletteViewer.Name = "ctrlPaletteViewer";
			this.ctrlPaletteViewer.PaletteScale = 11;
			this.ctrlPaletteViewer.SelectedPalette = 0;
			this.ctrlPaletteViewer.SelectionMode = Mesen.GUI.Debugger.PaletteSelectionMode.None;
			this.ctrlPaletteViewer.Size = new System.Drawing.Size(176, 176);
			this.ctrlPaletteViewer.TabIndex = 12;
			this.ctrlPaletteViewer.TabStop = false;
			// 
			// lblPresets
			// 
			this.lblPresets.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPresets.AutoSize = true;
			this.lblPresets.Location = new System.Drawing.Point(3, 192);
			this.lblPresets.Name = "lblPresets";
			this.lblPresets.Size = new System.Drawing.Size(45, 13);
			this.lblPresets.TabIndex = 13;
			this.lblPresets.Text = "Presets:";
			// 
			// tableLayoutPanel3
			// 
			this.tableLayoutPanel3.ColumnCount = 4;
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.Controls.Add(this.btnPresetBg1, 0, 0);
			this.tableLayoutPanel3.Controls.Add(this.btnPresetBg2, 1, 0);
			this.tableLayoutPanel3.Controls.Add(this.btnPresetBg3, 2, 0);
			this.tableLayoutPanel3.Controls.Add(this.btnPresetBg4, 3, 0);
			this.tableLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel3.Location = new System.Drawing.Point(74, 185);
			this.tableLayoutPanel3.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel3.Name = "tableLayoutPanel3";
			this.tableLayoutPanel3.RowCount = 1;
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.Size = new System.Drawing.Size(120, 28);
			this.tableLayoutPanel3.TabIndex = 14;
			// 
			// btnPresetBg1
			// 
			this.btnPresetBg1.Location = new System.Drawing.Point(3, 3);
			this.btnPresetBg1.Name = "btnPresetBg1";
			this.btnPresetBg1.Size = new System.Drawing.Size(23, 23);
			this.btnPresetBg1.TabIndex = 0;
			this.btnPresetBg1.Text = "1";
			this.btnPresetBg1.UseVisualStyleBackColor = true;
			// 
			// btnPresetBg2
			// 
			this.btnPresetBg2.Location = new System.Drawing.Point(32, 3);
			this.btnPresetBg2.Name = "btnPresetBg2";
			this.btnPresetBg2.Size = new System.Drawing.Size(23, 23);
			this.btnPresetBg2.TabIndex = 1;
			this.btnPresetBg2.Text = "2";
			this.btnPresetBg2.UseVisualStyleBackColor = true;
			// 
			// btnPresetBg3
			// 
			this.btnPresetBg3.Location = new System.Drawing.Point(61, 3);
			this.btnPresetBg3.Name = "btnPresetBg3";
			this.btnPresetBg3.Size = new System.Drawing.Size(23, 23);
			this.btnPresetBg3.TabIndex = 2;
			this.btnPresetBg3.Text = "3";
			this.btnPresetBg3.UseVisualStyleBackColor = true;
			// 
			// btnPresetBg4
			// 
			this.btnPresetBg4.Location = new System.Drawing.Point(90, 3);
			this.btnPresetBg4.Name = "btnPresetBg4";
			this.btnPresetBg4.Size = new System.Drawing.Size(23, 23);
			this.btnPresetBg4.TabIndex = 3;
			this.btnPresetBg4.Text = "4";
			this.btnPresetBg4.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel4
			// 
			this.tableLayoutPanel4.ColumnCount = 2;
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel4.Controls.Add(this.btnPresetOam1, 0, 0);
			this.tableLayoutPanel4.Controls.Add(this.btnPresetOam2, 1, 0);
			this.tableLayoutPanel4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel4.Location = new System.Drawing.Point(74, 213);
			this.tableLayoutPanel4.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel4.Name = "tableLayoutPanel4";
			this.tableLayoutPanel4.RowCount = 1;
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Size = new System.Drawing.Size(120, 28);
			this.tableLayoutPanel4.TabIndex = 15;
			// 
			// btnPresetOam1
			// 
			this.btnPresetOam1.Location = new System.Drawing.Point(3, 3);
			this.btnPresetOam1.Name = "btnPresetOam1";
			this.btnPresetOam1.Size = new System.Drawing.Size(45, 22);
			this.btnPresetOam1.TabIndex = 4;
			this.btnPresetOam1.Text = "OAM1";
			this.btnPresetOam1.UseVisualStyleBackColor = true;
			// 
			// btnPresetOam2
			// 
			this.btnPresetOam2.Location = new System.Drawing.Point(54, 3);
			this.btnPresetOam2.Name = "btnPresetOam2";
			this.btnPresetOam2.Size = new System.Drawing.Size(45, 22);
			this.btnPresetOam2.TabIndex = 5;
			this.btnPresetOam2.Text = "OAM2";
			this.btnPresetOam2.UseVisualStyleBackColor = true;
			// 
			// lblTileAddress
			// 
			this.lblTileAddress.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblTileAddress.AutoSize = true;
			this.lblTileAddress.Location = new System.Drawing.Point(3, 429);
			this.lblTileAddress.Name = "lblTileAddress";
			this.lblTileAddress.Size = new System.Drawing.Size(68, 13);
			this.lblTileAddress.TabIndex = 16;
			this.lblTileAddress.Text = "Tile Address:";
			// 
			// txtTileAddress
			// 
			this.txtTileAddress.Location = new System.Drawing.Point(77, 426);
			this.txtTileAddress.Name = "txtTileAddress";
			this.txtTileAddress.ReadOnly = true;
			this.txtTileAddress.Size = new System.Drawing.Size(60, 20);
			this.txtTileAddress.TabIndex = 17;
			// 
			// lblTileLayout
			// 
			this.lblTileLayout.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblTileLayout.AutoSize = true;
			this.lblTileLayout.Location = new System.Drawing.Point(3, 61);
			this.lblTileLayout.Name = "lblTileLayout";
			this.lblTileLayout.Size = new System.Drawing.Size(62, 13);
			this.lblTileLayout.TabIndex = 18;
			this.lblTileLayout.Text = "Tile Layout:";
			// 
			// cboLayout
			// 
			this.cboLayout.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboLayout.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboLayout.FormattingEnabled = true;
			this.cboLayout.Items.AddRange(new object[] {
            "2 BPP",
            "4 BPP",
            "8 BPP",
            "8 BPP - Direct Color Mode",
            "Mode 7",
            "Mode 7 - Direct Color Mode"});
			this.cboLayout.Location = new System.Drawing.Point(77, 57);
			this.cboLayout.Name = "cboLayout";
			this.cboLayout.Size = new System.Drawing.Size(114, 21);
			this.cboLayout.TabIndex = 19;
			// 
			// ctrlImagePanel
			// 
			this.ctrlImagePanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlImagePanel.GridSizeX = 0;
			this.ctrlImagePanel.GridSizeY = 0;
			this.ctrlImagePanel.Image = null;
			this.ctrlImagePanel.ImageScale = 1;
			this.ctrlImagePanel.ImageSize = new System.Drawing.Size(0, 0);
			this.ctrlImagePanel.Location = new System.Drawing.Point(3, 3);
			this.ctrlImagePanel.Name = "ctrlImagePanel";
			this.ctrlImagePanel.Overlay = new System.Drawing.Rectangle(0, 0, 0, 0);
			this.ctrlImagePanel.Selection = new System.Drawing.Rectangle(0, 0, 0, 0);
			this.ctrlImagePanel.SelectionWrapPosition = 0;
			this.ctrlImagePanel.Size = new System.Drawing.Size(531, 516);
			this.ctrlImagePanel.TabIndex = 8;
			this.ctrlImagePanel.MouseClick += new System.Windows.Forms.MouseEventHandler(this.ctrlImagePanel_MouseClick);
			// 
			// ctrlMesenMenuStrip1
			// 
			this.ctrlMesenMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFile,
            this.mnuView});
			this.ctrlMesenMenuStrip1.Location = new System.Drawing.Point(0, 0);
			this.ctrlMesenMenuStrip1.Name = "ctrlMesenMenuStrip1";
			this.ctrlMesenMenuStrip1.Size = new System.Drawing.Size(737, 24);
			this.ctrlMesenMenuStrip1.TabIndex = 9;
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
			// frmTileViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(737, 574);
			this.Controls.Add(this.tableLayoutPanel1);
			this.Controls.Add(this.ctrlScanlineCycleSelect);
			this.Controls.Add(this.ctrlMesenMenuStrip1);
			this.Name = "frmTileViewer";
			this.Text = "Tile Viewer";
			this.Controls.SetChildIndex(this.ctrlMesenMenuStrip1, 0);
			this.Controls.SetChildIndex(this.ctrlScanlineCycleSelect, 0);
			this.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.ctrlPaletteViewer)).EndInit();
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel4.ResumeLayout(false);
			this.ctrlMesenMenuStrip1.ResumeLayout(false);
			this.ctrlMesenMenuStrip1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private Controls.ctrlScanlineCycleSelect ctrlScanlineCycleSelect;
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
		private PpuViewer.ctrlImagePanel ctrlImagePanel;
		private GUI.Controls.ctrlMesenMenuStrip ctrlMesenMenuStrip1;
		private System.Windows.Forms.ToolStripMenuItem mnuFile;
		private System.Windows.Forms.ToolStripMenuItem mnuClose;
		private System.Windows.Forms.ToolStripMenuItem mnuView;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoRefresh;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuRefresh;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuZoomIn;
		private System.Windows.Forms.ToolStripMenuItem mnuZoomOut;
		private System.Windows.Forms.Label lblPresets;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
		private System.Windows.Forms.Button btnPresetBg1;
		private System.Windows.Forms.Button btnPresetBg2;
		private System.Windows.Forms.Button btnPresetBg3;
		private System.Windows.Forms.Button btnPresetBg4;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel4;
		private System.Windows.Forms.Button btnPresetOam1;
		private System.Windows.Forms.Button btnPresetOam2;
		private System.Windows.Forms.Label lblTileAddress;
		private System.Windows.Forms.TextBox txtTileAddress;
		private System.Windows.Forms.Label lblTileLayout;
		private System.Windows.Forms.ComboBox cboLayout;
		private System.Windows.Forms.ToolStripMenuItem mnuCopyToClipboard;
		private System.Windows.Forms.ToolStripMenuItem mnuSaveAsPng;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
	}
}