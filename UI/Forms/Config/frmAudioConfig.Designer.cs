namespace Mesen.GUI.Forms.Config
{
	partial class frmAudioConfig
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
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.tpgGeneral = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.lblVolumeReductionSettings = new System.Windows.Forms.Label();
			this.chkEnableAudio = new System.Windows.Forms.CheckBox();
			this.lblSampleRate = new System.Windows.Forms.Label();
			this.lblAudioLatency = new System.Windows.Forms.Label();
			this.cboSampleRate = new System.Windows.Forms.ComboBox();
			this.lblAudioDevice = new System.Windows.Forms.Label();
			this.cboAudioDevice = new System.Windows.Forms.ComboBox();
			this.tableLayoutPanel7 = new System.Windows.Forms.TableLayoutPanel();
			this.lblLatencyWarning = new System.Windows.Forms.Label();
			this.picLatencyWarning = new System.Windows.Forms.PictureBox();
			this.lblLatencyMs = new System.Windows.Forms.Label();
			this.nudLatency = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.tableLayoutPanel8 = new System.Windows.Forms.TableLayoutPanel();
			this.chkReduceSoundInBackground = new System.Windows.Forms.CheckBox();
			this.chkReduceSoundInFastForward = new System.Windows.Forms.CheckBox();
			this.trkVolumeReduction = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.chkMuteSoundInBackground = new System.Windows.Forms.CheckBox();
			this.trkMaster = new Mesen.GUI.Controls.ctrlHorizontalTrackbar();
			this.lblVolume = new System.Windows.Forms.Label();
			this.tpgEqualizer = new System.Windows.Forms.TabPage();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.chkEnableEqualizer = new System.Windows.Forms.CheckBox();
			this.tlpEqualizer = new System.Windows.Forms.TableLayoutPanel();
			this.trkBand6Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand5Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand4Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand3Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand2Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand1Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand11Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand12Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand13Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand14Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand15Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand16Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand7Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand8Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand9Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand10Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand17Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand18Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand19Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.trkBand20Gain = new Mesen.GUI.Controls.ctrlTrackbar();
			this.tpgAdvanced = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.chkDisableDynamicSampleRate = new Mesen.GUI.Controls.ctrlRiskyOption();
			this.tabControl1.SuspendLayout();
			this.tpgGeneral.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.tableLayoutPanel7.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picLatencyWarning)).BeginInit();
			this.tableLayoutPanel8.SuspendLayout();
			this.tpgEqualizer.SuspendLayout();
			this.groupBox1.SuspendLayout();
			this.tlpEqualizer.SuspendLayout();
			this.tpgAdvanced.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 378);
			this.baseConfigPanel.Size = new System.Drawing.Size(492, 29);
			this.baseConfigPanel.TabIndex = 4;
			// 
			// tabControl1
			// 
			this.tabControl1.Controls.Add(this.tpgGeneral);
			this.tabControl1.Controls.Add(this.tpgEqualizer);
			this.tabControl1.Controls.Add(this.tpgAdvanced);
			this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabControl1.Location = new System.Drawing.Point(0, 0);
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(492, 378);
			this.tabControl1.TabIndex = 2;
			// 
			// tpgGeneral
			// 
			this.tpgGeneral.Controls.Add(this.tableLayoutPanel2);
			this.tpgGeneral.Location = new System.Drawing.Point(4, 22);
			this.tpgGeneral.Name = "tpgGeneral";
			this.tpgGeneral.Padding = new System.Windows.Forms.Padding(3);
			this.tpgGeneral.Size = new System.Drawing.Size(484, 352);
			this.tpgGeneral.TabIndex = 2;
			this.tpgGeneral.Text = "General";
			this.tpgGeneral.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 2;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.Controls.Add(this.lblVolumeReductionSettings, 0, 5);
			this.tableLayoutPanel2.Controls.Add(this.chkEnableAudio, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.lblSampleRate, 0, 2);
			this.tableLayoutPanel2.Controls.Add(this.lblAudioLatency, 0, 3);
			this.tableLayoutPanel2.Controls.Add(this.cboSampleRate, 1, 2);
			this.tableLayoutPanel2.Controls.Add(this.lblAudioDevice, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.cboAudioDevice, 1, 1);
			this.tableLayoutPanel2.Controls.Add(this.tableLayoutPanel7, 1, 3);
			this.tableLayoutPanel2.Controls.Add(this.tableLayoutPanel8, 0, 6);
			this.tableLayoutPanel2.Controls.Add(this.trkMaster, 1, 4);
			this.tableLayoutPanel2.Controls.Add(this.lblVolume, 0, 4);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel2.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 10;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 22F));
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(478, 346);
			this.tableLayoutPanel2.TabIndex = 3;
			// 
			// lblVolumeReductionSettings
			// 
			this.lblVolumeReductionSettings.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblVolumeReductionSettings.AutoSize = true;
			this.tableLayoutPanel2.SetColumnSpan(this.lblVolumeReductionSettings, 2);
			this.lblVolumeReductionSettings.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblVolumeReductionSettings.Location = new System.Drawing.Point(0, 174);
			this.lblVolumeReductionSettings.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblVolumeReductionSettings.Name = "lblVolumeReductionSettings";
			this.lblVolumeReductionSettings.Size = new System.Drawing.Size(94, 13);
			this.lblVolumeReductionSettings.TabIndex = 24;
			this.lblVolumeReductionSettings.Text = "Volume Reduction";
			// 
			// chkEnableAudio
			// 
			this.chkEnableAudio.AutoSize = true;
			this.tableLayoutPanel2.SetColumnSpan(this.chkEnableAudio, 2);
			this.chkEnableAudio.Location = new System.Drawing.Point(6, 6);
			this.chkEnableAudio.Margin = new System.Windows.Forms.Padding(6, 6, 6, 3);
			this.chkEnableAudio.Name = "chkEnableAudio";
			this.chkEnableAudio.Size = new System.Drawing.Size(89, 17);
			this.chkEnableAudio.TabIndex = 3;
			this.chkEnableAudio.Text = "Enable Audio";
			this.chkEnableAudio.UseVisualStyleBackColor = true;
			// 
			// lblSampleRate
			// 
			this.lblSampleRate.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblSampleRate.AutoSize = true;
			this.lblSampleRate.Location = new System.Drawing.Point(3, 60);
			this.lblSampleRate.Name = "lblSampleRate";
			this.lblSampleRate.Size = new System.Drawing.Size(71, 13);
			this.lblSampleRate.TabIndex = 0;
			this.lblSampleRate.Text = "Sample Rate:";
			// 
			// lblAudioLatency
			// 
			this.lblAudioLatency.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblAudioLatency.AutoSize = true;
			this.lblAudioLatency.Location = new System.Drawing.Point(3, 87);
			this.lblAudioLatency.Name = "lblAudioLatency";
			this.lblAudioLatency.Size = new System.Drawing.Size(48, 13);
			this.lblAudioLatency.TabIndex = 0;
			this.lblAudioLatency.Text = "Latency:";
			// 
			// cboSampleRate
			// 
			this.cboSampleRate.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboSampleRate.FormattingEnabled = true;
			this.cboSampleRate.Items.AddRange(new object[] {
            "11,025 Hz",
            "22,050 Hz",
            "32,000 Hz",
            "44,100 Hz",
            "48,000 Hz",
            "96,000 Hz"});
			this.cboSampleRate.Location = new System.Drawing.Point(80, 56);
			this.cboSampleRate.Name = "cboSampleRate";
			this.cboSampleRate.Size = new System.Drawing.Size(75, 21);
			this.cboSampleRate.TabIndex = 5;
			// 
			// lblAudioDevice
			// 
			this.lblAudioDevice.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblAudioDevice.AutoSize = true;
			this.lblAudioDevice.Location = new System.Drawing.Point(3, 33);
			this.lblAudioDevice.Name = "lblAudioDevice";
			this.lblAudioDevice.Size = new System.Drawing.Size(44, 13);
			this.lblAudioDevice.TabIndex = 6;
			this.lblAudioDevice.Text = "Device:";
			// 
			// cboAudioDevice
			// 
			this.cboAudioDevice.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboAudioDevice.FormattingEnabled = true;
			this.cboAudioDevice.Location = new System.Drawing.Point(80, 29);
			this.cboAudioDevice.Name = "cboAudioDevice";
			this.cboAudioDevice.Size = new System.Drawing.Size(209, 21);
			this.cboAudioDevice.TabIndex = 7;
			// 
			// tableLayoutPanel7
			// 
			this.tableLayoutPanel7.AutoSize = true;
			this.tableLayoutPanel7.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.tableLayoutPanel7.ColumnCount = 4;
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel7.Controls.Add(this.lblLatencyWarning, 3, 0);
			this.tableLayoutPanel7.Controls.Add(this.picLatencyWarning, 2, 0);
			this.tableLayoutPanel7.Controls.Add(this.lblLatencyMs, 1, 0);
			this.tableLayoutPanel7.Controls.Add(this.nudLatency, 0, 0);
			this.tableLayoutPanel7.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel7.Location = new System.Drawing.Point(77, 80);
			this.tableLayoutPanel7.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel7.Name = "tableLayoutPanel7";
			this.tableLayoutPanel7.RowCount = 1;
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel7.Size = new System.Drawing.Size(401, 27);
			this.tableLayoutPanel7.TabIndex = 15;
			// 
			// lblLatencyWarning
			// 
			this.lblLatencyWarning.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblLatencyWarning.AutoSize = true;
			this.lblLatencyWarning.Location = new System.Drawing.Point(98, 7);
			this.lblLatencyWarning.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblLatencyWarning.Name = "lblLatencyWarning";
			this.lblLatencyWarning.Size = new System.Drawing.Size(192, 13);
			this.lblLatencyWarning.TabIndex = 4;
			this.lblLatencyWarning.Text = "Low values may cause sound problems";
			this.lblLatencyWarning.Visible = false;
			// 
			// picLatencyWarning
			// 
			this.picLatencyWarning.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.picLatencyWarning.BackgroundImage = global::Mesen.GUI.Properties.Resources.Warning;
			this.picLatencyWarning.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
			this.picLatencyWarning.Location = new System.Drawing.Point(82, 5);
			this.picLatencyWarning.Margin = new System.Windows.Forms.Padding(5, 3, 0, 3);
			this.picLatencyWarning.Name = "picLatencyWarning";
			this.picLatencyWarning.Size = new System.Drawing.Size(16, 16);
			this.picLatencyWarning.TabIndex = 3;
			this.picLatencyWarning.TabStop = false;
			this.picLatencyWarning.Visible = false;
			// 
			// lblLatencyMs
			// 
			this.lblLatencyMs.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblLatencyMs.AutoSize = true;
			this.lblLatencyMs.Location = new System.Drawing.Point(54, 7);
			this.lblLatencyMs.Name = "lblLatencyMs";
			this.lblLatencyMs.Size = new System.Drawing.Size(20, 13);
			this.lblLatencyMs.TabIndex = 2;
			this.lblLatencyMs.Text = "ms";
			// 
			// nudLatency
			// 
			this.nudLatency.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.nudLatency.DecimalPlaces = 0;
			this.nudLatency.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudLatency.Location = new System.Drawing.Point(3, 3);
			this.nudLatency.Maximum = new decimal(new int[] {
            300,
            0,
            0,
            0});
			this.nudLatency.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudLatency.Minimum = new decimal(new int[] {
            15,
            0,
            0,
            0});
			this.nudLatency.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudLatency.Name = "nudLatency";
			this.nudLatency.Size = new System.Drawing.Size(45, 21);
			this.nudLatency.TabIndex = 1;
			this.nudLatency.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
			// 
			// tableLayoutPanel8
			// 
			this.tableLayoutPanel8.ColumnCount = 2;
			this.tableLayoutPanel2.SetColumnSpan(this.tableLayoutPanel8, 2);
			this.tableLayoutPanel8.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel8.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel8.Controls.Add(this.chkReduceSoundInBackground, 0, 1);
			this.tableLayoutPanel8.Controls.Add(this.chkReduceSoundInFastForward, 0, 2);
			this.tableLayoutPanel8.Controls.Add(this.trkVolumeReduction, 1, 1);
			this.tableLayoutPanel8.Controls.Add(this.chkMuteSoundInBackground, 0, 0);
			this.tableLayoutPanel8.Location = new System.Drawing.Point(10, 190);
			this.tableLayoutPanel8.Margin = new System.Windows.Forms.Padding(10, 3, 0, 0);
			this.tableLayoutPanel8.Name = "tableLayoutPanel8";
			this.tableLayoutPanel8.RowCount = 4;
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel8.Size = new System.Drawing.Size(453, 81);
			this.tableLayoutPanel8.TabIndex = 25;
			// 
			// chkReduceSoundInBackground
			// 
			this.chkReduceSoundInBackground.AutoSize = true;
			this.chkReduceSoundInBackground.Location = new System.Drawing.Point(3, 26);
			this.chkReduceSoundInBackground.Name = "chkReduceSoundInBackground";
			this.chkReduceSoundInBackground.Size = new System.Drawing.Size(164, 17);
			this.chkReduceSoundInBackground.TabIndex = 13;
			this.chkReduceSoundInBackground.Text = "Reduce when in background";
			this.chkReduceSoundInBackground.UseVisualStyleBackColor = true;
			this.chkReduceSoundInBackground.CheckedChanged += new System.EventHandler(this.chkReduceVolume_CheckedChanged);
			// 
			// chkReduceSoundInFastForward
			// 
			this.chkReduceSoundInFastForward.AutoSize = true;
			this.chkReduceSoundInFastForward.Location = new System.Drawing.Point(3, 49);
			this.chkReduceSoundInFastForward.Name = "chkReduceSoundInFastForward";
			this.chkReduceSoundInFastForward.Size = new System.Drawing.Size(225, 17);
			this.chkReduceSoundInFastForward.TabIndex = 16;
			this.chkReduceSoundInFastForward.Text = "Reduce when fast forwarding or rewinding";
			this.chkReduceSoundInFastForward.UseVisualStyleBackColor = true;
			this.chkReduceSoundInFastForward.CheckedChanged += new System.EventHandler(this.chkReduceVolume_CheckedChanged);
			// 
			// trkVolumeReduction
			// 
			this.trkVolumeReduction.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkVolumeReduction.Enabled = false;
			this.trkVolumeReduction.Location = new System.Drawing.Point(231, 23);
			this.trkVolumeReduction.Margin = new System.Windows.Forms.Padding(0);
			this.trkVolumeReduction.Maximum = 100;
			this.trkVolumeReduction.MaximumSize = new System.Drawing.Size(400, 55);
			this.trkVolumeReduction.Minimum = 0;
			this.trkVolumeReduction.MinimumSize = new System.Drawing.Size(150, 55);
			this.trkVolumeReduction.Name = "trkVolumeReduction";
			this.tableLayoutPanel8.SetRowSpan(this.trkVolumeReduction, 2);
			this.trkVolumeReduction.Size = new System.Drawing.Size(222, 55);
			this.trkVolumeReduction.TabIndex = 17;
			this.trkVolumeReduction.Text = "Volume Reduction";
			this.trkVolumeReduction.Value = 50;
			// 
			// chkMuteSoundInBackground
			// 
			this.chkMuteSoundInBackground.AutoSize = true;
			this.chkMuteSoundInBackground.Location = new System.Drawing.Point(3, 3);
			this.chkMuteSoundInBackground.Name = "chkMuteSoundInBackground";
			this.chkMuteSoundInBackground.Size = new System.Drawing.Size(182, 17);
			this.chkMuteSoundInBackground.TabIndex = 18;
			this.chkMuteSoundInBackground.Text = "Mute sound when in background";
			this.chkMuteSoundInBackground.UseVisualStyleBackColor = true;
			this.chkMuteSoundInBackground.CheckedChanged += new System.EventHandler(this.chkMuteWhenInBackground_CheckedChanged);
			// 
			// trkMaster
			// 
			this.trkMaster.Location = new System.Drawing.Point(77, 110);
			this.trkMaster.Margin = new System.Windows.Forms.Padding(0, 3, 0, 0);
			this.trkMaster.Maximum = 100;
			this.trkMaster.MaximumSize = new System.Drawing.Size(400, 55);
			this.trkMaster.Minimum = 0;
			this.trkMaster.MinimumSize = new System.Drawing.Size(150, 55);
			this.trkMaster.Name = "trkMaster";
			this.trkMaster.Size = new System.Drawing.Size(222, 55);
			this.trkMaster.TabIndex = 26;
			this.trkMaster.Text = "Master";
			this.trkMaster.Value = 50;
			// 
			// lblVolume
			// 
			this.lblVolume.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblVolume.AutoSize = true;
			this.lblVolume.Location = new System.Drawing.Point(3, 129);
			this.lblVolume.Name = "lblVolume";
			this.lblVolume.Size = new System.Drawing.Size(45, 13);
			this.lblVolume.TabIndex = 27;
			this.lblVolume.Text = "Volume:";
			// 
			// tpgEqualizer
			// 
			this.tpgEqualizer.Controls.Add(this.groupBox1);
			this.tpgEqualizer.Location = new System.Drawing.Point(4, 22);
			this.tpgEqualizer.Name = "tpgEqualizer";
			this.tpgEqualizer.Padding = new System.Windows.Forms.Padding(3);
			this.tpgEqualizer.Size = new System.Drawing.Size(484, 352);
			this.tpgEqualizer.TabIndex = 6;
			this.tpgEqualizer.Text = "Equalizer";
			this.tpgEqualizer.UseVisualStyleBackColor = true;
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.chkEnableEqualizer);
			this.groupBox1.Controls.Add(this.tlpEqualizer);
			this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.groupBox1.Location = new System.Drawing.Point(3, 3);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(478, 346);
			this.groupBox1.TabIndex = 4;
			this.groupBox1.TabStop = false;
			// 
			// chkEnableEqualizer
			// 
			this.chkEnableEqualizer.AutoSize = true;
			this.chkEnableEqualizer.BackColor = System.Drawing.SystemColors.ControlLightLight;
			this.chkEnableEqualizer.Location = new System.Drawing.Point(7, 0);
			this.chkEnableEqualizer.Name = "chkEnableEqualizer";
			this.chkEnableEqualizer.Padding = new System.Windows.Forms.Padding(5, 0, 0, 0);
			this.chkEnableEqualizer.Size = new System.Drawing.Size(110, 17);
			this.chkEnableEqualizer.TabIndex = 5;
			this.chkEnableEqualizer.Text = "Enable Equalizer";
			this.chkEnableEqualizer.UseVisualStyleBackColor = false;
			this.chkEnableEqualizer.CheckedChanged += new System.EventHandler(this.chkEnableEqualizer_CheckedChanged);
			// 
			// tlpEqualizer
			// 
			this.tlpEqualizer.ColumnCount = 10;
			this.tlpEqualizer.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 10F));
			this.tlpEqualizer.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 10F));
			this.tlpEqualizer.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 10F));
			this.tlpEqualizer.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 10F));
			this.tlpEqualizer.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 10F));
			this.tlpEqualizer.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 10F));
			this.tlpEqualizer.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 10F));
			this.tlpEqualizer.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 10F));
			this.tlpEqualizer.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 10F));
			this.tlpEqualizer.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 10F));
			this.tlpEqualizer.Controls.Add(this.trkBand6Gain, 5, 0);
			this.tlpEqualizer.Controls.Add(this.trkBand5Gain, 4, 0);
			this.tlpEqualizer.Controls.Add(this.trkBand4Gain, 3, 0);
			this.tlpEqualizer.Controls.Add(this.trkBand3Gain, 2, 0);
			this.tlpEqualizer.Controls.Add(this.trkBand2Gain, 1, 0);
			this.tlpEqualizer.Controls.Add(this.trkBand1Gain, 0, 0);
			this.tlpEqualizer.Controls.Add(this.trkBand11Gain, 0, 1);
			this.tlpEqualizer.Controls.Add(this.trkBand12Gain, 1, 1);
			this.tlpEqualizer.Controls.Add(this.trkBand13Gain, 2, 1);
			this.tlpEqualizer.Controls.Add(this.trkBand14Gain, 3, 1);
			this.tlpEqualizer.Controls.Add(this.trkBand15Gain, 4, 1);
			this.tlpEqualizer.Controls.Add(this.trkBand16Gain, 5, 1);
			this.tlpEqualizer.Controls.Add(this.trkBand7Gain, 6, 0);
			this.tlpEqualizer.Controls.Add(this.trkBand8Gain, 7, 0);
			this.tlpEqualizer.Controls.Add(this.trkBand9Gain, 8, 0);
			this.tlpEqualizer.Controls.Add(this.trkBand10Gain, 9, 0);
			this.tlpEqualizer.Controls.Add(this.trkBand17Gain, 6, 1);
			this.tlpEqualizer.Controls.Add(this.trkBand18Gain, 7, 1);
			this.tlpEqualizer.Controls.Add(this.trkBand19Gain, 8, 1);
			this.tlpEqualizer.Controls.Add(this.trkBand20Gain, 9, 1);
			this.tlpEqualizer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpEqualizer.Enabled = false;
			this.tlpEqualizer.Location = new System.Drawing.Point(3, 16);
			this.tlpEqualizer.Name = "tlpEqualizer";
			this.tlpEqualizer.RowCount = 2;
			this.tlpEqualizer.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpEqualizer.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpEqualizer.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpEqualizer.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpEqualizer.Size = new System.Drawing.Size(472, 327);
			this.tlpEqualizer.TabIndex = 3;
			// 
			// trkBand6Gain
			// 
			this.trkBand6Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand6Gain.LargeChange = 10;
			this.trkBand6Gain.Location = new System.Drawing.Point(235, 0);
			this.trkBand6Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand6Gain.Maximum = 200;
			this.trkBand6Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand6Gain.Minimum = -200;
			this.trkBand6Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand6Gain.Multiplier = 10;
			this.trkBand6Gain.Name = "trkBand6Gain";
			this.trkBand6Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand6Gain.SmallChange = 1;
			this.trkBand6Gain.TabIndex = 16;
			this.trkBand6Gain.Text = "225 Hz";
			this.trkBand6Gain.TickFrequency = 20;
			this.trkBand6Gain.Value = 500;
			// 
			// trkBand5Gain
			// 
			this.trkBand5Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand5Gain.LargeChange = 10;
			this.trkBand5Gain.Location = new System.Drawing.Point(188, 0);
			this.trkBand5Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand5Gain.Maximum = 200;
			this.trkBand5Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand5Gain.Minimum = -200;
			this.trkBand5Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand5Gain.Multiplier = 10;
			this.trkBand5Gain.Name = "trkBand5Gain";
			this.trkBand5Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand5Gain.SmallChange = 1;
			this.trkBand5Gain.TabIndex = 15;
			this.trkBand5Gain.Text = "160 Hz";
			this.trkBand5Gain.TickFrequency = 20;
			this.trkBand5Gain.Value = 500;
			// 
			// trkBand4Gain
			// 
			this.trkBand4Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand4Gain.LargeChange = 10;
			this.trkBand4Gain.Location = new System.Drawing.Point(141, 0);
			this.trkBand4Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand4Gain.Maximum = 200;
			this.trkBand4Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand4Gain.Minimum = -200;
			this.trkBand4Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand4Gain.Multiplier = 10;
			this.trkBand4Gain.Name = "trkBand4Gain";
			this.trkBand4Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand4Gain.SmallChange = 1;
			this.trkBand4Gain.TabIndex = 14;
			this.trkBand4Gain.Text = "113 Hz";
			this.trkBand4Gain.TickFrequency = 20;
			this.trkBand4Gain.Value = 500;
			// 
			// trkBand3Gain
			// 
			this.trkBand3Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand3Gain.LargeChange = 10;
			this.trkBand3Gain.Location = new System.Drawing.Point(94, 0);
			this.trkBand3Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand3Gain.Maximum = 200;
			this.trkBand3Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand3Gain.Minimum = -200;
			this.trkBand3Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand3Gain.Multiplier = 10;
			this.trkBand3Gain.Name = "trkBand3Gain";
			this.trkBand3Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand3Gain.SmallChange = 1;
			this.trkBand3Gain.TabIndex = 13;
			this.trkBand3Gain.Text = "80 Hz";
			this.trkBand3Gain.TickFrequency = 20;
			this.trkBand3Gain.Value = 500;
			// 
			// trkBand2Gain
			// 
			this.trkBand2Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand2Gain.LargeChange = 10;
			this.trkBand2Gain.Location = new System.Drawing.Point(47, 0);
			this.trkBand2Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand2Gain.Maximum = 200;
			this.trkBand2Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand2Gain.Minimum = -200;
			this.trkBand2Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand2Gain.Multiplier = 10;
			this.trkBand2Gain.Name = "trkBand2Gain";
			this.trkBand2Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand2Gain.SmallChange = 1;
			this.trkBand2Gain.TabIndex = 12;
			this.trkBand2Gain.Text = "56 Hz";
			this.trkBand2Gain.TickFrequency = 20;
			this.trkBand2Gain.Value = 500;
			// 
			// trkBand1Gain
			// 
			this.trkBand1Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand1Gain.LargeChange = 10;
			this.trkBand1Gain.Location = new System.Drawing.Point(0, 0);
			this.trkBand1Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand1Gain.Maximum = 200;
			this.trkBand1Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand1Gain.Minimum = -200;
			this.trkBand1Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand1Gain.Multiplier = 10;
			this.trkBand1Gain.Name = "trkBand1Gain";
			this.trkBand1Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand1Gain.SmallChange = 1;
			this.trkBand1Gain.TabIndex = 11;
			this.trkBand1Gain.Text = "40 Hz";
			this.trkBand1Gain.TickFrequency = 20;
			this.trkBand1Gain.Value = 500;
			// 
			// trkBand11Gain
			// 
			this.trkBand11Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand11Gain.LargeChange = 10;
			this.trkBand11Gain.Location = new System.Drawing.Point(0, 160);
			this.trkBand11Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand11Gain.Maximum = 200;
			this.trkBand11Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand11Gain.Minimum = -200;
			this.trkBand11Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand11Gain.Multiplier = 10;
			this.trkBand11Gain.Name = "trkBand11Gain";
			this.trkBand11Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand11Gain.SmallChange = 1;
			this.trkBand11Gain.TabIndex = 17;
			this.trkBand11Gain.Text = "1.0 kHz";
			this.trkBand11Gain.TickFrequency = 20;
			this.trkBand11Gain.Value = 500;
			// 
			// trkBand12Gain
			// 
			this.trkBand12Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand12Gain.LargeChange = 10;
			this.trkBand12Gain.Location = new System.Drawing.Point(47, 160);
			this.trkBand12Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand12Gain.Maximum = 200;
			this.trkBand12Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand12Gain.Minimum = -200;
			this.trkBand12Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand12Gain.Multiplier = 10;
			this.trkBand12Gain.Name = "trkBand12Gain";
			this.trkBand12Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand12Gain.SmallChange = 1;
			this.trkBand12Gain.TabIndex = 18;
			this.trkBand12Gain.Text = "2.0 kHz";
			this.trkBand12Gain.TickFrequency = 20;
			this.trkBand12Gain.Value = 500;
			// 
			// trkBand13Gain
			// 
			this.trkBand13Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand13Gain.LargeChange = 10;
			this.trkBand13Gain.Location = new System.Drawing.Point(94, 160);
			this.trkBand13Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand13Gain.Maximum = 200;
			this.trkBand13Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand13Gain.Minimum = -200;
			this.trkBand13Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand13Gain.Multiplier = 10;
			this.trkBand13Gain.Name = "trkBand13Gain";
			this.trkBand13Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand13Gain.SmallChange = 1;
			this.trkBand13Gain.TabIndex = 19;
			this.trkBand13Gain.Text = "3.0 kHz";
			this.trkBand13Gain.TickFrequency = 20;
			this.trkBand13Gain.Value = 500;
			// 
			// trkBand14Gain
			// 
			this.trkBand14Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand14Gain.LargeChange = 10;
			this.trkBand14Gain.Location = new System.Drawing.Point(141, 160);
			this.trkBand14Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand14Gain.Maximum = 200;
			this.trkBand14Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand14Gain.Minimum = -200;
			this.trkBand14Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand14Gain.Multiplier = 10;
			this.trkBand14Gain.Name = "trkBand14Gain";
			this.trkBand14Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand14Gain.SmallChange = 1;
			this.trkBand14Gain.TabIndex = 20;
			this.trkBand14Gain.Text = "4.0 kHz";
			this.trkBand14Gain.TickFrequency = 20;
			this.trkBand14Gain.Value = 500;
			// 
			// trkBand15Gain
			// 
			this.trkBand15Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand15Gain.LargeChange = 10;
			this.trkBand15Gain.Location = new System.Drawing.Point(188, 160);
			this.trkBand15Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand15Gain.Maximum = 200;
			this.trkBand15Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand15Gain.Minimum = -200;
			this.trkBand15Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand15Gain.Multiplier = 10;
			this.trkBand15Gain.Name = "trkBand15Gain";
			this.trkBand15Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand15Gain.SmallChange = 1;
			this.trkBand15Gain.TabIndex = 21;
			this.trkBand15Gain.Text = "5.0 kHz";
			this.trkBand15Gain.TickFrequency = 20;
			this.trkBand15Gain.Value = 500;
			// 
			// trkBand16Gain
			// 
			this.trkBand16Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand16Gain.LargeChange = 10;
			this.trkBand16Gain.Location = new System.Drawing.Point(235, 160);
			this.trkBand16Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand16Gain.Maximum = 200;
			this.trkBand16Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand16Gain.Minimum = -200;
			this.trkBand16Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand16Gain.Multiplier = 10;
			this.trkBand16Gain.Name = "trkBand16Gain";
			this.trkBand16Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand16Gain.SmallChange = 1;
			this.trkBand16Gain.TabIndex = 22;
			this.trkBand16Gain.Text = "6.0 kHz";
			this.trkBand16Gain.TickFrequency = 20;
			this.trkBand16Gain.Value = 500;
			// 
			// trkBand7Gain
			// 
			this.trkBand7Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand7Gain.LargeChange = 10;
			this.trkBand7Gain.Location = new System.Drawing.Point(282, 0);
			this.trkBand7Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand7Gain.Maximum = 200;
			this.trkBand7Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand7Gain.Minimum = -200;
			this.trkBand7Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand7Gain.Multiplier = 10;
			this.trkBand7Gain.Name = "trkBand7Gain";
			this.trkBand7Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand7Gain.SmallChange = 1;
			this.trkBand7Gain.TabIndex = 23;
			this.trkBand7Gain.Text = "320 Hz";
			this.trkBand7Gain.TickFrequency = 20;
			this.trkBand7Gain.Value = 500;
			// 
			// trkBand8Gain
			// 
			this.trkBand8Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand8Gain.LargeChange = 10;
			this.trkBand8Gain.Location = new System.Drawing.Point(329, 0);
			this.trkBand8Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand8Gain.Maximum = 200;
			this.trkBand8Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand8Gain.Minimum = -200;
			this.trkBand8Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand8Gain.Multiplier = 10;
			this.trkBand8Gain.Name = "trkBand8Gain";
			this.trkBand8Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand8Gain.SmallChange = 1;
			this.trkBand8Gain.TabIndex = 24;
			this.trkBand8Gain.Text = "450 Hz";
			this.trkBand8Gain.TickFrequency = 20;
			this.trkBand8Gain.Value = 500;
			// 
			// trkBand9Gain
			// 
			this.trkBand9Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand9Gain.LargeChange = 10;
			this.trkBand9Gain.Location = new System.Drawing.Point(376, 0);
			this.trkBand9Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand9Gain.Maximum = 200;
			this.trkBand9Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand9Gain.Minimum = -200;
			this.trkBand9Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand9Gain.Multiplier = 10;
			this.trkBand9Gain.Name = "trkBand9Gain";
			this.trkBand9Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand9Gain.SmallChange = 1;
			this.trkBand9Gain.TabIndex = 25;
			this.trkBand9Gain.Text = "600 Hz";
			this.trkBand9Gain.TickFrequency = 20;
			this.trkBand9Gain.Value = 500;
			// 
			// trkBand10Gain
			// 
			this.trkBand10Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand10Gain.LargeChange = 10;
			this.trkBand10Gain.Location = new System.Drawing.Point(423, 0);
			this.trkBand10Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand10Gain.Maximum = 200;
			this.trkBand10Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand10Gain.Minimum = -200;
			this.trkBand10Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand10Gain.Multiplier = 10;
			this.trkBand10Gain.Name = "trkBand10Gain";
			this.trkBand10Gain.Size = new System.Drawing.Size(49, 160);
			this.trkBand10Gain.SmallChange = 1;
			this.trkBand10Gain.TabIndex = 26;
			this.trkBand10Gain.Text = "750 Hz";
			this.trkBand10Gain.TickFrequency = 20;
			this.trkBand10Gain.Value = 500;
			// 
			// trkBand17Gain
			// 
			this.trkBand17Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand17Gain.LargeChange = 10;
			this.trkBand17Gain.Location = new System.Drawing.Point(282, 160);
			this.trkBand17Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand17Gain.Maximum = 200;
			this.trkBand17Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand17Gain.Minimum = -200;
			this.trkBand17Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand17Gain.Multiplier = 10;
			this.trkBand17Gain.Name = "trkBand17Gain";
			this.trkBand17Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand17Gain.SmallChange = 1;
			this.trkBand17Gain.TabIndex = 27;
			this.trkBand17Gain.Text = "7.0 kHz";
			this.trkBand17Gain.TickFrequency = 20;
			this.trkBand17Gain.Value = 500;
			// 
			// trkBand18Gain
			// 
			this.trkBand18Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand18Gain.LargeChange = 10;
			this.trkBand18Gain.Location = new System.Drawing.Point(329, 160);
			this.trkBand18Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand18Gain.Maximum = 200;
			this.trkBand18Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand18Gain.Minimum = -200;
			this.trkBand18Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand18Gain.Multiplier = 10;
			this.trkBand18Gain.Name = "trkBand18Gain";
			this.trkBand18Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand18Gain.SmallChange = 1;
			this.trkBand18Gain.TabIndex = 28;
			this.trkBand18Gain.Text = "10.0 kHz";
			this.trkBand18Gain.TickFrequency = 20;
			this.trkBand18Gain.Value = 500;
			// 
			// trkBand19Gain
			// 
			this.trkBand19Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand19Gain.LargeChange = 10;
			this.trkBand19Gain.Location = new System.Drawing.Point(376, 160);
			this.trkBand19Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand19Gain.Maximum = 200;
			this.trkBand19Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand19Gain.Minimum = -200;
			this.trkBand19Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand19Gain.Multiplier = 10;
			this.trkBand19Gain.Name = "trkBand19Gain";
			this.trkBand19Gain.Size = new System.Drawing.Size(47, 160);
			this.trkBand19Gain.SmallChange = 1;
			this.trkBand19Gain.TabIndex = 29;
			this.trkBand19Gain.Text = "12.5 kHz";
			this.trkBand19Gain.TickFrequency = 20;
			this.trkBand19Gain.Value = 500;
			// 
			// trkBand20Gain
			// 
			this.trkBand20Gain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkBand20Gain.LargeChange = 10;
			this.trkBand20Gain.Location = new System.Drawing.Point(423, 160);
			this.trkBand20Gain.Margin = new System.Windows.Forms.Padding(0);
			this.trkBand20Gain.Maximum = 200;
			this.trkBand20Gain.MaximumSize = new System.Drawing.Size(63, 160);
			this.trkBand20Gain.Minimum = -200;
			this.trkBand20Gain.MinimumSize = new System.Drawing.Size(34, 160);
			this.trkBand20Gain.Multiplier = 10;
			this.trkBand20Gain.Name = "trkBand20Gain";
			this.trkBand20Gain.Size = new System.Drawing.Size(49, 160);
			this.trkBand20Gain.SmallChange = 1;
			this.trkBand20Gain.TabIndex = 30;
			this.trkBand20Gain.Text = "15 kHz";
			this.trkBand20Gain.TickFrequency = 20;
			this.trkBand20Gain.Value = 500;
			// 
			// tpgAdvanced
			// 
			this.tpgAdvanced.Controls.Add(this.tableLayoutPanel1);
			this.tpgAdvanced.Location = new System.Drawing.Point(4, 22);
			this.tpgAdvanced.Name = "tpgAdvanced";
			this.tpgAdvanced.Padding = new System.Windows.Forms.Padding(3);
			this.tpgAdvanced.Size = new System.Drawing.Size(484, 352);
			this.tpgAdvanced.TabIndex = 7;
			this.tpgAdvanced.Text = "Advanced";
			this.tpgAdvanced.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 1;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.chkDisableDynamicSampleRate, 0, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(478, 346);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// chkDisableDynamicSampleRate
			// 
			this.chkDisableDynamicSampleRate.Checked = false;
			this.chkDisableDynamicSampleRate.Location = new System.Drawing.Point(0, 0);
			this.chkDisableDynamicSampleRate.Name = "chkDisableDynamicSampleRate";
			this.chkDisableDynamicSampleRate.Size = new System.Drawing.Size(463, 24);
			this.chkDisableDynamicSampleRate.TabIndex = 5;
			this.chkDisableDynamicSampleRate.Text = "Disable dynamic sample rate";
			// 
			// frmAudioConfig
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(492, 407);
			this.Controls.Add(this.tabControl1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "frmAudioConfig";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Audio Config";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tabControl1, 0);
			this.tabControl1.ResumeLayout(false);
			this.tpgGeneral.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.tableLayoutPanel7.ResumeLayout(false);
			this.tableLayoutPanel7.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picLatencyWarning)).EndInit();
			this.tableLayoutPanel8.ResumeLayout(false);
			this.tableLayoutPanel8.PerformLayout();
			this.tpgEqualizer.ResumeLayout(false);
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			this.tlpEqualizer.ResumeLayout(false);
			this.tpgAdvanced.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tpgGeneral;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.Label lblVolumeReductionSettings;
		private System.Windows.Forms.CheckBox chkEnableAudio;
		private System.Windows.Forms.Label lblSampleRate;
		private System.Windows.Forms.Label lblAudioLatency;
		private System.Windows.Forms.ComboBox cboSampleRate;
		private System.Windows.Forms.Label lblAudioDevice;
		private System.Windows.Forms.ComboBox cboAudioDevice;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel7;
		private System.Windows.Forms.Label lblLatencyWarning;
		private System.Windows.Forms.PictureBox picLatencyWarning;
		private System.Windows.Forms.Label lblLatencyMs;
		private Controls.MesenNumericUpDown nudLatency;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel8;
		private System.Windows.Forms.CheckBox chkReduceSoundInBackground;
		private System.Windows.Forms.CheckBox chkReduceSoundInFastForward;
		private Controls.ctrlHorizontalTrackbar trkVolumeReduction;
		private System.Windows.Forms.CheckBox chkMuteSoundInBackground;
		private System.Windows.Forms.TabPage tpgEqualizer;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.CheckBox chkEnableEqualizer;
		private System.Windows.Forms.TableLayoutPanel tlpEqualizer;
		private Controls.ctrlTrackbar trkBand6Gain;
		private Controls.ctrlTrackbar trkBand5Gain;
		private Controls.ctrlTrackbar trkBand4Gain;
		private Controls.ctrlTrackbar trkBand3Gain;
		private Controls.ctrlTrackbar trkBand2Gain;
		private Controls.ctrlTrackbar trkBand1Gain;
		private Controls.ctrlTrackbar trkBand11Gain;
		private Controls.ctrlTrackbar trkBand12Gain;
		private Controls.ctrlTrackbar trkBand13Gain;
		private Controls.ctrlTrackbar trkBand14Gain;
		private Controls.ctrlTrackbar trkBand15Gain;
		private Controls.ctrlTrackbar trkBand16Gain;
		private Controls.ctrlTrackbar trkBand7Gain;
		private Controls.ctrlTrackbar trkBand8Gain;
		private Controls.ctrlTrackbar trkBand9Gain;
		private Controls.ctrlTrackbar trkBand10Gain;
		private Controls.ctrlTrackbar trkBand17Gain;
		private Controls.ctrlTrackbar trkBand18Gain;
		private Controls.ctrlTrackbar trkBand19Gain;
		private Controls.ctrlTrackbar trkBand20Gain;
		private Controls.ctrlHorizontalTrackbar trkMaster;
		private System.Windows.Forms.Label lblVolume;
		private System.Windows.Forms.TabPage tpgAdvanced;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private Controls.ctrlRiskyOption chkDisableDynamicSampleRate;
	}
}