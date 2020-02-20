namespace Mesen.GUI.Forms.Config
{
	partial class frmPreferences
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
			this.tabMain = new System.Windows.Forms.TabControl();
			this.tpgGeneral = new System.Windows.Forms.TabPage();
			this.tlpMain = new System.Windows.Forms.TableLayoutPanel();
			this.chkPauseOnMovieEnd = new System.Windows.Forms.CheckBox();
			this.chkAllowBackgroundInput = new System.Windows.Forms.CheckBox();
			this.flowLayoutPanel8 = new System.Windows.Forms.FlowLayoutPanel();
			this.lblPauseIn = new System.Windows.Forms.Label();
			this.chkPauseWhenInBackground = new System.Windows.Forms.CheckBox();
			this.chkPauseInMenuAndConfig = new System.Windows.Forms.CheckBox();
			this.chkPauseInDebugger = new System.Windows.Forms.CheckBox();
			this.lblPauseBackgroundSettings = new System.Windows.Forms.Label();
			this.chkAutoHideMenu = new System.Windows.Forms.CheckBox();
			this.chkSingleInstance = new System.Windows.Forms.CheckBox();
			this.chkAutomaticallyCheckForUpdates = new System.Windows.Forms.CheckBox();
			this.flowLayoutPanel2 = new System.Windows.Forms.FlowLayoutPanel();
			this.lblDisplayLanguage = new System.Windows.Forms.Label();
			this.cboDisplayLanguage = new System.Windows.Forms.ComboBox();
			this.lblMiscSettings = new System.Windows.Forms.Label();
			this.chkAutoLoadPatches = new System.Windows.Forms.CheckBox();
			this.tableLayoutPanel5 = new System.Windows.Forms.TableLayoutPanel();
			this.btnOpenMesenFolder = new System.Windows.Forms.Button();
			this.btnResetSettings = new System.Windows.Forms.Button();
			this.tpgShortcuts = new System.Windows.Forms.TabPage();
			this.ctrlEmulatorShortcuts = new Mesen.GUI.Forms.Config.ctrlEmulatorShortcuts();
			this.tpgFiles = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel6 = new System.Windows.Forms.TableLayoutPanel();
			this.grpPathOverrides = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel10 = new System.Windows.Forms.TableLayoutPanel();
			this.psGame = new Mesen.GUI.Forms.Config.ctrlPathSelection();
			this.chkGameOverride = new System.Windows.Forms.CheckBox();
			this.psWave = new Mesen.GUI.Forms.Config.ctrlPathSelection();
			this.psMovies = new Mesen.GUI.Forms.Config.ctrlPathSelection();
			this.psSaveData = new Mesen.GUI.Forms.Config.ctrlPathSelection();
			this.psSaveStates = new Mesen.GUI.Forms.Config.ctrlPathSelection();
			this.psScreenshots = new Mesen.GUI.Forms.Config.ctrlPathSelection();
			this.psAvi = new Mesen.GUI.Forms.Config.ctrlPathSelection();
			this.chkAviOverride = new System.Windows.Forms.CheckBox();
			this.chkScreenshotsOverride = new System.Windows.Forms.CheckBox();
			this.chkSaveDataOverride = new System.Windows.Forms.CheckBox();
			this.chkWaveOverride = new System.Windows.Forms.CheckBox();
			this.chkSaveStatesOverride = new System.Windows.Forms.CheckBox();
			this.chkMoviesOverride = new System.Windows.Forms.CheckBox();
			this.grpFileAssociations = new System.Windows.Forms.GroupBox();
			this.tlpFileFormat = new System.Windows.Forms.TableLayoutPanel();
			this.chkBsFormat = new System.Windows.Forms.CheckBox();
			this.chkSpcFormat = new System.Windows.Forms.CheckBox();
			this.chkRomFormat = new System.Windows.Forms.CheckBox();
			this.chkMssFormat = new System.Windows.Forms.CheckBox();
			this.chkMsmFormat = new System.Windows.Forms.CheckBox();
			this.grpDataStorageLocation = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel7 = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel8 = new System.Windows.Forms.TableLayoutPanel();
			this.radStorageDocuments = new System.Windows.Forms.RadioButton();
			this.radStoragePortable = new System.Windows.Forms.RadioButton();
			this.tableLayoutPanel9 = new System.Windows.Forms.TableLayoutPanel();
			this.lblLocation = new System.Windows.Forms.Label();
			this.lblDataLocation = new System.Windows.Forms.Label();
			this.tpgAdvanced = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.chkDisableGameSelectionScreen = new System.Windows.Forms.CheckBox();
			this.lblAdvancedMisc = new System.Windows.Forms.Label();
			this.flowLayoutPanel6 = new System.Windows.Forms.FlowLayoutPanel();
			this.lblRewind = new System.Windows.Forms.Label();
			this.nudRewindBufferSize = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblRewindMinutes = new System.Windows.Forms.Label();
			this.chkDisplayTitleBarInfo = new System.Windows.Forms.CheckBox();
			this.chkShowGameTimer = new System.Windows.Forms.CheckBox();
			this.chkShowFrameCounter = new System.Windows.Forms.CheckBox();
			this.chkDisableOsd = new System.Windows.Forms.CheckBox();
			this.lblUiDisplaySettings = new System.Windows.Forms.Label();
			this.lblWindowSettings = new System.Windows.Forms.Label();
			this.chkAlwaysOnTop = new System.Windows.Forms.CheckBox();
			this.chkShowFps = new System.Windows.Forms.CheckBox();
			this.chkShowDebugInfo = new System.Windows.Forms.CheckBox();
			this.tabMain.SuspendLayout();
			this.tpgGeneral.SuspendLayout();
			this.tlpMain.SuspendLayout();
			this.flowLayoutPanel8.SuspendLayout();
			this.flowLayoutPanel2.SuspendLayout();
			this.tableLayoutPanel5.SuspendLayout();
			this.tpgShortcuts.SuspendLayout();
			this.tpgFiles.SuspendLayout();
			this.tableLayoutPanel6.SuspendLayout();
			this.grpPathOverrides.SuspendLayout();
			this.tableLayoutPanel10.SuspendLayout();
			this.grpFileAssociations.SuspendLayout();
			this.tlpFileFormat.SuspendLayout();
			this.grpDataStorageLocation.SuspendLayout();
			this.tableLayoutPanel7.SuspendLayout();
			this.tableLayoutPanel8.SuspendLayout();
			this.tableLayoutPanel9.SuspendLayout();
			this.tpgAdvanced.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.flowLayoutPanel6.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 415);
			this.baseConfigPanel.Size = new System.Drawing.Size(548, 29);
			this.baseConfigPanel.TabIndex = 4;
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tpgGeneral);
			this.tabMain.Controls.Add(this.tpgShortcuts);
			this.tabMain.Controls.Add(this.tpgFiles);
			this.tabMain.Controls.Add(this.tpgAdvanced);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(548, 415);
			this.tabMain.TabIndex = 3;
			// 
			// tpgGeneral
			// 
			this.tpgGeneral.Controls.Add(this.tlpMain);
			this.tpgGeneral.Location = new System.Drawing.Point(4, 22);
			this.tpgGeneral.Name = "tpgGeneral";
			this.tpgGeneral.Padding = new System.Windows.Forms.Padding(3);
			this.tpgGeneral.Size = new System.Drawing.Size(540, 389);
			this.tpgGeneral.TabIndex = 0;
			this.tpgGeneral.Text = "General";
			this.tpgGeneral.UseVisualStyleBackColor = true;
			// 
			// tlpMain
			// 
			this.tlpMain.ColumnCount = 1;
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpMain.Controls.Add(this.chkPauseOnMovieEnd, 0, 6);
			this.tlpMain.Controls.Add(this.chkAllowBackgroundInput, 0, 5);
			this.tlpMain.Controls.Add(this.flowLayoutPanel8, 0, 4);
			this.tlpMain.Controls.Add(this.lblPauseBackgroundSettings, 0, 3);
			this.tlpMain.Controls.Add(this.chkAutoHideMenu, 0, 9);
			this.tlpMain.Controls.Add(this.chkSingleInstance, 0, 2);
			this.tlpMain.Controls.Add(this.chkAutomaticallyCheckForUpdates, 0, 1);
			this.tlpMain.Controls.Add(this.flowLayoutPanel2, 0, 0);
			this.tlpMain.Controls.Add(this.lblMiscSettings, 0, 7);
			this.tlpMain.Controls.Add(this.chkAutoLoadPatches, 0, 8);
			this.tlpMain.Controls.Add(this.tableLayoutPanel5, 0, 11);
			this.tlpMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpMain.Location = new System.Drawing.Point(3, 3);
			this.tlpMain.Name = "tlpMain";
			this.tlpMain.RowCount = 12;
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.Size = new System.Drawing.Size(534, 383);
			this.tlpMain.TabIndex = 1;
			// 
			// chkPauseOnMovieEnd
			// 
			this.chkPauseOnMovieEnd.AutoSize = true;
			this.chkPauseOnMovieEnd.Location = new System.Drawing.Point(13, 143);
			this.chkPauseOnMovieEnd.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkPauseOnMovieEnd.Name = "chkPauseOnMovieEnd";
			this.chkPauseOnMovieEnd.Size = new System.Drawing.Size(199, 17);
			this.chkPauseOnMovieEnd.TabIndex = 31;
			this.chkPauseOnMovieEnd.Text = "Pause when a movie finishes playing";
			this.chkPauseOnMovieEnd.UseVisualStyleBackColor = true;
			// 
			// chkAllowBackgroundInput
			// 
			this.chkAllowBackgroundInput.AutoSize = true;
			this.chkAllowBackgroundInput.Location = new System.Drawing.Point(13, 120);
			this.chkAllowBackgroundInput.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkAllowBackgroundInput.Name = "chkAllowBackgroundInput";
			this.chkAllowBackgroundInput.Size = new System.Drawing.Size(177, 17);
			this.chkAllowBackgroundInput.TabIndex = 30;
			this.chkAllowBackgroundInput.Text = "Allow input when in background";
			this.chkAllowBackgroundInput.UseVisualStyleBackColor = true;
			// 
			// flowLayoutPanel8
			// 
			this.flowLayoutPanel8.Controls.Add(this.lblPauseIn);
			this.flowLayoutPanel8.Controls.Add(this.chkPauseWhenInBackground);
			this.flowLayoutPanel8.Controls.Add(this.chkPauseInMenuAndConfig);
			this.flowLayoutPanel8.Controls.Add(this.chkPauseInDebugger);
			this.flowLayoutPanel8.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowLayoutPanel8.Location = new System.Drawing.Point(7, 95);
			this.flowLayoutPanel8.Margin = new System.Windows.Forms.Padding(7, 3, 0, 0);
			this.flowLayoutPanel8.Name = "flowLayoutPanel8";
			this.flowLayoutPanel8.Size = new System.Drawing.Size(527, 22);
			this.flowLayoutPanel8.TabIndex = 29;
			// 
			// lblPauseIn
			// 
			this.lblPauseIn.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPauseIn.AutoSize = true;
			this.lblPauseIn.Location = new System.Drawing.Point(3, 4);
			this.lblPauseIn.Margin = new System.Windows.Forms.Padding(3, 0, 3, 1);
			this.lblPauseIn.Name = "lblPauseIn";
			this.lblPauseIn.Size = new System.Drawing.Size(80, 13);
			this.lblPauseIn.TabIndex = 0;
			this.lblPauseIn.Text = "Pause when in:";
			// 
			// chkPauseWhenInBackground
			// 
			this.chkPauseWhenInBackground.AutoSize = true;
			this.chkPauseWhenInBackground.Location = new System.Drawing.Point(89, 3);
			this.chkPauseWhenInBackground.Name = "chkPauseWhenInBackground";
			this.chkPauseWhenInBackground.Size = new System.Drawing.Size(84, 17);
			this.chkPauseWhenInBackground.TabIndex = 13;
			this.chkPauseWhenInBackground.Text = "Background";
			this.chkPauseWhenInBackground.UseVisualStyleBackColor = true;
			// 
			// chkPauseInMenuAndConfig
			// 
			this.chkPauseInMenuAndConfig.AutoSize = true;
			this.chkPauseInMenuAndConfig.Location = new System.Drawing.Point(179, 3);
			this.chkPauseInMenuAndConfig.Name = "chkPauseInMenuAndConfig";
			this.chkPauseInMenuAndConfig.Size = new System.Drawing.Size(142, 17);
			this.chkPauseInMenuAndConfig.TabIndex = 16;
			this.chkPauseInMenuAndConfig.Text = "Menu and config dialogs";
			this.chkPauseInMenuAndConfig.UseVisualStyleBackColor = true;
			// 
			// chkPauseInDebugger
			// 
			this.chkPauseInDebugger.AutoSize = true;
			this.chkPauseInDebugger.Location = new System.Drawing.Point(327, 3);
			this.chkPauseInDebugger.Name = "chkPauseInDebugger";
			this.chkPauseInDebugger.Size = new System.Drawing.Size(103, 17);
			this.chkPauseInDebugger.TabIndex = 18;
			this.chkPauseInDebugger.Text = "Debugging tools";
			this.chkPauseInDebugger.UseVisualStyleBackColor = true;
			// 
			// lblPauseBackgroundSettings
			// 
			this.lblPauseBackgroundSettings.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblPauseBackgroundSettings.AutoSize = true;
			this.lblPauseBackgroundSettings.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblPauseBackgroundSettings.Location = new System.Drawing.Point(0, 79);
			this.lblPauseBackgroundSettings.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblPauseBackgroundSettings.Name = "lblPauseBackgroundSettings";
			this.lblPauseBackgroundSettings.Size = new System.Drawing.Size(141, 13);
			this.lblPauseBackgroundSettings.TabIndex = 26;
			this.lblPauseBackgroundSettings.Text = "Pause/Background Settings";
			// 
			// chkAutoHideMenu
			// 
			this.chkAutoHideMenu.AutoSize = true;
			this.chkAutoHideMenu.Location = new System.Drawing.Point(13, 209);
			this.chkAutoHideMenu.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkAutoHideMenu.Name = "chkAutoHideMenu";
			this.chkAutoHideMenu.Size = new System.Drawing.Size(158, 17);
			this.chkAutoHideMenu.TabIndex = 25;
			this.chkAutoHideMenu.Text = "Automatically hide menu bar";
			this.chkAutoHideMenu.UseVisualStyleBackColor = true;
			// 
			// chkSingleInstance
			// 
			this.chkSingleInstance.AutoSize = true;
			this.chkSingleInstance.Location = new System.Drawing.Point(3, 52);
			this.chkSingleInstance.Name = "chkSingleInstance";
			this.chkSingleInstance.Size = new System.Drawing.Size(228, 17);
			this.chkSingleInstance.TabIndex = 11;
			this.chkSingleInstance.Text = "Only allow one instance of Mesen at a time";
			this.chkSingleInstance.UseVisualStyleBackColor = true;
			// 
			// chkAutomaticallyCheckForUpdates
			// 
			this.chkAutomaticallyCheckForUpdates.AutoSize = true;
			this.chkAutomaticallyCheckForUpdates.Location = new System.Drawing.Point(3, 29);
			this.chkAutomaticallyCheckForUpdates.Name = "chkAutomaticallyCheckForUpdates";
			this.chkAutomaticallyCheckForUpdates.Size = new System.Drawing.Size(177, 17);
			this.chkAutomaticallyCheckForUpdates.TabIndex = 17;
			this.chkAutomaticallyCheckForUpdates.Text = "Automatically check for updates";
			this.chkAutomaticallyCheckForUpdates.UseVisualStyleBackColor = true;
			// 
			// flowLayoutPanel2
			// 
			this.flowLayoutPanel2.Controls.Add(this.lblDisplayLanguage);
			this.flowLayoutPanel2.Controls.Add(this.cboDisplayLanguage);
			this.flowLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowLayoutPanel2.Location = new System.Drawing.Point(0, 0);
			this.flowLayoutPanel2.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel2.Name = "flowLayoutPanel2";
			this.flowLayoutPanel2.Size = new System.Drawing.Size(534, 26);
			this.flowLayoutPanel2.TabIndex = 18;
			this.flowLayoutPanel2.Visible = false;
			// 
			// lblDisplayLanguage
			// 
			this.lblDisplayLanguage.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblDisplayLanguage.AutoSize = true;
			this.lblDisplayLanguage.Location = new System.Drawing.Point(3, 7);
			this.lblDisplayLanguage.Name = "lblDisplayLanguage";
			this.lblDisplayLanguage.Size = new System.Drawing.Size(95, 13);
			this.lblDisplayLanguage.TabIndex = 0;
			this.lblDisplayLanguage.Text = "Display Language:";
			// 
			// cboDisplayLanguage
			// 
			this.cboDisplayLanguage.FormattingEnabled = true;
			this.cboDisplayLanguage.Location = new System.Drawing.Point(104, 3);
			this.cboDisplayLanguage.Name = "cboDisplayLanguage";
			this.cboDisplayLanguage.Size = new System.Drawing.Size(206, 21);
			this.cboDisplayLanguage.TabIndex = 1;
			// 
			// lblMiscSettings
			// 
			this.lblMiscSettings.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblMiscSettings.AutoSize = true;
			this.lblMiscSettings.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblMiscSettings.Location = new System.Drawing.Point(0, 170);
			this.lblMiscSettings.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblMiscSettings.Name = "lblMiscSettings";
			this.lblMiscSettings.Size = new System.Drawing.Size(73, 13);
			this.lblMiscSettings.TabIndex = 22;
			this.lblMiscSettings.Text = "Misc. Settings";
			// 
			// chkAutoLoadPatches
			// 
			this.chkAutoLoadPatches.AutoSize = true;
			this.chkAutoLoadPatches.Location = new System.Drawing.Point(13, 186);
			this.chkAutoLoadPatches.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkAutoLoadPatches.Name = "chkAutoLoadPatches";
			this.chkAutoLoadPatches.Size = new System.Drawing.Size(198, 17);
			this.chkAutoLoadPatches.TabIndex = 9;
			this.chkAutoLoadPatches.Text = "Automatically load IPS/BPS patches";
			this.chkAutoLoadPatches.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel5
			// 
			this.tableLayoutPanel5.ColumnCount = 3;
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel5.Controls.Add(this.btnOpenMesenFolder, 0, 0);
			this.tableLayoutPanel5.Controls.Add(this.btnResetSettings, 2, 0);
			this.tableLayoutPanel5.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel5.Location = new System.Drawing.Point(0, 354);
			this.tableLayoutPanel5.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel5.Name = "tableLayoutPanel5";
			this.tableLayoutPanel5.RowCount = 1;
			this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel5.Size = new System.Drawing.Size(534, 29);
			this.tableLayoutPanel5.TabIndex = 24;
			// 
			// btnOpenMesenFolder
			// 
			this.btnOpenMesenFolder.AutoSize = true;
			this.btnOpenMesenFolder.Location = new System.Drawing.Point(3, 3);
			this.btnOpenMesenFolder.Name = "btnOpenMesenFolder";
			this.btnOpenMesenFolder.Size = new System.Drawing.Size(120, 23);
			this.btnOpenMesenFolder.TabIndex = 16;
			this.btnOpenMesenFolder.Text = "Open Mesen-S Folder";
			this.btnOpenMesenFolder.UseVisualStyleBackColor = true;
			this.btnOpenMesenFolder.Click += new System.EventHandler(this.btnOpenMesenFolder_Click);
			// 
			// btnResetSettings
			// 
			this.btnResetSettings.AutoSize = true;
			this.btnResetSettings.Location = new System.Drawing.Point(431, 3);
			this.btnResetSettings.Name = "btnResetSettings";
			this.btnResetSettings.Size = new System.Drawing.Size(100, 23);
			this.btnResetSettings.TabIndex = 17;
			this.btnResetSettings.Text = "Reset All Settings";
			this.btnResetSettings.UseVisualStyleBackColor = true;
			this.btnResetSettings.Click += new System.EventHandler(this.btnResetSettings_Click);
			// 
			// tpgShortcuts
			// 
			this.tpgShortcuts.Controls.Add(this.ctrlEmulatorShortcuts);
			this.tpgShortcuts.Location = new System.Drawing.Point(4, 22);
			this.tpgShortcuts.Name = "tpgShortcuts";
			this.tpgShortcuts.Padding = new System.Windows.Forms.Padding(3);
			this.tpgShortcuts.Size = new System.Drawing.Size(540, 389);
			this.tpgShortcuts.TabIndex = 7;
			this.tpgShortcuts.Text = "Shortcut Keys";
			this.tpgShortcuts.UseVisualStyleBackColor = true;
			// 
			// ctrlEmulatorShortcuts
			// 
			this.ctrlEmulatorShortcuts.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlEmulatorShortcuts.Location = new System.Drawing.Point(3, 3);
			this.ctrlEmulatorShortcuts.Name = "ctrlEmulatorShortcuts";
			this.ctrlEmulatorShortcuts.Size = new System.Drawing.Size(534, 383);
			this.ctrlEmulatorShortcuts.TabIndex = 0;
			// 
			// tpgFiles
			// 
			this.tpgFiles.Controls.Add(this.tableLayoutPanel6);
			this.tpgFiles.Location = new System.Drawing.Point(4, 22);
			this.tpgFiles.Name = "tpgFiles";
			this.tpgFiles.Padding = new System.Windows.Forms.Padding(3);
			this.tpgFiles.Size = new System.Drawing.Size(540, 389);
			this.tpgFiles.TabIndex = 2;
			this.tpgFiles.Text = "Folders/Files";
			this.tpgFiles.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel6
			// 
			this.tableLayoutPanel6.ColumnCount = 1;
			this.tableLayoutPanel6.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel6.Controls.Add(this.grpPathOverrides, 0, 2);
			this.tableLayoutPanel6.Controls.Add(this.grpFileAssociations, 0, 0);
			this.tableLayoutPanel6.Controls.Add(this.grpDataStorageLocation, 0, 1);
			this.tableLayoutPanel6.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel6.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel6.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel6.Name = "tableLayoutPanel6";
			this.tableLayoutPanel6.RowCount = 4;
			this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel6.Size = new System.Drawing.Size(534, 383);
			this.tableLayoutPanel6.TabIndex = 13;
			// 
			// grpPathOverrides
			// 
			this.grpPathOverrides.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.grpPathOverrides.Controls.Add(this.tableLayoutPanel10);
			this.grpPathOverrides.Location = new System.Drawing.Point(3, 198);
			this.grpPathOverrides.Name = "grpPathOverrides";
			this.grpPathOverrides.Size = new System.Drawing.Size(528, 186);
			this.grpPathOverrides.TabIndex = 15;
			this.grpPathOverrides.TabStop = false;
			this.grpPathOverrides.Text = "Folder Overrides";
			// 
			// tableLayoutPanel10
			// 
			this.tableLayoutPanel10.ColumnCount = 2;
			this.tableLayoutPanel10.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel10.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel10.Controls.Add(this.psGame, 1, 0);
			this.tableLayoutPanel10.Controls.Add(this.chkGameOverride, 0, 0);
			this.tableLayoutPanel10.Controls.Add(this.psWave, 1, 2);
			this.tableLayoutPanel10.Controls.Add(this.psMovies, 1, 3);
			this.tableLayoutPanel10.Controls.Add(this.psSaveData, 1, 4);
			this.tableLayoutPanel10.Controls.Add(this.psSaveStates, 1, 5);
			this.tableLayoutPanel10.Controls.Add(this.psScreenshots, 1, 6);
			this.tableLayoutPanel10.Controls.Add(this.psAvi, 1, 7);
			this.tableLayoutPanel10.Controls.Add(this.chkAviOverride, 0, 7);
			this.tableLayoutPanel10.Controls.Add(this.chkScreenshotsOverride, 0, 6);
			this.tableLayoutPanel10.Controls.Add(this.chkSaveDataOverride, 0, 4);
			this.tableLayoutPanel10.Controls.Add(this.chkWaveOverride, 0, 2);
			this.tableLayoutPanel10.Controls.Add(this.chkSaveStatesOverride, 0, 5);
			this.tableLayoutPanel10.Controls.Add(this.chkMoviesOverride, 0, 3);
			this.tableLayoutPanel10.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel10.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel10.Name = "tableLayoutPanel10";
			this.tableLayoutPanel10.RowCount = 9;
			this.tableLayoutPanel10.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel10.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 5F));
			this.tableLayoutPanel10.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel10.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel10.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel10.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel10.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel10.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel10.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel10.Size = new System.Drawing.Size(522, 167);
			this.tableLayoutPanel10.TabIndex = 0;
			// 
			// psGame
			// 
			this.psGame.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.psGame.DisabledText = "";
			this.psGame.Location = new System.Drawing.Point(94, 0);
			this.psGame.Margin = new System.Windows.Forms.Padding(0);
			this.psGame.MaximumSize = new System.Drawing.Size(1000, 20);
			this.psGame.MinimumSize = new System.Drawing.Size(0, 20);
			this.psGame.Name = "psGame";
			this.psGame.Size = new System.Drawing.Size(428, 20);
			this.psGame.TabIndex = 13;
			// 
			// chkGameOverride
			// 
			this.chkGameOverride.AutoSize = true;
			this.chkGameOverride.Location = new System.Drawing.Point(3, 3);
			this.chkGameOverride.Name = "chkGameOverride";
			this.chkGameOverride.Size = new System.Drawing.Size(62, 17);
			this.chkGameOverride.TabIndex = 12;
			this.chkGameOverride.Text = "Games:";
			this.chkGameOverride.UseVisualStyleBackColor = true;
			this.chkGameOverride.CheckedChanged += new System.EventHandler(this.chkOverride_CheckedChanged);
			// 
			// psWave
			// 
			this.psWave.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.psWave.DisabledText = "";
			this.psWave.Location = new System.Drawing.Point(94, 28);
			this.psWave.Margin = new System.Windows.Forms.Padding(0);
			this.psWave.MaximumSize = new System.Drawing.Size(1000, 20);
			this.psWave.MinimumSize = new System.Drawing.Size(0, 20);
			this.psWave.Name = "psWave";
			this.psWave.Size = new System.Drawing.Size(428, 20);
			this.psWave.TabIndex = 6;
			// 
			// psMovies
			// 
			this.psMovies.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.psMovies.DisabledText = "";
			this.psMovies.Location = new System.Drawing.Point(94, 51);
			this.psMovies.Margin = new System.Windows.Forms.Padding(0);
			this.psMovies.MaximumSize = new System.Drawing.Size(1000, 20);
			this.psMovies.MinimumSize = new System.Drawing.Size(0, 20);
			this.psMovies.Name = "psMovies";
			this.psMovies.Size = new System.Drawing.Size(428, 20);
			this.psMovies.TabIndex = 7;
			// 
			// psSaveData
			// 
			this.psSaveData.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.psSaveData.DisabledText = "";
			this.psSaveData.Location = new System.Drawing.Point(94, 74);
			this.psSaveData.Margin = new System.Windows.Forms.Padding(0);
			this.psSaveData.MaximumSize = new System.Drawing.Size(1000, 20);
			this.psSaveData.MinimumSize = new System.Drawing.Size(0, 20);
			this.psSaveData.Name = "psSaveData";
			this.psSaveData.Size = new System.Drawing.Size(428, 20);
			this.psSaveData.TabIndex = 8;
			// 
			// psSaveStates
			// 
			this.psSaveStates.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.psSaveStates.DisabledText = "";
			this.psSaveStates.Location = new System.Drawing.Point(94, 97);
			this.psSaveStates.Margin = new System.Windows.Forms.Padding(0);
			this.psSaveStates.MaximumSize = new System.Drawing.Size(1000, 20);
			this.psSaveStates.MinimumSize = new System.Drawing.Size(0, 20);
			this.psSaveStates.Name = "psSaveStates";
			this.psSaveStates.Size = new System.Drawing.Size(428, 20);
			this.psSaveStates.TabIndex = 9;
			// 
			// psScreenshots
			// 
			this.psScreenshots.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.psScreenshots.DisabledText = "";
			this.psScreenshots.Location = new System.Drawing.Point(94, 120);
			this.psScreenshots.Margin = new System.Windows.Forms.Padding(0);
			this.psScreenshots.MaximumSize = new System.Drawing.Size(1000, 20);
			this.psScreenshots.MinimumSize = new System.Drawing.Size(0, 20);
			this.psScreenshots.Name = "psScreenshots";
			this.psScreenshots.Size = new System.Drawing.Size(428, 20);
			this.psScreenshots.TabIndex = 10;
			// 
			// psAvi
			// 
			this.psAvi.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.psAvi.DisabledText = "";
			this.psAvi.Location = new System.Drawing.Point(94, 143);
			this.psAvi.Margin = new System.Windows.Forms.Padding(0);
			this.psAvi.MaximumSize = new System.Drawing.Size(1000, 20);
			this.psAvi.MinimumSize = new System.Drawing.Size(0, 20);
			this.psAvi.Name = "psAvi";
			this.psAvi.Size = new System.Drawing.Size(428, 20);
			this.psAvi.TabIndex = 11;
			// 
			// chkAviOverride
			// 
			this.chkAviOverride.AutoSize = true;
			this.chkAviOverride.Location = new System.Drawing.Point(3, 146);
			this.chkAviOverride.Name = "chkAviOverride";
			this.chkAviOverride.Size = new System.Drawing.Size(61, 17);
			this.chkAviOverride.TabIndex = 4;
			this.chkAviOverride.Text = "Videos:";
			this.chkAviOverride.UseVisualStyleBackColor = true;
			this.chkAviOverride.CheckedChanged += new System.EventHandler(this.chkOverride_CheckedChanged);
			// 
			// chkScreenshotsOverride
			// 
			this.chkScreenshotsOverride.AutoSize = true;
			this.chkScreenshotsOverride.Location = new System.Drawing.Point(3, 123);
			this.chkScreenshotsOverride.Name = "chkScreenshotsOverride";
			this.chkScreenshotsOverride.Size = new System.Drawing.Size(88, 17);
			this.chkScreenshotsOverride.TabIndex = 3;
			this.chkScreenshotsOverride.Text = "Screenshots:";
			this.chkScreenshotsOverride.UseVisualStyleBackColor = true;
			this.chkScreenshotsOverride.CheckedChanged += new System.EventHandler(this.chkOverride_CheckedChanged);
			// 
			// chkSaveDataOverride
			// 
			this.chkSaveDataOverride.AutoSize = true;
			this.chkSaveDataOverride.Location = new System.Drawing.Point(3, 77);
			this.chkSaveDataOverride.Name = "chkSaveDataOverride";
			this.chkSaveDataOverride.Size = new System.Drawing.Size(80, 17);
			this.chkSaveDataOverride.TabIndex = 0;
			this.chkSaveDataOverride.Text = "Save Data:";
			this.chkSaveDataOverride.UseVisualStyleBackColor = true;
			this.chkSaveDataOverride.CheckedChanged += new System.EventHandler(this.chkOverride_CheckedChanged);
			// 
			// chkWaveOverride
			// 
			this.chkWaveOverride.AutoSize = true;
			this.chkWaveOverride.Location = new System.Drawing.Point(3, 31);
			this.chkWaveOverride.Name = "chkWaveOverride";
			this.chkWaveOverride.Size = new System.Drawing.Size(56, 17);
			this.chkWaveOverride.TabIndex = 5;
			this.chkWaveOverride.Text = "Audio:";
			this.chkWaveOverride.UseVisualStyleBackColor = true;
			this.chkWaveOverride.CheckedChanged += new System.EventHandler(this.chkOverride_CheckedChanged);
			// 
			// chkSaveStatesOverride
			// 
			this.chkSaveStatesOverride.AutoSize = true;
			this.chkSaveStatesOverride.Location = new System.Drawing.Point(3, 100);
			this.chkSaveStatesOverride.Name = "chkSaveStatesOverride";
			this.chkSaveStatesOverride.Size = new System.Drawing.Size(87, 17);
			this.chkSaveStatesOverride.TabIndex = 2;
			this.chkSaveStatesOverride.Text = "Save States:";
			this.chkSaveStatesOverride.UseVisualStyleBackColor = true;
			this.chkSaveStatesOverride.CheckedChanged += new System.EventHandler(this.chkOverride_CheckedChanged);
			// 
			// chkMoviesOverride
			// 
			this.chkMoviesOverride.AutoSize = true;
			this.chkMoviesOverride.Location = new System.Drawing.Point(3, 54);
			this.chkMoviesOverride.Name = "chkMoviesOverride";
			this.chkMoviesOverride.Size = new System.Drawing.Size(63, 17);
			this.chkMoviesOverride.TabIndex = 1;
			this.chkMoviesOverride.Text = "Movies:";
			this.chkMoviesOverride.UseVisualStyleBackColor = true;
			this.chkMoviesOverride.CheckedChanged += new System.EventHandler(this.chkOverride_CheckedChanged);
			// 
			// grpFileAssociations
			// 
			this.grpFileAssociations.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.grpFileAssociations.Controls.Add(this.tlpFileFormat);
			this.grpFileAssociations.Location = new System.Drawing.Point(3, 3);
			this.grpFileAssociations.Name = "grpFileAssociations";
			this.grpFileAssociations.Size = new System.Drawing.Size(528, 86);
			this.grpFileAssociations.TabIndex = 12;
			this.grpFileAssociations.TabStop = false;
			this.grpFileAssociations.Text = "File Associations";
			// 
			// tlpFileFormat
			// 
			this.tlpFileFormat.ColumnCount = 2;
			this.tlpFileFormat.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpFileFormat.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpFileFormat.Controls.Add(this.chkBsFormat, 0, 2);
			this.tlpFileFormat.Controls.Add(this.chkSpcFormat, 0, 1);
			this.tlpFileFormat.Controls.Add(this.chkRomFormat, 0, 0);
			this.tlpFileFormat.Controls.Add(this.chkMssFormat, 1, 0);
			this.tlpFileFormat.Controls.Add(this.chkMsmFormat, 1, 1);
			this.tlpFileFormat.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpFileFormat.Location = new System.Drawing.Point(3, 16);
			this.tlpFileFormat.Name = "tlpFileFormat";
			this.tlpFileFormat.RowCount = 4;
			this.tlpFileFormat.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpFileFormat.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpFileFormat.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpFileFormat.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpFileFormat.Size = new System.Drawing.Size(522, 67);
			this.tlpFileFormat.TabIndex = 0;
			// 
			// chkBsFormat
			// 
			this.chkBsFormat.AutoSize = true;
			this.chkBsFormat.Location = new System.Drawing.Point(3, 49);
			this.chkBsFormat.Name = "chkBsFormat";
			this.chkBsFormat.Size = new System.Drawing.Size(147, 17);
			this.chkBsFormat.TabIndex = 17;
			this.chkBsFormat.Text = ".BS (BS-X memory packs)";
			this.chkBsFormat.UseVisualStyleBackColor = true;
			// 
			// chkSpcFormat
			// 
			this.chkSpcFormat.AutoSize = true;
			this.chkSpcFormat.Location = new System.Drawing.Point(3, 26);
			this.chkSpcFormat.Name = "chkSpcFormat";
			this.chkSpcFormat.Size = new System.Drawing.Size(143, 17);
			this.chkSpcFormat.TabIndex = 16;
			this.chkSpcFormat.Text = ".SPC (Sound/music files)";
			this.chkSpcFormat.UseVisualStyleBackColor = true;
			// 
			// chkRomFormat
			// 
			this.chkRomFormat.AutoSize = true;
			this.chkRomFormat.Location = new System.Drawing.Point(3, 3);
			this.chkRomFormat.Name = "chkRomFormat";
			this.chkRomFormat.Size = new System.Drawing.Size(141, 17);
			this.chkRomFormat.TabIndex = 10;
			this.chkRomFormat.Text = ".SFC, .SMC, .SWC, .FIG";
			this.chkRomFormat.UseVisualStyleBackColor = true;
			// 
			// chkMssFormat
			// 
			this.chkMssFormat.AutoSize = true;
			this.chkMssFormat.Location = new System.Drawing.Point(264, 3);
			this.chkMssFormat.Name = "chkMssFormat";
			this.chkMssFormat.Size = new System.Drawing.Size(159, 17);
			this.chkMssFormat.TabIndex = 15;
			this.chkMssFormat.Text = ".MSS (Mesen-S Save State)";
			this.chkMssFormat.UseVisualStyleBackColor = true;
			this.chkMssFormat.Visible = false;
			// 
			// chkMsmFormat
			// 
			this.chkMsmFormat.AutoSize = true;
			this.chkMsmFormat.Location = new System.Drawing.Point(264, 26);
			this.chkMsmFormat.Name = "chkMsmFormat";
			this.chkMsmFormat.Size = new System.Drawing.Size(142, 17);
			this.chkMsmFormat.TabIndex = 11;
			this.chkMsmFormat.Text = ".MSM (Mesen-S Movies)";
			this.chkMsmFormat.UseVisualStyleBackColor = true;
			this.chkMsmFormat.Visible = false;
			// 
			// grpDataStorageLocation
			// 
			this.grpDataStorageLocation.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.grpDataStorageLocation.Controls.Add(this.tableLayoutPanel7);
			this.grpDataStorageLocation.Location = new System.Drawing.Point(3, 95);
			this.grpDataStorageLocation.Name = "grpDataStorageLocation";
			this.grpDataStorageLocation.Size = new System.Drawing.Size(528, 97);
			this.grpDataStorageLocation.TabIndex = 14;
			this.grpDataStorageLocation.TabStop = false;
			this.grpDataStorageLocation.Text = "Data Storage Location";
			// 
			// tableLayoutPanel7
			// 
			this.tableLayoutPanel7.ColumnCount = 1;
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel7.Controls.Add(this.tableLayoutPanel8, 0, 0);
			this.tableLayoutPanel7.Controls.Add(this.tableLayoutPanel9, 0, 1);
			this.tableLayoutPanel7.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel7.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel7.Name = "tableLayoutPanel7";
			this.tableLayoutPanel7.RowCount = 3;
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel7.Size = new System.Drawing.Size(522, 78);
			this.tableLayoutPanel7.TabIndex = 13;
			// 
			// tableLayoutPanel8
			// 
			this.tableLayoutPanel8.ColumnCount = 1;
			this.tableLayoutPanel8.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel8.Controls.Add(this.radStorageDocuments, 0, 0);
			this.tableLayoutPanel8.Controls.Add(this.radStoragePortable, 0, 1);
			this.tableLayoutPanel8.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel8.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel8.Name = "tableLayoutPanel8";
			this.tableLayoutPanel8.RowCount = 2;
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel8.Size = new System.Drawing.Size(516, 47);
			this.tableLayoutPanel8.TabIndex = 0;
			// 
			// radStorageDocuments
			// 
			this.radStorageDocuments.AutoSize = true;
			this.radStorageDocuments.Checked = true;
			this.radStorageDocuments.Cursor = System.Windows.Forms.Cursors.Default;
			this.radStorageDocuments.Location = new System.Drawing.Point(3, 3);
			this.radStorageDocuments.Name = "radStorageDocuments";
			this.radStorageDocuments.Size = new System.Drawing.Size(207, 17);
			this.radStorageDocuments.TabIndex = 0;
			this.radStorageDocuments.TabStop = true;
			this.radStorageDocuments.Text = "Store Mesen-S\'s data in my user profile";
			this.radStorageDocuments.UseVisualStyleBackColor = true;
			this.radStorageDocuments.CheckedChanged += new System.EventHandler(this.radStorageDocuments_CheckedChanged);
			// 
			// radStoragePortable
			// 
			this.radStoragePortable.AutoSize = true;
			this.radStoragePortable.Cursor = System.Windows.Forms.Cursors.Default;
			this.radStoragePortable.Location = new System.Drawing.Point(3, 26);
			this.radStoragePortable.Name = "radStoragePortable";
			this.radStoragePortable.Size = new System.Drawing.Size(298, 17);
			this.radStoragePortable.TabIndex = 1;
			this.radStoragePortable.Text = "Store Mesen-S\'s data in the same folder as the application";
			this.radStoragePortable.UseVisualStyleBackColor = true;
			this.radStoragePortable.CheckedChanged += new System.EventHandler(this.radStorageDocuments_CheckedChanged);
			// 
			// tableLayoutPanel9
			// 
			this.tableLayoutPanel9.AutoSize = true;
			this.tableLayoutPanel9.ColumnCount = 2;
			this.tableLayoutPanel9.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel9.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel9.Controls.Add(this.lblLocation, 1, 0);
			this.tableLayoutPanel9.Controls.Add(this.lblDataLocation, 0, 0);
			this.tableLayoutPanel9.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel9.Location = new System.Drawing.Point(3, 56);
			this.tableLayoutPanel9.Name = "tableLayoutPanel9";
			this.tableLayoutPanel9.Padding = new System.Windows.Forms.Padding(0, 5, 0, 0);
			this.tableLayoutPanel9.RowCount = 1;
			this.tableLayoutPanel9.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel9.Size = new System.Drawing.Size(516, 18);
			this.tableLayoutPanel9.TabIndex = 39;
			// 
			// lblLocation
			// 
			this.lblLocation.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lblLocation.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lblLocation.ForeColor = System.Drawing.SystemColors.ControlText;
			this.lblLocation.Location = new System.Drawing.Point(48, 5);
			this.lblLocation.Name = "lblLocation";
			this.lblLocation.Size = new System.Drawing.Size(465, 13);
			this.lblLocation.TabIndex = 1;
			this.lblLocation.Text = "....";
			// 
			// lblDataLocation
			// 
			this.lblDataLocation.AutoSize = true;
			this.lblDataLocation.Location = new System.Drawing.Point(3, 5);
			this.lblDataLocation.Name = "lblDataLocation";
			this.lblDataLocation.Size = new System.Drawing.Size(39, 13);
			this.lblDataLocation.TabIndex = 0;
			this.lblDataLocation.Text = "Folder:";
			// 
			// tpgAdvanced
			// 
			this.tpgAdvanced.Controls.Add(this.tableLayoutPanel1);
			this.tpgAdvanced.Location = new System.Drawing.Point(4, 22);
			this.tpgAdvanced.Name = "tpgAdvanced";
			this.tpgAdvanced.Padding = new System.Windows.Forms.Padding(3);
			this.tpgAdvanced.Size = new System.Drawing.Size(540, 389);
			this.tpgAdvanced.TabIndex = 1;
			this.tpgAdvanced.Text = "Advanced";
			this.tpgAdvanced.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 1;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.chkDisableGameSelectionScreen, 0, 4);
			this.tableLayoutPanel1.Controls.Add(this.lblAdvancedMisc, 0, 10);
			this.tableLayoutPanel1.Controls.Add(this.flowLayoutPanel6, 0, 11);
			this.tableLayoutPanel1.Controls.Add(this.chkDisplayTitleBarInfo, 0, 5);
			this.tableLayoutPanel1.Controls.Add(this.chkShowGameTimer, 0, 8);
			this.tableLayoutPanel1.Controls.Add(this.chkShowFrameCounter, 0, 7);
			this.tableLayoutPanel1.Controls.Add(this.chkDisableOsd, 0, 3);
			this.tableLayoutPanel1.Controls.Add(this.lblUiDisplaySettings, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.lblWindowSettings, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.chkAlwaysOnTop, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.chkShowFps, 0, 6);
			this.tableLayoutPanel1.Controls.Add(this.chkShowDebugInfo, 0, 9);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 12;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(534, 383);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// chkDisableGameSelectionScreen
			// 
			this.chkDisableGameSelectionScreen.AutoSize = true;
			this.chkDisableGameSelectionScreen.Location = new System.Drawing.Point(13, 89);
			this.chkDisableGameSelectionScreen.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkDisableGameSelectionScreen.Name = "chkDisableGameSelectionScreen";
			this.chkDisableGameSelectionScreen.Size = new System.Drawing.Size(170, 17);
			this.chkDisableGameSelectionScreen.TabIndex = 37;
			this.chkDisableGameSelectionScreen.Text = "Disable game selection screen";
			this.chkDisableGameSelectionScreen.UseVisualStyleBackColor = true;
			// 
			// lblAdvancedMisc
			// 
			this.lblAdvancedMisc.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblAdvancedMisc.AutoSize = true;
			this.lblAdvancedMisc.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblAdvancedMisc.Location = new System.Drawing.Point(0, 231);
			this.lblAdvancedMisc.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblAdvancedMisc.Name = "lblAdvancedMisc";
			this.lblAdvancedMisc.Size = new System.Drawing.Size(73, 13);
			this.lblAdvancedMisc.TabIndex = 36;
			this.lblAdvancedMisc.Text = "Misc. Settings";
			// 
			// flowLayoutPanel6
			// 
			this.flowLayoutPanel6.Controls.Add(this.lblRewind);
			this.flowLayoutPanel6.Controls.Add(this.nudRewindBufferSize);
			this.flowLayoutPanel6.Controls.Add(this.lblRewindMinutes);
			this.flowLayoutPanel6.Dock = System.Windows.Forms.DockStyle.Top;
			this.flowLayoutPanel6.Location = new System.Drawing.Point(10, 247);
			this.flowLayoutPanel6.Margin = new System.Windows.Forms.Padding(10, 3, 0, 0);
			this.flowLayoutPanel6.Name = "flowLayoutPanel6";
			this.flowLayoutPanel6.Size = new System.Drawing.Size(524, 23);
			this.flowLayoutPanel6.TabIndex = 35;
			// 
			// lblRewind
			// 
			this.lblRewind.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRewind.AutoSize = true;
			this.lblRewind.Location = new System.Drawing.Point(3, 4);
			this.lblRewind.Name = "lblRewind";
			this.lblRewind.Size = new System.Drawing.Size(142, 13);
			this.lblRewind.TabIndex = 3;
			this.lblRewind.Text = "Keep rewind data for the last";
			// 
			// nudRewindBufferSize
			// 
			this.nudRewindBufferSize.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.nudRewindBufferSize.DecimalPlaces = 0;
			this.nudRewindBufferSize.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudRewindBufferSize.IsHex = false;
			this.nudRewindBufferSize.Location = new System.Drawing.Point(148, 0);
			this.nudRewindBufferSize.Margin = new System.Windows.Forms.Padding(0);
			this.nudRewindBufferSize.Maximum = new decimal(new int[] {
            900,
            0,
            0,
            0});
			this.nudRewindBufferSize.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudRewindBufferSize.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudRewindBufferSize.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudRewindBufferSize.Name = "nudRewindBufferSize";
			this.nudRewindBufferSize.Size = new System.Drawing.Size(42, 21);
			this.nudRewindBufferSize.TabIndex = 1;
			this.nudRewindBufferSize.Value = new decimal(new int[] {
            30,
            0,
            0,
            0});
			// 
			// lblRewindMinutes
			// 
			this.lblRewindMinutes.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRewindMinutes.AutoSize = true;
			this.lblRewindMinutes.Location = new System.Drawing.Point(193, 4);
			this.lblRewindMinutes.Name = "lblRewindMinutes";
			this.lblRewindMinutes.Size = new System.Drawing.Size(175, 13);
			this.lblRewindMinutes.TabIndex = 2;
			this.lblRewindMinutes.Text = "minutes (Memory Usage ≈5MB/min)";
			// 
			// chkDisplayTitleBarInfo
			// 
			this.chkDisplayTitleBarInfo.AutoSize = true;
			this.chkDisplayTitleBarInfo.Location = new System.Drawing.Point(13, 112);
			this.chkDisplayTitleBarInfo.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkDisplayTitleBarInfo.Name = "chkDisplayTitleBarInfo";
			this.chkDisplayTitleBarInfo.Size = new System.Drawing.Size(210, 17);
			this.chkDisplayTitleBarInfo.TabIndex = 8;
			this.chkDisplayTitleBarInfo.Text = "Display additional information in title bar";
			this.chkDisplayTitleBarInfo.UseVisualStyleBackColor = true;
			// 
			// chkShowGameTimer
			// 
			this.chkShowGameTimer.AutoSize = true;
			this.chkShowGameTimer.Location = new System.Drawing.Point(13, 181);
			this.chkShowGameTimer.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkShowGameTimer.Name = "chkShowGameTimer";
			this.chkShowGameTimer.Size = new System.Drawing.Size(107, 17);
			this.chkShowGameTimer.TabIndex = 11;
			this.chkShowGameTimer.Text = "Show game timer";
			this.chkShowGameTimer.UseVisualStyleBackColor = true;
			// 
			// chkShowFrameCounter
			// 
			this.chkShowFrameCounter.AutoSize = true;
			this.chkShowFrameCounter.Location = new System.Drawing.Point(13, 158);
			this.chkShowFrameCounter.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkShowFrameCounter.Name = "chkShowFrameCounter";
			this.chkShowFrameCounter.Size = new System.Drawing.Size(121, 17);
			this.chkShowFrameCounter.TabIndex = 12;
			this.chkShowFrameCounter.Text = "Show frame counter";
			this.chkShowFrameCounter.UseVisualStyleBackColor = true;
			// 
			// chkDisableOsd
			// 
			this.chkDisableOsd.AutoSize = true;
			this.chkDisableOsd.Location = new System.Drawing.Point(13, 66);
			this.chkDisableOsd.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkDisableOsd.Name = "chkDisableOsd";
			this.chkDisableOsd.Size = new System.Drawing.Size(178, 17);
			this.chkDisableOsd.TabIndex = 14;
			this.chkDisableOsd.Text = "Disable on-screen display (OSD)";
			this.chkDisableOsd.UseVisualStyleBackColor = true;
			// 
			// lblUiDisplaySettings
			// 
			this.lblUiDisplaySettings.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblUiDisplaySettings.AutoSize = true;
			this.lblUiDisplaySettings.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblUiDisplaySettings.Location = new System.Drawing.Point(0, 50);
			this.lblUiDisplaySettings.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblUiDisplaySettings.Name = "lblUiDisplaySettings";
			this.lblUiDisplaySettings.Size = new System.Drawing.Size(96, 13);
			this.lblUiDisplaySettings.TabIndex = 25;
			this.lblUiDisplaySettings.Text = "UI Display Settings";
			// 
			// lblWindowSettings
			// 
			this.lblWindowSettings.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblWindowSettings.AutoSize = true;
			this.lblWindowSettings.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblWindowSettings.Location = new System.Drawing.Point(0, 7);
			this.lblWindowSettings.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblWindowSettings.Name = "lblWindowSettings";
			this.lblWindowSettings.Size = new System.Drawing.Size(87, 13);
			this.lblWindowSettings.TabIndex = 32;
			this.lblWindowSettings.Text = "Window Settings";
			// 
			// chkAlwaysOnTop
			// 
			this.chkAlwaysOnTop.AutoSize = true;
			this.chkAlwaysOnTop.Location = new System.Drawing.Point(13, 23);
			this.chkAlwaysOnTop.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkAlwaysOnTop.Name = "chkAlwaysOnTop";
			this.chkAlwaysOnTop.Size = new System.Drawing.Size(210, 17);
			this.chkAlwaysOnTop.TabIndex = 29;
			this.chkAlwaysOnTop.Text = "Always display on top of other windows";
			this.chkAlwaysOnTop.UseVisualStyleBackColor = true;
			// 
			// chkShowFps
			// 
			this.chkShowFps.AutoSize = true;
			this.chkShowFps.Location = new System.Drawing.Point(13, 135);
			this.chkShowFps.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkShowFps.Name = "chkShowFps";
			this.chkShowFps.Size = new System.Drawing.Size(76, 17);
			this.chkShowFps.TabIndex = 33;
			this.chkShowFps.Text = "Show FPS";
			this.chkShowFps.UseVisualStyleBackColor = true;
			// 
			// chkShowDebugInfo
			// 
			this.chkShowDebugInfo.AutoSize = true;
			this.chkShowDebugInfo.Location = new System.Drawing.Point(13, 204);
			this.chkShowDebugInfo.Margin = new System.Windows.Forms.Padding(13, 3, 3, 3);
			this.chkShowDebugInfo.Name = "chkShowDebugInfo";
			this.chkShowDebugInfo.Size = new System.Drawing.Size(140, 17);
			this.chkShowDebugInfo.TabIndex = 34;
			this.chkShowDebugInfo.Text = "Show debug information";
			this.chkShowDebugInfo.UseVisualStyleBackColor = true;
			// 
			// frmPreferences
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(548, 444);
			this.Controls.Add(this.tabMain);
			this.Name = "frmPreferences";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Preferences";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tabMain, 0);
			this.tabMain.ResumeLayout(false);
			this.tpgGeneral.ResumeLayout(false);
			this.tlpMain.ResumeLayout(false);
			this.tlpMain.PerformLayout();
			this.flowLayoutPanel8.ResumeLayout(false);
			this.flowLayoutPanel8.PerformLayout();
			this.flowLayoutPanel2.ResumeLayout(false);
			this.flowLayoutPanel2.PerformLayout();
			this.tableLayoutPanel5.ResumeLayout(false);
			this.tableLayoutPanel5.PerformLayout();
			this.tpgShortcuts.ResumeLayout(false);
			this.tpgFiles.ResumeLayout(false);
			this.tableLayoutPanel6.ResumeLayout(false);
			this.grpPathOverrides.ResumeLayout(false);
			this.tableLayoutPanel10.ResumeLayout(false);
			this.tableLayoutPanel10.PerformLayout();
			this.grpFileAssociations.ResumeLayout(false);
			this.tlpFileFormat.ResumeLayout(false);
			this.tlpFileFormat.PerformLayout();
			this.grpDataStorageLocation.ResumeLayout(false);
			this.tableLayoutPanel7.ResumeLayout(false);
			this.tableLayoutPanel7.PerformLayout();
			this.tableLayoutPanel8.ResumeLayout(false);
			this.tableLayoutPanel8.PerformLayout();
			this.tableLayoutPanel9.ResumeLayout(false);
			this.tableLayoutPanel9.PerformLayout();
			this.tpgAdvanced.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.flowLayoutPanel6.ResumeLayout(false);
			this.flowLayoutPanel6.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tpgGeneral;
		private System.Windows.Forms.TabPage tpgShortcuts;
		private System.Windows.Forms.TabPage tpgFiles;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel6;
		private System.Windows.Forms.GroupBox grpPathOverrides;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel10;
		private ctrlPathSelection psGame;
		private System.Windows.Forms.CheckBox chkGameOverride;
		private ctrlPathSelection psWave;
		private ctrlPathSelection psMovies;
		private ctrlPathSelection psSaveData;
		private ctrlPathSelection psSaveStates;
		private ctrlPathSelection psScreenshots;
		private ctrlPathSelection psAvi;
		private System.Windows.Forms.CheckBox chkAviOverride;
		private System.Windows.Forms.CheckBox chkScreenshotsOverride;
		private System.Windows.Forms.CheckBox chkSaveDataOverride;
		private System.Windows.Forms.CheckBox chkWaveOverride;
		private System.Windows.Forms.CheckBox chkSaveStatesOverride;
		private System.Windows.Forms.CheckBox chkMoviesOverride;
		private System.Windows.Forms.GroupBox grpFileAssociations;
		private System.Windows.Forms.TableLayoutPanel tlpFileFormat;
		private System.Windows.Forms.CheckBox chkRomFormat;
		private System.Windows.Forms.CheckBox chkMsmFormat;
		private System.Windows.Forms.CheckBox chkMssFormat;
		private System.Windows.Forms.GroupBox grpDataStorageLocation;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel7;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel8;
		private System.Windows.Forms.RadioButton radStorageDocuments;
		private System.Windows.Forms.RadioButton radStoragePortable;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel9;
		private System.Windows.Forms.Label lblLocation;
		private System.Windows.Forms.Label lblDataLocation;
		private System.Windows.Forms.TabPage tpgAdvanced;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.CheckBox chkDisplayTitleBarInfo;
		private System.Windows.Forms.CheckBox chkShowGameTimer;
		private System.Windows.Forms.CheckBox chkShowFrameCounter;
		private System.Windows.Forms.CheckBox chkDisableOsd;
		private System.Windows.Forms.Label lblUiDisplaySettings;
		private System.Windows.Forms.Label lblWindowSettings;
		private System.Windows.Forms.CheckBox chkAlwaysOnTop;
		private System.Windows.Forms.CheckBox chkShowFps;
		private System.Windows.Forms.TableLayoutPanel tlpMain;
		private System.Windows.Forms.CheckBox chkSingleInstance;
		private System.Windows.Forms.CheckBox chkAutomaticallyCheckForUpdates;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel2;
		private System.Windows.Forms.Label lblDisplayLanguage;
		private System.Windows.Forms.ComboBox cboDisplayLanguage;
		private System.Windows.Forms.Label lblMiscSettings;
		private System.Windows.Forms.CheckBox chkAutoLoadPatches;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel5;
		private System.Windows.Forms.Button btnOpenMesenFolder;
		private System.Windows.Forms.Button btnResetSettings;
		private ctrlEmulatorShortcuts ctrlEmulatorShortcuts;
		private System.Windows.Forms.CheckBox chkShowDebugInfo;
		private System.Windows.Forms.CheckBox chkAutoHideMenu;
		private System.Windows.Forms.Label lblAdvancedMisc;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel6;
		private System.Windows.Forms.Label lblRewind;
		private Controls.MesenNumericUpDown nudRewindBufferSize;
		private System.Windows.Forms.Label lblRewindMinutes;
		private System.Windows.Forms.CheckBox chkAllowBackgroundInput;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel8;
		private System.Windows.Forms.Label lblPauseIn;
		private System.Windows.Forms.CheckBox chkPauseWhenInBackground;
		private System.Windows.Forms.CheckBox chkPauseInMenuAndConfig;
		private System.Windows.Forms.CheckBox chkPauseInDebugger;
		private System.Windows.Forms.Label lblPauseBackgroundSettings;
		private System.Windows.Forms.CheckBox chkPauseOnMovieEnd;
		private System.Windows.Forms.CheckBox chkDisableGameSelectionScreen;
		private System.Windows.Forms.CheckBox chkSpcFormat;
	  private System.Windows.Forms.CheckBox chkBsFormat;
   }
}