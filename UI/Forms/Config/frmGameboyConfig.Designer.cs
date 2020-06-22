namespace Mesen.GUI.Forms.Config
{
	partial class frmGameboyConfig
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
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.lblModel = new System.Windows.Forms.Label();
			this.cboGameboyModel = new System.Windows.Forms.ComboBox();
			this.chkUseSgb2 = new System.Windows.Forms.CheckBox();
			this.tpgVideo = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel7 = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel8 = new System.Windows.Forms.TableLayoutPanel();
			this.lblGbObj0 = new System.Windows.Forms.Label();
			this.picGbBgPal0 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.picGbBgPal1 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.picGbBgPal2 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.picGbBgPal3 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.btnSelectPreset = new System.Windows.Forms.Button();
			this.picGbObj0Pal0 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.picGbObj0Pal1 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.picGbObj0Pal2 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.picGbObj0Pal3 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.picGbObj1Pal0 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.picGbObj1Pal1 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.picGbObj1Pal2 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.picGbObj1Pal3 = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.lblGbBackground = new System.Windows.Forms.Label();
			this.lblGbObj1 = new System.Windows.Forms.Label();
			this.chkGbcAdjustColors = new System.Windows.Forms.CheckBox();
			this.chkGbBlendFrames = new System.Windows.Forms.CheckBox();
			this.lblGameboyPalette = new System.Windows.Forms.Label();
			this.lblLcdSettings = new System.Windows.Forms.Label();
			this.ctxGbColorPresets = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuGbColorPresetGrayscale = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGbColorPresetGreen = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGbColorPresetBrown = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGbColorPresetGrayscaleConstrast = new System.Windows.Forms.ToolStripMenuItem();
			this.tabMain.SuspendLayout();
			this.tpgGeneral.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.tpgVideo.SuspendLayout();
			this.tableLayoutPanel7.SuspendLayout();
			this.tableLayoutPanel8.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picGbBgPal0)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbBgPal1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbBgPal2)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbBgPal3)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj0Pal0)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj0Pal1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj0Pal2)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj0Pal3)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj1Pal0)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj1Pal1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj1Pal2)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj1Pal3)).BeginInit();
			this.ctxGbColorPresets.SuspendLayout();
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
			this.tabMain.Controls.Add(this.tpgVideo);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(445, 290);
			this.tabMain.TabIndex = 2;
			// 
			// tpgGeneral
			// 
			this.tpgGeneral.Controls.Add(this.tableLayoutPanel1);
			this.tpgGeneral.Location = new System.Drawing.Point(4, 22);
			this.tpgGeneral.Name = "tpgGeneral";
			this.tpgGeneral.Padding = new System.Windows.Forms.Padding(3);
			this.tpgGeneral.Size = new System.Drawing.Size(437, 264);
			this.tpgGeneral.TabIndex = 2;
			this.tpgGeneral.Text = "General";
			this.tpgGeneral.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.lblModel, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.cboGameboyModel, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.chkUseSgb2, 0, 1);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 3;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(431, 258);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// lblModel
			// 
			this.lblModel.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblModel.AutoSize = true;
			this.lblModel.Location = new System.Drawing.Point(3, 7);
			this.lblModel.Name = "lblModel";
			this.lblModel.Size = new System.Drawing.Size(39, 13);
			this.lblModel.TabIndex = 0;
			this.lblModel.Text = "Model:";
			// 
			// cboGameboyModel
			// 
			this.cboGameboyModel.FormattingEnabled = true;
			this.cboGameboyModel.Location = new System.Drawing.Point(48, 3);
			this.cboGameboyModel.Name = "cboGameboyModel";
			this.cboGameboyModel.Size = new System.Drawing.Size(119, 21);
			this.cboGameboyModel.TabIndex = 1;
			// 
			// chkUseSgb2
			// 
			this.chkUseSgb2.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.chkUseSgb2, 2);
			this.chkUseSgb2.Location = new System.Drawing.Point(3, 30);
			this.chkUseSgb2.Name = "chkUseSgb2";
			this.chkUseSgb2.Size = new System.Drawing.Size(237, 17);
			this.chkUseSgb2.TabIndex = 2;
			this.chkUseSgb2.Text = "Use Super Game Boy 2 timings and behavior";
			this.chkUseSgb2.UseVisualStyleBackColor = true;
			// 
			// tpgVideo
			// 
			this.tpgVideo.Controls.Add(this.tableLayoutPanel7);
			this.tpgVideo.Location = new System.Drawing.Point(4, 22);
			this.tpgVideo.Name = "tpgVideo";
			this.tpgVideo.Padding = new System.Windows.Forms.Padding(3);
			this.tpgVideo.Size = new System.Drawing.Size(437, 264);
			this.tpgVideo.TabIndex = 6;
			this.tpgVideo.Text = "Video";
			this.tpgVideo.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel7
			// 
			this.tableLayoutPanel7.ColumnCount = 1;
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel7.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel7.Controls.Add(this.tableLayoutPanel8, 0, 1);
			this.tableLayoutPanel7.Controls.Add(this.chkGbcAdjustColors, 0, 3);
			this.tableLayoutPanel7.Controls.Add(this.chkGbBlendFrames, 0, 4);
			this.tableLayoutPanel7.Controls.Add(this.lblGameboyPalette, 0, 0);
			this.tableLayoutPanel7.Controls.Add(this.lblLcdSettings, 0, 2);
			this.tableLayoutPanel7.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel7.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel7.Name = "tableLayoutPanel7";
			this.tableLayoutPanel7.RowCount = 6;
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel7.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel7.Size = new System.Drawing.Size(431, 258);
			this.tableLayoutPanel7.TabIndex = 0;
			// 
			// tableLayoutPanel8
			// 
			this.tableLayoutPanel8.ColumnCount = 6;
			this.tableLayoutPanel8.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel8.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel8.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel8.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel8.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel8.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel8.Controls.Add(this.lblGbObj0, 0, 1);
			this.tableLayoutPanel8.Controls.Add(this.picGbBgPal0, 1, 0);
			this.tableLayoutPanel8.Controls.Add(this.picGbBgPal1, 2, 0);
			this.tableLayoutPanel8.Controls.Add(this.picGbBgPal2, 3, 0);
			this.tableLayoutPanel8.Controls.Add(this.picGbBgPal3, 4, 0);
			this.tableLayoutPanel8.Controls.Add(this.btnSelectPreset, 5, 0);
			this.tableLayoutPanel8.Controls.Add(this.picGbObj0Pal0, 1, 1);
			this.tableLayoutPanel8.Controls.Add(this.picGbObj0Pal1, 2, 1);
			this.tableLayoutPanel8.Controls.Add(this.picGbObj0Pal2, 3, 1);
			this.tableLayoutPanel8.Controls.Add(this.picGbObj0Pal3, 4, 1);
			this.tableLayoutPanel8.Controls.Add(this.picGbObj1Pal0, 1, 2);
			this.tableLayoutPanel8.Controls.Add(this.picGbObj1Pal1, 2, 2);
			this.tableLayoutPanel8.Controls.Add(this.picGbObj1Pal2, 3, 2);
			this.tableLayoutPanel8.Controls.Add(this.picGbObj1Pal3, 4, 2);
			this.tableLayoutPanel8.Controls.Add(this.lblGbBackground, 0, 0);
			this.tableLayoutPanel8.Controls.Add(this.lblGbObj1, 0, 2);
			this.tableLayoutPanel8.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel8.Location = new System.Drawing.Point(0, 20);
			this.tableLayoutPanel8.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel8.Name = "tableLayoutPanel8";
			this.tableLayoutPanel8.RowCount = 4;
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel8.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel8.Size = new System.Drawing.Size(431, 83);
			this.tableLayoutPanel8.TabIndex = 7;
			// 
			// lblGbObj0
			// 
			this.lblGbObj0.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblGbObj0.AutoSize = true;
			this.lblGbObj0.Location = new System.Drawing.Point(3, 36);
			this.lblGbObj0.Name = "lblGbObj0";
			this.lblGbObj0.Size = new System.Drawing.Size(58, 13);
			this.lblGbObj0.TabIndex = 21;
			this.lblGbObj0.Text = "Sprites #1:";
			// 
			// picGbBgPal0
			// 
			this.picGbBgPal0.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbBgPal0.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbBgPal0.Location = new System.Drawing.Point(77, 3);
			this.picGbBgPal0.Name = "picGbBgPal0";
			this.picGbBgPal0.Size = new System.Drawing.Size(21, 21);
			this.picGbBgPal0.TabIndex = 10;
			this.picGbBgPal0.TabStop = false;
			// 
			// picGbBgPal1
			// 
			this.picGbBgPal1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbBgPal1.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbBgPal1.Location = new System.Drawing.Point(104, 3);
			this.picGbBgPal1.Name = "picGbBgPal1";
			this.picGbBgPal1.Size = new System.Drawing.Size(21, 21);
			this.picGbBgPal1.TabIndex = 7;
			this.picGbBgPal1.TabStop = false;
			// 
			// picGbBgPal2
			// 
			this.picGbBgPal2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbBgPal2.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbBgPal2.Location = new System.Drawing.Point(131, 3);
			this.picGbBgPal2.Name = "picGbBgPal2";
			this.picGbBgPal2.Size = new System.Drawing.Size(21, 21);
			this.picGbBgPal2.TabIndex = 8;
			this.picGbBgPal2.TabStop = false;
			// 
			// picGbBgPal3
			// 
			this.picGbBgPal3.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbBgPal3.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbBgPal3.Location = new System.Drawing.Point(158, 3);
			this.picGbBgPal3.Name = "picGbBgPal3";
			this.picGbBgPal3.Size = new System.Drawing.Size(21, 21);
			this.picGbBgPal3.TabIndex = 9;
			this.picGbBgPal3.TabStop = false;
			// 
			// btnSelectPreset
			// 
			this.btnSelectPreset.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.btnSelectPreset.AutoSize = true;
			this.btnSelectPreset.Image = global::Mesen.GUI.Properties.Resources.DownArrow;
			this.btnSelectPreset.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
			this.btnSelectPreset.Location = new System.Drawing.Point(185, 3);
			this.btnSelectPreset.Name = "btnSelectPreset";
			this.btnSelectPreset.Padding = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.btnSelectPreset.Size = new System.Drawing.Size(99, 23);
			this.btnSelectPreset.TabIndex = 11;
			this.btnSelectPreset.Text = "Select Preset...";
			this.btnSelectPreset.TextImageRelation = System.Windows.Forms.TextImageRelation.TextBeforeImage;
			this.btnSelectPreset.UseVisualStyleBackColor = true;
			this.btnSelectPreset.Click += new System.EventHandler(this.btnSelectPreset_Click);
			// 
			// picGbObj0Pal0
			// 
			this.picGbObj0Pal0.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbObj0Pal0.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbObj0Pal0.Location = new System.Drawing.Point(77, 32);
			this.picGbObj0Pal0.Name = "picGbObj0Pal0";
			this.picGbObj0Pal0.Size = new System.Drawing.Size(21, 21);
			this.picGbObj0Pal0.TabIndex = 12;
			this.picGbObj0Pal0.TabStop = false;
			// 
			// picGbObj0Pal1
			// 
			this.picGbObj0Pal1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbObj0Pal1.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbObj0Pal1.Location = new System.Drawing.Point(104, 32);
			this.picGbObj0Pal1.Name = "picGbObj0Pal1";
			this.picGbObj0Pal1.Size = new System.Drawing.Size(21, 21);
			this.picGbObj0Pal1.TabIndex = 13;
			this.picGbObj0Pal1.TabStop = false;
			// 
			// picGbObj0Pal2
			// 
			this.picGbObj0Pal2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbObj0Pal2.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbObj0Pal2.Location = new System.Drawing.Point(131, 32);
			this.picGbObj0Pal2.Name = "picGbObj0Pal2";
			this.picGbObj0Pal2.Size = new System.Drawing.Size(21, 21);
			this.picGbObj0Pal2.TabIndex = 14;
			this.picGbObj0Pal2.TabStop = false;
			// 
			// picGbObj0Pal3
			// 
			this.picGbObj0Pal3.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbObj0Pal3.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbObj0Pal3.Location = new System.Drawing.Point(158, 32);
			this.picGbObj0Pal3.Name = "picGbObj0Pal3";
			this.picGbObj0Pal3.Size = new System.Drawing.Size(21, 21);
			this.picGbObj0Pal3.TabIndex = 15;
			this.picGbObj0Pal3.TabStop = false;
			// 
			// picGbObj1Pal0
			// 
			this.picGbObj1Pal0.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbObj1Pal0.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbObj1Pal0.Location = new System.Drawing.Point(77, 59);
			this.picGbObj1Pal0.Name = "picGbObj1Pal0";
			this.picGbObj1Pal0.Size = new System.Drawing.Size(21, 21);
			this.picGbObj1Pal0.TabIndex = 16;
			this.picGbObj1Pal0.TabStop = false;
			// 
			// picGbObj1Pal1
			// 
			this.picGbObj1Pal1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbObj1Pal1.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbObj1Pal1.Location = new System.Drawing.Point(104, 59);
			this.picGbObj1Pal1.Name = "picGbObj1Pal1";
			this.picGbObj1Pal1.Size = new System.Drawing.Size(21, 21);
			this.picGbObj1Pal1.TabIndex = 17;
			this.picGbObj1Pal1.TabStop = false;
			// 
			// picGbObj1Pal2
			// 
			this.picGbObj1Pal2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbObj1Pal2.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbObj1Pal2.Location = new System.Drawing.Point(131, 59);
			this.picGbObj1Pal2.Name = "picGbObj1Pal2";
			this.picGbObj1Pal2.Size = new System.Drawing.Size(21, 21);
			this.picGbObj1Pal2.TabIndex = 18;
			this.picGbObj1Pal2.TabStop = false;
			// 
			// picGbObj1Pal3
			// 
			this.picGbObj1Pal3.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picGbObj1Pal3.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picGbObj1Pal3.Location = new System.Drawing.Point(158, 59);
			this.picGbObj1Pal3.Name = "picGbObj1Pal3";
			this.picGbObj1Pal3.Size = new System.Drawing.Size(21, 21);
			this.picGbObj1Pal3.TabIndex = 19;
			this.picGbObj1Pal3.TabStop = false;
			// 
			// lblGbBackground
			// 
			this.lblGbBackground.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblGbBackground.AutoSize = true;
			this.lblGbBackground.Location = new System.Drawing.Point(3, 8);
			this.lblGbBackground.Name = "lblGbBackground";
			this.lblGbBackground.Size = new System.Drawing.Size(68, 13);
			this.lblGbBackground.TabIndex = 20;
			this.lblGbBackground.Text = "Background:";
			// 
			// lblGbObj1
			// 
			this.lblGbObj1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblGbObj1.AutoSize = true;
			this.lblGbObj1.Location = new System.Drawing.Point(3, 63);
			this.lblGbObj1.Name = "lblGbObj1";
			this.lblGbObj1.Size = new System.Drawing.Size(58, 13);
			this.lblGbObj1.TabIndex = 22;
			this.lblGbObj1.Text = "Sprites #2:";
			// 
			// chkGbcAdjustColors
			// 
			this.chkGbcAdjustColors.AutoSize = true;
			this.chkGbcAdjustColors.Location = new System.Drawing.Point(3, 126);
			this.chkGbcAdjustColors.Name = "chkGbcAdjustColors";
			this.chkGbcAdjustColors.Size = new System.Drawing.Size(182, 17);
			this.chkGbcAdjustColors.TabIndex = 4;
			this.chkGbcAdjustColors.Text = "Enable GBC LCD color emulation";
			this.chkGbcAdjustColors.UseVisualStyleBackColor = true;
			// 
			// chkGbBlendFrames
			// 
			this.chkGbBlendFrames.AutoSize = true;
			this.chkGbBlendFrames.Location = new System.Drawing.Point(3, 149);
			this.chkGbBlendFrames.Name = "chkGbBlendFrames";
			this.chkGbBlendFrames.Size = new System.Drawing.Size(155, 17);
			this.chkGbBlendFrames.TabIndex = 3;
			this.chkGbBlendFrames.Text = "Enable LCD frame blending";
			this.chkGbBlendFrames.UseVisualStyleBackColor = true;
			// 
			// lblGameboyPalette
			// 
			this.lblGameboyPalette.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblGameboyPalette.AutoSize = true;
			this.lblGameboyPalette.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblGameboyPalette.Location = new System.Drawing.Point(0, 7);
			this.lblGameboyPalette.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblGameboyPalette.Name = "lblGameboyPalette";
			this.lblGameboyPalette.Size = new System.Drawing.Size(126, 13);
			this.lblGameboyPalette.TabIndex = 27;
			this.lblGameboyPalette.Text = "Game Boy (DMG) Palette";
			// 
			// lblLcdSettings
			// 
			this.lblLcdSettings.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblLcdSettings.AutoSize = true;
			this.lblLcdSettings.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblLcdSettings.Location = new System.Drawing.Point(0, 110);
			this.lblLcdSettings.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblLcdSettings.Name = "lblLcdSettings";
			this.lblLcdSettings.Size = new System.Drawing.Size(69, 13);
			this.lblLcdSettings.TabIndex = 28;
			this.lblLcdSettings.Text = "LCD Settings";
			// 
			// ctxGbColorPresets
			// 
			this.ctxGbColorPresets.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuGbColorPresetGrayscale,
            this.mnuGbColorPresetGrayscaleConstrast,
            this.mnuGbColorPresetGreen,
            this.mnuGbColorPresetBrown});
			this.ctxGbColorPresets.Name = "contextPicturePresets";
			this.ctxGbColorPresets.Size = new System.Drawing.Size(208, 114);
			// 
			// mnuGbColorPresetGrayscale
			// 
			this.mnuGbColorPresetGrayscale.Name = "mnuGbColorPresetGrayscale";
			this.mnuGbColorPresetGrayscale.Size = new System.Drawing.Size(207, 22);
			this.mnuGbColorPresetGrayscale.Text = "Grayscale";
			this.mnuGbColorPresetGrayscale.Click += new System.EventHandler(this.mnuGbColorPresetGrayscale_Click);
			// 
			// mnuGbColorPresetGreen
			// 
			this.mnuGbColorPresetGreen.Name = "mnuGbColorPresetGreen";
			this.mnuGbColorPresetGreen.Size = new System.Drawing.Size(207, 22);
			this.mnuGbColorPresetGreen.Text = "Green";
			this.mnuGbColorPresetGreen.Click += new System.EventHandler(this.mnuGbColorPresetGreen_Click);
			// 
			// mnuGbColorPresetBrown
			// 
			this.mnuGbColorPresetBrown.Name = "mnuGbColorPresetBrown";
			this.mnuGbColorPresetBrown.Size = new System.Drawing.Size(207, 22);
			this.mnuGbColorPresetBrown.Text = "Brown";
			this.mnuGbColorPresetBrown.Click += new System.EventHandler(this.mnuGbColorPresetBrown_Click);
			// 
			// mnuGbColorPresetGrayscaleConstrast
			// 
			this.mnuGbColorPresetGrayscaleConstrast.Name = "mnuGbColorPresetGrayscaleConstrast";
			this.mnuGbColorPresetGrayscaleConstrast.Size = new System.Drawing.Size(207, 22);
			this.mnuGbColorPresetGrayscaleConstrast.Text = "Grayscale (High contrast)";
			this.mnuGbColorPresetGrayscaleConstrast.Click += new System.EventHandler(this.mnuGbColorPresetGrayscaleConstrast_Click);
			// 
			// frmGameboyConfig
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(445, 319);
			this.Controls.Add(this.tabMain);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "frmGameboyConfig";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Game Boy Config";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tabMain, 0);
			this.tabMain.ResumeLayout(false);
			this.tpgGeneral.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.tpgVideo.ResumeLayout(false);
			this.tableLayoutPanel7.ResumeLayout(false);
			this.tableLayoutPanel7.PerformLayout();
			this.tableLayoutPanel8.ResumeLayout(false);
			this.tableLayoutPanel8.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picGbBgPal0)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbBgPal1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbBgPal2)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbBgPal3)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj0Pal0)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj0Pal1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj0Pal2)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj0Pal3)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj1Pal0)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj1Pal1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj1Pal2)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picGbObj1Pal3)).EndInit();
			this.ctxGbColorPresets.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tpgGeneral;
	  private System.Windows.Forms.TabPage tpgVideo;
	  private System.Windows.Forms.TableLayoutPanel tableLayoutPanel7;
	  private System.Windows.Forms.Label lblModel;
	  private System.Windows.Forms.ComboBox cboGameboyModel;
	  private System.Windows.Forms.CheckBox chkUseSgb2;
	  private System.Windows.Forms.CheckBox chkGbBlendFrames;
	  private System.Windows.Forms.CheckBox chkGbcAdjustColors;
	  private System.Windows.Forms.TableLayoutPanel tableLayoutPanel8;
	  private Debugger.ctrlColorPicker picGbBgPal1;
	  private Debugger.ctrlColorPicker picGbBgPal0;
	  private Debugger.ctrlColorPicker picGbBgPal2;
	  private Debugger.ctrlColorPicker picGbBgPal3;
	  private System.Windows.Forms.Button btnSelectPreset;
	  private System.Windows.Forms.ContextMenuStrip ctxGbColorPresets;
	  private System.Windows.Forms.ToolStripMenuItem mnuGbColorPresetGreen;
	  private System.Windows.Forms.ToolStripMenuItem mnuGbColorPresetGrayscale;
	  private System.Windows.Forms.ToolStripMenuItem mnuGbColorPresetBrown;
	  private System.Windows.Forms.Label lblGbObj0;
	  private Debugger.ctrlColorPicker picGbObj0Pal0;
	  private Debugger.ctrlColorPicker picGbObj0Pal1;
	  private Debugger.ctrlColorPicker picGbObj0Pal2;
	  private Debugger.ctrlColorPicker picGbObj0Pal3;
	  private Debugger.ctrlColorPicker picGbObj1Pal0;
	  private Debugger.ctrlColorPicker picGbObj1Pal1;
	  private Debugger.ctrlColorPicker picGbObj1Pal2;
	  private Debugger.ctrlColorPicker picGbObj1Pal3;
	  private System.Windows.Forms.Label lblGbBackground;
	  private System.Windows.Forms.Label lblGbObj1;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
	  private System.Windows.Forms.Label lblGameboyPalette;
	  private System.Windows.Forms.Label lblLcdSettings;
	  private System.Windows.Forms.ToolStripMenuItem mnuGbColorPresetGrayscaleConstrast;
   }
}