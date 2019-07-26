namespace Mesen.GUI.Debugger
{
	partial class frmBreakpoint
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
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.lblBreakpointType = new System.Windows.Forms.Label();
			this.flowLayoutPanel2 = new System.Windows.Forms.FlowLayoutPanel();
			this.chkRead = new System.Windows.Forms.CheckBox();
			this.chkWrite = new System.Windows.Forms.CheckBox();
			this.chkExec = new System.Windows.Forms.CheckBox();
			this.lblBreakOn = new System.Windows.Forms.Label();
			this.lblAddress = new System.Windows.Forms.Label();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.picHelp = new System.Windows.Forms.PictureBox();
			this.txtCondition = new System.Windows.Forms.TextBox();
			this.picExpressionWarning = new System.Windows.Forms.PictureBox();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.txtAddress = new System.Windows.Forms.TextBox();
			this.radSpecificAddress = new System.Windows.Forms.RadioButton();
			this.radAnyAddress = new System.Windows.Forms.RadioButton();
			this.lblAddressSign = new System.Windows.Forms.Label();
			this.radRange = new System.Windows.Forms.RadioButton();
			this.tableLayoutPanel5 = new System.Windows.Forms.TableLayoutPanel();
			this.txtTo = new System.Windows.Forms.TextBox();
			this.lblTo = new System.Windows.Forms.Label();
			this.txtFrom = new System.Windows.Forms.TextBox();
			this.lblFrom = new System.Windows.Forms.Label();
			this.tableLayoutPanel4 = new System.Windows.Forms.TableLayoutPanel();
			this.cboBreakpointType = new Mesen.GUI.Debugger.Controls.ComboBoxWithSeparator();
			this.lblRange = new System.Windows.Forms.Label();
			this.tableLayoutPanel6 = new System.Windows.Forms.TableLayoutPanel();
			this.label1 = new System.Windows.Forms.Label();
			this.lblCondition = new System.Windows.Forms.Label();
			this.chkMarkOnEventViewer = new System.Windows.Forms.CheckBox();
			this.chkEnabled = new System.Windows.Forms.CheckBox();
			this.tableLayoutPanel7 = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel1.SuspendLayout();
			this.flowLayoutPanel2.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picHelp)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picExpressionWarning)).BeginInit();
			this.tableLayoutPanel3.SuspendLayout();
			this.tableLayoutPanel5.SuspendLayout();
			this.tableLayoutPanel4.SuspendLayout();
			this.tableLayoutPanel6.SuspendLayout();
			this.tableLayoutPanel7.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 197);
			this.baseConfigPanel.Size = new System.Drawing.Size(426, 29);
			this.baseConfigPanel.TabIndex = 4;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.lblBreakpointType, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.flowLayoutPanel2, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.lblBreakOn, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.lblAddress, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 1, 3);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel3, 1, 2);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel4, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel6, 0, 3);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 6;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(426, 226);
			this.tableLayoutPanel1.TabIndex = 2;
			// 
			// lblBreakpointType
			// 
			this.lblBreakpointType.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblBreakpointType.AutoSize = true;
			this.lblBreakpointType.Location = new System.Drawing.Point(3, 8);
			this.lblBreakpointType.Margin = new System.Windows.Forms.Padding(3, 4, 3, 0);
			this.lblBreakpointType.Name = "lblBreakpointType";
			this.lblBreakpointType.Size = new System.Drawing.Size(88, 13);
			this.lblBreakpointType.TabIndex = 12;
			this.lblBreakpointType.Text = "Breakpoint Type:";
			// 
			// flowLayoutPanel2
			// 
			this.flowLayoutPanel2.Controls.Add(this.chkRead);
			this.flowLayoutPanel2.Controls.Add(this.chkWrite);
			this.flowLayoutPanel2.Controls.Add(this.chkExec);
			this.flowLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowLayoutPanel2.Location = new System.Drawing.Point(94, 25);
			this.flowLayoutPanel2.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel2.Name = "flowLayoutPanel2";
			this.flowLayoutPanel2.Size = new System.Drawing.Size(332, 23);
			this.flowLayoutPanel2.TabIndex = 4;
			// 
			// chkRead
			// 
			this.chkRead.AutoSize = true;
			this.chkRead.Location = new System.Drawing.Point(3, 3);
			this.chkRead.Name = "chkRead";
			this.chkRead.Size = new System.Drawing.Size(52, 17);
			this.chkRead.TabIndex = 4;
			this.chkRead.Text = "Read";
			this.chkRead.UseVisualStyleBackColor = true;
			// 
			// chkWrite
			// 
			this.chkWrite.AutoSize = true;
			this.chkWrite.Location = new System.Drawing.Point(61, 3);
			this.chkWrite.Name = "chkWrite";
			this.chkWrite.Size = new System.Drawing.Size(51, 17);
			this.chkWrite.TabIndex = 5;
			this.chkWrite.Text = "Write";
			this.chkWrite.UseVisualStyleBackColor = true;
			// 
			// chkExec
			// 
			this.chkExec.AutoSize = true;
			this.chkExec.Location = new System.Drawing.Point(118, 3);
			this.chkExec.Name = "chkExec";
			this.chkExec.Size = new System.Drawing.Size(73, 17);
			this.chkExec.TabIndex = 3;
			this.chkExec.Text = "Execution";
			this.chkExec.UseVisualStyleBackColor = true;
			// 
			// lblBreakOn
			// 
			this.lblBreakOn.AutoSize = true;
			this.lblBreakOn.Location = new System.Drawing.Point(3, 29);
			this.lblBreakOn.Margin = new System.Windows.Forms.Padding(3, 4, 3, 0);
			this.lblBreakOn.Name = "lblBreakOn";
			this.lblBreakOn.Size = new System.Drawing.Size(53, 13);
			this.lblBreakOn.TabIndex = 0;
			this.lblBreakOn.Text = "Break on:";
			// 
			// lblAddress
			// 
			this.lblAddress.AutoSize = true;
			this.lblAddress.Location = new System.Drawing.Point(3, 53);
			this.lblAddress.Margin = new System.Windows.Forms.Padding(3, 5, 3, 0);
			this.lblAddress.Name = "lblAddress";
			this.lblAddress.Size = new System.Drawing.Size(48, 13);
			this.lblAddress.TabIndex = 3;
			this.lblAddress.Text = "Address:";
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 3;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel2.Controls.Add(this.picHelp, 2, 0);
			this.tableLayoutPanel2.Controls.Add(this.txtCondition, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.picExpressionWarning, 1, 0);
			this.tableLayoutPanel2.Location = new System.Drawing.Point(94, 123);
			this.tableLayoutPanel2.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 1;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(329, 65);
			this.tableLayoutPanel2.TabIndex = 10;
			// 
			// picHelp
			// 
			this.picHelp.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.picHelp.Image = global::Mesen.GUI.Properties.Resources.Help;
			this.picHelp.Location = new System.Drawing.Point(308, 24);
			this.picHelp.Margin = new System.Windows.Forms.Padding(3, 5, 3, 3);
			this.picHelp.Name = "picHelp";
			this.picHelp.Size = new System.Drawing.Size(18, 18);
			this.picHelp.TabIndex = 8;
			this.picHelp.TabStop = false;
			// 
			// txtCondition
			// 
			this.txtCondition.AcceptsReturn = true;
			this.txtCondition.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtCondition.Location = new System.Drawing.Point(3, 3);
			this.txtCondition.MaxLength = 900;
			this.txtCondition.Multiline = true;
			this.txtCondition.Name = "txtCondition";
			this.txtCondition.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.txtCondition.Size = new System.Drawing.Size(275, 59);
			this.txtCondition.TabIndex = 6;
			// 
			// picExpressionWarning
			// 
			this.picExpressionWarning.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.picExpressionWarning.Image = global::Mesen.GUI.Properties.Resources.Warning;
			this.picExpressionWarning.Location = new System.Drawing.Point(284, 24);
			this.picExpressionWarning.Margin = new System.Windows.Forms.Padding(3, 5, 3, 3);
			this.picExpressionWarning.Name = "picExpressionWarning";
			this.picExpressionWarning.Size = new System.Drawing.Size(18, 18);
			this.picExpressionWarning.TabIndex = 9;
			this.picExpressionWarning.TabStop = false;
			this.picExpressionWarning.Visible = false;
			// 
			// tableLayoutPanel3
			// 
			this.tableLayoutPanel3.ColumnCount = 4;
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.Controls.Add(this.txtAddress, 2, 0);
			this.tableLayoutPanel3.Controls.Add(this.radSpecificAddress, 0, 0);
			this.tableLayoutPanel3.Controls.Add(this.radAnyAddress, 0, 2);
			this.tableLayoutPanel3.Controls.Add(this.lblAddressSign, 1, 0);
			this.tableLayoutPanel3.Controls.Add(this.radRange, 0, 1);
			this.tableLayoutPanel3.Controls.Add(this.tableLayoutPanel5, 1, 1);
			this.tableLayoutPanel3.Location = new System.Drawing.Point(94, 48);
			this.tableLayoutPanel3.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel3.Name = "tableLayoutPanel3";
			this.tableLayoutPanel3.RowCount = 4;
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Size = new System.Drawing.Size(271, 75);
			this.tableLayoutPanel3.TabIndex = 11;
			// 
			// txtAddress
			// 
			this.txtAddress.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.txtAddress.Location = new System.Drawing.Point(85, 3);
			this.txtAddress.Margin = new System.Windows.Forms.Padding(0, 3, 3, 3);
			this.txtAddress.Name = "txtAddress";
			this.txtAddress.Size = new System.Drawing.Size(64, 20);
			this.txtAddress.TabIndex = 5;
			this.txtAddress.Enter += new System.EventHandler(this.txtAddress_Enter);
			// 
			// radSpecificAddress
			// 
			this.radSpecificAddress.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.radSpecificAddress.AutoSize = true;
			this.radSpecificAddress.Location = new System.Drawing.Point(3, 4);
			this.radSpecificAddress.Name = "radSpecificAddress";
			this.radSpecificAddress.Size = new System.Drawing.Size(66, 17);
			this.radSpecificAddress.TabIndex = 7;
			this.radSpecificAddress.TabStop = true;
			this.radSpecificAddress.Text = "Specific:";
			this.radSpecificAddress.UseVisualStyleBackColor = true;
			// 
			// radAnyAddress
			// 
			this.radAnyAddress.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.radAnyAddress.AutoSize = true;
			this.radAnyAddress.Location = new System.Drawing.Point(3, 52);
			this.radAnyAddress.Name = "radAnyAddress";
			this.radAnyAddress.Size = new System.Drawing.Size(43, 17);
			this.radAnyAddress.TabIndex = 8;
			this.radAnyAddress.TabStop = true;
			this.radAnyAddress.Text = "Any";
			this.radAnyAddress.UseVisualStyleBackColor = true;
			// 
			// lblAddressSign
			// 
			this.lblAddressSign.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblAddressSign.AutoSize = true;
			this.lblAddressSign.Location = new System.Drawing.Point(72, 6);
			this.lblAddressSign.Margin = new System.Windows.Forms.Padding(0);
			this.lblAddressSign.Name = "lblAddressSign";
			this.lblAddressSign.Size = new System.Drawing.Size(13, 13);
			this.lblAddressSign.TabIndex = 9;
			this.lblAddressSign.Text = "$";
			// 
			// radRange
			// 
			this.radRange.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.radRange.AutoSize = true;
			this.radRange.Location = new System.Drawing.Point(3, 29);
			this.radRange.Name = "radRange";
			this.radRange.Size = new System.Drawing.Size(60, 17);
			this.radRange.TabIndex = 10;
			this.radRange.TabStop = true;
			this.radRange.Text = "Range:";
			this.radRange.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel5
			// 
			this.tableLayoutPanel5.ColumnCount = 4;
			this.tableLayoutPanel3.SetColumnSpan(this.tableLayoutPanel5, 2);
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel5.Controls.Add(this.txtTo, 3, 0);
			this.tableLayoutPanel5.Controls.Add(this.lblTo, 2, 0);
			this.tableLayoutPanel5.Controls.Add(this.txtFrom, 1, 0);
			this.tableLayoutPanel5.Controls.Add(this.lblFrom, 0, 0);
			this.tableLayoutPanel5.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel5.Location = new System.Drawing.Point(72, 26);
			this.tableLayoutPanel5.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel5.Name = "tableLayoutPanel5";
			this.tableLayoutPanel5.RowCount = 1;
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel5.Size = new System.Drawing.Size(199, 23);
			this.tableLayoutPanel5.TabIndex = 12;
			// 
			// txtTo
			// 
			this.txtTo.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.txtTo.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtTo.Location = new System.Drawing.Point(131, 3);
			this.txtTo.Margin = new System.Windows.Forms.Padding(0, 3, 3, 3);
			this.txtTo.Name = "txtTo";
			this.txtTo.Size = new System.Drawing.Size(65, 20);
			this.txtTo.TabIndex = 13;
			this.txtTo.Enter += new System.EventHandler(this.txtTo_Enter);
			// 
			// lblTo
			// 
			this.lblTo.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblTo.AutoSize = true;
			this.lblTo.Location = new System.Drawing.Point(106, 5);
			this.lblTo.Margin = new System.Windows.Forms.Padding(0);
			this.lblTo.Name = "lblTo";
			this.lblTo.Size = new System.Drawing.Size(25, 13);
			this.lblTo.TabIndex = 11;
			this.lblTo.Text = "to $";
			// 
			// txtFrom
			// 
			this.txtFrom.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.txtFrom.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtFrom.Location = new System.Drawing.Point(39, 3);
			this.txtFrom.Margin = new System.Windows.Forms.Padding(0, 3, 3, 3);
			this.txtFrom.Name = "txtFrom";
			this.txtFrom.Size = new System.Drawing.Size(64, 20);
			this.txtFrom.TabIndex = 12;
			this.txtFrom.Enter += new System.EventHandler(this.txtFrom_Enter);
			// 
			// lblFrom
			// 
			this.lblFrom.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblFrom.AutoSize = true;
			this.lblFrom.Location = new System.Drawing.Point(0, 5);
			this.lblFrom.Margin = new System.Windows.Forms.Padding(0);
			this.lblFrom.Name = "lblFrom";
			this.lblFrom.Size = new System.Drawing.Size(39, 13);
			this.lblFrom.TabIndex = 10;
			this.lblFrom.Text = "From $";
			// 
			// tableLayoutPanel4
			// 
			this.tableLayoutPanel4.ColumnCount = 2;
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Controls.Add(this.cboBreakpointType, 0, 0);
			this.tableLayoutPanel4.Controls.Add(this.lblRange, 1, 0);
			this.tableLayoutPanel4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel4.Location = new System.Drawing.Point(94, 0);
			this.tableLayoutPanel4.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel4.Name = "tableLayoutPanel4";
			this.tableLayoutPanel4.RowCount = 1;
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Size = new System.Drawing.Size(332, 25);
			this.tableLayoutPanel4.TabIndex = 13;
			// 
			// cboBreakpointType
			// 
			this.cboBreakpointType.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
			this.cboBreakpointType.FormattingEnabled = true;
			this.cboBreakpointType.Location = new System.Drawing.Point(3, 3);
			this.cboBreakpointType.Name = "cboBreakpointType";
			this.cboBreakpointType.Size = new System.Drawing.Size(121, 21);
			this.cboBreakpointType.TabIndex = 13;
			this.cboBreakpointType.SelectedIndexChanged += new System.EventHandler(this.cboBreakpointType_SelectedIndexChanged);
			// 
			// lblRange
			// 
			this.lblRange.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRange.AutoSize = true;
			this.lblRange.ForeColor = System.Drawing.SystemColors.ControlDarkDark;
			this.lblRange.Location = new System.Drawing.Point(130, 6);
			this.lblRange.Name = "lblRange";
			this.lblRange.Size = new System.Drawing.Size(40, 13);
			this.lblRange.TabIndex = 6;
			this.lblRange.Text = "(range)";
			// 
			// tableLayoutPanel6
			// 
			this.tableLayoutPanel6.ColumnCount = 1;
			this.tableLayoutPanel6.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel6.Controls.Add(this.label1, 0, 1);
			this.tableLayoutPanel6.Controls.Add(this.lblCondition, 0, 0);
			this.tableLayoutPanel6.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel6.Location = new System.Drawing.Point(0, 123);
			this.tableLayoutPanel6.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel6.Name = "tableLayoutPanel6";
			this.tableLayoutPanel6.RowCount = 2;
			this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel6.Size = new System.Drawing.Size(94, 65);
			this.tableLayoutPanel6.TabIndex = 14;
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.ForeColor = System.Drawing.SystemColors.GrayText;
			this.label1.Location = new System.Drawing.Point(3, 32);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(50, 13);
			this.label1.TabIndex = 8;
			this.label1.Text = "(optional)";
			// 
			// lblCondition
			// 
			this.lblCondition.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblCondition.AutoSize = true;
			this.lblCondition.Location = new System.Drawing.Point(3, 19);
			this.lblCondition.Name = "lblCondition";
			this.lblCondition.Size = new System.Drawing.Size(54, 13);
			this.lblCondition.TabIndex = 7;
			this.lblCondition.Text = "Condition:";
			// 
			// chkMarkOnEventViewer
			// 
			this.chkMarkOnEventViewer.AutoSize = true;
			this.chkMarkOnEventViewer.Location = new System.Drawing.Point(3, 3);
			this.chkMarkOnEventViewer.Margin = new System.Windows.Forms.Padding(3, 0, 3, 0);
			this.chkMarkOnEventViewer.Name = "chkMarkOnEventViewer";
			this.chkMarkOnEventViewer.Size = new System.Drawing.Size(131, 17);
			this.chkMarkOnEventViewer.TabIndex = 15;
			this.chkMarkOnEventViewer.Text = "Mark on Event Viewer";
			this.chkMarkOnEventViewer.UseVisualStyleBackColor = true;
			// 
			// chkEnabled
			// 
			this.chkEnabled.AutoSize = true;
			this.chkEnabled.Location = new System.Drawing.Point(3, 20);
			this.chkEnabled.Margin = new System.Windows.Forms.Padding(3, 0, 3, 0);
			this.chkEnabled.Name = "chkEnabled";
			this.chkEnabled.Size = new System.Drawing.Size(104, 17);
			this.chkEnabled.TabIndex = 2;
			this.chkEnabled.Text = "Break Execution";
			this.chkEnabled.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel7
			// 
			this.tableLayoutPanel7.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.tableLayoutPanel7.ColumnCount = 1;
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel7.Controls.Add(this.chkEnabled, 0, 3);
			this.tableLayoutPanel7.Controls.Add(this.chkMarkOnEventViewer, 0, 2);
			this.tableLayoutPanel7.Location = new System.Drawing.Point(0, 188);
			this.tableLayoutPanel7.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel7.Name = "tableLayoutPanel7";
			this.tableLayoutPanel7.RowCount = 4;
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel7.Size = new System.Drawing.Size(243, 37);
			this.tableLayoutPanel7.TabIndex = 3;
			// 
			// frmBreakpoint
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(426, 226);
			this.Controls.Add(this.tableLayoutPanel7);
			this.Controls.Add(this.tableLayoutPanel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "frmBreakpoint";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Breakpoint";
			this.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tableLayoutPanel7, 0);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.flowLayoutPanel2.ResumeLayout(false);
			this.flowLayoutPanel2.PerformLayout();
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picHelp)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picExpressionWarning)).EndInit();
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel3.PerformLayout();
			this.tableLayoutPanel5.ResumeLayout(false);
			this.tableLayoutPanel5.PerformLayout();
			this.tableLayoutPanel4.ResumeLayout(false);
			this.tableLayoutPanel4.PerformLayout();
			this.tableLayoutPanel6.ResumeLayout(false);
			this.tableLayoutPanel6.PerformLayout();
			this.tableLayoutPanel7.ResumeLayout(false);
			this.tableLayoutPanel7.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label lblBreakOn;
		private System.Windows.Forms.Label lblAddress;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel2;
		private System.Windows.Forms.TextBox txtAddress;
		private System.Windows.Forms.CheckBox chkEnabled;
		private System.Windows.Forms.TextBox txtCondition;
		private System.Windows.Forms.Label lblCondition;
		private System.Windows.Forms.PictureBox picHelp;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.CheckBox chkExec;
		private System.Windows.Forms.CheckBox chkRead;
		private System.Windows.Forms.CheckBox chkWrite;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
		private System.Windows.Forms.RadioButton radSpecificAddress;
		private System.Windows.Forms.RadioButton radAnyAddress;
		private System.Windows.Forms.Label lblAddressSign;
		private System.Windows.Forms.PictureBox picExpressionWarning;
		private System.Windows.Forms.RadioButton radRange;
		private System.Windows.Forms.Label lblFrom;
		private System.Windows.Forms.TextBox txtFrom;
		private System.Windows.Forms.Label lblTo;
		private System.Windows.Forms.TextBox txtTo;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel5;
		private System.Windows.Forms.Label lblBreakpointType;
		private Mesen.GUI.Debugger.Controls.ComboBoxWithSeparator cboBreakpointType;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel4;
		private System.Windows.Forms.Label lblRange;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel6;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.CheckBox chkMarkOnEventViewer;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel7;
	}
}