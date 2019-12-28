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
			this.chkBlendHighResolutionModes = new System.Windows.Forms.CheckBox();
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
			this.tpgOverscan = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.picOverscan = new System.Windows.Forms.PictureBox();
			this.tableLayoutPanel11 = new System.Windows.Forms.TableLayoutPanel();
			this.nudOverscanTop = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblTop = new System.Windows.Forms.Label();
			this.tableLayoutPanel12 = new System.Windows.Forms.TableLayoutPanel();
			this.nudOverscanBottom = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblBottom = new System.Windows.Forms.Label();
			this.tableLayoutPanel13 = new System.Windows.Forms.TableLayoutPanel();
			this.nudOverscanRight = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblRight = new System.Windows.Forms.Label();
			this.tableLayoutPanel14 = new System.Windows.Forms.TableLayoutPanel();
			this.nudOverscanLeft = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblLeft = new System.Windows.Forms.Label();
			this.tpgAdvanced = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.chkDisableFrameSkipping = new Mesen.GUI.Controls.ctrlRiskyOption();
			this.chkHideBgLayer0 = new Mesen.GUI.Controls.ctrlRiskyOption();
			this.chkHideBgLayer1 = new Mesen.GUI.Controls.ctrlRiskyOption();
			this.chkHideBgLayer2 = new Mesen.GUI.Controls.ctrlRiskyOption();
			this.chkHideBgLayer3 = new Mesen.GUI.Controls.ctrlRiskyOption();
			this.chkHideSprites = new Mesen.GUI.Controls.ctrlRiskyOption();
			this.ctxPicturePresets = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuPresetComposite = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPresetSVideo = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPresetRgb = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPresetMonochrome = new System.Windows.Forms.ToolStripMenuItem();
			this.flpResolution = new System.Windows.Forms.FlowLayoutPanel();
			this.lblFullscreenResolution = new System.Windows.Forms.Label();
			this.cboFullscreenResolution = new System.Windows.Forms.ComboBox();
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
			this.tpgOverscan.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picOverscan)).BeginInit();
			this.tableLayoutPanel11.SuspendLayout();
			this.tableLayoutPanel12.SuspendLayout();
			this.tableLayoutPanel13.SuspendLayout();
			this.tableLayoutPanel14.SuspendLayout();
			this.tpgAdvanced.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.ctxPicturePresets.SuspendLayout();
			this.flpResolution.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 427);
			this.baseConfigPanel.Size = new System.Drawing.Size(574, 29);
			this.baseConfigPanel.TabIndex = 4;
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tpgGeneral);
			this.tabMain.Controls.Add(this.tpgPicture);
			this.tabMain.Controls.Add(this.tpgOverscan);
			this.tabMain.Controls.Add(this.tpgAdvanced);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(574, 427);
			this.tabMain.TabIndex = 2;
			// 
			// tpgGeneral
			// 
			this.tpgGeneral.Controls.Add(this.tlpMain);
			this.tpgGeneral.Location = new System.Drawing.Point(4, 22);
			this.tpgGeneral.Name = "tpgGeneral";
			this.tpgGeneral.Padding = new System.Windows.Forms.Padding(3);
			this.tpgGeneral.Size = new System.Drawing.Size(566, 401);
			this.tpgGeneral.TabIndex = 5;
			this.tpgGeneral.Text = "General";
			this.tpgGeneral.UseVisualStyleBackColor = true;
			// 
			// tlpMain
			// 
			this.tlpMain.ColumnCount = 2;
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpMain.Controls.Add(this.flpResolution, 0, 5);
			this.tlpMain.Controls.Add(this.chkUseExclusiveFullscreen, 0, 4);
			this.tlpMain.Controls.Add(this.lblVideoScale, 0, 0);
			this.tlpMain.Controls.Add(this.chkVerticalSync, 0, 3);
			this.tlpMain.Controls.Add(this.lblDisplayRatio, 0, 1);
			this.tlpMain.Controls.Add(this.nudScale, 1, 0);
			this.tlpMain.Controls.Add(this.flowLayoutPanel6, 1, 1);
			this.tlpMain.Controls.Add(this.chkFullscreenForceIntegerScale, 0, 7);
			this.tlpMain.Controls.Add(this.chkIntegerFpsMode, 0, 2);
			this.tlpMain.Controls.Add(this.flpRefreshRate, 0, 6);
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
			this.tlpMain.Size = new System.Drawing.Size(560, 395);
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
			this.chkUseExclusiveFullscreen.CheckedChanged += new System.EventHandler(this.chkUseExclusiveFullscreen_CheckedChanged);
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
			this.nudScale.IsHex = false;
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
			this.nudCustomRatio.IsHex = false;
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
			this.chkFullscreenForceIntegerScale.Location = new System.Drawing.Point(3, 173);
			this.chkFullscreenForceIntegerScale.Name = "chkFullscreenForceIntegerScale";
			this.chkFullscreenForceIntegerScale.Size = new System.Drawing.Size(289, 17);
			this.chkFullscreenForceIntegerScale.TabIndex = 23;
			this.chkFullscreenForceIntegerScale.Text = "Use integer scale values when entering fullscreen mode";
			this.chkFullscreenForceIntegerScale.UseVisualStyleBackColor = true;
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
			// 
			// flpRefreshRate
			// 
			this.tlpMain.SetColumnSpan(this.flpRefreshRate, 2);
			this.flpRefreshRate.Controls.Add(this.lblRequestedRefreshRate);
			this.flpRefreshRate.Controls.Add(this.cboRefreshRate);
			this.flpRefreshRate.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flpRefreshRate.Location = new System.Drawing.Point(30, 143);
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
            "50 Hz",
            "60 Hz",
            "100 Hz",
            "120 Hz",
            "200 Hz",
            "240 Hz"});
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
			this.tpgPicture.Size = new System.Drawing.Size(566, 401);
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
			this.tableLayoutPanel5.Size = new System.Drawing.Size(560, 395);
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
			this.tableLayoutPanel7.Location = new System.Drawing.Point(0, 365);
			this.tableLayoutPanel7.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel7.Name = "tableLayoutPanel7";
			this.tableLayoutPanel7.RowCount = 1;
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel7.Size = new System.Drawing.Size(280, 30);
			this.tableLayoutPanel7.TabIndex = 3;
			// 
			// btnSelectPreset
			// 
			this.btnSelectPreset.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.btnSelectPreset.AutoSize = true;
			this.btnSelectPreset.Image = global::Mesen.GUI.Properties.Resources.DownArrow;
			this.btnSelectPreset.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
			this.btnSelectPreset.Location = new System.Drawing.Point(178, 4);
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
			this.btnResetPictureSettings.Location = new System.Drawing.Point(3, 4);
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
			this.grpNtscFilter.Size = new System.Drawing.Size(278, 368);
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
			this.tlpNtscFilter.Size = new System.Drawing.Size(272, 349);
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
			this.grpCommon.Location = new System.Drawing.Point(0, 27);
			this.grpCommon.Margin = new System.Windows.Forms.Padding(0, 0, 2, 0);
			this.grpCommon.Name = "grpCommon";
			this.grpCommon.Padding = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.grpCommon.Size = new System.Drawing.Size(278, 266);
			this.grpCommon.TabIndex = 3;
			this.grpCommon.TabStop = false;
			this.grpCommon.Text = "Common Settings";
			// 
			// tableLayoutPanel4
			// 
			this.tableLayoutPanel4.ColumnCount = 1;
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Controls.Add(this.chkBlendHighResolutionModes, 0, 5);
			this.tableLayoutPanel4.Controls.Add(this.chkBilinearInterpolation, 0, 4);
			this.tableLayoutPanel4.Controls.Add(this.trkBrightness, 0, 0);
			this.tableLayoutPanel4.Controls.Add(this.trkContrast, 0, 1);
			this.tableLayoutPanel4.Controls.Add(this.trkHue, 0, 2);
			this.tableLayoutPanel4.Controls.Add(this.trkSaturation, 0, 3);
			this.tableLayoutPanel4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel4.Location = new System.Drawing.Point(3, 15);
			this.tableLayoutPanel4.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel4.Name = "tableLayoutPanel4";
			this.tableLayoutPanel4.RowCount = 7;
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Size = new System.Drawing.Size(272, 249);
			this.tableLayoutPanel4.TabIndex = 4;
			// 
			// chkBlendHighResolutionModes
			// 
			this.chkBlendHighResolutionModes.AutoSize = true;
			this.tableLayoutPanel4.SetColumnSpan(this.chkBlendHighResolutionModes, 2);
			this.chkBlendHighResolutionModes.Location = new System.Drawing.Point(3, 226);
			this.chkBlendHighResolutionModes.Name = "chkBlendHighResolutionModes";
			this.chkBlendHighResolutionModes.Size = new System.Drawing.Size(158, 17);
			this.chkBlendHighResolutionModes.TabIndex = 29;
			this.chkBlendHighResolutionModes.Text = "Blend high resolution modes";
			this.chkBlendHighResolutionModes.UseVisualStyleBackColor = true;
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
			this.grpScanlines.Location = new System.Drawing.Point(0, 293);
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
			// tpgOverscan
			// 
			this.tpgOverscan.Controls.Add(this.tableLayoutPanel1);
			this.tpgOverscan.Location = new System.Drawing.Point(4, 22);
			this.tpgOverscan.Name = "tpgOverscan";
			this.tpgOverscan.Padding = new System.Windows.Forms.Padding(3);
			this.tpgOverscan.Size = new System.Drawing.Size(566, 401);
			this.tpgOverscan.TabIndex = 6;
			this.tpgOverscan.Text = "Overscan";
			this.tpgOverscan.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 3;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 262F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.Controls.Add(this.picOverscan, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel11, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel12, 1, 2);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel13, 2, 1);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel14, 0, 1);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 3;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 246F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(560, 395);
			this.tableLayoutPanel1.TabIndex = 1;
			// 
			// picOverscan
			// 
			this.picOverscan.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picOverscan.Dock = System.Windows.Forms.DockStyle.Fill;
			this.picOverscan.Location = new System.Drawing.Point(152, 77);
			this.picOverscan.Name = "picOverscan";
			this.picOverscan.Size = new System.Drawing.Size(256, 240);
			this.picOverscan.TabIndex = 1;
			this.picOverscan.TabStop = false;
			// 
			// tableLayoutPanel11
			// 
			this.tableLayoutPanel11.ColumnCount = 1;
			this.tableLayoutPanel11.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel11.Controls.Add(this.nudOverscanTop, 0, 1);
			this.tableLayoutPanel11.Controls.Add(this.lblTop, 0, 0);
			this.tableLayoutPanel11.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel11.Location = new System.Drawing.Point(149, 0);
			this.tableLayoutPanel11.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel11.Name = "tableLayoutPanel11";
			this.tableLayoutPanel11.RowCount = 2;
			this.tableLayoutPanel11.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel11.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel11.Size = new System.Drawing.Size(262, 74);
			this.tableLayoutPanel11.TabIndex = 4;
			// 
			// nudOverscanTop
			// 
			this.nudOverscanTop.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.nudOverscanTop.DecimalPlaces = 0;
			this.nudOverscanTop.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudOverscanTop.IsHex = false;
			this.nudOverscanTop.Location = new System.Drawing.Point(110, 53);
			this.nudOverscanTop.Margin = new System.Windows.Forms.Padding(0);
			this.nudOverscanTop.Maximum = new decimal(new int[] {
            100,
            0,
            0,
            0});
			this.nudOverscanTop.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudOverscanTop.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudOverscanTop.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudOverscanTop.Name = "nudOverscanTop";
			this.nudOverscanTop.Size = new System.Drawing.Size(41, 21);
			this.nudOverscanTop.TabIndex = 2;
			this.nudOverscanTop.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudOverscanTop.ValueChanged += new System.EventHandler(this.nudOverscan_ValueChanged);
			// 
			// lblTop
			// 
			this.lblTop.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.lblTop.AutoSize = true;
			this.lblTop.Location = new System.Drawing.Point(118, 40);
			this.lblTop.Name = "lblTop";
			this.lblTop.Size = new System.Drawing.Size(26, 13);
			this.lblTop.TabIndex = 0;
			this.lblTop.Text = "Top";
			this.lblTop.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// tableLayoutPanel12
			// 
			this.tableLayoutPanel12.ColumnCount = 1;
			this.tableLayoutPanel12.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel12.Controls.Add(this.nudOverscanBottom, 0, 1);
			this.tableLayoutPanel12.Controls.Add(this.lblBottom, 0, 0);
			this.tableLayoutPanel12.Location = new System.Drawing.Point(149, 320);
			this.tableLayoutPanel12.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel12.Name = "tableLayoutPanel12";
			this.tableLayoutPanel12.RowCount = 2;
			this.tableLayoutPanel12.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel12.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel12.Size = new System.Drawing.Size(262, 58);
			this.tableLayoutPanel12.TabIndex = 5;
			// 
			// nudOverscanBottom
			// 
			this.nudOverscanBottom.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.nudOverscanBottom.DecimalPlaces = 0;
			this.nudOverscanBottom.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudOverscanBottom.IsHex = false;
			this.nudOverscanBottom.Location = new System.Drawing.Point(110, 13);
			this.nudOverscanBottom.Margin = new System.Windows.Forms.Padding(0);
			this.nudOverscanBottom.Maximum = new decimal(new int[] {
            100,
            0,
            0,
            0});
			this.nudOverscanBottom.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudOverscanBottom.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudOverscanBottom.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudOverscanBottom.Name = "nudOverscanBottom";
			this.nudOverscanBottom.Size = new System.Drawing.Size(41, 21);
			this.nudOverscanBottom.TabIndex = 2;
			this.nudOverscanBottom.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudOverscanBottom.ValueChanged += new System.EventHandler(this.nudOverscan_ValueChanged);
			// 
			// lblBottom
			// 
			this.lblBottom.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.lblBottom.AutoSize = true;
			this.lblBottom.Location = new System.Drawing.Point(111, 0);
			this.lblBottom.Name = "lblBottom";
			this.lblBottom.Size = new System.Drawing.Size(40, 13);
			this.lblBottom.TabIndex = 0;
			this.lblBottom.Text = "Bottom";
			this.lblBottom.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// tableLayoutPanel13
			// 
			this.tableLayoutPanel13.ColumnCount = 2;
			this.tableLayoutPanel13.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel13.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel13.Controls.Add(this.nudOverscanRight, 0, 2);
			this.tableLayoutPanel13.Controls.Add(this.lblRight, 0, 1);
			this.tableLayoutPanel13.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel13.Location = new System.Drawing.Point(411, 74);
			this.tableLayoutPanel13.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel13.Name = "tableLayoutPanel13";
			this.tableLayoutPanel13.RowCount = 4;
			this.tableLayoutPanel13.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel13.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel13.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel13.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel13.Size = new System.Drawing.Size(149, 246);
			this.tableLayoutPanel13.TabIndex = 6;
			// 
			// nudOverscanRight
			// 
			this.nudOverscanRight.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.nudOverscanRight.DecimalPlaces = 0;
			this.nudOverscanRight.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudOverscanRight.IsHex = false;
			this.nudOverscanRight.Location = new System.Drawing.Point(0, 119);
			this.nudOverscanRight.Margin = new System.Windows.Forms.Padding(0);
			this.nudOverscanRight.Maximum = new decimal(new int[] {
            100,
            0,
            0,
            0});
			this.nudOverscanRight.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudOverscanRight.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudOverscanRight.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudOverscanRight.Name = "nudOverscanRight";
			this.nudOverscanRight.Size = new System.Drawing.Size(41, 21);
			this.nudOverscanRight.TabIndex = 1;
			this.nudOverscanRight.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudOverscanRight.ValueChanged += new System.EventHandler(this.nudOverscan_ValueChanged);
			// 
			// lblRight
			// 
			this.lblRight.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.lblRight.AutoSize = true;
			this.lblRight.Location = new System.Drawing.Point(4, 106);
			this.lblRight.Name = "lblRight";
			this.lblRight.Size = new System.Drawing.Size(32, 13);
			this.lblRight.TabIndex = 0;
			this.lblRight.Text = "Right";
			this.lblRight.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// tableLayoutPanel14
			// 
			this.tableLayoutPanel14.ColumnCount = 2;
			this.tableLayoutPanel14.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel14.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel14.Controls.Add(this.nudOverscanLeft, 1, 2);
			this.tableLayoutPanel14.Controls.Add(this.lblLeft, 1, 1);
			this.tableLayoutPanel14.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel14.Location = new System.Drawing.Point(0, 74);
			this.tableLayoutPanel14.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel14.Name = "tableLayoutPanel14";
			this.tableLayoutPanel14.RowCount = 4;
			this.tableLayoutPanel14.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel14.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel14.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel14.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel14.Size = new System.Drawing.Size(149, 246);
			this.tableLayoutPanel14.TabIndex = 7;
			// 
			// nudOverscanLeft
			// 
			this.nudOverscanLeft.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.nudOverscanLeft.DecimalPlaces = 0;
			this.nudOverscanLeft.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudOverscanLeft.IsHex = false;
			this.nudOverscanLeft.Location = new System.Drawing.Point(108, 119);
			this.nudOverscanLeft.Margin = new System.Windows.Forms.Padding(0);
			this.nudOverscanLeft.Maximum = new decimal(new int[] {
            100,
            0,
            0,
            0});
			this.nudOverscanLeft.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudOverscanLeft.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudOverscanLeft.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudOverscanLeft.Name = "nudOverscanLeft";
			this.nudOverscanLeft.Size = new System.Drawing.Size(41, 21);
			this.nudOverscanLeft.TabIndex = 2;
			this.nudOverscanLeft.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudOverscanLeft.ValueChanged += new System.EventHandler(this.nudOverscan_ValueChanged);
			// 
			// lblLeft
			// 
			this.lblLeft.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.lblLeft.AutoSize = true;
			this.lblLeft.Location = new System.Drawing.Point(116, 106);
			this.lblLeft.Name = "lblLeft";
			this.lblLeft.Size = new System.Drawing.Size(25, 13);
			this.lblLeft.TabIndex = 0;
			this.lblLeft.Text = "Left";
			this.lblLeft.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// tpgAdvanced
			// 
			this.tpgAdvanced.Controls.Add(this.tableLayoutPanel2);
			this.tpgAdvanced.Location = new System.Drawing.Point(4, 22);
			this.tpgAdvanced.Name = "tpgAdvanced";
			this.tpgAdvanced.Padding = new System.Windows.Forms.Padding(3);
			this.tpgAdvanced.Size = new System.Drawing.Size(566, 401);
			this.tpgAdvanced.TabIndex = 7;
			this.tpgAdvanced.Text = "Advanced";
			this.tpgAdvanced.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 1;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Controls.Add(this.chkDisableFrameSkipping, 0, 5);
			this.tableLayoutPanel2.Controls.Add(this.chkHideBgLayer0, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkHideBgLayer1, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkHideBgLayer2, 0, 2);
			this.tableLayoutPanel2.Controls.Add(this.chkHideBgLayer3, 0, 3);
			this.tableLayoutPanel2.Controls.Add(this.chkHideSprites, 0, 4);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 7;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(560, 395);
			this.tableLayoutPanel2.TabIndex = 0;
			// 
			// chkDisableFrameSkipping
			// 
			this.chkDisableFrameSkipping.Checked = false;
			this.chkDisableFrameSkipping.Dock = System.Windows.Forms.DockStyle.Top;
			this.chkDisableFrameSkipping.Location = new System.Drawing.Point(0, 120);
			this.chkDisableFrameSkipping.Name = "chkDisableFrameSkipping";
			this.chkDisableFrameSkipping.Size = new System.Drawing.Size(560, 24);
			this.chkDisableFrameSkipping.TabIndex = 5;
			this.chkDisableFrameSkipping.Text = "Disable frame skipping when fast-forwarding";
			// 
			// chkHideBgLayer0
			// 
			this.chkHideBgLayer0.Checked = false;
			this.chkHideBgLayer0.Dock = System.Windows.Forms.DockStyle.Top;
			this.chkHideBgLayer0.Location = new System.Drawing.Point(0, 0);
			this.chkHideBgLayer0.Name = "chkHideBgLayer0";
			this.chkHideBgLayer0.Size = new System.Drawing.Size(560, 24);
			this.chkHideBgLayer0.TabIndex = 0;
			this.chkHideBgLayer0.Text = "Hide background layer 0";
			// 
			// chkHideBgLayer1
			// 
			this.chkHideBgLayer1.Checked = false;
			this.chkHideBgLayer1.Dock = System.Windows.Forms.DockStyle.Top;
			this.chkHideBgLayer1.Location = new System.Drawing.Point(0, 24);
			this.chkHideBgLayer1.Name = "chkHideBgLayer1";
			this.chkHideBgLayer1.Size = new System.Drawing.Size(560, 24);
			this.chkHideBgLayer1.TabIndex = 1;
			this.chkHideBgLayer1.Text = "Hide background layer 1";
			// 
			// chkHideBgLayer2
			// 
			this.chkHideBgLayer2.Checked = false;
			this.chkHideBgLayer2.Dock = System.Windows.Forms.DockStyle.Top;
			this.chkHideBgLayer2.Location = new System.Drawing.Point(0, 48);
			this.chkHideBgLayer2.Name = "chkHideBgLayer2";
			this.chkHideBgLayer2.Size = new System.Drawing.Size(560, 24);
			this.chkHideBgLayer2.TabIndex = 2;
			this.chkHideBgLayer2.Text = "Hide background layer 2";
			// 
			// chkHideBgLayer3
			// 
			this.chkHideBgLayer3.Checked = false;
			this.chkHideBgLayer3.Dock = System.Windows.Forms.DockStyle.Top;
			this.chkHideBgLayer3.Location = new System.Drawing.Point(0, 72);
			this.chkHideBgLayer3.Name = "chkHideBgLayer3";
			this.chkHideBgLayer3.Size = new System.Drawing.Size(560, 24);
			this.chkHideBgLayer3.TabIndex = 3;
			this.chkHideBgLayer3.Text = "Hide background layer 3";
			// 
			// chkHideSprites
			// 
			this.chkHideSprites.Checked = false;
			this.chkHideSprites.Dock = System.Windows.Forms.DockStyle.Top;
			this.chkHideSprites.Location = new System.Drawing.Point(0, 96);
			this.chkHideSprites.Name = "chkHideSprites";
			this.chkHideSprites.Size = new System.Drawing.Size(560, 24);
			this.chkHideSprites.TabIndex = 4;
			this.chkHideSprites.Text = "Hide sprites";
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
			// flpResolution
			// 
			this.tlpMain.SetColumnSpan(this.flpResolution, 2);
			this.flpResolution.Controls.Add(this.lblFullscreenResolution);
			this.flpResolution.Controls.Add(this.cboFullscreenResolution);
			this.flpResolution.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flpResolution.Location = new System.Drawing.Point(30, 116);
			this.flpResolution.Margin = new System.Windows.Forms.Padding(30, 0, 0, 0);
			this.flpResolution.Name = "flpResolution";
			this.flpResolution.Size = new System.Drawing.Size(530, 27);
			this.flpResolution.TabIndex = 28;
			this.flpResolution.Visible = false;
			// 
			// lblFullscreenResolution
			// 
			this.lblFullscreenResolution.Anchor = System.Windows.Forms.AnchorStyles.Right;
			this.lblFullscreenResolution.AutoSize = true;
			this.lblFullscreenResolution.Location = new System.Drawing.Point(3, 7);
			this.lblFullscreenResolution.Name = "lblFullscreenResolution";
			this.lblFullscreenResolution.Size = new System.Drawing.Size(111, 13);
			this.lblFullscreenResolution.TabIndex = 17;
			this.lblFullscreenResolution.Text = "Fullscreen Resolution:";
			// 
			// cboFullscreenResolution
			// 
			this.cboFullscreenResolution.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboFullscreenResolution.FormattingEnabled = true;
			this.cboFullscreenResolution.Items.AddRange(new object[] {
            "3840x2160",
            "2560x1440",
            "2160x1200",
            "1920x1440",
            "1920x1200",
            "1920x1080",
            "1680x1050",
            "1600x1200",
            "1600x1024",
            "1600x900",
            "1366x768",
            "1360x768",
            "1280x1024",
            "1280x960",
            "1280x800",
            "1280x768",
            "1280x720",
            "1152x864",
            "1024x768",
            "800x600",
            "640x480"});
			this.cboFullscreenResolution.Location = new System.Drawing.Point(120, 3);
			this.cboFullscreenResolution.Name = "cboFullscreenResolution";
			this.cboFullscreenResolution.Size = new System.Drawing.Size(85, 21);
			this.cboFullscreenResolution.TabIndex = 25;
			// 
			// frmVideoConfig
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(574, 456);
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
			this.tpgOverscan.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.picOverscan)).EndInit();
			this.tableLayoutPanel11.ResumeLayout(false);
			this.tableLayoutPanel11.PerformLayout();
			this.tableLayoutPanel12.ResumeLayout(false);
			this.tableLayoutPanel12.PerformLayout();
			this.tableLayoutPanel13.ResumeLayout(false);
			this.tableLayoutPanel13.PerformLayout();
			this.tableLayoutPanel14.ResumeLayout(false);
			this.tableLayoutPanel14.PerformLayout();
			this.tpgAdvanced.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.ctxPicturePresets.ResumeLayout(false);
			this.flpResolution.ResumeLayout(false);
			this.flpResolution.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

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
		private System.Windows.Forms.TabPage tpgOverscan;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.PictureBox picOverscan;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel11;
		private Controls.MesenNumericUpDown nudOverscanTop;
		private System.Windows.Forms.Label lblTop;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel12;
		private Controls.MesenNumericUpDown nudOverscanBottom;
		private System.Windows.Forms.Label lblBottom;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel13;
		private Controls.MesenNumericUpDown nudOverscanRight;
		private System.Windows.Forms.Label lblRight;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel14;
		private Controls.MesenNumericUpDown nudOverscanLeft;
		private System.Windows.Forms.Label lblLeft;
		private System.Windows.Forms.TabPage tpgAdvanced;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private Controls.ctrlRiskyOption chkHideBgLayer0;
		private Controls.ctrlRiskyOption chkHideBgLayer1;
		private Controls.ctrlRiskyOption chkHideBgLayer2;
		private Controls.ctrlRiskyOption chkHideBgLayer3;
		private Controls.ctrlRiskyOption chkHideSprites;
		private Controls.ctrlRiskyOption chkDisableFrameSkipping;
		private System.Windows.Forms.CheckBox chkBlendHighResolutionModes;
	  private System.Windows.Forms.FlowLayoutPanel flpResolution;
	  private System.Windows.Forms.Label lblFullscreenResolution;
	  private System.Windows.Forms.ComboBox cboFullscreenResolution;
   }
}