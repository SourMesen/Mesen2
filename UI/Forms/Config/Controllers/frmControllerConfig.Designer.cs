namespace Mesen.GUI.Forms.Config
{
	partial class frmControllerConfig
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(frmControllerConfig));
			this.tabMain = new System.Windows.Forms.TabControl();
			this.tpgSet1 = new System.Windows.Forms.TabPage();
			this.ctrlController0 = new Mesen.GUI.Forms.Config.ctrlStandardController();
			this.tpgSet2 = new System.Windows.Forms.TabPage();
			this.ctrlController1 = new Mesen.GUI.Forms.Config.ctrlStandardController();
			this.tpgSet3 = new System.Windows.Forms.TabPage();
			this.ctrlController2 = new Mesen.GUI.Forms.Config.ctrlStandardController();
			this.tpgSet4 = new System.Windows.Forms.TabPage();
			this.ctrlController3 = new Mesen.GUI.Forms.Config.ctrlStandardController();
			this.imageList = new System.Windows.Forms.ImageList(this.components);
			this.btnClear = new System.Windows.Forms.Button();
			this.flowLayoutPanel2 = new System.Windows.Forms.FlowLayoutPanel();
			this.tlpMain = new System.Windows.Forms.TableLayoutPanel();
			this.tlpStandardController = new System.Windows.Forms.TableLayoutPanel();
			this.panel1 = new System.Windows.Forms.Panel();
			this.lblTurboFast = new System.Windows.Forms.Label();
			this.lblSlow = new System.Windows.Forms.Label();
			this.trkTurboSpeed = new System.Windows.Forms.TrackBar();
			this.lblTurboSpeed = new System.Windows.Forms.Label();
			this.btnSelectPreset = new System.Windows.Forms.Button();
			this.mnuStripPreset = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuKeyboard = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuWasdLayout = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuArrowLayout = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuXboxController = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuXboxLayout1 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuXboxLayout2 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPs4Controller = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPs4Layout1 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPs4Layout2 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuSnes30Controller = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuSnes30Layout1 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuSnes30Layout2 = new System.Windows.Forms.ToolStripMenuItem();
			this.ctrlKeyBindingHint1 = new Mesen.GUI.Forms.Config.ctrlKeyBindingHint();
			this.baseConfigPanel.SuspendLayout();
			this.tabMain.SuspendLayout();
			this.tpgSet1.SuspendLayout();
			this.tpgSet2.SuspendLayout();
			this.tpgSet3.SuspendLayout();
			this.tpgSet4.SuspendLayout();
			this.flowLayoutPanel2.SuspendLayout();
			this.tlpMain.SuspendLayout();
			this.tlpStandardController.SuspendLayout();
			this.panel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.trkTurboSpeed)).BeginInit();
			this.mnuStripPreset.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Controls.Add(this.flowLayoutPanel2);
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 389);
			this.baseConfigPanel.Size = new System.Drawing.Size(628, 29);
			this.baseConfigPanel.TabIndex = 4;
			this.baseConfigPanel.Controls.SetChildIndex(this.flowLayoutPanel2, 0);
			// 
			// tabMain
			// 
			this.tabMain.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tlpMain.SetColumnSpan(this.tabMain, 3);
			this.tabMain.Controls.Add(this.tpgSet1);
			this.tabMain.Controls.Add(this.tpgSet2);
			this.tabMain.Controls.Add(this.tpgSet3);
			this.tabMain.Controls.Add(this.tpgSet4);
			this.tabMain.ImageList = this.imageList;
			this.tabMain.Location = new System.Drawing.Point(3, 3);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(622, 306);
			this.tabMain.TabIndex = 3;
			// 
			// tpgSet1
			// 
			this.tpgSet1.Controls.Add(this.ctrlController0);
			this.tpgSet1.Location = new System.Drawing.Point(4, 23);
			this.tpgSet1.Name = "tpgSet1";
			this.tpgSet1.Size = new System.Drawing.Size(614, 279);
			this.tpgSet1.TabIndex = 0;
			this.tpgSet1.Text = "Key Set #1";
			this.tpgSet1.UseVisualStyleBackColor = true;
			// 
			// ctrlController0
			// 
			this.ctrlController0.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlController0.Location = new System.Drawing.Point(0, 0);
			this.ctrlController0.Name = "ctrlController0";
			this.ctrlController0.PortNumber = 0;
			this.ctrlController0.Size = new System.Drawing.Size(614, 279);
			this.ctrlController0.TabIndex = 0;
			// 
			// tpgSet2
			// 
			this.tpgSet2.Controls.Add(this.ctrlController1);
			this.tpgSet2.Location = new System.Drawing.Point(4, 23);
			this.tpgSet2.Name = "tpgSet2";
			this.tpgSet2.Size = new System.Drawing.Size(614, 279);
			this.tpgSet2.TabIndex = 1;
			this.tpgSet2.Text = "Key Set #2";
			this.tpgSet2.UseVisualStyleBackColor = true;
			// 
			// ctrlController1
			// 
			this.ctrlController1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlController1.Location = new System.Drawing.Point(0, 0);
			this.ctrlController1.Name = "ctrlController1";
			this.ctrlController1.PortNumber = 0;
			this.ctrlController1.Size = new System.Drawing.Size(614, 279);
			this.ctrlController1.TabIndex = 1;
			// 
			// tpgSet3
			// 
			this.tpgSet3.Controls.Add(this.ctrlController2);
			this.tpgSet3.Location = new System.Drawing.Point(4, 23);
			this.tpgSet3.Name = "tpgSet3";
			this.tpgSet3.Size = new System.Drawing.Size(614, 279);
			this.tpgSet3.TabIndex = 2;
			this.tpgSet3.Text = "Key Set #3";
			this.tpgSet3.UseVisualStyleBackColor = true;
			// 
			// ctrlController2
			// 
			this.ctrlController2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlController2.Location = new System.Drawing.Point(0, 0);
			this.ctrlController2.Name = "ctrlController2";
			this.ctrlController2.PortNumber = 0;
			this.ctrlController2.Size = new System.Drawing.Size(614, 279);
			this.ctrlController2.TabIndex = 1;
			// 
			// tpgSet4
			// 
			this.tpgSet4.Controls.Add(this.ctrlController3);
			this.tpgSet4.Location = new System.Drawing.Point(4, 23);
			this.tpgSet4.Name = "tpgSet4";
			this.tpgSet4.Size = new System.Drawing.Size(614, 279);
			this.tpgSet4.TabIndex = 3;
			this.tpgSet4.Text = "Key Set #4";
			this.tpgSet4.UseVisualStyleBackColor = true;
			// 
			// ctrlController3
			// 
			this.ctrlController3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlController3.Location = new System.Drawing.Point(0, 0);
			this.ctrlController3.Name = "ctrlController3";
			this.ctrlController3.PortNumber = 0;
			this.ctrlController3.Size = new System.Drawing.Size(614, 279);
			this.ctrlController3.TabIndex = 1;
			// 
			// imageList
			// 
			this.imageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList.ImageStream")));
			this.imageList.TransparentColor = System.Drawing.Color.Transparent;
			this.imageList.Images.SetKeyName(0, "Keyboard");
			this.imageList.Images.SetKeyName(1, "Controller");
			// 
			// btnClear
			// 
			this.btnClear.AutoSize = true;
			this.btnClear.Location = new System.Drawing.Point(3, 3);
			this.btnClear.Name = "btnClear";
			this.btnClear.Size = new System.Drawing.Size(105, 23);
			this.btnClear.TabIndex = 3;
			this.btnClear.Text = "Clear Key Bindings";
			this.btnClear.UseVisualStyleBackColor = true;
			this.btnClear.Click += new System.EventHandler(this.btnClear_Click);
			// 
			// flowLayoutPanel2
			// 
			this.flowLayoutPanel2.Controls.Add(this.btnClear);
			this.flowLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Left;
			this.flowLayoutPanel2.Location = new System.Drawing.Point(0, 0);
			this.flowLayoutPanel2.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel2.Name = "flowLayoutPanel2";
			this.flowLayoutPanel2.Size = new System.Drawing.Size(264, 29);
			this.flowLayoutPanel2.TabIndex = 5;
			// 
			// tlpMain
			// 
			this.tlpMain.ColumnCount = 3;
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpMain.Controls.Add(this.tabMain, 0, 1);
			this.tlpMain.Controls.Add(this.tlpStandardController, 0, 2);
			this.tlpMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpMain.Location = new System.Drawing.Point(0, 31);
			this.tlpMain.Margin = new System.Windows.Forms.Padding(0);
			this.tlpMain.Name = "tlpMain";
			this.tlpMain.RowCount = 3;
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpMain.Size = new System.Drawing.Size(628, 358);
			this.tlpMain.TabIndex = 23;
			// 
			// tlpStandardController
			// 
			this.tlpStandardController.ColumnCount = 3;
			this.tlpMain.SetColumnSpan(this.tlpStandardController, 3);
			this.tlpStandardController.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpStandardController.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpStandardController.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpStandardController.Controls.Add(this.panel1, 2, 1);
			this.tlpStandardController.Controls.Add(this.trkTurboSpeed, 2, 0);
			this.tlpStandardController.Controls.Add(this.lblTurboSpeed, 1, 0);
			this.tlpStandardController.Controls.Add(this.btnSelectPreset, 0, 0);
			this.tlpStandardController.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpStandardController.Location = new System.Drawing.Point(0, 312);
			this.tlpStandardController.Margin = new System.Windows.Forms.Padding(0);
			this.tlpStandardController.Name = "tlpStandardController";
			this.tlpStandardController.RowCount = 2;
			this.tlpStandardController.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 32F));
			this.tlpStandardController.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpStandardController.Size = new System.Drawing.Size(628, 46);
			this.tlpStandardController.TabIndex = 6;
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.lblTurboFast);
			this.panel1.Controls.Add(this.lblSlow);
			this.panel1.Location = new System.Drawing.Point(505, 32);
			this.panel1.Margin = new System.Windows.Forms.Padding(0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(120, 14);
			this.panel1.TabIndex = 2;
			// 
			// lblTurboFast
			// 
			this.lblTurboFast.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.lblTurboFast.Location = new System.Drawing.Point(70, 0);
			this.lblTurboFast.Name = "lblTurboFast";
			this.lblTurboFast.Size = new System.Drawing.Size(47, 15);
			this.lblTurboFast.TabIndex = 1;
			this.lblTurboFast.Text = "Fast";
			this.lblTurboFast.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// lblSlow
			// 
			this.lblSlow.AutoSize = true;
			this.lblSlow.Location = new System.Drawing.Point(3, 0);
			this.lblSlow.Name = "lblSlow";
			this.lblSlow.Size = new System.Drawing.Size(30, 13);
			this.lblSlow.TabIndex = 0;
			this.lblSlow.Text = "Slow";
			// 
			// trkTurboSpeed
			// 
			this.trkTurboSpeed.LargeChange = 2;
			this.trkTurboSpeed.Location = new System.Drawing.Point(508, 3);
			this.trkTurboSpeed.Maximum = 3;
			this.trkTurboSpeed.Name = "trkTurboSpeed";
			this.trkTurboSpeed.Size = new System.Drawing.Size(117, 26);
			this.trkTurboSpeed.TabIndex = 0;
			// 
			// lblTurboSpeed
			// 
			this.lblTurboSpeed.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblTurboSpeed.AutoSize = true;
			this.lblTurboSpeed.Location = new System.Drawing.Point(430, 9);
			this.lblTurboSpeed.Name = "lblTurboSpeed";
			this.lblTurboSpeed.Size = new System.Drawing.Size(72, 13);
			this.lblTurboSpeed.TabIndex = 1;
			this.lblTurboSpeed.Text = "Turbo Speed:";
			// 
			// btnSelectPreset
			// 
			this.btnSelectPreset.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.btnSelectPreset.AutoSize = true;
			this.btnSelectPreset.Image = global::Mesen.GUI.Properties.Resources.DownArrow;
			this.btnSelectPreset.Location = new System.Drawing.Point(3, 4);
			this.btnSelectPreset.Name = "btnSelectPreset";
			this.btnSelectPreset.Size = new System.Drawing.Size(105, 23);
			this.btnSelectPreset.TabIndex = 4;
			this.btnSelectPreset.Text = "Select Preset...";
			this.btnSelectPreset.TextImageRelation = System.Windows.Forms.TextImageRelation.TextBeforeImage;
			this.btnSelectPreset.UseVisualStyleBackColor = true;
			this.btnSelectPreset.Click += new System.EventHandler(this.btnSelectPreset_Click);
			// 
			// mnuStripPreset
			// 
			this.mnuStripPreset.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuKeyboard,
            this.toolStripMenuItem1,
            this.mnuXboxController,
            this.mnuPs4Controller,
            this.mnuSnes30Controller});
			this.mnuStripPreset.Name = "mnuStripPreset";
			this.mnuStripPreset.Size = new System.Drawing.Size(170, 98);
			// 
			// mnuKeyboard
			// 
			this.mnuKeyboard.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuWasdLayout,
            this.mnuArrowLayout});
			this.mnuKeyboard.Name = "mnuKeyboard";
			this.mnuKeyboard.Size = new System.Drawing.Size(169, 22);
			this.mnuKeyboard.Text = "Keyboard";
			// 
			// mnuWasdLayout
			// 
			this.mnuWasdLayout.Name = "mnuWasdLayout";
			this.mnuWasdLayout.Size = new System.Drawing.Size(172, 22);
			this.mnuWasdLayout.Text = "WASD Layout";
			this.mnuWasdLayout.Click += new System.EventHandler(this.mnuWasdLayout_Click);
			// 
			// mnuArrowLayout
			// 
			this.mnuArrowLayout.Name = "mnuArrowLayout";
			this.mnuArrowLayout.Size = new System.Drawing.Size(172, 22);
			this.mnuArrowLayout.Text = "Arrow Keys Layout";
			this.mnuArrowLayout.Click += new System.EventHandler(this.mnuArrowLayout_Click);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(166, 6);
			// 
			// mnuXboxController
			// 
			this.mnuXboxController.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuXboxLayout1,
            this.mnuXboxLayout2});
			this.mnuXboxController.Name = "mnuXboxController";
			this.mnuXboxController.Size = new System.Drawing.Size(169, 22);
			this.mnuXboxController.Text = "Xbox Controller";
			// 
			// mnuXboxLayout1
			// 
			this.mnuXboxLayout1.Name = "mnuXboxLayout1";
			this.mnuXboxLayout1.Size = new System.Drawing.Size(143, 22);
			this.mnuXboxLayout1.Text = "Controller #1";
			this.mnuXboxLayout1.Click += new System.EventHandler(this.mnuXboxLayout1_Click);
			// 
			// mnuXboxLayout2
			// 
			this.mnuXboxLayout2.Name = "mnuXboxLayout2";
			this.mnuXboxLayout2.Size = new System.Drawing.Size(143, 22);
			this.mnuXboxLayout2.Text = "Controller #2";
			this.mnuXboxLayout2.Click += new System.EventHandler(this.mnuXboxLayout2_Click);
			// 
			// mnuPs4Controller
			// 
			this.mnuPs4Controller.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuPs4Layout1,
            this.mnuPs4Layout2});
			this.mnuPs4Controller.Name = "mnuPs4Controller";
			this.mnuPs4Controller.Size = new System.Drawing.Size(169, 22);
			this.mnuPs4Controller.Text = "PS4 Controller";
			// 
			// mnuPs4Layout1
			// 
			this.mnuPs4Layout1.Name = "mnuPs4Layout1";
			this.mnuPs4Layout1.Size = new System.Drawing.Size(143, 22);
			this.mnuPs4Layout1.Text = "Controller #1";
			this.mnuPs4Layout1.Click += new System.EventHandler(this.mnuPs4Layout1_Click);
			// 
			// mnuPs4Layout2
			// 
			this.mnuPs4Layout2.Name = "mnuPs4Layout2";
			this.mnuPs4Layout2.Size = new System.Drawing.Size(143, 22);
			this.mnuPs4Layout2.Text = "Controller #2";
			this.mnuPs4Layout2.Click += new System.EventHandler(this.mnuPs4Layout2_Click);
			// 
			// mnuSnes30Controller
			// 
			this.mnuSnes30Controller.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuSnes30Layout1,
            this.mnuSnes30Layout2});
			this.mnuSnes30Controller.Name = "mnuSnes30Controller";
			this.mnuSnes30Controller.Size = new System.Drawing.Size(169, 22);
			this.mnuSnes30Controller.Text = "SNES30 Controller";
			// 
			// mnuSnes30Layout1
			// 
			this.mnuSnes30Layout1.Name = "mnuSnes30Layout1";
			this.mnuSnes30Layout1.Size = new System.Drawing.Size(143, 22);
			this.mnuSnes30Layout1.Text = "Controller #1";
			this.mnuSnes30Layout1.Click += new System.EventHandler(this.mnuSnes30Layout1_Click);
			// 
			// mnuSnes30Layout2
			// 
			this.mnuSnes30Layout2.Name = "mnuSnes30Layout2";
			this.mnuSnes30Layout2.Size = new System.Drawing.Size(143, 22);
			this.mnuSnes30Layout2.Text = "Controller #2";
			this.mnuSnes30Layout2.Click += new System.EventHandler(this.mnuSnes30Layout2_Click);
			// 
			// ctrlKeyBindingHint1
			// 
			this.ctrlKeyBindingHint1.Dock = System.Windows.Forms.DockStyle.Top;
			this.ctrlKeyBindingHint1.Location = new System.Drawing.Point(0, 0);
			this.ctrlKeyBindingHint1.Name = "ctrlKeyBindingHint1";
			this.ctrlKeyBindingHint1.Padding = new System.Windows.Forms.Padding(3, 0, 3, 0);
			this.ctrlKeyBindingHint1.Size = new System.Drawing.Size(628, 31);
			this.ctrlKeyBindingHint1.TabIndex = 24;
			// 
			// frmControllerConfig
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(628, 418);
			this.Controls.Add(this.tlpMain);
			this.Controls.Add(this.ctrlKeyBindingHint1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "frmControllerConfig";
			this.Text = "Standard Controller";
			this.Controls.SetChildIndex(this.ctrlKeyBindingHint1, 0);
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tlpMain, 0);
			this.baseConfigPanel.ResumeLayout(false);
			this.tabMain.ResumeLayout(false);
			this.tpgSet1.ResumeLayout(false);
			this.tpgSet2.ResumeLayout(false);
			this.tpgSet3.ResumeLayout(false);
			this.tpgSet4.ResumeLayout(false);
			this.flowLayoutPanel2.ResumeLayout(false);
			this.flowLayoutPanel2.PerformLayout();
			this.tlpMain.ResumeLayout(false);
			this.tlpStandardController.ResumeLayout(false);
			this.tlpStandardController.PerformLayout();
			this.panel1.ResumeLayout(false);
			this.panel1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.trkTurboSpeed)).EndInit();
			this.mnuStripPreset.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel2;
		private System.Windows.Forms.Button btnClear;
		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tpgSet1;
		private System.Windows.Forms.TabPage tpgSet2;
		private System.Windows.Forms.TabPage tpgSet3;
		private System.Windows.Forms.TabPage tpgSet4;
		private System.Windows.Forms.TableLayoutPanel tlpMain;
		private System.Windows.Forms.TrackBar trkTurboSpeed;
		private System.Windows.Forms.Label lblTurboSpeed;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Label lblTurboFast;
		private System.Windows.Forms.Label lblSlow;
		private System.Windows.Forms.Button btnSelectPreset;
		private System.Windows.Forms.ContextMenuStrip mnuStripPreset;
		private System.Windows.Forms.ToolStripMenuItem mnuKeyboard;
		private System.Windows.Forms.ToolStripMenuItem mnuWasdLayout;
		private System.Windows.Forms.ToolStripMenuItem mnuArrowLayout;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuXboxController;
		private System.Windows.Forms.ToolStripMenuItem mnuXboxLayout1;
		private System.Windows.Forms.ToolStripMenuItem mnuXboxLayout2;
		private System.Windows.Forms.ToolStripMenuItem mnuSnes30Controller;
		private System.Windows.Forms.ToolStripMenuItem mnuPs4Controller;
		private System.Windows.Forms.ToolStripMenuItem mnuPs4Layout1;
		private System.Windows.Forms.ToolStripMenuItem mnuPs4Layout2;
		private System.Windows.Forms.ToolStripMenuItem mnuSnes30Layout1;
		private System.Windows.Forms.ToolStripMenuItem mnuSnes30Layout2;
		private System.Windows.Forms.ImageList imageList;
		private System.Windows.Forms.TableLayoutPanel tlpStandardController;
		private ctrlStandardController ctrlController0;
		private ctrlStandardController ctrlController1;
		private ctrlStandardController ctrlController2;
		private ctrlStandardController ctrlController3;
		private ctrlKeyBindingHint ctrlKeyBindingHint1;
	}
}