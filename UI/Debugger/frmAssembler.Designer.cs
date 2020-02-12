namespace Mesen.GUI.Debugger
{
	partial class frmAssembler
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
			this.components = new System.ComponentModel.Container();
			this.btnOk = new System.Windows.Forms.Button();
			this.btnCancel = new System.Windows.Forms.Button();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.lstErrors = new System.Windows.Forms.ListBox();
			this.grpSettings = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
			this.label1 = new System.Windows.Forms.Label();
			this.txtStartAddress = new System.Windows.Forms.TextBox();
			this.picStartAddressWarning = new System.Windows.Forms.PictureBox();
			this.flowLayoutPanel2 = new System.Windows.Forms.FlowLayoutPanel();
			this.label2 = new System.Windows.Forms.Label();
			this.lblByteUsage = new System.Windows.Forms.Label();
			this.picSizeWarning = new System.Windows.Forms.PictureBox();
			this.lblNoChanges = new System.Windows.Forms.Label();
			this.flowLayoutPanel3 = new System.Windows.Forms.FlowLayoutPanel();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.panel1 = new System.Windows.Forms.Panel();
			this.txtCode = new FastColoredTextBoxNS.FastColoredTextBox();
			this.contextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuCopy = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuCut = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPaste = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSelectAll = new System.Windows.Forms.ToolStripMenuItem();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.ctrlHexBox = new Be.Windows.Forms.HexBox();
			this.menuStrip1 = new Mesen.GUI.Controls.ctrlMesenMenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuClose = new System.Windows.Forms.ToolStripMenuItem();
			this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.fontSizeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuIncreaseFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuDecreaseFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuResetFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSelectFont = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuConfigureColors = new System.Windows.Forms.ToolStripMenuItem();
			this.tableLayoutPanel1.SuspendLayout();
			this.grpSettings.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.flowLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picStartAddressWarning)).BeginInit();
			this.flowLayoutPanel2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picSizeWarning)).BeginInit();
			this.flowLayoutPanel3.SuspendLayout();
			this.groupBox1.SuspendLayout();
			this.panel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.txtCode)).BeginInit();
			this.contextMenu.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.menuStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// btnOk
			// 
			this.btnOk.Location = new System.Drawing.Point(120, 3);
			this.btnOk.Name = "btnOk";
			this.btnOk.Size = new System.Drawing.Size(70, 23);
			this.btnOk.TabIndex = 0;
			this.btnOk.Text = "Apply";
			this.btnOk.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.btnOk.UseVisualStyleBackColor = true;
			this.btnOk.Click += new System.EventHandler(this.btnOk_Click);
			// 
			// btnCancel
			// 
			this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnCancel.Location = new System.Drawing.Point(196, 3);
			this.btnCancel.Name = "btnCancel";
			this.btnCancel.Size = new System.Drawing.Size(70, 23);
			this.btnCancel.TabIndex = 1;
			this.btnCancel.Text = "Cancel";
			this.btnCancel.UseVisualStyleBackColor = true;
			this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.lstErrors, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.grpSettings, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.groupBox1, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.groupBox2, 1, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 24);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 141F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(683, 432);
			this.tableLayoutPanel1.TabIndex = 2;
			// 
			// lstErrors
			// 
			this.lstErrors.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstErrors.FormattingEnabled = true;
			this.lstErrors.Location = new System.Drawing.Point(3, 294);
			this.lstErrors.Name = "lstErrors";
			this.lstErrors.Size = new System.Drawing.Size(396, 135);
			this.lstErrors.TabIndex = 2;
			this.lstErrors.DoubleClick += new System.EventHandler(this.lstErrors_DoubleClick);
			// 
			// grpSettings
			// 
			this.grpSettings.Controls.Add(this.tableLayoutPanel2);
			this.grpSettings.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpSettings.Location = new System.Drawing.Point(405, 294);
			this.grpSettings.Name = "grpSettings";
			this.grpSettings.Size = new System.Drawing.Size(275, 135);
			this.grpSettings.TabIndex = 3;
			this.grpSettings.TabStop = false;
			this.grpSettings.Text = "Settings";
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 1;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel2.Controls.Add(this.flowLayoutPanel1, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.flowLayoutPanel2, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.lblNoChanges, 0, 2);
			this.tableLayoutPanel2.Controls.Add(this.flowLayoutPanel3, 0, 3);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 4;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.Size = new System.Drawing.Size(269, 116);
			this.tableLayoutPanel2.TabIndex = 0;
			// 
			// flowLayoutPanel1
			// 
			this.flowLayoutPanel1.Controls.Add(this.label1);
			this.flowLayoutPanel1.Controls.Add(this.txtStartAddress);
			this.flowLayoutPanel1.Controls.Add(this.picStartAddressWarning);
			this.flowLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.flowLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel1.Name = "flowLayoutPanel1";
			this.flowLayoutPanel1.Size = new System.Drawing.Size(269, 25);
			this.flowLayoutPanel1.TabIndex = 3;
			// 
			// label1
			// 
			this.label1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(3, 6);
			this.label1.Margin = new System.Windows.Forms.Padding(3, 0, 0, 0);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(81, 13);
			this.label1.TabIndex = 0;
			this.label1.Text = "Start address: $";
			// 
			// txtStartAddress
			// 
			this.txtStartAddress.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.txtStartAddress.Location = new System.Drawing.Point(84, 3);
			this.txtStartAddress.Margin = new System.Windows.Forms.Padding(0, 3, 3, 3);
			this.txtStartAddress.MaxLength = 6;
			this.txtStartAddress.Name = "txtStartAddress";
			this.txtStartAddress.Size = new System.Drawing.Size(59, 20);
			this.txtStartAddress.TabIndex = 1;
			this.txtStartAddress.TextChanged += new System.EventHandler(this.txtStartAddress_TextChanged);
			// 
			// picStartAddressWarning
			// 
			this.picStartAddressWarning.Image = global::Mesen.GUI.Properties.Resources.Warning;
			this.picStartAddressWarning.Location = new System.Drawing.Point(149, 5);
			this.picStartAddressWarning.Margin = new System.Windows.Forms.Padding(3, 5, 3, 3);
			this.picStartAddressWarning.Name = "picStartAddressWarning";
			this.picStartAddressWarning.Size = new System.Drawing.Size(18, 18);
			this.picStartAddressWarning.TabIndex = 11;
			this.picStartAddressWarning.TabStop = false;
			this.picStartAddressWarning.Visible = false;
			// 
			// flowLayoutPanel2
			// 
			this.flowLayoutPanel2.Controls.Add(this.label2);
			this.flowLayoutPanel2.Controls.Add(this.lblByteUsage);
			this.flowLayoutPanel2.Controls.Add(this.picSizeWarning);
			this.flowLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowLayoutPanel2.Location = new System.Drawing.Point(0, 25);
			this.flowLayoutPanel2.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel2.Name = "flowLayoutPanel2";
			this.flowLayoutPanel2.Size = new System.Drawing.Size(269, 25);
			this.flowLayoutPanel2.TabIndex = 4;
			// 
			// label2
			// 
			this.label2.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(3, 6);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(63, 13);
			this.label2.TabIndex = 2;
			this.label2.Text = "Byte usage:";
			// 
			// lblByteUsage
			// 
			this.lblByteUsage.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblByteUsage.AutoSize = true;
			this.lblByteUsage.Location = new System.Drawing.Point(72, 6);
			this.lblByteUsage.Name = "lblByteUsage";
			this.lblByteUsage.Size = new System.Drawing.Size(30, 13);
			this.lblByteUsage.TabIndex = 3;
			this.lblByteUsage.Text = "0 / 0";
			// 
			// picSizeWarning
			// 
			this.picSizeWarning.Image = global::Mesen.GUI.Properties.Resources.Warning;
			this.picSizeWarning.Location = new System.Drawing.Point(108, 5);
			this.picSizeWarning.Margin = new System.Windows.Forms.Padding(3, 5, 3, 3);
			this.picSizeWarning.Name = "picSizeWarning";
			this.picSizeWarning.Size = new System.Drawing.Size(18, 18);
			this.picSizeWarning.TabIndex = 10;
			this.picSizeWarning.TabStop = false;
			this.picSizeWarning.Visible = false;
			// 
			// lblNoChanges
			// 
			this.lblNoChanges.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.lblNoChanges.AutoSize = true;
			this.lblNoChanges.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
			this.lblNoChanges.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblNoChanges.Location = new System.Drawing.Point(104, 70);
			this.lblNoChanges.Margin = new System.Windows.Forms.Padding(3, 0, 3, 3);
			this.lblNoChanges.Name = "lblNoChanges";
			this.lblNoChanges.Size = new System.Drawing.Size(162, 13);
			this.lblNoChanges.TabIndex = 2;
			this.lblNoChanges.Text = "New code matches original code";
			// 
			// flowLayoutPanel3
			// 
			this.flowLayoutPanel3.Controls.Add(this.btnCancel);
			this.flowLayoutPanel3.Controls.Add(this.btnOk);
			this.flowLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.flowLayoutPanel3.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
			this.flowLayoutPanel3.Location = new System.Drawing.Point(0, 86);
			this.flowLayoutPanel3.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel3.Name = "flowLayoutPanel3";
			this.flowLayoutPanel3.Size = new System.Drawing.Size(269, 30);
			this.flowLayoutPanel3.TabIndex = 5;
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.panel1);
			this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.groupBox1.Location = new System.Drawing.Point(3, 3);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(396, 285);
			this.groupBox1.TabIndex = 4;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Code Editor";
			// 
			// panel1
			// 
			this.panel1.BackColor = System.Drawing.SystemColors.ControlDark;
			this.panel1.Controls.Add(this.txtCode);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(3, 16);
			this.panel1.Name = "panel1";
			this.panel1.Padding = new System.Windows.Forms.Padding(1);
			this.panel1.Size = new System.Drawing.Size(390, 266);
			this.panel1.TabIndex = 4;
			// 
			// txtCode
			// 
			this.txtCode.AutoCompleteBracketsList = new char[] {
        '(',
        ')',
        '{',
        '}',
        '[',
        ']',
        '\"',
        '\"',
        '\'',
        '\''};
			this.txtCode.AutoIndentChars = false;
			this.txtCode.AutoIndentExistingLines = false;
			this.txtCode.AutoScrollMinSize = new System.Drawing.Size(43, 14);
			this.txtCode.BackBrush = null;
			this.txtCode.CharHeight = 14;
			this.txtCode.CharWidth = 8;
			this.txtCode.ContextMenuStrip = this.contextMenu;
			this.txtCode.CurrentLineColor = System.Drawing.Color.Gainsboro;
			this.txtCode.Cursor = System.Windows.Forms.Cursors.IBeam;
			this.txtCode.DisabledColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))));
			this.txtCode.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtCode.Font = new System.Drawing.Font("Courier New", 9.75F);
			this.txtCode.IsReplaceMode = false;
			this.txtCode.Location = new System.Drawing.Point(1, 1);
			this.txtCode.Name = "txtCode";
			this.txtCode.Paddings = new System.Windows.Forms.Padding(0);
			this.txtCode.ReservedCountOfLineNumberChars = 3;
			this.txtCode.SelectionColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(255)))));
			this.txtCode.Size = new System.Drawing.Size(388, 264);
			this.txtCode.TabIndex = 6;
			this.txtCode.TabLength = 2;
			this.txtCode.Zoom = 100;
			this.txtCode.TextChanged += new System.EventHandler<FastColoredTextBoxNS.TextChangedEventArgs>(this.txtCode_TextChanged);
			// 
			// contextMenu
			// 
			this.contextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuCopy,
            this.mnuCut,
            this.mnuPaste,
            this.toolStripMenuItem5,
            this.mnuSelectAll});
			this.contextMenu.Name = "contextMenu";
			this.contextMenu.Size = new System.Drawing.Size(123, 98);
			// 
			// mnuCopy
			// 
			this.mnuCopy.Name = "mnuCopy";
			this.mnuCopy.Size = new System.Drawing.Size(122, 22);
			this.mnuCopy.Text = "Copy";
			this.mnuCopy.Click += new System.EventHandler(this.mnuCopy_Click);
			// 
			// mnuCut
			// 
			this.mnuCut.Name = "mnuCut";
			this.mnuCut.Size = new System.Drawing.Size(122, 22);
			this.mnuCut.Text = "Cut";
			this.mnuCut.Click += new System.EventHandler(this.mnuCut_Click);
			// 
			// mnuPaste
			// 
			this.mnuPaste.Name = "mnuPaste";
			this.mnuPaste.Size = new System.Drawing.Size(122, 22);
			this.mnuPaste.Text = "Paste";
			this.mnuPaste.Click += new System.EventHandler(this.mnuPaste_Click);
			// 
			// toolStripMenuItem5
			// 
			this.toolStripMenuItem5.Name = "toolStripMenuItem5";
			this.toolStripMenuItem5.Size = new System.Drawing.Size(119, 6);
			// 
			// mnuSelectAll
			// 
			this.mnuSelectAll.Name = "mnuSelectAll";
			this.mnuSelectAll.Size = new System.Drawing.Size(122, 22);
			this.mnuSelectAll.Text = "Select All";
			this.mnuSelectAll.Click += new System.EventHandler(this.mnuSelectAll_Click);
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.ctrlHexBox);
			this.groupBox2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.groupBox2.Location = new System.Drawing.Point(405, 3);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(275, 285);
			this.groupBox2.TabIndex = 5;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Assembled Byte Code";
			// 
			// ctrlHexBox
			// 
			this.ctrlHexBox.ByteColorProvider = null;
			this.ctrlHexBox.ByteEditingMode = false;
			this.ctrlHexBox.BytesPerLine = 8;
			this.ctrlHexBox.ColumnInfoVisible = true;
			this.ctrlHexBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlHexBox.EnablePerByteNavigation = false;
			this.ctrlHexBox.Font = new System.Drawing.Font("Segoe UI", 9F);
			this.ctrlHexBox.HighDensityMode = false;
			this.ctrlHexBox.InfoBackColor = System.Drawing.Color.DarkGray;
			this.ctrlHexBox.LineInfoVisible = true;
			this.ctrlHexBox.Location = new System.Drawing.Point(3, 16);
			this.ctrlHexBox.Name = "ctrlHexBox";
			this.ctrlHexBox.ReadOnly = true;
			this.ctrlHexBox.ShadowSelectionColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(60)))), ((int)(((byte)(188)))), ((int)(((byte)(255)))));
			this.ctrlHexBox.Size = new System.Drawing.Size(269, 266);
			this.ctrlHexBox.TabIndex = 1;
			this.ctrlHexBox.UseFixedBytesPerLine = true;
			this.ctrlHexBox.VScrollBarVisible = true;
			// 
			// menuStrip1
			// 
			this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.viewToolStripMenuItem});
			this.menuStrip1.Location = new System.Drawing.Point(0, 0);
			this.menuStrip1.Name = "menuStrip1";
			this.menuStrip1.Size = new System.Drawing.Size(683, 24);
			this.menuStrip1.TabIndex = 3;
			this.menuStrip1.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuClose});
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
			this.fileToolStripMenuItem.Text = "File";
			// 
			// mnuClose
			// 
			this.mnuClose.Image = global::Mesen.GUI.Properties.Resources.Exit;
			this.mnuClose.Name = "mnuClose";
			this.mnuClose.Size = new System.Drawing.Size(103, 22);
			this.mnuClose.Text = "Close";
			this.mnuClose.Click += new System.EventHandler(this.mnuClose_Click);
			// 
			// viewToolStripMenuItem
			// 
			this.viewToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fontSizeToolStripMenuItem,
            this.mnuConfigureColors});
			this.viewToolStripMenuItem.Name = "viewToolStripMenuItem";
			this.viewToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
			this.viewToolStripMenuItem.Text = "View";
			// 
			// fontSizeToolStripMenuItem
			// 
			this.fontSizeToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuIncreaseFontSize,
            this.mnuDecreaseFontSize,
            this.mnuResetFontSize,
            this.toolStripMenuItem2,
            this.mnuSelectFont});
			this.fontSizeToolStripMenuItem.Image = global::Mesen.GUI.Properties.Resources.Font;
			this.fontSizeToolStripMenuItem.Name = "fontSizeToolStripMenuItem";
			this.fontSizeToolStripMenuItem.Size = new System.Drawing.Size(180, 22);
			this.fontSizeToolStripMenuItem.Text = "Font Options";
			// 
			// mnuIncreaseFontSize
			// 
			this.mnuIncreaseFontSize.Name = "mnuIncreaseFontSize";
			this.mnuIncreaseFontSize.ShortcutKeyDisplayString = "";
			this.mnuIncreaseFontSize.Size = new System.Drawing.Size(157, 22);
			this.mnuIncreaseFontSize.Text = "Increase Size";
			this.mnuIncreaseFontSize.Click += new System.EventHandler(this.mnuIncreaseFontSize_Click);
			// 
			// mnuDecreaseFontSize
			// 
			this.mnuDecreaseFontSize.Name = "mnuDecreaseFontSize";
			this.mnuDecreaseFontSize.ShortcutKeyDisplayString = "";
			this.mnuDecreaseFontSize.Size = new System.Drawing.Size(157, 22);
			this.mnuDecreaseFontSize.Text = "Decrease Size";
			this.mnuDecreaseFontSize.Click += new System.EventHandler(this.mnuDecreaseFontSize_Click);
			// 
			// mnuResetFontSize
			// 
			this.mnuResetFontSize.Name = "mnuResetFontSize";
			this.mnuResetFontSize.ShortcutKeyDisplayString = "";
			this.mnuResetFontSize.Size = new System.Drawing.Size(157, 22);
			this.mnuResetFontSize.Text = "Reset to Default";
			this.mnuResetFontSize.Click += new System.EventHandler(this.mnuResetFontSize_Click);
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(154, 6);
			// 
			// mnuSelectFont
			// 
			this.mnuSelectFont.Name = "mnuSelectFont";
			this.mnuSelectFont.Size = new System.Drawing.Size(157, 22);
			this.mnuSelectFont.Text = "Select Font...";
			this.mnuSelectFont.Click += new System.EventHandler(this.mnuSelectFont_Click);
			// 
			// mnuConfigureColors
			// 
			this.mnuConfigureColors.Image = global::Mesen.GUI.Properties.Resources.PipetteSmall;
			this.mnuConfigureColors.Name = "mnuConfigureColors";
			this.mnuConfigureColors.Size = new System.Drawing.Size(180, 22);
			this.mnuConfigureColors.Text = "Configure Colors";
			this.mnuConfigureColors.Click += new System.EventHandler(this.mnuConfigureColors_Click);
			// 
			// frmAssembler
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.CancelButton = this.btnCancel;
			this.ClientSize = new System.Drawing.Size(683, 456);
			this.Controls.Add(this.tableLayoutPanel1);
			this.Controls.Add(this.menuStrip1);
			this.MainMenuStrip = this.menuStrip1;
			this.MinimumSize = new System.Drawing.Size(551, 402);
			this.Name = "frmAssembler";
			this.Text = "Assembler";
			this.Controls.SetChildIndex(this.menuStrip1, 0);
			this.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.grpSettings.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.flowLayoutPanel1.ResumeLayout(false);
			this.flowLayoutPanel1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picStartAddressWarning)).EndInit();
			this.flowLayoutPanel2.ResumeLayout(false);
			this.flowLayoutPanel2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picSizeWarning)).EndInit();
			this.flowLayoutPanel3.ResumeLayout(false);
			this.groupBox1.ResumeLayout(false);
			this.panel1.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.txtCode)).EndInit();
			this.contextMenu.ResumeLayout(false);
			this.groupBox2.ResumeLayout(false);
			this.menuStrip1.ResumeLayout(false);
			this.menuStrip1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private Be.Windows.Forms.HexBox ctrlHexBox;
		private System.Windows.Forms.ListBox lstErrors;
		private System.Windows.Forms.GroupBox grpSettings;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox txtStartAddress;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel2;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label lblByteUsage;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel3;
		private System.Windows.Forms.Button btnCancel;
		private System.Windows.Forms.Button btnOk;
		private System.Windows.Forms.PictureBox picSizeWarning;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Label lblNoChanges;
		private System.Windows.Forms.PictureBox picStartAddressWarning;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.GroupBox groupBox2;
		private FastColoredTextBoxNS.FastColoredTextBox txtCode;
		private Mesen.GUI.Controls.ctrlMesenMenuStrip menuStrip1;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuClose;
		private System.Windows.Forms.ToolStripMenuItem viewToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuConfigureColors;
		private System.Windows.Forms.ToolStripMenuItem fontSizeToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuIncreaseFontSize;
		private System.Windows.Forms.ToolStripMenuItem mnuDecreaseFontSize;
		private System.Windows.Forms.ToolStripMenuItem mnuResetFontSize;
		private System.Windows.Forms.ContextMenuStrip contextMenu;
		private System.Windows.Forms.ToolStripMenuItem mnuCopy;
		private System.Windows.Forms.ToolStripMenuItem mnuCut;
		private System.Windows.Forms.ToolStripMenuItem mnuPaste;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem5;
		private System.Windows.Forms.ToolStripMenuItem mnuSelectAll;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuSelectFont;
	}
}