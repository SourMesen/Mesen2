namespace Mesen.GUI.Forms.Config
{
	partial class frmVideoConfig
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
			this.tabMain = new System.Windows.Forms.TabControl();
			this.tpgGeneral = new System.Windows.Forms.TabPage();
			this.tlpMain = new System.Windows.Forms.TableLayoutPanel();
			this.chkUseExclusiveFullscreen = new System.Windows.Forms.CheckBox();
			this.lblVideoScale = new System.Windows.Forms.Label();
			this.chkVerticalSync = new System.Windows.Forms.CheckBox();
			this.lblDisplayRatio = new System.Windows.Forms.Label();
			this.nudScale = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.flowLayoutPanel6 = new System.Windows.Forms.FlowLayoutPanel();
			this.cboAspectRatio = new System.Windows.Forms.ComboBox();
			this.lblCustomRatio = new System.Windows.Forms.Label();
			this.nudCustomRatio = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.chkFullscreenForceIntegerScale = new System.Windows.Forms.CheckBox();
			this.chkShowFps = new System.Windows.Forms.CheckBox();
			this.chkIntegerFpsMode = new System.Windows.Forms.CheckBox();
			this.flpRefreshRate = new System.Windows.Forms.FlowLayoutPanel();
			this.lblRequestedRefreshRate = new System.Windows.Forms.Label();
			this.cboRefreshRate = new System.Windows.Forms.ComboBox();
			this.tpgPicture = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel5 = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel7 = new System.Windows.Forms.TableLayoutPanel();
			this.btnSelectPreset = new System.Windows.Forms.Button();
			this.btnResetPictureSettings = new System.Windows.Forms.Button();
			this.grpNtscFilter = new System.Windows.Forms.GroupBox();
			this.tlpNtscFilter = new System.Windows.Forms.TableLayoutPanel();
			this.chkMergeFields = new System.Windows.Forms.CheckBox();
			this.trkArtifacts = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.trkBleed = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.trkFringing = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.trkGamma = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.trkResolution = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.trkSharpness = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.grpCommon = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel4 = new System.Windows.Forms.TableLayoutPanel();
			this.chkBilinearInterpolation = new System.Windows.Forms.CheckBox();
			this.trkBrightness = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.trkContrast = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.trkHue = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.trkSaturation = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.grpScanlines = new System.Windows.Forms.GroupBox();
			this.trkScanlines = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.tableLayoutPanel8 = new System.Windows.Forms.TableLayoutPanel();
			this.cboFilter = new System.Windows.Forms.ComboBox();
			this.lblVideoFilter = new System.Windows.Forms.Label();
			this.ctxPicturePresets = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuPresetComposite = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPresetSVideo = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPresetRgb = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPresetMonochrome = new System.Windows.Forms.ToolStripMenuItem();
			this.tabMain.SuspendLayout();
			this.tpgGeneral.SuspendLayout();
			this.tlpMain.SuspendLayout();
			this.flowLayoutPanel6.SuspendLayout();
			this.flpRefreshRate.SuspendLayout();
			this.tpgPicture.SuspendLayout();
			this.tableLayoutPanel5.SuspendLayout();
			this.tableLayoutPanel7.SuspendLayout();
			this.grpNtscFilter.SuspendLayout();
			this.tlpNtscFilter.SuspendLayout();
			this.grpCommon.SuspendLayout();
			this.tableLayoutPanel4.SuspendLayout();
			this.grpScanlines.SuspendLayout();
			this.tableLayoutPanel8.SuspendLayout();
			this.ctxPicturePresets.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 408);
			this.baseConfigPanel.Size = new System.Drawing.Size(574, 29);
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tpgGeneral);
			this.tabMain.Controls.Add(this.tpgPicture);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(574, 408);
			this.tabMain.TabIndex = 2;
			// 
			// tpgGeneral
			// 
			this.tpgGeneral.Controls.Add(this.tlpMain);
			this.tpgGeneral.Location = new System.Drawing.Point(4, 22);
			this.tpgGeneral.Name = "tpgGeneral";
			this.tpgGeneral.Padding = new System.Windows.Forms.Padding(3);
			this.tpgGeneral.Size = new System.Drawing.Size(566, 382);
			this.tpgGeneral.TabIndex = 5;
			this.tpgGeneral.Text = "General";
			this.tpgGeneral.UseVisualStyleBackColor = true;
			// 
			// tlpMain
			// 
			this.tlpMain.ColumnCount = 2;
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpMain.Controls.Add(this.chkUseExclusiveFullscreen, 0, 4);
			this.tlpMain.Controls.Add(this.lblVideoScale, 0, 0);
			this.tlpMain.Controls.Add(this.chkVerticalSync, 0, 3);
			this.tlpMain.Controls.Add(this.lblDisplayRatio, 0, 1);
			this.tlpMain.Controls.Add(this.nudScale, 1, 0);
			this.tlpMain.Controls.Add(this.flowLayoutPanel6, 1, 1);
			this.tlpMain.Controls.Add(this.chkFullscreenForceIntegerScale, 0, 6);
			this.tlpMain.Controls.Add(this.chkShowFps, 0, 7);
			this.tlpMain.Controls.Add(this.chkIntegerFpsMode, 0, 2);
			this.tlpMain.Controls.Add(this.flpRefreshRate, 0, 5);
			this.tlpMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpMain.Location = new System.Drawing.Point(3, 3);
			this.tlpMain.Margin = new System.Windows.Forms.Padding(0);
			this.tlpMain.Name = "tlpMain";
			this.tlpMain.RowCount = 9;
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpMain.Size = new System.Drawing.Size(560, 376);
			this.tlpMain.TabIndex = 1;
			// 
			// chkUseExclusiveFullscreen
			// 
			this.chkUseExclusiveFullscreen.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.chkUseExclusiveFullscreen.AutoSize = true;
			this.tlpMain.SetColumnSpan(this.chkUseExclusiveFullscreen, 2);
			this.chkUseExclusiveFullscreen.Location = new System.Drawing.Point(3, 96);
			this.chkUseExclusiveFullscreen.Name = "chkUseExclusiveFullscreen";
			this.chkUseExclusiveFullscreen.Size = new System.Drawing.Size(169, 17);
			this.chkUseExclusiveFullscreen.TabIndex = 24;
			this.chkUseExclusiveFullscreen.Text = "Use exclusive fullscreen mode";
			this.chkUseExclusiveFullscreen.UseVisualStyleBackColor = true;
			this.chkUseExclusiveFullscreen.Visible = false;
			// 
			// lblVideoScale
			// 
			this.lblVideoScale.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblVideoScale.AutoSize = true;
			this.lblVideoScale.Location = new System.Drawing.Point(3, 4);
			this.lblVideoScale.Name = "lblVideoScale";
			this.lblVideoScale.Size = new System.Drawing.Size(37, 13);
			this.lblVideoScale.TabIndex = 11;
			this.lblVideoScale.Text = "Scale:";
			// 
			// chkVerticalSync
			// 
			this.chkVerticalSync.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.chkVerticalSync.AutoSize = true;
			this.tlpMain.SetColumnSpan(this.chkVerticalSync, 2);
			this.chkVerticalSync.Location = new System.Drawing.Point(3, 73);
			this.chkVerticalSync.Name = "chkVerticalSync";
			this.chkVerticalSync.Size = new System.Drawing.Size(121, 17);
			this.chkVerticalSync.TabIndex = 15;
			this.chkVerticalSync.Text = "Enable vertical sync";
			this.chkVerticalSync.UseVisualStyleBackColor = true;
			// 
			// lblDisplayRatio
			// 
			this.lblDisplayRatio.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblDisplayRatio.AutoSize = true;
			this.lblDisplayRatio.Location = new System.Drawing.Point(3, 27);
			this.lblDisplayRatio.Name = "lblDisplayRatio";
			this.lblDisplayRatio.Size = new System.Drawing.Size(71, 13);
			this.lblDisplayRatio.TabIndex = 17;
			this.lblDisplayRatio.Text = "Aspect Ratio:";
			// 
			// nudScale
			// 
			this.nudScale.DecimalPlaces = 2;
			this.nudScale.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudScale.Location = new System.Drawing.Point(77, 0);
			this.nudScale.Margin = new System.Windows.Forms.Padding(0);
			this.nudScale.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
			this.nudScale.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudScale.Minimum = new decimal(new int[] {
            5,
            0,
            0,
            65536});
			this.nudScale.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudScale.Name = "nudScale";
			this.nudScale.Size = new System.Drawing.Size(48, 21);
			this.nudScale.TabIndex = 21;
			this.nudScale.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			// 
			// flowLayoutPanel6
			// 
			this.flowLayoutPanel6.Controls.Add(this.cboAspectRatio);
			this.flowLayoutPanel6.Controls.Add(this.lblCustomRatio);
			this.flowLayoutPanel6.Controls.Add(this.nudCustomRatio);
			this.flowLayoutPanel6.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowLayoutPanel6.Location = new System.Drawing.Point(77, 21);
			this.flowLayoutPanel6.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel6.Name = "flowLayoutPanel6";
			this.flowLayoutPanel6.Size = new System.Drawing.Size(483, 26);
			this.flowLayoutPanel6.TabIndex = 22;
			// 
			// cboAspectRatio
			// 
			this.cboAspectRatio.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboAspectRatio.FormattingEnabled = true;
			this.cboAspectRatio.Items.AddRange(new object[] {
            "Auto",
            "NTSC (8:7)",
            "PAL (18:13)",
            "Standard (4:3)",
            "Widescreen (16:9)"});
			this.cboAspectRatio.Location = new System.Drawing.Point(3, 3);
			this.cboAspectRatio.Name = "cboAspectRatio";
			this.cboAspectRatio.Size = new System.Drawing.Size(197, 21);
			this.cboAspectRatio.TabIndex = 16;
			// 
			// lblCustomRatio
			// 
			this.lblCustomRatio.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblCustomRatio.AutoSize = true;
			this.lblCustomRatio.Location = new System.Drawing.Point(206, 7);
			this.lblCustomRatio.Name = "lblCustomRatio";
			this.lblCustomRatio.Size = new System.Drawing.Size(76, 13);
			this.lblCustomRatio.TabIndex = 17;
			this.lblCustomRatio.Text = "Custom Ratio: ";
			this.lblCustomRatio.Visible = false;
			// 
			// nudCustomRatio
			// 
			this.nudCustomRatio.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.nudCustomRatio.DecimalPlaces = 3;
			this.nudCustomRatio.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
			this.nudCustomRatio.Location = new System.Drawing.Point(285, 3);
			this.nudCustomRatio.Margin = new System.Windows.Forms.Padding(0);
			this.nudCustomRatio.Maximum = new decimal(new int[] {
            5,
            0,
            0,
            0});
			this.nudCustomRatio.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudCustomRatio.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            65536});
			this.nudCustomRatio.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudCustomRatio.Name = "nudCustomRatio";
			this.nudCustomRatio.Size = new System.Drawing.Size(48, 21);
			this.nudCustomRatio.TabIndex = 22;
			this.nudCustomRatio.Value = new decimal(new int[] {
            1,
            0,
            0,
            65536});
			this.nudCustomRatio.Visible = false;
			// 
			// chkFullscreenForceIntegerScale
			// 
			this.chkFullscreenForceIntegerScale.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.chkFullscreenForceIntegerScale.AutoSize = true;
			this.tlpMain.SetColumnSpan(this.chkFullscreenForceIntegerScale, 2);
			this.chkFullscreenForceIntegerScale.Location = new System.Drawing.Point(3, 146);
			this.chkFullscreenForceIntegerScale.Name = "chkFullscreenForceIntegerScale";
			this.chkFullscreenForceIntegerScale.Size = new System.Drawing.Size(289, 17);
			this.chkFullscreenForceIntegerScale.TabIndex = 23;
			this.chkFullscreenForceIntegerScale.Text = "Use integer scale values when entering fullscreen mode";
			this.chkFullscreenForceIntegerScale.UseVisualStyleBackColor = true;
			this.chkFullscreenForceIntegerScale.Visible = false;
			// 
			// chkShowFps
			// 
			this.chkShowFps.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.chkShowFps.AutoSize = true;
			this.tlpMain.SetColumnSpan(this.chkShowFps, 2);
			this.chkShowFps.Location = new System.Drawing.Point(3, 169);
			this.chkShowFps.Name = "chkShowFps";
			this.chkShowFps.Size = new System.Drawing.Size(76, 17);
			this.chkShowFps.TabIndex = 9;
			this.chkShowFps.Text = "Show FPS";
			this.chkShowFps.UseVisualStyleBackColor = true;
			this.chkShowFps.Visible = false;
			// 
			// chkIntegerFpsMode
			// 
			this.chkIntegerFpsMode.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.chkIntegerFpsMode.AutoSize = true;
			this.tlpMain.SetColumnSpan(this.chkIntegerFpsMode, 2);
			this.chkIntegerFpsMode.Location = new System.Drawing.Point(3, 50);
			this.chkIntegerFpsMode.Name = "chkIntegerFpsMode";
			this.chkIntegerFpsMode.Size = new System.Drawing.Size(308, 17);
			this.chkIntegerFpsMode.TabIndex = 24;
			this.chkIntegerFpsMode.Text = "Enable integer FPS mode (e.g: run at 60 fps instead of 60.1)";
			this.chkIntegerFpsMode.UseVisualStyleBackColor = true;
			this.chkIntegerFpsMode.Visible = false;
			// 
			// flpRefreshRate
			// 
			this.tlpMain.SetColumnSpan(this.flpRefreshRate, 2);
			this.flpRefreshRate.Controls.Add(this.lblRequestedRefreshRate);
			this.flpRefreshRate.Controls.Add(this.cboRefreshRate);
			this.flpRefreshRate.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flpRefreshRate.Location = new System.Drawing.Point(30, 116);
			this.flpRefreshRate.Margin = new System.Windows.Forms.Padding(30, 0, 0, 0);
			this.flpRefreshRate.Name = "flpRefreshRate";
			this.flpRefreshRate.Size = new System.Drawing.Size(530, 27);
			this.flpRefreshRate.TabIndex = 26;
			this.flpRefreshRate.Visible = false;
			// 
			// lblRequestedRefreshRate
			// 
			this.lblRequestedRefreshRate.Anchor = System.Windows.Forms.AnchorStyles.Right;
			this.lblRequestedRefreshRate.AutoSize = true;
			this.lblRequestedRefreshRate.Location = new System.Drawing.Point(3, 7);
			this.lblRequestedRefreshRate.Name = "lblRequestedRefreshRate";
			this.lblRequestedRefreshRate.Size = new System.Drawing.Size(128, 13);
			this.lblRequestedRefreshRate.TabIndex = 17;
			this.lblRequestedRefreshRate.Text = "Requested Refresh Rate:";
			// 
			// cboRefreshRate
			// 
			this.cboRefreshRate.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboRefreshRate.FormattingEnabled = true;
			this.cboRefreshRate.Items.AddRange(new object[] {
            "Auto",
            "NTSC (8:7)",
            "PAL (18:13)",
            "Standard (4:3)",
            "Widescreen (16:9)"});
			this.cboRefreshRate.Location = new System.Drawing.Point(137, 3);
			this.cboRefreshRate.Name = "cboRefreshRate";
			this.cboRefreshRate.Size = new System.Drawing.Size(68, 21);
			this.cboRefreshRate.TabIndex = 25;
			// 
			// tpgPicture
			// 
			this.tpgPicture.Controls.Add(this.tableLayoutPanel5);
			this.tpgPicture.Location = new System.Drawing.Point(4, 22);
			this.tpgPicture.Name = "tpgPicture";
			this.tpgPicture.Padding = new System.Windows.Forms.Padding(3);
			this.tpgPicture.Size = new System.Drawing.Size(566, 382);
			this.tpgPicture.TabIndex = 4;
			this.tpgPicture.Text = "Picture";
			this.tpgPicture.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel5
			// 
			this.tableLayoutPanel5.ColumnCount = 2;
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel5.Controls.Add(this.tableLayoutPanel7, 0, 3);
			this.tableLayoutPanel5.Controls.Add(this.grpNtscFilter, 1, 1);
			this.tableLayoutPanel5.Controls.Add(this.grpCommon, 0, 1);
			this.tableLayoutPanel5.Controls.Add(this.grpScanlines, 0, 2);
			this.tableLayoutPanel5.Controls.Add(this.tableLayoutPanel8, 0, 0);
			this.tableLayoutPanel5.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel5.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel5.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel5.Name = "tableLayoutPanel5";
			this.tableLayoutPanel5.RowCount = 4;
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel5.Size = new System.Drawing.Size(560, 376);
			this.tableLayoutPanel5.TabIndex = 5;
			// 
			// tableLayoutPanel7
			// 
			this.tableLayoutPanel7.ColumnCount = 2;
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 36.92308F));
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 63.07692F));
			this.tableLayoutPanel7.Controls.Add(this.btnSelectPreset, 1, 0);
			this.tableLayoutPanel7.Controls.Add(this.btnResetPictureSettings, 0, 0);
			this.tableLayoutPanel7.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel7.Location = new System.Drawing.Point(0, 341);
			this.tableLayoutPanel7.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel7.Name = "tableLayoutPanel7";
			this.tableLayoutPanel7.RowCount = 1;
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel7.Size = new System.Drawing.Size(280, 35);
			this.tableLayoutPanel7.TabIndex = 3;
			// 
			// btnSelectPreset
			// 
			this.btnSelectPreset.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.btnSelectPreset.AutoSize = true;
			this.btnSelectPreset.Image = global::Mesen.GUI.Properties.Resources.DownArrow;
			this.btnSelectPreset.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
			this.btnSelectPreset.Location = new System.Drawing.Point(178, 9);
			this.btnSelectPreset.Name = "btnSelectPreset";
			this.btnSelectPreset.Padding = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.btnSelectPreset.Size = new System.Drawing.Size(99, 23);
			this.btnSelectPreset.TabIndex = 3;
			this.btnSelectPreset.Text = "Select Preset...";
			this.btnSelectPreset.TextImageRelation = System.Windows.Forms.TextImageRelation.TextBeforeImage;
			this.btnSelectPreset.UseVisualStyleBackColor = true;
			this.btnSelectPreset.Click += new System.EventHandler(this.btnSelectPreset_Click);
			// 
			// btnResetPictureSettings
			// 
			this.btnResetPictureSettings.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.btnResetPictureSettings.AutoSize = true;
			this.btnResetPictureSettings.Location = new System.Drawing.Point(3, 9);
			this.btnResetPictureSettings.Name = "btnResetPictureSettings";
			this.btnResetPictureSettings.Size = new System.Drawing.Size(45, 23);
			this.btnResetPictureSettings.TabIndex = 3;
			this.btnResetPictureSettings.Text = "Reset";
			this.btnResetPictureSettings.UseVisualStyleBackColor = true;
			this.btnResetPictureSettings.Click += new System.EventHandler(this.btnResetPictureSettings_Click);
			// 
			// grpNtscFilter
			// 
			this.grpNtscFilter.Controls.Add(this.tlpNtscFilter);
			this.grpNtscFilter.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpNtscFilter.Location = new System.Drawing.Point(282, 27);
			this.grpNtscFilter.Margin = new System.Windows.Forms.Padding(2, 0, 0, 0);
			this.grpNtscFilter.Name = "grpNtscFilter";
			this.tableLayoutPanel5.SetRowSpan(this.grpNtscFilter, 3);
			this.grpNtscFilter.Size = new System.Drawing.Size(278, 349);
			this.grpNtscFilter.TabIndex = 4;
			this.grpNtscFilter.TabStop = false;
			this.grpNtscFilter.Text = "NTSC Filter";
			// 
			// tlpNtscFilter
			// 
			this.tlpNtscFilter.ColumnCount = 1;
			this.tlpNtscFilter.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpNtscFilter.Controls.Add(this.chkMergeFields, 0, 6);
			this.tlpNtscFilter.Controls.Add(this.trkArtifacts, 0, 0);
			this.tlpNtscFilter.Controls.Add(this.trkBleed, 0, 1);
			this.tlpNtscFilter.Controls.Add(this.trkFringing, 0, 2);
			this.tlpNtscFilter.Controls.Add(this.trkGamma, 0, 3);
			this.tlpNtscFilter.Controls.Add(this.trkResolution, 0, 4);
			this.tlpNtscFilter.Controls.Add(this.trkSharpness, 0, 5);
			this.tlpNtscFilter.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpNtscFilter.Location = new System.Drawing.Point(3, 16);
			this.tlpNtscFilter.Margin = new System.Windows.Forms.Padding(0);
			this.tlpNtscFilter.Name = "tlpNtscFilter";
			this.tlpNtscFilter.RowCount = 7;
			this.tlpNtscFilter.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpNtscFilter.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpNtscFilter.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpNtscFilter.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpNtscFilter.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpNtscFilter.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpNtscFilter.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpNtscFilter.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpNtscFilter.Size = new System.Drawing.Size(272, 330);
			this.tlpNtscFilter.TabIndex = 5;
			// 
			// chkMergeFields
			// 
			this.chkMergeFields.AutoSize = true;
			this.chkMergeFields.Location = new System.Drawing.Point(3, 303);
			this.chkMergeFields.Name = "chkMergeFields";
			this.chkMergeFields.Size = new System.Drawing.Size(86, 17);
			this.chkMergeFields.TabIndex = 30;
			this.chkMergeFields.Text = "Merge Fields";
			this.chkMergeFields.UseVisualStyleBackColor = true;
			// 
			// trkArtifacts
			// 
			this.trkArtifacts.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkArtifacts.Location = new System.Drawing.Point(0, 0);
			this.trkArtifacts.Margin = new System.Windows.Forms.Padding(0);
			this.trkArtifacts.Maximum = 100;
			this.trkArtifacts.MaximumSize = new System.Drawing.Size(0, 60);
			this.trkArtifacts.Minimum = -100;
			this.trkArtifacts.MinimumSize = new System.Drawing.Size(206, 50);
			this.trkArtifacts.Name = "trkArtifacts";
			this.trkArtifacts.Size = new System.Drawing.Size(272, 50);
			this.trkArtifacts.TabIndex = 24;
			this.trkArtifacts.Text = "Artifacts";
			this.trkArtifacts.Value = 0;
			// 
			// trkBleed
			// 
			this.trkBleed.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBleed.Location = new System.Drawing.Point(0, 50);
			this.trkBleed.Margin = new System.Windows.Forms.Padding(0);
			this.trkBleed.Maximum = 100;
			this.trkBleed.MaximumSize = new System.Drawing.Size(400, 55);
			this.trkBleed.Minimum = -100;
			this.trkBleed.MinimumSize = new System.Drawing.Size(206, 50);
			this.trkBleed.Name = "trkBleed";
			this.trkBleed.Size = new System.Drawing.Size(272, 50);
			this.trkBleed.TabIndex = 25;
			this.trkBleed.Text = "Bleed";
			this.trkBleed.Value = 0;
			// 
			// trkFringing
			// 
			this.trkFringing.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkFringing.Location = new System.Drawing.Point(0, 100);
			this.trkFringing.Margin = new System.Windows.Forms.Padding(0);
			this.trkFringing.Maximum = 100;
			this.trkFringing.MaximumSize = new System.Drawing.Size(0, 41);
			this.trkFringing.Minimum = -100;
			this.trkFringing.MinimumSize = new System.Drawing.Size(206, 50);
			this.trkFringing.Name = "trkFringing";
			this.trkFringing.Size = new System.Drawing.Size(272, 50);
			this.trkFringing.TabIndex = 26;
			this.trkFringing.Text = "Fringing";
			this.trkFringing.Value = 0;
			// 
			// trkGamma
			// 
			this.trkGamma.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkGamma.Location = new System.Drawing.Point(0, 150);
			this.trkGamma.Margin = new System.Windows.Forms.Padding(0);
			this.trkGamma.Maximum = 100;
			this.trkGamma.MaximumSize = new System.Drawing.Size(0, 41);
			this.trkGamma.Minimum = -100;
			this.trkGamma.MinimumSize = new System.Drawing.Size(206, 50);
			this.trkGamma.Name = "trkGamma";
			this.trkGamma.Size = new System.Drawing.Size(272, 50);
			this.trkGamma.TabIndex = 27;
			this.trkGamma.Text = "Gamma";
			this.trkGamma.Value = 0;
			// 
			// trkResolution
			// 
			this.trkResolution.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkResolution.Location = new System.Drawing.Point(0, 200);
			this.trkResolution.Margin = new System.Windows.Forms.Padding(0);
			this.trkResolution.Maximum = 100;
			this.trkResolution.MaximumSize = new System.Drawing.Size(0, 41);
			this.trkResolution.Minimum = -100;
			this.trkResolution.MinimumSize = new System.Drawing.Size(206, 50);
			this.trkResolution.Name = "trkResolution";
			this.trkResolution.Size = new System.Drawing.Size(272, 50);
			this.trkResolution.TabIndex = 28;
			this.trkResolution.Text = "Resolution";
			this.trkResolution.Value = 0;
			// 
			// trkSharpness
			// 
			this.trkSharpness.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkSharpness.Location = new System.Drawing.Point(0, 250);
			this.trkSharpness.Margin = new System.Windows.Forms.Padding(0);
			this.trkSharpness.Maximum = 100;
			this.trkSharpness.MaximumSize = new System.Drawing.Size(0, 41);
			this.trkSharpness.Minimum = -100;
			this.trkSharpness.MinimumSize = new System.Drawing.Size(206, 50);
			this.trkSharpness.Name = "trkSharpness";
			this.trkSharpness.Size = new System.Drawing.Size(272, 50);
			this.trkSharpness.TabIndex = 29;
			this.trkSharpness.Text = "Sharpness";
			this.trkSharpness.Value = 0;
			// 
			// grpCommon
			// 
			this.grpCommon.Controls.Add(this.tableLayoutPanel4);
			this.grpCommon.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpCommon.Location = new System.Drawing.Point(0, 27);
			this.grpCommon.Margin = new System.Windows.Forms.Padding(0, 0, 2, 0);
			this.grpCommon.Name = "grpCommon";
			this.grpCommon.Padding = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.grpCommon.Size = new System.Drawing.Size(278, 242);
			this.grpCommon.TabIndex = 3;
			this.grpCommon.TabStop = false;
			this.grpCommon.Text = "Common Settings";
			// 
			// tableLayoutPanel4
			// 
			this.tableLayoutPanel4.ColumnCount = 1;
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel4.Controls.Add(this.chkBilinearInterpolation, 0, 4);
			this.tableLayoutPanel4.Controls.Add(this.trkBrightness, 0, 0);
			this.tableLayoutPanel4.Controls.Add(this.trkContrast, 0, 1);
			this.tableLayoutPanel4.Controls.Add(this.trkHue, 0, 2);
			this.tableLayoutPanel4.Controls.Add(this.trkSaturation, 0, 3);
			this.tableLayoutPanel4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel4.Location = new System.Drawing.Point(3, 15);
			this.tableLayoutPanel4.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel4.Name = "tableLayoutPanel4";
			this.tableLayoutPanel4.RowCount = 5;
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Size = new System.Drawing.Size(272, 225);
			this.tableLayoutPanel4.TabIndex = 4;
			// 
			// chkBilinearInterpolation
			// 
			this.chkBilinearInterpolation.AutoSize = true;
			this.tableLayoutPanel4.SetColumnSpan(this.chkBilinearInterpolation, 2);
			this.chkBilinearInterpolation.Location = new System.Drawing.Point(3, 203);
			this.chkBilinearInterpolation.Name = "chkBilinearInterpolation";
			this.chkBilinearInterpolation.Size = new System.Drawing.Size(206, 17);
			this.chkBilinearInterpolation.TabIndex = 28;
			this.chkBilinearInterpolation.Text = "Use bilinear interpolation when scaling";
			this.chkBilinearInterpolation.UseVisualStyleBackColor = true;
			// 
			// trkBrightness
			// 
			this.trkBrightness.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBrightness.Location = new System.Drawing.Point(0, 0);
			this.trkBrightness.Margin = new System.Windows.Forms.Padding(0);
			this.trkBrightness.Maximum = 100;
			this.trkBrightness.MaximumSize = new System.Drawing.Size(0, 60);
			this.trkBrightness.Minimum = -100;
			this.trkBrightness.MinimumSize = new System.Drawing.Size(206, 50);
			this.trkBrightness.Name = "trkBrightness";
			this.trkBrightness.Size = new System.Drawing.Size(272, 50);
			this.trkBrightness.TabIndex = 24;
			this.trkBrightness.Text = "Brightness";
			this.trkBrightness.Value = 0;
			// 
			// trkContrast
			// 
			this.trkContrast.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkContrast.Location = new System.Drawing.Point(0, 50);
			this.trkContrast.Margin = new System.Windows.Forms.Padding(0);
			this.trkContrast.Maximum = 100;
			this.trkContrast.MaximumSize = new System.Drawing.Size(400, 55);
			this.trkContrast.Minimum = -100;
			this.trkContrast.MinimumSize = new System.Drawing.Size(206, 50);
			this.trkContrast.Name = "trkContrast";
			this.trkContrast.Size = new System.Drawing.Size(272, 50);
			this.trkContrast.TabIndex = 25;
			this.trkContrast.Text = "Contrast";
			this.trkContrast.Value = 0;
			// 
			// trkHue
			// 
			this.trkHue.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkHue.Location = new System.Drawing.Point(0, 100);
			this.trkHue.Margin = new System.Windows.Forms.Padding(0);
			this.trkHue.Maximum = 100;
			this.trkHue.MaximumSize = new System.Drawing.Size(0, 41);
			this.trkHue.Minimum = -100;
			this.trkHue.MinimumSize = new System.Drawing.Size(206, 50);
			this.trkHue.Name = "trkHue";
			this.trkHue.Size = new System.Drawing.Size(272, 50);
			this.trkHue.TabIndex = 26;
			this.trkHue.Text = "Hue";
			this.trkHue.Value = 0;
			// 
			// trkSaturation
			// 
			this.trkSaturation.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkSaturation.Location = new System.Drawing.Point(0, 150);
			this.trkSaturation.Margin = new System.Windows.Forms.Padding(0);
			this.trkSaturation.Maximum = 100;
			this.trkSaturation.MaximumSize = new System.Drawing.Size(0, 41);
			this.trkSaturation.Minimum = -100;
			this.trkSaturation.MinimumSize = new System.Drawing.Size(206, 50);
			this.trkSaturation.Name = "trkSaturation";
			this.trkSaturation.Size = new System.Drawing.Size(272, 50);
			this.trkSaturation.TabIndex = 27;
			this.trkSaturation.Text = "Saturation";
			this.trkSaturation.Value = 0;
			// 
			// grpScanlines
			// 
			this.grpScanlines.Controls.Add(this.trkScanlines);
			this.grpScanlines.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpScanlines.Location = new System.Drawing.Point(0, 269);
			this.grpScanlines.Margin = new System.Windows.Forms.Padding(0, 0, 2, 0);
			this.grpScanlines.Name = "grpScanlines";
			this.grpScanlines.Size = new System.Drawing.Size(278, 72);
			this.grpScanlines.TabIndex = 5;
			this.grpScanlines.TabStop = false;
			this.grpScanlines.Text = "Scanlines";
			// 
			// trkScanlines
			// 
			this.trkScanlines.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkScanlines.Location = new System.Drawing.Point(3, 16);
			this.trkScanlines.Margin = new System.Windows.Forms.Padding(0);
			this.trkScanlines.Maximum = 100;
			this.trkScanlines.MaximumSize = new System.Drawing.Size(0, 41);
			this.trkScanlines.Minimum = 0;
			this.trkScanlines.MinimumSize = new System.Drawing.Size(206, 50);
			this.trkScanlines.Name = "trkScanlines";
			this.trkScanlines.Size = new System.Drawing.Size(272, 50);
			this.trkScanlines.TabIndex = 28;
			this.trkScanlines.Text = "Scanlines";
			this.trkScanlines.Value = 0;
			// 
			// tableLayoutPanel8
			// 
			this.tableLayoutPanel8.ColumnCount = 2;
			this.tableLayoutPanel5.SetColumnSpan(this.tableLayoutPanel8, 2);
			this.tableLayoutPanel8.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel8.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel8.Controls.Add(this.cboFilter, 1, 0);
			this.tableLayoutPanel8.Controls.Add(this.lblVideoFilter, 0, 0);
			this.tableLayoutPanel8.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel8.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel8.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel8.Name = "tableLayoutPanel8";
			this.tableLayoutPanel8.RowCount = 1;
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 27F));
			this.tableLayoutPanel8.Size = new System.Drawing.Size(560, 27);
			this.tableLayoutPanel8.TabIndex = 6;
			// 
			// cboFilter
			// 
			this.cboFilter.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboFilter.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboFilter.FormattingEnabled = true;
			this.cboFilter.Items.AddRange(new object[] {
            "None",
            "NTSC"});
			this.cboFilter.Location = new System.Drawing.Point(41, 3);
			this.cboFilter.Name = "cboFilter";
			this.cboFilter.Size = new System.Drawing.Size(516, 21);
			this.cboFilter.TabIndex = 15;
			// 
			// lblVideoFilter
			// 
			this.lblVideoFilter.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblVideoFilter.AutoSize = true;
			this.lblVideoFilter.Location = new System.Drawing.Point(3, 7);
			this.lblVideoFilter.Name = "lblVideoFilter";
			this.lblVideoFilter.Size = new System.Drawing.Size(32, 13);
			this.lblVideoFilter.TabIndex = 13;
			this.lblVideoFilter.Text = "Filter:";
			// 
			// ctxPicturePresets
			// 
			this.ctxPicturePresets.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuPresetComposite,
            this.mnuPresetSVideo,
            this.mnuPresetRgb,
            this.mnuPresetMonochrome});
			this.ctxPicturePresets.Name = "contextPicturePresets";
			this.ctxPicturePresets.Size = new System.Drawing.Size(148, 92);
			// 
			// mnuPresetComposite
			// 
			this.mnuPresetComposite.Name = "mnuPresetComposite";
			this.mnuPresetComposite.Size = new System.Drawing.Size(147, 22);
			this.mnuPresetComposite.Text = "Composite";
			this.mnuPresetComposite.Click += new System.EventHandler(this.mnuPresetComposite_Click);
			// 
			// mnuPresetSVideo
			// 
			this.mnuPresetSVideo.Name = "mnuPresetSVideo";
			this.mnuPresetSVideo.Size = new System.Drawing.Size(147, 22);
			this.mnuPresetSVideo.Text = "S-Video";
			this.mnuPresetSVideo.Click += new System.EventHandler(this.mnuPresetSVideo_Click);
			// 
			// mnuPresetRgb
			// 
			this.mnuPresetRgb.Name = "mnuPresetRgb";
			this.mnuPresetRgb.Size = new System.Drawing.Size(147, 22);
			this.mnuPresetRgb.Text = "RGB";
			this.mnuPresetRgb.Click += new System.EventHandler(this.mnuPresetRgb_Click);
			// 
			// mnuPresetMonochrome
			// 
			this.mnuPresetMonochrome.Name = "mnuPresetMonochrome";
			this.mnuPresetMonochrome.Size = new System.Drawing.Size(147, 22);
			this.mnuPresetMonochrome.Text = "Monochrome";
			this.mnuPresetMonochrome.Click += new System.EventHandler(this.mnuPresetMonochrome_Click);
			// 
			// frmVideoConfig
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(574, 437);
			this.Controls.Add(this.tabMain);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "frmVideoConfig";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "frmVideoConfig";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tabMain, 0);
			this.tabMain.ResumeLayout(false);
			this.tpgGeneral.ResumeLayout(false);
			this.tlpMain.ResumeLayout(false);
			this.tlpMain.PerformLayout();
			this.flowLayoutPanel6.ResumeLayout(false);
			this.flowLayoutPanel6.PerformLayout();
			this.flpRefreshRate.ResumeLayout(false);
			this.flpRefreshRate.PerformLayout();
			this.tpgPicture.ResumeLayout(false);
			this.tableLayoutPanel5.ResumeLayout(false);
			this.tableLayoutPanel7.ResumeLayout(false);
			this.tableLayoutPanel7.PerformLayout();
			this.grpNtscFilter.ResumeLayout(false);
			this.tlpNtscFilter.ResumeLayout(false);
			this.tlpNtscFilter.PerformLayout();
			this.grpCommon.ResumeLayout(false);
			this.tableLayoutPanel4.ResumeLayout(false);
			this.tableLayoutPanel4.PerformLayout();
			this.grpScanlines.ResumeLayout(false);
			this.tableLayoutPanel8.ResumeLayout(false);
			this.tableLayoutPanel8.PerformLayout();
			this.ctxPicturePresets.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tpgGeneral;
		private System.Windows.Forms.TableLayoutPanel tlpMain;
		private System.Windows.Forms.CheckBox chkUseExclusiveFullscreen;
		private System.Windows.Forms.Label lblVideoScale;
		private System.Windows.Forms.CheckBox chkVerticalSync;
		private System.Windows.Forms.Label lblDisplayRatio;
		private Controls.MesenNumericUpDown nudScale;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel6;
		private System.Windows.Forms.ComboBox cboAspectRatio;
		private System.Windows.Forms.Label lblCustomRatio;
		private Controls.MesenNumericUpDown nudCustomRatio;
		private System.Windows.Forms.CheckBox chkFullscreenForceIntegerScale;
		private System.Windows.Forms.CheckBox chkShowFps;
		private System.Windows.Forms.CheckBox chkIntegerFpsMode;
		private System.Windows.Forms.FlowLayoutPanel flpRefreshRate;
		private System.Windows.Forms.Label lblRequestedRefreshRate;
		private System.Windows.Forms.ComboBox cboRefreshRate;
		private System.Windows.Forms.TabPage tpgPicture;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel5;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel7;
		private System.Windows.Forms.Button btnSelectPreset;
		private System.Windows.Forms.Button btnResetPictureSettings;
		private System.Windows.Forms.GroupBox grpNtscFilter;
		private System.Windows.Forms.TableLayoutPanel tlpNtscFilter;
		private System.Windows.Forms.CheckBox chkMergeFields;
		private Controls.ctrlHorizontalTrackbar trkArtifacts;
		private Controls.ctrlHorizontalTrackbar trkBleed;
		private Controls.ctrlHorizontalTrackbar trkFringing;
		private Controls.ctrlHorizontalTrackbar trkGamma;
		private Controls.ctrlHorizontalTrackbar trkResolution;
		private Controls.ctrlHorizontalTrackbar trkSharpness;
		private System.Windows.Forms.GroupBox grpCommon;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel4;
		private System.Windows.Forms.CheckBox chkBilinearInterpolation;
		private Controls.ctrlHorizontalTrackbar trkBrightness;
		private Controls.ctrlHorizontalTrackbar trkContrast;
		private Controls.ctrlHorizontalTrackbar trkHue;
		private Controls.ctrlHorizontalTrackbar trkSaturation;
		private System.Windows.Forms.GroupBox grpScanlines;
		private Controls.ctrlHorizontalTrackbar trkScanlines;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel8;
		private System.Windows.Forms.ComboBox cboFilter;
		private System.Windows.Forms.Label lblVideoFilter;
		private System.Windows.Forms.ContextMenuStrip ctxPicturePresets;
		private System.Windows.Forms.ToolStripMenuItem mnuPresetComposite;
		private System.Windows.Forms.ToolStripMenuItem mnuPresetSVideo;
		private System.Windows.Forms.ToolStripMenuItem mnuPresetRgb;
		private System.Windows.Forms.ToolStripMenuItem mnuPresetMonochrome;
	}
}