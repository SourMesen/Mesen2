namespace Mesen.GUI.Forms.Config
{
	partial class frmEmulationConfig
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(frmEmulationConfig));
			this.tabMain = new System.Windows.Forms.TabControl();
			this.tpgGeneral = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel4 = new System.Windows.Forms.TableLayoutPanel();
			this.flowLayoutPanel5 = new System.Windows.Forms.FlowLayoutPanel();
			this.nudRunAheadFrames = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblRunAheadFrames = new System.Windows.Forms.Label();
			this.lblRunAhead = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.flowLayoutPanel9 = new System.Windows.Forms.FlowLayoutPanel();
			this.nudTurboSpeed = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblTurboSpeedHint = new System.Windows.Forms.Label();
			this.lblTurboSpeed = new System.Windows.Forms.Label();
			this.flowLayoutPanel6 = new System.Windows.Forms.FlowLayoutPanel();
			this.nudEmulationSpeed = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblEmuSpeedHint = new System.Windows.Forms.Label();
			this.lblEmulationSpeed = new System.Windows.Forms.Label();
			this.lblRewindSpeed = new System.Windows.Forms.Label();
			this.flowLayoutPanel10 = new System.Windows.Forms.FlowLayoutPanel();
			this.nudRewindSpeed = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblRewindSpeedHint = new System.Windows.Forms.Label();
			this.cboRegion = new System.Windows.Forms.ComboBox();
			this.tpgGameboy = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel7 = new System.Windows.Forms.TableLayoutPanel();
			this.lblGameboy = new System.Windows.Forms.Label();
			this.cboGameboyModel = new System.Windows.Forms.ComboBox();
			this.tpgBsx = new System.Windows.Forms.TabPage();
			this.grpBsxDateTime = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel6 = new System.Windows.Forms.TableLayoutPanel();
			this.dtpBsxCustomDate = new System.Windows.Forms.DateTimePicker();
			this.radBsxLocalTime = new System.Windows.Forms.RadioButton();
			this.radBsxCustomTime = new System.Windows.Forms.RadioButton();
			this.dtpBsxCustomTime = new System.Windows.Forms.DateTimePicker();
			this.tpgAdvanced = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.chkEnableRandomPowerOnState = new Mesen.GUI.Controls.ctrlRiskyOption();
			this.cboRamPowerOnState = new System.Windows.Forms.ComboBox();
			this.lblRamPowerOnState = new System.Windows.Forms.Label();
			this.chkEnableStrictBoardMappings = new Mesen.GUI.Controls.ctrlRiskyOption();
			this.tpgOverclocking = new System.Windows.Forms.TabPage();
			this.picHint = new System.Windows.Forms.PictureBox();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.lblOverclockHint = new System.Windows.Forms.Label();
			this.grpPpuTiming = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel5 = new System.Windows.Forms.TableLayoutPanel();
			this.nudExtraScanlinesAfterNmi = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.nudExtraScanlinesBeforeNmi = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblExtraScanlinesBeforeNmi = new System.Windows.Forms.Label();
			this.lblExtraScanlinesAfterNmi = new System.Windows.Forms.Label();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.nudGsuClockSpeed = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblGsuClockSpeed = new System.Windows.Forms.Label();
			this.tabMain.SuspendLayout();
			this.tpgGeneral.SuspendLayout();
			this.tableLayoutPanel4.SuspendLayout();
			this.flowLayoutPanel5.SuspendLayout();
			this.flowLayoutPanel9.SuspendLayout();
			this.flowLayoutPanel6.SuspendLayout();
			this.flowLayoutPanel10.SuspendLayout();
			this.tpgGameboy.SuspendLayout();
			this.tableLayoutPanel7.SuspendLayout();
			this.tpgBsx.SuspendLayout();
			this.grpBsxDateTime.SuspendLayout();
			this.tableLayoutPanel6.SuspendLayout();
			this.tpgAdvanced.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.tpgOverclocking.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picHint)).BeginInit();
			this.tableLayoutPanel3.SuspendLayout();
			this.grpPpuTiming.SuspendLayout();
			this.tableLayoutPanel5.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 290);
			this.baseConfigPanel.Size = new System.Drawing.Size(445, 29);
			this.baseConfigPanel.TabIndex = 4;
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tpgGeneral);
			this.tabMain.Controls.Add(this.tpgGameboy);
			this.tabMain.Controls.Add(this.tpgBsx);
			this.tabMain.Controls.Add(this.tpgAdvanced);
			this.tabMain.Controls.Add(this.tpgOverclocking);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(445, 290);
			this.tabMain.TabIndex = 2;
			// 
			// tpgGeneral
			// 
			this.tpgGeneral.Controls.Add(this.tableLayoutPanel4);
			this.tpgGeneral.Location = new System.Drawing.Point(4, 22);
			this.tpgGeneral.Name = "tpgGeneral";
			this.tpgGeneral.Padding = new System.Windows.Forms.Padding(3);
			this.tpgGeneral.Size = new System.Drawing.Size(437, 264);
			this.tpgGeneral.TabIndex = 2;
			this.tpgGeneral.Text = "General";
			this.tpgGeneral.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel4
			// 
			this.tableLayoutPanel4.AutoSize = true;
			this.tableLayoutPanel4.ColumnCount = 2;
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Controls.Add(this.flowLayoutPanel5, 1, 4);
			this.tableLayoutPanel4.Controls.Add(this.lblRunAhead, 0, 4);
			this.tableLayoutPanel4.Controls.Add(this.label1, 0, 3);
			this.tableLayoutPanel4.Controls.Add(this.flowLayoutPanel9, 1, 1);
			this.tableLayoutPanel4.Controls.Add(this.lblTurboSpeed, 0, 1);
			this.tableLayoutPanel4.Controls.Add(this.flowLayoutPanel6, 1, 0);
			this.tableLayoutPanel4.Controls.Add(this.lblEmulationSpeed, 0, 0);
			this.tableLayoutPanel4.Controls.Add(this.lblRewindSpeed, 0, 2);
			this.tableLayoutPanel4.Controls.Add(this.flowLayoutPanel10, 1, 2);
			this.tableLayoutPanel4.Controls.Add(this.cboRegion, 1, 3);
			this.tableLayoutPanel4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel4.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel4.Name = "tableLayoutPanel4";
			this.tableLayoutPanel4.RowCount = 6;
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Size = new System.Drawing.Size(431, 258);
			this.tableLayoutPanel4.TabIndex = 0;
			// 
			// flowLayoutPanel5
			// 
			this.flowLayoutPanel5.AutoSize = true;
			this.flowLayoutPanel5.Controls.Add(this.nudRunAheadFrames);
			this.flowLayoutPanel5.Controls.Add(this.lblRunAheadFrames);
			this.flowLayoutPanel5.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowLayoutPanel5.Location = new System.Drawing.Point(111, 108);
			this.flowLayoutPanel5.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel5.Name = "flowLayoutPanel5";
			this.flowLayoutPanel5.Size = new System.Drawing.Size(320, 27);
			this.flowLayoutPanel5.TabIndex = 20;
			// 
			// nudRunAheadFrames
			// 
			this.nudRunAheadFrames.DecimalPlaces = 0;
			this.nudRunAheadFrames.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudRunAheadFrames.IsHex = false;
			this.nudRunAheadFrames.Location = new System.Drawing.Point(3, 3);
			this.nudRunAheadFrames.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
			this.nudRunAheadFrames.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudRunAheadFrames.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudRunAheadFrames.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudRunAheadFrames.Name = "nudRunAheadFrames";
			this.nudRunAheadFrames.Size = new System.Drawing.Size(48, 21);
			this.nudRunAheadFrames.TabIndex = 1;
			this.nudRunAheadFrames.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
			// 
			// lblRunAheadFrames
			// 
			this.lblRunAheadFrames.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRunAheadFrames.AutoSize = true;
			this.lblRunAheadFrames.Location = new System.Drawing.Point(57, 7);
			this.lblRunAheadFrames.Name = "lblRunAheadFrames";
			this.lblRunAheadFrames.Size = new System.Drawing.Size(236, 13);
			this.lblRunAheadFrames.TabIndex = 2;
			this.lblRunAheadFrames.Text = "frames (reduces input lag, increases CPU usage)";
			// 
			// lblRunAhead
			// 
			this.lblRunAhead.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRunAhead.AutoSize = true;
			this.lblRunAhead.Location = new System.Drawing.Point(3, 115);
			this.lblRunAhead.Name = "lblRunAhead";
			this.lblRunAhead.Size = new System.Drawing.Size(64, 13);
			this.lblRunAhead.TabIndex = 19;
			this.lblRunAhead.Text = "Run Ahead:";
			// 
			// label1
			// 
			this.label1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(3, 88);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(44, 13);
			this.label1.TabIndex = 17;
			this.label1.Text = "Region:";
			// 
			// flowLayoutPanel9
			// 
			this.flowLayoutPanel9.AutoSize = true;
			this.flowLayoutPanel9.Controls.Add(this.nudTurboSpeed);
			this.flowLayoutPanel9.Controls.Add(this.lblTurboSpeedHint);
			this.flowLayoutPanel9.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowLayoutPanel9.Location = new System.Drawing.Point(111, 27);
			this.flowLayoutPanel9.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel9.Name = "flowLayoutPanel9";
			this.flowLayoutPanel9.Size = new System.Drawing.Size(320, 27);
			this.flowLayoutPanel9.TabIndex = 14;
			// 
			// nudTurboSpeed
			// 
			this.nudTurboSpeed.DecimalPlaces = 0;
			this.nudTurboSpeed.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudTurboSpeed.IsHex = false;
			this.nudTurboSpeed.Location = new System.Drawing.Point(3, 3);
			this.nudTurboSpeed.Maximum = new decimal(new int[] {
            5000,
            0,
            0,
            0});
			this.nudTurboSpeed.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudTurboSpeed.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudTurboSpeed.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudTurboSpeed.Name = "nudTurboSpeed";
			this.nudTurboSpeed.Size = new System.Drawing.Size(48, 21);
			this.nudTurboSpeed.TabIndex = 1;
			this.nudTurboSpeed.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
			// 
			// lblTurboSpeedHint
			// 
			this.lblTurboSpeedHint.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblTurboSpeedHint.AutoSize = true;
			this.lblTurboSpeedHint.Location = new System.Drawing.Point(57, 7);
			this.lblTurboSpeedHint.Name = "lblTurboSpeedHint";
			this.lblTurboSpeedHint.Size = new System.Drawing.Size(121, 13);
			this.lblTurboSpeedHint.TabIndex = 2;
			this.lblTurboSpeedHint.Text = "%  (0 = Maximum speed)";
			// 
			// lblTurboSpeed
			// 
			this.lblTurboSpeed.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblTurboSpeed.AutoSize = true;
			this.lblTurboSpeed.Location = new System.Drawing.Point(3, 34);
			this.lblTurboSpeed.Name = "lblTurboSpeed";
			this.lblTurboSpeed.Size = new System.Drawing.Size(105, 13);
			this.lblTurboSpeed.TabIndex = 13;
			this.lblTurboSpeed.Text = "Fast Forward Speed:";
			// 
			// flowLayoutPanel6
			// 
			this.flowLayoutPanel6.AutoSize = true;
			this.flowLayoutPanel6.Controls.Add(this.nudEmulationSpeed);
			this.flowLayoutPanel6.Controls.Add(this.lblEmuSpeedHint);
			this.flowLayoutPanel6.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowLayoutPanel6.Location = new System.Drawing.Point(111, 0);
			this.flowLayoutPanel6.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel6.Name = "flowLayoutPanel6";
			this.flowLayoutPanel6.Size = new System.Drawing.Size(320, 27);
			this.flowLayoutPanel6.TabIndex = 11;
			// 
			// nudEmulationSpeed
			// 
			this.nudEmulationSpeed.DecimalPlaces = 0;
			this.nudEmulationSpeed.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudEmulationSpeed.IsHex = false;
			this.nudEmulationSpeed.Location = new System.Drawing.Point(3, 3);
			this.nudEmulationSpeed.Maximum = new decimal(new int[] {
            5000,
            0,
            0,
            0});
			this.nudEmulationSpeed.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudEmulationSpeed.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudEmulationSpeed.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudEmulationSpeed.Name = "nudEmulationSpeed";
			this.nudEmulationSpeed.Size = new System.Drawing.Size(48, 21);
			this.nudEmulationSpeed.TabIndex = 1;
			this.nudEmulationSpeed.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
			// 
			// lblEmuSpeedHint
			// 
			this.lblEmuSpeedHint.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblEmuSpeedHint.AutoSize = true;
			this.lblEmuSpeedHint.Location = new System.Drawing.Point(57, 7);
			this.lblEmuSpeedHint.Name = "lblEmuSpeedHint";
			this.lblEmuSpeedHint.Size = new System.Drawing.Size(121, 13);
			this.lblEmuSpeedHint.TabIndex = 2;
			this.lblEmuSpeedHint.Text = "%  (0 = Maximum speed)";
			// 
			// lblEmulationSpeed
			// 
			this.lblEmulationSpeed.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblEmulationSpeed.AutoSize = true;
			this.lblEmulationSpeed.Location = new System.Drawing.Point(3, 7);
			this.lblEmulationSpeed.Name = "lblEmulationSpeed";
			this.lblEmulationSpeed.Size = new System.Drawing.Size(90, 13);
			this.lblEmulationSpeed.TabIndex = 12;
			this.lblEmulationSpeed.Text = "Emulation Speed:";
			// 
			// lblRewindSpeed
			// 
			this.lblRewindSpeed.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRewindSpeed.AutoSize = true;
			this.lblRewindSpeed.Location = new System.Drawing.Point(3, 61);
			this.lblRewindSpeed.Name = "lblRewindSpeed";
			this.lblRewindSpeed.Size = new System.Drawing.Size(80, 13);
			this.lblRewindSpeed.TabIndex = 15;
			this.lblRewindSpeed.Text = "Rewind Speed:";
			// 
			// flowLayoutPanel10
			// 
			this.flowLayoutPanel10.AutoSize = true;
			this.flowLayoutPanel10.Controls.Add(this.nudRewindSpeed);
			this.flowLayoutPanel10.Controls.Add(this.lblRewindSpeedHint);
			this.flowLayoutPanel10.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowLayoutPanel10.Location = new System.Drawing.Point(111, 54);
			this.flowLayoutPanel10.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel10.Name = "flowLayoutPanel10";
			this.flowLayoutPanel10.Size = new System.Drawing.Size(320, 27);
			this.flowLayoutPanel10.TabIndex = 16;
			// 
			// nudRewindSpeed
			// 
			this.nudRewindSpeed.DecimalPlaces = 0;
			this.nudRewindSpeed.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudRewindSpeed.IsHex = false;
			this.nudRewindSpeed.Location = new System.Drawing.Point(3, 3);
			this.nudRewindSpeed.Maximum = new decimal(new int[] {
            5000,
            0,
            0,
            0});
			this.nudRewindSpeed.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudRewindSpeed.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudRewindSpeed.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudRewindSpeed.Name = "nudRewindSpeed";
			this.nudRewindSpeed.Size = new System.Drawing.Size(48, 21);
			this.nudRewindSpeed.TabIndex = 1;
			this.nudRewindSpeed.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
			// 
			// lblRewindSpeedHint
			// 
			this.lblRewindSpeedHint.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRewindSpeedHint.AutoSize = true;
			this.lblRewindSpeedHint.Location = new System.Drawing.Point(57, 7);
			this.lblRewindSpeedHint.Name = "lblRewindSpeedHint";
			this.lblRewindSpeedHint.Size = new System.Drawing.Size(121, 13);
			this.lblRewindSpeedHint.TabIndex = 2;
			this.lblRewindSpeedHint.Text = "%  (0 = Maximum speed)";
			// 
			// cboRegion
			// 
			this.cboRegion.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboRegion.FormattingEnabled = true;
			this.cboRegion.Location = new System.Drawing.Point(114, 84);
			this.cboRegion.Name = "cboRegion";
			this.cboRegion.Size = new System.Drawing.Size(121, 21);
			this.cboRegion.TabIndex = 18;
			// 
			// tpgGameboy
			// 
			this.tpgGameboy.Controls.Add(this.tableLayoutPanel7);
			this.tpgGameboy.Location = new System.Drawing.Point(4, 22);
			this.tpgGameboy.Name = "tpgGameboy";
			this.tpgGameboy.Padding = new System.Windows.Forms.Padding(3);
			this.tpgGameboy.Size = new System.Drawing.Size(437, 264);
			this.tpgGameboy.TabIndex = 6;
			this.tpgGameboy.Text = "Game Boy";
			this.tpgGameboy.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel7
			// 
			this.tableLayoutPanel7.ColumnCount = 2;
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel7.Controls.Add(this.lblGameboy, 0, 0);
			this.tableLayoutPanel7.Controls.Add(this.cboGameboyModel, 1, 0);
			this.tableLayoutPanel7.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel7.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel7.Name = "tableLayoutPanel7";
			this.tableLayoutPanel7.RowCount = 2;
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel7.Size = new System.Drawing.Size(431, 258);
			this.tableLayoutPanel7.TabIndex = 0;
			// 
			// lblGameboy
			// 
			this.lblGameboy.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblGameboy.AutoSize = true;
			this.lblGameboy.Location = new System.Drawing.Point(3, 7);
			this.lblGameboy.Name = "lblGameboy";
			this.lblGameboy.Size = new System.Drawing.Size(91, 13);
			this.lblGameboy.TabIndex = 0;
			this.lblGameboy.Text = "Game Boy Model:";
			// 
			// cboGameboyModel
			// 
			this.cboGameboyModel.FormattingEnabled = true;
			this.cboGameboyModel.Location = new System.Drawing.Point(100, 3);
			this.cboGameboyModel.Name = "cboGameboyModel";
			this.cboGameboyModel.Size = new System.Drawing.Size(119, 21);
			this.cboGameboyModel.TabIndex = 1;
			// 
			// tpgBsx
			// 
			this.tpgBsx.Controls.Add(this.grpBsxDateTime);
			this.tpgBsx.Location = new System.Drawing.Point(4, 22);
			this.tpgBsx.Name = "tpgBsx";
			this.tpgBsx.Padding = new System.Windows.Forms.Padding(3);
			this.tpgBsx.Size = new System.Drawing.Size(437, 264);
			this.tpgBsx.TabIndex = 5;
			this.tpgBsx.Text = "BS-X";
			this.tpgBsx.UseVisualStyleBackColor = true;
			// 
			// grpBsxDateTime
			// 
			this.grpBsxDateTime.Controls.Add(this.tableLayoutPanel6);
			this.grpBsxDateTime.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpBsxDateTime.Location = new System.Drawing.Point(3, 3);
			this.grpBsxDateTime.Name = "grpBsxDateTime";
			this.grpBsxDateTime.Size = new System.Drawing.Size(431, 258);
			this.grpBsxDateTime.TabIndex = 1;
			this.grpBsxDateTime.TabStop = false;
			this.grpBsxDateTime.Text = "BS-X/Satellaview Date and Time Settings";
			// 
			// tableLayoutPanel6
			// 
			this.tableLayoutPanel6.ColumnCount = 3;
			this.tableLayoutPanel6.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel6.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel6.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel6.Controls.Add(this.dtpBsxCustomDate, 1, 1);
			this.tableLayoutPanel6.Controls.Add(this.radBsxLocalTime, 0, 0);
			this.tableLayoutPanel6.Controls.Add(this.radBsxCustomTime, 0, 1);
			this.tableLayoutPanel6.Controls.Add(this.dtpBsxCustomTime, 2, 1);
			this.tableLayoutPanel6.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel6.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel6.Name = "tableLayoutPanel6";
			this.tableLayoutPanel6.RowCount = 3;
			this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel6.Size = new System.Drawing.Size(425, 239);
			this.tableLayoutPanel6.TabIndex = 0;
			// 
			// dtpBsxCustomDate
			// 
			this.dtpBsxCustomDate.CustomFormat = "";
			this.dtpBsxCustomDate.Dock = System.Windows.Forms.DockStyle.Fill;
			this.dtpBsxCustomDate.Format = System.Windows.Forms.DateTimePickerFormat.Short;
			this.dtpBsxCustomDate.Location = new System.Drawing.Point(160, 26);
			this.dtpBsxCustomDate.MaxDate = new System.DateTime(2100, 12, 31, 0, 0, 0, 0);
			this.dtpBsxCustomDate.MinDate = new System.DateTime(1970, 1, 1, 0, 0, 0, 0);
			this.dtpBsxCustomDate.Name = "dtpBsxCustomDate";
			this.dtpBsxCustomDate.Size = new System.Drawing.Size(128, 20);
			this.dtpBsxCustomDate.TabIndex = 3;
			this.dtpBsxCustomDate.Value = new System.DateTime(2020, 1, 1, 0, 0, 0, 0);
			this.dtpBsxCustomDate.Enter += new System.EventHandler(this.dtpBsxCustomDate_Enter);
			// 
			// radBsxLocalTime
			// 
			this.radBsxLocalTime.AutoSize = true;
			this.radBsxLocalTime.Location = new System.Drawing.Point(3, 3);
			this.radBsxLocalTime.Name = "radBsxLocalTime";
			this.radBsxLocalTime.Size = new System.Drawing.Size(127, 17);
			this.radBsxLocalTime.TabIndex = 0;
			this.radBsxLocalTime.TabStop = true;
			this.radBsxLocalTime.Text = "Use current local time";
			this.radBsxLocalTime.UseVisualStyleBackColor = true;
			// 
			// radBsxCustomTime
			// 
			this.radBsxCustomTime.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.radBsxCustomTime.AutoSize = true;
			this.radBsxCustomTime.Location = new System.Drawing.Point(3, 27);
			this.radBsxCustomTime.Name = "radBsxCustomTime";
			this.radBsxCustomTime.Size = new System.Drawing.Size(151, 17);
			this.radBsxCustomTime.TabIndex = 1;
			this.radBsxCustomTime.TabStop = true;
			this.radBsxCustomTime.Text = "Use custom date and time:";
			this.radBsxCustomTime.UseVisualStyleBackColor = true;
			// 
			// dtpBsxCustomTime
			// 
			this.dtpBsxCustomTime.CustomFormat = "";
			this.dtpBsxCustomTime.Dock = System.Windows.Forms.DockStyle.Fill;
			this.dtpBsxCustomTime.Format = System.Windows.Forms.DateTimePickerFormat.Time;
			this.dtpBsxCustomTime.Location = new System.Drawing.Point(294, 26);
			this.dtpBsxCustomTime.MaxDate = new System.DateTime(2000, 1, 2, 0, 0, 0, 0);
			this.dtpBsxCustomTime.MinDate = new System.DateTime(2000, 1, 1, 0, 0, 0, 0);
			this.dtpBsxCustomTime.Name = "dtpBsxCustomTime";
			this.dtpBsxCustomTime.ShowUpDown = true;
			this.dtpBsxCustomTime.Size = new System.Drawing.Size(128, 20);
			this.dtpBsxCustomTime.TabIndex = 2;
			this.dtpBsxCustomTime.Value = new System.DateTime(2000, 1, 1, 0, 0, 0, 0);
			this.dtpBsxCustomTime.Enter += new System.EventHandler(this.dtpBsxCustomTime_Enter);
			// 
			// tpgAdvanced
			// 
			this.tpgAdvanced.Controls.Add(this.tableLayoutPanel2);
			this.tpgAdvanced.Location = new System.Drawing.Point(4, 22);
			this.tpgAdvanced.Name = "tpgAdvanced";
			this.tpgAdvanced.Padding = new System.Windows.Forms.Padding(3);
			this.tpgAdvanced.Size = new System.Drawing.Size(437, 264);
			this.tpgAdvanced.TabIndex = 3;
			this.tpgAdvanced.Text = "Advanced";
			this.tpgAdvanced.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 2;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Controls.Add(this.chkEnableRandomPowerOnState, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.cboRamPowerOnState, 1, 0);
			this.tableLayoutPanel2.Controls.Add(this.lblRamPowerOnState, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkEnableStrictBoardMappings, 0, 2);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 4;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(431, 258);
			this.tableLayoutPanel2.TabIndex = 5;
			// 
			// chkEnableRandomPowerOnState
			// 
			this.chkEnableRandomPowerOnState.Checked = false;
			this.tableLayoutPanel2.SetColumnSpan(this.chkEnableRandomPowerOnState, 2);
			this.chkEnableRandomPowerOnState.Location = new System.Drawing.Point(0, 27);
			this.chkEnableRandomPowerOnState.Name = "chkEnableRandomPowerOnState";
			this.chkEnableRandomPowerOnState.Size = new System.Drawing.Size(273, 24);
			this.chkEnableRandomPowerOnState.TabIndex = 6;
			this.chkEnableRandomPowerOnState.Text = "Randomize power-on state";
			// 
			// cboRamPowerOnState
			// 
			this.cboRamPowerOnState.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboRamPowerOnState.FormattingEnabled = true;
			this.cboRamPowerOnState.Location = new System.Drawing.Point(168, 3);
			this.cboRamPowerOnState.Name = "cboRamPowerOnState";
			this.cboRamPowerOnState.Size = new System.Drawing.Size(176, 21);
			this.cboRamPowerOnState.TabIndex = 1;
			// 
			// lblRamPowerOnState
			// 
			this.lblRamPowerOnState.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRamPowerOnState.AutoSize = true;
			this.lblRamPowerOnState.Location = new System.Drawing.Point(3, 7);
			this.lblRamPowerOnState.Name = "lblRamPowerOnState";
			this.lblRamPowerOnState.Size = new System.Drawing.Size(159, 13);
			this.lblRamPowerOnState.TabIndex = 0;
			this.lblRamPowerOnState.Text = "Default power on state for RAM:";
			// 
			// chkEnableStrictBoardMappings
			// 
			this.chkEnableStrictBoardMappings.Checked = false;
			this.tableLayoutPanel2.SetColumnSpan(this.chkEnableStrictBoardMappings, 2);
			this.chkEnableStrictBoardMappings.Location = new System.Drawing.Point(0, 51);
			this.chkEnableStrictBoardMappings.Name = "chkEnableStrictBoardMappings";
			this.chkEnableStrictBoardMappings.Size = new System.Drawing.Size(400, 24);
			this.chkEnableStrictBoardMappings.TabIndex = 7;
			this.chkEnableStrictBoardMappings.Text = "Use strict board mappings (breaks some romhacks)";
			// 
			// tpgOverclocking
			// 
			this.tpgOverclocking.Controls.Add(this.picHint);
			this.tpgOverclocking.Controls.Add(this.tableLayoutPanel3);
			this.tpgOverclocking.Location = new System.Drawing.Point(4, 22);
			this.tpgOverclocking.Name = "tpgOverclocking";
			this.tpgOverclocking.Size = new System.Drawing.Size(437, 264);
			this.tpgOverclocking.TabIndex = 4;
			this.tpgOverclocking.Text = "Overclocking";
			this.tpgOverclocking.UseVisualStyleBackColor = true;
			// 
			// picHint
			// 
			this.picHint.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.picHint.BackgroundImage = global::Mesen.GUI.Properties.Resources.Help;
			this.picHint.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
			this.picHint.Location = new System.Drawing.Point(8, 20);
			this.picHint.Margin = new System.Windows.Forms.Padding(3, 5, 3, 3);
			this.picHint.Name = "picHint";
			this.picHint.Size = new System.Drawing.Size(16, 16);
			this.picHint.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
			this.picHint.TabIndex = 8;
			this.picHint.TabStop = false;
			// 
			// tableLayoutPanel3
			// 
			this.tableLayoutPanel3.ColumnCount = 1;
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Controls.Add(this.lblOverclockHint, 0, 0);
			this.tableLayoutPanel3.Controls.Add(this.grpPpuTiming, 0, 1);
			this.tableLayoutPanel3.Controls.Add(this.tableLayoutPanel1, 0, 2);
			this.tableLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel3.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel3.Name = "tableLayoutPanel3";
			this.tableLayoutPanel3.RowCount = 4;
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Size = new System.Drawing.Size(437, 264);
			this.tableLayoutPanel3.TabIndex = 1;
			// 
			// lblOverclockHint
			// 
			this.lblOverclockHint.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.lblOverclockHint.Location = new System.Drawing.Point(3, 0);
			this.lblOverclockHint.Name = "lblOverclockHint";
			this.lblOverclockHint.Padding = new System.Windows.Forms.Padding(25, 0, 0, 0);
			this.lblOverclockHint.Size = new System.Drawing.Size(400, 54);
			this.lblOverclockHint.TabIndex = 1;
			this.lblOverclockHint.Text = resources.GetString("lblOverclockHint.Text");
			// 
			// grpPpuTiming
			// 
			this.grpPpuTiming.Controls.Add(this.tableLayoutPanel5);
			this.grpPpuTiming.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpPpuTiming.Location = new System.Drawing.Point(3, 57);
			this.grpPpuTiming.Name = "grpPpuTiming";
			this.grpPpuTiming.Size = new System.Drawing.Size(431, 71);
			this.grpPpuTiming.TabIndex = 7;
			this.grpPpuTiming.TabStop = false;
			this.grpPpuTiming.Text = "PPU Vertical Blank Configuration";
			// 
			// tableLayoutPanel5
			// 
			this.tableLayoutPanel5.ColumnCount = 2;
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel5.Controls.Add(this.nudExtraScanlinesAfterNmi, 1, 1);
			this.tableLayoutPanel5.Controls.Add(this.nudExtraScanlinesBeforeNmi, 1, 0);
			this.tableLayoutPanel5.Controls.Add(this.lblExtraScanlinesBeforeNmi, 0, 0);
			this.tableLayoutPanel5.Controls.Add(this.lblExtraScanlinesAfterNmi, 0, 1);
			this.tableLayoutPanel5.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel5.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel5.Name = "tableLayoutPanel5";
			this.tableLayoutPanel5.RowCount = 3;
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel5.Size = new System.Drawing.Size(425, 52);
			this.tableLayoutPanel5.TabIndex = 0;
			// 
			// nudExtraScanlinesAfterNmi
			// 
			this.nudExtraScanlinesAfterNmi.DecimalPlaces = 0;
			this.nudExtraScanlinesAfterNmi.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudExtraScanlinesAfterNmi.IsHex = false;
			this.nudExtraScanlinesAfterNmi.Location = new System.Drawing.Point(165, 30);
			this.nudExtraScanlinesAfterNmi.Margin = new System.Windows.Forms.Padding(0, 3, 0, 3);
			this.nudExtraScanlinesAfterNmi.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
			this.nudExtraScanlinesAfterNmi.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudExtraScanlinesAfterNmi.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudExtraScanlinesAfterNmi.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudExtraScanlinesAfterNmi.Name = "nudExtraScanlinesAfterNmi";
			this.nudExtraScanlinesAfterNmi.Size = new System.Drawing.Size(46, 21);
			this.nudExtraScanlinesAfterNmi.TabIndex = 3;
			this.nudExtraScanlinesAfterNmi.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
			// 
			// nudExtraScanlinesBeforeNmi
			// 
			this.nudExtraScanlinesBeforeNmi.DecimalPlaces = 0;
			this.nudExtraScanlinesBeforeNmi.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudExtraScanlinesBeforeNmi.IsHex = false;
			this.nudExtraScanlinesBeforeNmi.Location = new System.Drawing.Point(165, 3);
			this.nudExtraScanlinesBeforeNmi.Margin = new System.Windows.Forms.Padding(0, 3, 0, 3);
			this.nudExtraScanlinesBeforeNmi.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
			this.nudExtraScanlinesBeforeNmi.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudExtraScanlinesBeforeNmi.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudExtraScanlinesBeforeNmi.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudExtraScanlinesBeforeNmi.Name = "nudExtraScanlinesBeforeNmi";
			this.nudExtraScanlinesBeforeNmi.Size = new System.Drawing.Size(46, 21);
			this.nudExtraScanlinesBeforeNmi.TabIndex = 2;
			this.nudExtraScanlinesBeforeNmi.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
			// 
			// lblExtraScanlinesBeforeNmi
			// 
			this.lblExtraScanlinesBeforeNmi.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblExtraScanlinesBeforeNmi.AutoSize = true;
			this.lblExtraScanlinesBeforeNmi.Location = new System.Drawing.Point(3, 7);
			this.lblExtraScanlinesBeforeNmi.Name = "lblExtraScanlinesBeforeNmi";
			this.lblExtraScanlinesBeforeNmi.Size = new System.Drawing.Size(159, 13);
			this.lblExtraScanlinesBeforeNmi.TabIndex = 0;
			this.lblExtraScanlinesBeforeNmi.Text = "Additional scanlines before NMI:";
			// 
			// lblExtraScanlinesAfterNmi
			// 
			this.lblExtraScanlinesAfterNmi.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblExtraScanlinesAfterNmi.AutoSize = true;
			this.lblExtraScanlinesAfterNmi.Location = new System.Drawing.Point(3, 34);
			this.lblExtraScanlinesAfterNmi.Name = "lblExtraScanlinesAfterNmi";
			this.lblExtraScanlinesAfterNmi.Size = new System.Drawing.Size(150, 13);
			this.lblExtraScanlinesAfterNmi.TabIndex = 1;
			this.lblExtraScanlinesAfterNmi.Text = "Additional scanlines after NMI:";
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.nudGsuClockSpeed, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblGsuClockSpeed, 0, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 131);
			this.tableLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 1;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(437, 25);
			this.tableLayoutPanel1.TabIndex = 8;
			// 
			// nudGsuClockSpeed
			// 
			this.nudGsuClockSpeed.DecimalPlaces = 0;
			this.nudGsuClockSpeed.Increment = new decimal(new int[] {
            100,
            0,
            0,
            0});
			this.nudGsuClockSpeed.IsHex = false;
			this.nudGsuClockSpeed.Location = new System.Drawing.Point(138, 3);
			this.nudGsuClockSpeed.Margin = new System.Windows.Forms.Padding(0, 3, 0, 3);
			this.nudGsuClockSpeed.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
			this.nudGsuClockSpeed.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudGsuClockSpeed.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            0});
			this.nudGsuClockSpeed.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudGsuClockSpeed.Name = "nudGsuClockSpeed";
			this.nudGsuClockSpeed.Size = new System.Drawing.Size(46, 21);
			this.nudGsuClockSpeed.TabIndex = 4;
			this.nudGsuClockSpeed.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
			this.nudGsuClockSpeed.Leave += new System.EventHandler(this.nudGsuClockSpeed_Leave);
			// 
			// lblGsuClockSpeed
			// 
			this.lblGsuClockSpeed.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblGsuClockSpeed.AutoSize = true;
			this.lblGsuClockSpeed.Location = new System.Drawing.Point(3, 6);
			this.lblGsuClockSpeed.Name = "lblGsuClockSpeed";
			this.lblGsuClockSpeed.Size = new System.Drawing.Size(132, 13);
			this.lblGsuClockSpeed.TabIndex = 0;
			this.lblGsuClockSpeed.Text = "Super FX clock speed (%):";
			// 
			// frmEmulationConfig
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(445, 319);
			this.Controls.Add(this.tabMain);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "frmEmulationConfig";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Emulation Config";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tabMain, 0);
			this.tabMain.ResumeLayout(false);
			this.tpgGeneral.ResumeLayout(false);
			this.tpgGeneral.PerformLayout();
			this.tableLayoutPanel4.ResumeLayout(false);
			this.tableLayoutPanel4.PerformLayout();
			this.flowLayoutPanel5.ResumeLayout(false);
			this.flowLayoutPanel5.PerformLayout();
			this.flowLayoutPanel9.ResumeLayout(false);
			this.flowLayoutPanel9.PerformLayout();
			this.flowLayoutPanel6.ResumeLayout(false);
			this.flowLayoutPanel6.PerformLayout();
			this.flowLayoutPanel10.ResumeLayout(false);
			this.flowLayoutPanel10.PerformLayout();
			this.tpgGameboy.ResumeLayout(false);
			this.tableLayoutPanel7.ResumeLayout(false);
			this.tableLayoutPanel7.PerformLayout();
			this.tpgBsx.ResumeLayout(false);
			this.grpBsxDateTime.ResumeLayout(false);
			this.tableLayoutPanel6.ResumeLayout(false);
			this.tableLayoutPanel6.PerformLayout();
			this.tpgAdvanced.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.tpgOverclocking.ResumeLayout(false);
			this.tpgOverclocking.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picHint)).EndInit();
			this.tableLayoutPanel3.ResumeLayout(false);
			this.grpPpuTiming.ResumeLayout(false);
			this.tableLayoutPanel5.ResumeLayout(false);
			this.tableLayoutPanel5.PerformLayout();
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tpgGeneral;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel4;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel9;
		private Controls.MesenNumericUpDown nudTurboSpeed;
		private System.Windows.Forms.Label lblTurboSpeedHint;
		private System.Windows.Forms.Label lblTurboSpeed;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel6;
		private Controls.MesenNumericUpDown nudEmulationSpeed;
		private System.Windows.Forms.Label lblEmuSpeedHint;
		private System.Windows.Forms.Label lblEmulationSpeed;
		private System.Windows.Forms.Label lblRewindSpeed;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel10;
		private Controls.MesenNumericUpDown nudRewindSpeed;
		private System.Windows.Forms.Label lblRewindSpeedHint;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.ComboBox cboRegion;
		private System.Windows.Forms.TabPage tpgAdvanced;
		private System.Windows.Forms.Label lblRamPowerOnState;
		private System.Windows.Forms.ComboBox cboRamPowerOnState;
		private System.Windows.Forms.TabPage tpgOverclocking;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
		private System.Windows.Forms.Label lblOverclockHint;
		private System.Windows.Forms.GroupBox grpPpuTiming;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel5;
		private Controls.MesenNumericUpDown nudExtraScanlinesAfterNmi;
		private Controls.MesenNumericUpDown nudExtraScanlinesBeforeNmi;
		private System.Windows.Forms.Label lblExtraScanlinesBeforeNmi;
		private System.Windows.Forms.Label lblExtraScanlinesAfterNmi;
		private System.Windows.Forms.PictureBox picHint;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private Controls.MesenNumericUpDown nudGsuClockSpeed;
		private System.Windows.Forms.Label lblGsuClockSpeed;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private Controls.ctrlRiskyOption chkEnableRandomPowerOnState;
		private Controls.ctrlRiskyOption chkEnableStrictBoardMappings;
	  private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel5;
	  private Controls.MesenNumericUpDown nudRunAheadFrames;
	  private System.Windows.Forms.Label lblRunAheadFrames;
	  private System.Windows.Forms.Label lblRunAhead;
	  private System.Windows.Forms.TabPage tpgBsx;
	  private System.Windows.Forms.GroupBox grpBsxDateTime;
	  private System.Windows.Forms.TableLayoutPanel tableLayoutPanel6;
	  private System.Windows.Forms.RadioButton radBsxLocalTime;
	  private System.Windows.Forms.RadioButton radBsxCustomTime;
	  private System.Windows.Forms.DateTimePicker dtpBsxCustomTime;
	  private System.Windows.Forms.DateTimePicker dtpBsxCustomDate;
	  private System.Windows.Forms.TabPage tpgGameboy;
	  private System.Windows.Forms.TableLayoutPanel tableLayoutPanel7;
	  private System.Windows.Forms.Label lblGameboy;
	  private System.Windows.Forms.ComboBox cboGameboyModel;
   }
}