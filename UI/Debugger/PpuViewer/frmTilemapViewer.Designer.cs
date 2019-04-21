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
			this.btnLayer1 = new System.Windows.Forms.Button();
			this.btnLayer2 = new System.Windows.Forms.Button();
			this.btnLayer3 = new System.Windows.Forms.Button();
			this.btnLayer4 = new System.Windows.Forms.Button();
			this.ctrlScanlineCycleSelect = new Mesen.GUI.Debugger.Controls.ctrlScanlineCycleSelect();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.chkShowScrollOverlay = new System.Windows.Forms.CheckBox();
			this.chkShowTileGrid = new System.Windows.Forms.CheckBox();
			this.grpTileInfo = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel4 = new System.Windows.Forms.TableLayoutPanel();
			this.txtPalette = new System.Windows.Forms.TextBox();
			this.txtTileNumber = new System.Windows.Forms.TextBox();
			this.txtAddress = new System.Windows.Forms.TextBox();
			this.txtPosition = new System.Windows.Forms.TextBox();
			this.lblMap = new System.Windows.Forms.Label();
			this.lblPosition = new System.Windows.Forms.Label();
			this.lblAddress = new System.Windows.Forms.Label();
			this.lblValue = new System.Windows.Forms.Label();
			this.lblTileNumber = new System.Windows.Forms.Label();
			this.lblPalette = new System.Windows.Forms.Label();
			this.chkPriorityFlag = new System.Windows.Forms.CheckBox();
			this.chkHorizontalMirror = new System.Windows.Forms.CheckBox();
			this.chkVerticalMirror = new System.Windows.Forms.CheckBox();
			this.txtMapNumber = new System.Windows.Forms.TextBox();
			this.txtValue = new System.Windows.Forms.TextBox();
			this.grpLayerInfo = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel5 = new System.Windows.Forms.TableLayoutPanel();
			this.lblMapAddress = new System.Windows.Forms.Label();
			this.lblMapSize = new System.Windows.Forms.Label();
			this.lblBitDepth = new System.Windows.Forms.Label();
			this.lblTileSize = new System.Windows.Forms.Label();
			this.lblTilesetAddress = new System.Windows.Forms.Label();
			this.txtMapSize = new System.Windows.Forms.TextBox();
			this.txtMapAddress = new System.Windows.Forms.TextBox();
			this.txtTileSize = new System.Windows.Forms.TextBox();
			this.txtTilesetAddress = new System.Windows.Forms.TextBox();
			this.txtBitDepth = new System.Windows.Forms.TextBox();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.lblLayer = new System.Windows.Forms.Label();
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
			this.tableLayoutPanel1.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.grpTileInfo.SuspendLayout();
			this.tableLayoutPanel4.SuspendLayout();
			this.grpLayerInfo.SuspendLayout();
			this.tableLayoutPanel5.SuspendLayout();
			this.tableLayoutPanel3.SuspendLayout();
			this.ctrlMesenMenuStrip1.SuspendLayout();
			this.SuspendLayout();
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
			this.ctrlScanlineCycleSelect.Location = new System.Drawing.Point(0, 532);
			this.ctrlScanlineCycleSelect.Name = "ctrlScanlineCycleSelect";
			this.ctrlScanlineCycleSelect.Size = new System.Drawing.Size(668, 28);
			this.ctrlScanlineCycleSelect.TabIndex = 5;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel3, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.ctrlImagePanel, 0, 1);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 24);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(668, 508);
			this.tableLayoutPanel1.TabIndex = 7;
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 1;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Controls.Add(this.chkShowScrollOverlay, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkShowTileGrid, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.grpTileInfo, 0, 3);
			this.tableLayoutPanel2.Controls.Add(this.grpLayerInfo, 0, 2);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Right;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(522, 3);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 5;
			this.tableLayoutPanel1.SetRowSpan(this.tableLayoutPanel2, 2);
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(143, 502);
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
			// grpTileInfo
			// 
			this.grpTileInfo.Controls.Add(this.tableLayoutPanel4);
			this.grpTileInfo.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpTileInfo.Location = new System.Drawing.Point(3, 203);
			this.grpTileInfo.Name = "grpTileInfo";
			this.grpTileInfo.Size = new System.Drawing.Size(137, 268);
			this.grpTileInfo.TabIndex = 2;
			this.grpTileInfo.TabStop = false;
			this.grpTileInfo.Text = "Tile Information";
			// 
			// tableLayoutPanel4
			// 
			this.tableLayoutPanel4.ColumnCount = 2;
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel4.Controls.Add(this.txtPalette, 1, 7);
			this.tableLayoutPanel4.Controls.Add(this.txtTileNumber, 1, 6);
			this.tableLayoutPanel4.Controls.Add(this.txtAddress, 1, 3);
			this.tableLayoutPanel4.Controls.Add(this.txtPosition, 1, 1);
			this.tableLayoutPanel4.Controls.Add(this.lblMap, 0, 0);
			this.tableLayoutPanel4.Controls.Add(this.lblPosition, 0, 1);
			this.tableLayoutPanel4.Controls.Add(this.lblAddress, 0, 3);
			this.tableLayoutPanel4.Controls.Add(this.lblValue, 0, 4);
			this.tableLayoutPanel4.Controls.Add(this.lblTileNumber, 0, 6);
			this.tableLayoutPanel4.Controls.Add(this.lblPalette, 0, 7);
			this.tableLayoutPanel4.Controls.Add(this.chkPriorityFlag, 0, 8);
			this.tableLayoutPanel4.Controls.Add(this.chkHorizontalMirror, 0, 9);
			this.tableLayoutPanel4.Controls.Add(this.chkVerticalMirror, 0, 10);
			this.tableLayoutPanel4.Controls.Add(this.txtMapNumber, 1, 0);
			this.tableLayoutPanel4.Controls.Add(this.txtValue, 1, 4);
			this.tableLayoutPanel4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel4.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel4.Name = "tableLayoutPanel4";
			this.tableLayoutPanel4.RowCount = 12;
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 10F));
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 10F));
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Size = new System.Drawing.Size(131, 249);
			this.tableLayoutPanel4.TabIndex = 0;
			// 
			// txtPalette
			// 
			this.txtPalette.Location = new System.Drawing.Point(76, 153);
			this.txtPalette.Name = "txtPalette";
			this.txtPalette.ReadOnly = true;
			this.txtPalette.Size = new System.Drawing.Size(30, 20);
			this.txtPalette.TabIndex = 14;
			// 
			// txtTileNumber
			// 
			this.txtTileNumber.Location = new System.Drawing.Point(76, 127);
			this.txtTileNumber.Name = "txtTileNumber";
			this.txtTileNumber.ReadOnly = true;
			this.txtTileNumber.Size = new System.Drawing.Size(42, 20);
			this.txtTileNumber.TabIndex = 13;
			// 
			// txtAddress
			// 
			this.txtAddress.Location = new System.Drawing.Point(76, 65);
			this.txtAddress.Name = "txtAddress";
			this.txtAddress.ReadOnly = true;
			this.txtAddress.Size = new System.Drawing.Size(43, 20);
			this.txtAddress.TabIndex = 11;
			// 
			// txtPosition
			// 
			this.txtPosition.Location = new System.Drawing.Point(76, 29);
			this.txtPosition.Name = "txtPosition";
			this.txtPosition.ReadOnly = true;
			this.txtPosition.Size = new System.Drawing.Size(52, 20);
			this.txtPosition.TabIndex = 10;
			// 
			// lblMap
			// 
			this.lblMap.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblMap.AutoSize = true;
			this.lblMap.Location = new System.Drawing.Point(3, 6);
			this.lblMap.Name = "lblMap";
			this.lblMap.Size = new System.Drawing.Size(31, 13);
			this.lblMap.TabIndex = 0;
			this.lblMap.Text = "Map:";
			// 
			// lblPosition
			// 
			this.lblPosition.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPosition.AutoSize = true;
			this.lblPosition.Location = new System.Drawing.Point(3, 32);
			this.lblPosition.Name = "lblPosition";
			this.lblPosition.Size = new System.Drawing.Size(47, 13);
			this.lblPosition.TabIndex = 1;
			this.lblPosition.Text = "Position:";
			// 
			// lblAddress
			// 
			this.lblAddress.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblAddress.AutoSize = true;
			this.lblAddress.Location = new System.Drawing.Point(3, 68);
			this.lblAddress.Name = "lblAddress";
			this.lblAddress.Size = new System.Drawing.Size(48, 13);
			this.lblAddress.TabIndex = 2;
			this.lblAddress.Text = "Address:";
			// 
			// lblValue
			// 
			this.lblValue.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblValue.AutoSize = true;
			this.lblValue.Location = new System.Drawing.Point(3, 94);
			this.lblValue.Name = "lblValue";
			this.lblValue.Size = new System.Drawing.Size(37, 13);
			this.lblValue.TabIndex = 3;
			this.lblValue.Text = "Value:";
			// 
			// lblTileNumber
			// 
			this.lblTileNumber.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblTileNumber.AutoSize = true;
			this.lblTileNumber.Location = new System.Drawing.Point(3, 130);
			this.lblTileNumber.Name = "lblTileNumber";
			this.lblTileNumber.Size = new System.Drawing.Size(67, 13);
			this.lblTileNumber.TabIndex = 4;
			this.lblTileNumber.Text = "Tile Number:";
			// 
			// lblPalette
			// 
			this.lblPalette.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPalette.AutoSize = true;
			this.lblPalette.Location = new System.Drawing.Point(3, 156);
			this.lblPalette.Name = "lblPalette";
			this.lblPalette.Size = new System.Drawing.Size(43, 13);
			this.lblPalette.TabIndex = 5;
			this.lblPalette.Text = "Palette:";
			// 
			// chkPriorityFlag
			// 
			this.chkPriorityFlag.AutoCheck = false;
			this.chkPriorityFlag.AutoSize = true;
			this.tableLayoutPanel4.SetColumnSpan(this.chkPriorityFlag, 2);
			this.chkPriorityFlag.Location = new System.Drawing.Point(3, 179);
			this.chkPriorityFlag.Name = "chkPriorityFlag";
			this.chkPriorityFlag.Size = new System.Drawing.Size(57, 17);
			this.chkPriorityFlag.TabIndex = 6;
			this.chkPriorityFlag.Text = "Priority";
			this.chkPriorityFlag.UseVisualStyleBackColor = true;
			// 
			// chkHorizontalMirror
			// 
			this.chkHorizontalMirror.AutoCheck = false;
			this.chkHorizontalMirror.AutoSize = true;
			this.tableLayoutPanel4.SetColumnSpan(this.chkHorizontalMirror, 2);
			this.chkHorizontalMirror.Location = new System.Drawing.Point(3, 202);
			this.chkHorizontalMirror.Name = "chkHorizontalMirror";
			this.chkHorizontalMirror.Size = new System.Drawing.Size(102, 17);
			this.chkHorizontalMirror.TabIndex = 8;
			this.chkHorizontalMirror.Text = "Horizontal Mirror";
			this.chkHorizontalMirror.UseVisualStyleBackColor = true;
			// 
			// chkVerticalMirror
			// 
			this.chkVerticalMirror.AutoCheck = false;
			this.chkVerticalMirror.AutoSize = true;
			this.tableLayoutPanel4.SetColumnSpan(this.chkVerticalMirror, 2);
			this.chkVerticalMirror.Location = new System.Drawing.Point(3, 225);
			this.chkVerticalMirror.Name = "chkVerticalMirror";
			this.chkVerticalMirror.Size = new System.Drawing.Size(90, 17);
			this.chkVerticalMirror.TabIndex = 7;
			this.chkVerticalMirror.Text = "Vertical Mirror";
			this.chkVerticalMirror.UseVisualStyleBackColor = true;
			// 
			// txtMapNumber
			// 
			this.txtMapNumber.Location = new System.Drawing.Point(76, 3);
			this.txtMapNumber.Name = "txtMapNumber";
			this.txtMapNumber.ReadOnly = true;
			this.txtMapNumber.Size = new System.Drawing.Size(30, 20);
			this.txtMapNumber.TabIndex = 9;
			// 
			// txtValue
			// 
			this.txtValue.Location = new System.Drawing.Point(76, 91);
			this.txtValue.Name = "txtValue";
			this.txtValue.ReadOnly = true;
			this.txtValue.Size = new System.Drawing.Size(43, 20);
			this.txtValue.TabIndex = 12;
			// 
			// grpLayerInfo
			// 
			this.grpLayerInfo.Controls.Add(this.tableLayoutPanel5);
			this.grpLayerInfo.Location = new System.Drawing.Point(3, 49);
			this.grpLayerInfo.Name = "grpLayerInfo";
			this.grpLayerInfo.Size = new System.Drawing.Size(137, 148);
			this.grpLayerInfo.TabIndex = 3;
			this.grpLayerInfo.TabStop = false;
			this.grpLayerInfo.Text = "Layer Information";
			// 
			// tableLayoutPanel5
			// 
			this.tableLayoutPanel5.ColumnCount = 2;
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel5.Controls.Add(this.lblMapAddress, 0, 1);
			this.tableLayoutPanel5.Controls.Add(this.lblMapSize, 0, 0);
			this.tableLayoutPanel5.Controls.Add(this.lblBitDepth, 0, 4);
			this.tableLayoutPanel5.Controls.Add(this.lblTileSize, 0, 2);
			this.tableLayoutPanel5.Controls.Add(this.lblTilesetAddress, 0, 3);
			this.tableLayoutPanel5.Controls.Add(this.txtMapSize, 1, 0);
			this.tableLayoutPanel5.Controls.Add(this.txtMapAddress, 1, 1);
			this.tableLayoutPanel5.Controls.Add(this.txtTileSize, 1, 2);
			this.tableLayoutPanel5.Controls.Add(this.txtTilesetAddress, 1, 3);
			this.tableLayoutPanel5.Controls.Add(this.txtBitDepth, 1, 4);
			this.tableLayoutPanel5.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel5.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel5.Name = "tableLayoutPanel5";
			this.tableLayoutPanel5.RowCount = 6;
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel5.Size = new System.Drawing.Size(131, 129);
			this.tableLayoutPanel5.TabIndex = 0;
			// 
			// lblMapAddress
			// 
			this.lblMapAddress.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblMapAddress.AutoSize = true;
			this.lblMapAddress.Location = new System.Drawing.Point(3, 32);
			this.lblMapAddress.Name = "lblMapAddress";
			this.lblMapAddress.Size = new System.Drawing.Size(48, 13);
			this.lblMapAddress.TabIndex = 2;
			this.lblMapAddress.Text = "Address:";
			// 
			// lblMapSize
			// 
			this.lblMapSize.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblMapSize.AutoSize = true;
			this.lblMapSize.Location = new System.Drawing.Point(3, 6);
			this.lblMapSize.Name = "lblMapSize";
			this.lblMapSize.Size = new System.Drawing.Size(30, 13);
			this.lblMapSize.TabIndex = 1;
			this.lblMapSize.Text = "Size:";
			// 
			// lblBitDepth
			// 
			this.lblBitDepth.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblBitDepth.AutoSize = true;
			this.lblBitDepth.Location = new System.Drawing.Point(3, 110);
			this.lblBitDepth.Name = "lblBitDepth";
			this.lblBitDepth.Size = new System.Drawing.Size(54, 13);
			this.lblBitDepth.TabIndex = 3;
			this.lblBitDepth.Text = "Bit Depth:";
			// 
			// lblTileSize
			// 
			this.lblTileSize.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblTileSize.AutoSize = true;
			this.lblTileSize.Location = new System.Drawing.Point(3, 58);
			this.lblTileSize.Name = "lblTileSize";
			this.lblTileSize.Size = new System.Drawing.Size(50, 13);
			this.lblTileSize.TabIndex = 4;
			this.lblTileSize.Text = "Tile Size:";
			// 
			// lblTilesetAddress
			// 
			this.lblTilesetAddress.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblTilesetAddress.AutoSize = true;
			this.lblTilesetAddress.Location = new System.Drawing.Point(3, 84);
			this.lblTilesetAddress.Name = "lblTilesetAddress";
			this.lblTilesetAddress.Size = new System.Drawing.Size(66, 13);
			this.lblTilesetAddress.TabIndex = 5;
			this.lblTilesetAddress.Text = "Tileset Addr:";
			// 
			// txtMapSize
			// 
			this.txtMapSize.Location = new System.Drawing.Point(75, 3);
			this.txtMapSize.Name = "txtMapSize";
			this.txtMapSize.ReadOnly = true;
			this.txtMapSize.Size = new System.Drawing.Size(52, 20);
			this.txtMapSize.TabIndex = 13;
			// 
			// txtMapAddress
			// 
			this.txtMapAddress.Location = new System.Drawing.Point(75, 29);
			this.txtMapAddress.Name = "txtMapAddress";
			this.txtMapAddress.ReadOnly = true;
			this.txtMapAddress.Size = new System.Drawing.Size(43, 20);
			this.txtMapAddress.TabIndex = 14;
			// 
			// txtTileSize
			// 
			this.txtTileSize.Location = new System.Drawing.Point(75, 55);
			this.txtTileSize.Name = "txtTileSize";
			this.txtTileSize.ReadOnly = true;
			this.txtTileSize.Size = new System.Drawing.Size(43, 20);
			this.txtTileSize.TabIndex = 15;
			// 
			// txtTilesetAddress
			// 
			this.txtTilesetAddress.Location = new System.Drawing.Point(75, 81);
			this.txtTilesetAddress.Name = "txtTilesetAddress";
			this.txtTilesetAddress.ReadOnly = true;
			this.txtTilesetAddress.Size = new System.Drawing.Size(43, 20);
			this.txtTilesetAddress.TabIndex = 16;
			// 
			// txtBitDepth
			// 
			this.txtBitDepth.Location = new System.Drawing.Point(75, 107);
			this.txtBitDepth.Name = "txtBitDepth";
			this.txtBitDepth.ReadOnly = true;
			this.txtBitDepth.Size = new System.Drawing.Size(30, 20);
			this.txtBitDepth.TabIndex = 17;
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
			// ctrlImagePanel
			// 
			this.ctrlImagePanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlImagePanel.Image = null;
			this.ctrlImagePanel.ImageScale = 1;
			this.ctrlImagePanel.ImageSize = new System.Drawing.Size(0, 0);
			this.ctrlImagePanel.Location = new System.Drawing.Point(3, 31);
			this.ctrlImagePanel.Name = "ctrlImagePanel";
			this.ctrlImagePanel.Selection = new System.Drawing.Rectangle(0, 0, 0, 0);
			this.ctrlImagePanel.Size = new System.Drawing.Size(513, 474);
			this.ctrlImagePanel.TabIndex = 9;
			this.ctrlImagePanel.MouseClick += new System.Windows.Forms.MouseEventHandler(this.ctrlImagePanel_MouseClick);
			// 
			// ctrlMesenMenuStrip1
			// 
			this.ctrlMesenMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFile,
            this.mnuView});
			this.ctrlMesenMenuStrip1.Location = new System.Drawing.Point(0, 0);
			this.ctrlMesenMenuStrip1.Name = "ctrlMesenMenuStrip1";
			this.ctrlMesenMenuStrip1.Size = new System.Drawing.Size(668, 24);
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
			this.mnuClose.Size = new System.Drawing.Size(103, 22);
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
			this.mnuAutoRefresh.CheckedChanged += new System.EventHandler(this.mnuAutoRefresh_CheckedChanged);
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
			this.ClientSize = new System.Drawing.Size(668, 560);
			this.Controls.Add(this.tableLayoutPanel1);
			this.Controls.Add(this.ctrlScanlineCycleSelect);
			this.Controls.Add(this.ctrlMesenMenuStrip1);
			this.MainMenuStrip = this.ctrlMesenMenuStrip1;
			this.Name = "frmTilemapViewer";
			this.Text = "Tilemap Viewer";
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.grpTileInfo.ResumeLayout(false);
			this.tableLayoutPanel4.ResumeLayout(false);
			this.tableLayoutPanel4.PerformLayout();
			this.grpLayerInfo.ResumeLayout(false);
			this.tableLayoutPanel5.ResumeLayout(false);
			this.tableLayoutPanel5.PerformLayout();
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel3.PerformLayout();
			this.ctrlMesenMenuStrip1.ResumeLayout(false);
			this.ctrlMesenMenuStrip1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private System.Windows.Forms.Button btnLayer1;
		private System.Windows.Forms.Button btnLayer2;
		private System.Windows.Forms.Button btnLayer3;
		private System.Windows.Forms.Button btnLayer4;
		private Controls.ctrlScanlineCycleSelect ctrlScanlineCycleSelect;
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
		private System.Windows.Forms.GroupBox grpTileInfo;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel4;
		private System.Windows.Forms.TextBox txtPalette;
		private System.Windows.Forms.TextBox txtTileNumber;
		private System.Windows.Forms.TextBox txtAddress;
		private System.Windows.Forms.TextBox txtPosition;
		private System.Windows.Forms.Label lblMap;
		private System.Windows.Forms.Label lblPosition;
		private System.Windows.Forms.Label lblAddress;
		private System.Windows.Forms.Label lblValue;
		private System.Windows.Forms.Label lblTileNumber;
		private System.Windows.Forms.Label lblPalette;
		private System.Windows.Forms.CheckBox chkPriorityFlag;
		private System.Windows.Forms.CheckBox chkHorizontalMirror;
		private System.Windows.Forms.CheckBox chkVerticalMirror;
		private System.Windows.Forms.TextBox txtMapNumber;
		private System.Windows.Forms.TextBox txtValue;
		private System.Windows.Forms.GroupBox grpLayerInfo;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel5;
		private System.Windows.Forms.Label lblMapAddress;
		private System.Windows.Forms.Label lblMapSize;
		private System.Windows.Forms.Label lblBitDepth;
		private System.Windows.Forms.Label lblTileSize;
		private System.Windows.Forms.Label lblTilesetAddress;
		private System.Windows.Forms.TextBox txtMapSize;
		private System.Windows.Forms.TextBox txtMapAddress;
		private System.Windows.Forms.TextBox txtTileSize;
		private System.Windows.Forms.TextBox txtTilesetAddress;
		private System.Windows.Forms.TextBox txtBitDepth;
		private PpuViewer.ctrlImagePanel ctrlImagePanel;
	}
}