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
			this.tpgAdvanced = new System.Windows.Forms.TabPage();
			this.flowLayoutPanel8 = new System.Windows.Forms.FlowLayoutPanel();
			this.lblRamPowerOnState = new System.Windows.Forms.Label();
			this.cboRamPowerOnState = new System.Windows.Forms.ComboBox();
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
			this.flowLayoutPanel9.SuspendLayout();
			this.flowLayoutPanel6.SuspendLayout();
			this.flowLayoutPanel10.SuspendLayout();
			this.tpgAdvanced.SuspendLayout();
			this.flowLayoutPanel8.SuspendLayout();
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
			this.baseConfigPanel.Size = new System.Drawing.Size(414, 29);
			this.baseConfigPanel.TabIndex = 4;
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tpgGeneral);
			this.tabMain.Controls.Add(this.tpgAdvanced);
			this.tabMain.Controls.Add(this.tpgOverclocking);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(414, 290);
			this.tabMain.TabIndex = 2;
			// 
			// tpgGeneral
			// 
			this.tpgGeneral.Controls.Add(this.tableLayoutPanel4);
			this.tpgGeneral.Location = new System.Drawing.Point(4, 22);
			this.tpgGeneral.Name = "tpgGeneral";
			this.tpgGeneral.Padding = new System.Windows.Forms.Padding(3);
			this.tpgGeneral.Size = new System.Drawing.Size(406, 264);
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
			this.tableLayoutPanel4.RowCount = 5;
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Size = new System.Drawing.Size(400, 258);
			this.tableLayoutPanel4.TabIndex = 0;
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
			this.flowLayoutPanel9.Size = new System.Drawing.Size(289, 27);
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
			this.flowLayoutPanel6.Size = new System.Drawing.Size(289, 27);
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
			this.flowLayoutPanel10.Size = new System.Drawing.Size(289, 27);
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
			// tpgAdvanced
			// 
			this.tpgAdvanced.Controls.Add(this.flowLayoutPanel8);
			this.tpgAdvanced.Location = new System.Drawing.Point(4, 22);
			this.tpgAdvanced.Name = "tpgAdvanced";
			this.tpgAdvanced.Padding = new System.Windows.Forms.Padding(3);
			this.tpgAdvanced.Size = new System.Drawing.Size(406, 264);
			this.tpgAdvanced.TabIndex = 3;
			this.tpgAdvanced.Text = "Advanced";
			this.tpgAdvanced.UseVisualStyleBackColor = true;
			// 
			// flowLayoutPanel8
			// 
			this.flowLayoutPanel8.Controls.Add(this.lblRamPowerOnState);
			this.flowLayoutPanel8.Controls.Add(this.cboRamPowerOnState);
			this.flowLayoutPanel8.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowLayoutPanel8.Location = new System.Drawing.Point(3, 3);
			this.flowLayoutPanel8.Margin = new System.Windows.Forms.Padding(7, 0, 0, 0);
			this.flowLayoutPanel8.Name = "flowLayoutPanel8";
			this.flowLayoutPanel8.Size = new System.Drawing.Size(400, 258);
			this.flowLayoutPanel8.TabIndex = 4;
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
			// cboRamPowerOnState
			// 
			this.cboRamPowerOnState.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboRamPowerOnState.FormattingEnabled = true;
			this.cboRamPowerOnState.Location = new System.Drawing.Point(168, 3);
			this.cboRamPowerOnState.Name = "cboRamPowerOnState";
			this.cboRamPowerOnState.Size = new System.Drawing.Size(176, 21);
			this.cboRamPowerOnState.TabIndex = 1;
			// 
			// tpgOverclocking
			// 
			this.tpgOverclocking.Controls.Add(this.picHint);
			this.tpgOverclocking.Controls.Add(this.tableLayoutPanel3);
			this.tpgOverclocking.Location = new System.Drawing.Point(4, 22);
			this.tpgOverclocking.Name = "tpgOverclocking";
			this.tpgOverclocking.Size = new System.Drawing.Size(406, 264);
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
			this.tableLayoutPanel3.Size = new System.Drawing.Size(406, 264);
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
			this.grpPpuTiming.Size = new System.Drawing.Size(400, 71);
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
			this.tableLayoutPanel5.Size = new System.Drawing.Size(394, 52);
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
			this.tableLayoutPanel1.Size = new System.Drawing.Size(406, 25);
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
			this.ClientSize = new System.Drawing.Size(414, 319);
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
			this.flowLayoutPanel9.ResumeLayout(false);
			this.flowLayoutPanel9.PerformLayout();
			this.flowLayoutPanel6.ResumeLayout(false);
			this.flowLayoutPanel6.PerformLayout();
			this.flowLayoutPanel10.ResumeLayout(false);
			this.flowLayoutPanel10.PerformLayout();
			this.tpgAdvanced.ResumeLayout(false);
			this.flowLayoutPanel8.ResumeLayout(false);
			this.flowLayoutPanel8.PerformLayout();
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
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel8;
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
	}
}