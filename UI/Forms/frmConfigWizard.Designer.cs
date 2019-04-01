namespace Mesen.GUI.Forms
{
	partial class frmConfigWizard
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
			this.panel1 = new System.Windows.Forms.Panel();
			this.panel2 = new System.Windows.Forms.Panel();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.lblMiscOptions = new System.Windows.Forms.Label();
			this.picLogo = new System.Windows.Forms.PictureBox();
			this.lblStorageLocation = new System.Windows.Forms.Label();
			this.lblMesen = new System.Windows.Forms.Label();
			this.lblConfigWizard = new System.Windows.Forms.Label();
			this.lblInputMappings = new System.Windows.Forms.Label();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.chkArrows = new System.Windows.Forms.CheckBox();
			this.chkWasd = new System.Windows.Forms.CheckBox();
			this.chkPs4 = new System.Windows.Forms.CheckBox();
			this.picPs4 = new System.Windows.Forms.PictureBox();
			this.picXbox = new System.Windows.Forms.PictureBox();
			this.picWasd = new System.Windows.Forms.PictureBox();
			this.picArrows = new System.Windows.Forms.PictureBox();
			this.chkXbox = new System.Windows.Forms.CheckBox();
			this.lblInputHint = new System.Windows.Forms.Label();
			this.panel3 = new System.Windows.Forms.Panel();
			this.tableLayoutPanel4 = new System.Windows.Forms.TableLayoutPanel();
			this.radStorageDocuments = new System.Windows.Forms.RadioButton();
			this.radStoragePortable = new System.Windows.Forms.RadioButton();
			this.lblStorageHint = new System.Windows.Forms.Label();
			this.btnOk = new System.Windows.Forms.Button();
			this.lblInitialSetup = new System.Windows.Forms.Label();
			this.chkCreateShortcut = new System.Windows.Forms.CheckBox();
			this.lblCancel = new System.Windows.Forms.Label();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.lblLocation = new System.Windows.Forms.Label();
			this.lblDataLocation = new System.Windows.Forms.Label();
			this.panel1.SuspendLayout();
			this.panel2.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picLogo)).BeginInit();
			this.tableLayoutPanel3.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picPs4)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picXbox)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picWasd)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picArrows)).BeginInit();
			this.panel3.SuspendLayout();
			this.tableLayoutPanel4.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.BackColor = System.Drawing.SystemColors.ActiveBorder;
			this.panel1.Controls.Add(this.panel2);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Padding = new System.Windows.Forms.Padding(3);
			this.panel1.Size = new System.Drawing.Size(420, 484);
			this.panel1.TabIndex = 0;
			// 
			// panel2
			// 
			this.panel2.BackColor = System.Drawing.SystemColors.Window;
			this.panel2.Controls.Add(this.tableLayoutPanel1);
			this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel2.Location = new System.Drawing.Point(3, 3);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(414, 478);
			this.panel2.TabIndex = 0;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.lblMiscOptions, 0, 10);
			this.tableLayoutPanel1.Controls.Add(this.picLogo, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblStorageLocation, 0, 3);
			this.tableLayoutPanel1.Controls.Add(this.lblMesen, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblConfigWizard, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.lblInputMappings, 0, 7);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel3, 0, 9);
			this.tableLayoutPanel1.Controls.Add(this.lblInputHint, 0, 8);
			this.tableLayoutPanel1.Controls.Add(this.panel3, 0, 5);
			this.tableLayoutPanel1.Controls.Add(this.lblStorageHint, 0, 4);
			this.tableLayoutPanel1.Controls.Add(this.btnOk, 1, 13);
			this.tableLayoutPanel1.Controls.Add(this.lblInitialSetup, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.chkCreateShortcut, 0, 11);
			this.tableLayoutPanel1.Controls.Add(this.lblCancel, 0, 13);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 0, 6);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.Padding = new System.Windows.Forms.Padding(10);
			this.tableLayoutPanel1.RowCount = 14;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 26F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 46F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 35F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 45F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 28F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.Size = new System.Drawing.Size(414, 478);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// lblMiscOptions
			// 
			this.lblMiscOptions.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblMiscOptions.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.lblMiscOptions, 2);
			this.lblMiscOptions.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lblMiscOptions.ForeColor = System.Drawing.SystemColors.WindowFrame;
			this.lblMiscOptions.Location = new System.Drawing.Point(10, 396);
			this.lblMiscOptions.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblMiscOptions.Name = "lblMiscOptions";
			this.lblMiscOptions.Size = new System.Drawing.Size(72, 13);
			this.lblMiscOptions.TabIndex = 34;
			this.lblMiscOptions.Text = "Other Options";
			// 
			// picLogo
			// 
			this.picLogo.Anchor = System.Windows.Forms.AnchorStyles.Right;
			this.picLogo.Image = global::Mesen.GUI.Properties.Resources.MesenSIcon;
			this.picLogo.Location = new System.Drawing.Point(337, 14);
			this.picLogo.Name = "picLogo";
			this.tableLayoutPanel1.SetRowSpan(this.picLogo, 2);
			this.picLogo.Size = new System.Drawing.Size(64, 64);
			this.picLogo.TabIndex = 0;
			this.picLogo.TabStop = false;
			// 
			// lblStorageLocation
			// 
			this.lblStorageLocation.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblStorageLocation.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.lblStorageLocation, 2);
			this.lblStorageLocation.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lblStorageLocation.ForeColor = System.Drawing.SystemColors.WindowFrame;
			this.lblStorageLocation.Location = new System.Drawing.Point(10, 124);
			this.lblStorageLocation.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblStorageLocation.Name = "lblStorageLocation";
			this.lblStorageLocation.Size = new System.Drawing.Size(114, 13);
			this.lblStorageLocation.TabIndex = 24;
			this.lblStorageLocation.Text = "Data Storage Location";
			// 
			// lblMesen
			// 
			this.lblMesen.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblMesen.AutoSize = true;
			this.lblMesen.Location = new System.Drawing.Point(13, 23);
			this.lblMesen.Name = "lblMesen";
			this.lblMesen.Size = new System.Drawing.Size(131, 13);
			this.lblMesen.TabIndex = 1;
			this.lblMesen.Text = "Mesen-S - SNES Emulator";
			// 
			// lblConfigWizard
			// 
			this.lblConfigWizard.AutoSize = true;
			this.lblConfigWizard.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lblConfigWizard.Location = new System.Drawing.Point(13, 36);
			this.lblConfigWizard.Name = "lblConfigWizard";
			this.lblConfigWizard.Size = new System.Drawing.Size(184, 24);
			this.lblConfigWizard.TabIndex = 1;
			this.lblConfigWizard.Text = "Configuration Wizard";
			// 
			// lblInputMappings
			// 
			this.lblInputMappings.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblInputMappings.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.lblInputMappings, 2);
			this.lblInputMappings.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lblInputMappings.ForeColor = System.Drawing.SystemColors.WindowFrame;
			this.lblInputMappings.Location = new System.Drawing.Point(10, 252);
			this.lblInputMappings.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
			this.lblInputMappings.Name = "lblInputMappings";
			this.lblInputMappings.Size = new System.Drawing.Size(80, 13);
			this.lblInputMappings.TabIndex = 25;
			this.lblInputMappings.Text = "Input Mappings";
			// 
			// tableLayoutPanel3
			// 
			this.tableLayoutPanel3.ColumnCount = 4;
			this.tableLayoutPanel1.SetColumnSpan(this.tableLayoutPanel3, 2);
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 25F));
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 25F));
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 25F));
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 25F));
			this.tableLayoutPanel3.Controls.Add(this.chkArrows, 3, 1);
			this.tableLayoutPanel3.Controls.Add(this.chkWasd, 2, 1);
			this.tableLayoutPanel3.Controls.Add(this.chkPs4, 1, 1);
			this.tableLayoutPanel3.Controls.Add(this.picPs4, 1, 0);
			this.tableLayoutPanel3.Controls.Add(this.picXbox, 0, 0);
			this.tableLayoutPanel3.Controls.Add(this.picWasd, 2, 0);
			this.tableLayoutPanel3.Controls.Add(this.picArrows, 3, 0);
			this.tableLayoutPanel3.Controls.Add(this.chkXbox, 0, 1);
			this.tableLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel3.Location = new System.Drawing.Point(13, 288);
			this.tableLayoutPanel3.Name = "tableLayoutPanel3";
			this.tableLayoutPanel3.RowCount = 2;
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.Size = new System.Drawing.Size(388, 90);
			this.tableLayoutPanel3.TabIndex = 28;
			// 
			// chkArrows
			// 
			this.chkArrows.AutoSize = true;
			this.chkArrows.CheckAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.chkArrows.Checked = true;
			this.chkArrows.CheckState = System.Windows.Forms.CheckState.Checked;
			this.chkArrows.Dock = System.Windows.Forms.DockStyle.Fill;
			this.chkArrows.Location = new System.Drawing.Point(294, 73);
			this.chkArrows.Name = "chkArrows";
			this.chkArrows.Size = new System.Drawing.Size(91, 14);
			this.chkArrows.TabIndex = 33;
			this.chkArrows.UseVisualStyleBackColor = true;
			this.chkArrows.CheckedChanged += new System.EventHandler(this.chkArrows_CheckedChanged);
			// 
			// chkWasd
			// 
			this.chkWasd.AutoSize = true;
			this.chkWasd.CheckAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.chkWasd.Dock = System.Windows.Forms.DockStyle.Fill;
			this.chkWasd.Location = new System.Drawing.Point(197, 73);
			this.chkWasd.Name = "chkWasd";
			this.chkWasd.Size = new System.Drawing.Size(91, 14);
			this.chkWasd.TabIndex = 32;
			this.chkWasd.UseVisualStyleBackColor = true;
			this.chkWasd.CheckedChanged += new System.EventHandler(this.chkWasd_CheckedChanged);
			// 
			// chkPs4
			// 
			this.chkPs4.AutoSize = true;
			this.chkPs4.CheckAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.chkPs4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.chkPs4.Location = new System.Drawing.Point(100, 73);
			this.chkPs4.Name = "chkPs4";
			this.chkPs4.Size = new System.Drawing.Size(91, 14);
			this.chkPs4.TabIndex = 31;
			this.chkPs4.UseVisualStyleBackColor = true;
			// 
			// picPs4
			// 
			this.picPs4.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picPs4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.picPs4.Image = global::Mesen.GUI.Properties.Resources.PsIcon;
			this.picPs4.Location = new System.Drawing.Point(100, 3);
			this.picPs4.Name = "picPs4";
			this.picPs4.Size = new System.Drawing.Size(91, 64);
			this.picPs4.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.picPs4.TabIndex = 26;
			this.picPs4.TabStop = false;
			this.picPs4.Click += new System.EventHandler(this.picPs4_Click);
			// 
			// picXbox
			// 
			this.picXbox.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picXbox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.picXbox.Image = global::Mesen.GUI.Properties.Resources.XbIcon;
			this.picXbox.Location = new System.Drawing.Point(3, 3);
			this.picXbox.Name = "picXbox";
			this.picXbox.Size = new System.Drawing.Size(91, 64);
			this.picXbox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.picXbox.TabIndex = 27;
			this.picXbox.TabStop = false;
			this.picXbox.Click += new System.EventHandler(this.picXbox_Click);
			// 
			// picWasd
			// 
			this.picWasd.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picWasd.Dock = System.Windows.Forms.DockStyle.Fill;
			this.picWasd.Image = global::Mesen.GUI.Properties.Resources.WasdKeys;
			this.picWasd.Location = new System.Drawing.Point(197, 3);
			this.picWasd.Name = "picWasd";
			this.picWasd.Size = new System.Drawing.Size(91, 64);
			this.picWasd.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.picWasd.TabIndex = 28;
			this.picWasd.TabStop = false;
			this.picWasd.Click += new System.EventHandler(this.picWasd_Click);
			// 
			// picArrows
			// 
			this.picArrows.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picArrows.Dock = System.Windows.Forms.DockStyle.Fill;
			this.picArrows.Image = global::Mesen.GUI.Properties.Resources.ArrowKeys;
			this.picArrows.Location = new System.Drawing.Point(294, 3);
			this.picArrows.Name = "picArrows";
			this.picArrows.Size = new System.Drawing.Size(91, 64);
			this.picArrows.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.picArrows.TabIndex = 29;
			this.picArrows.TabStop = false;
			this.picArrows.Click += new System.EventHandler(this.picArrows_Click);
			// 
			// chkXbox
			// 
			this.chkXbox.AutoSize = true;
			this.chkXbox.CheckAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.chkXbox.Checked = true;
			this.chkXbox.CheckState = System.Windows.Forms.CheckState.Checked;
			this.chkXbox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.chkXbox.Location = new System.Drawing.Point(3, 73);
			this.chkXbox.Name = "chkXbox";
			this.chkXbox.Size = new System.Drawing.Size(91, 14);
			this.chkXbox.TabIndex = 30;
			this.chkXbox.UseVisualStyleBackColor = true;
			// 
			// lblInputHint
			// 
			this.lblInputHint.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblInputHint.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.lblInputHint, 2);
			this.lblInputHint.Location = new System.Drawing.Point(13, 268);
			this.lblInputHint.Name = "lblInputHint";
			this.lblInputHint.Size = new System.Drawing.Size(177, 13);
			this.lblInputHint.TabIndex = 29;
			this.lblInputHint.Text = "Select your preferred input methods:";
			// 
			// panel3
			// 
			this.tableLayoutPanel1.SetColumnSpan(this.panel3, 2);
			this.panel3.Controls.Add(this.tableLayoutPanel4);
			this.panel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel3.Location = new System.Drawing.Point(10, 157);
			this.panel3.Margin = new System.Windows.Forms.Padding(0);
			this.panel3.Name = "panel3";
			this.panel3.Padding = new System.Windows.Forms.Padding(8, 0, 0, 0);
			this.panel3.Size = new System.Drawing.Size(394, 50);
			this.panel3.TabIndex = 30;
			// 
			// tableLayoutPanel4
			// 
			this.tableLayoutPanel4.ColumnCount = 1;
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Controls.Add(this.radStorageDocuments, 0, 0);
			this.tableLayoutPanel4.Controls.Add(this.radStoragePortable, 0, 1);
			this.tableLayoutPanel4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel4.Location = new System.Drawing.Point(8, 0);
			this.tableLayoutPanel4.Name = "tableLayoutPanel4";
			this.tableLayoutPanel4.RowCount = 2;
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel4.Size = new System.Drawing.Size(386, 50);
			this.tableLayoutPanel4.TabIndex = 0;
			// 
			// radStorageDocuments
			// 
			this.radStorageDocuments.AutoSize = true;
			this.radStorageDocuments.Checked = true;
			this.radStorageDocuments.Cursor = System.Windows.Forms.Cursors.Hand;
			this.radStorageDocuments.Dock = System.Windows.Forms.DockStyle.Fill;
			this.radStorageDocuments.Location = new System.Drawing.Point(3, 3);
			this.radStorageDocuments.Name = "radStorageDocuments";
			this.radStorageDocuments.Size = new System.Drawing.Size(380, 19);
			this.radStorageDocuments.TabIndex = 0;
			this.radStorageDocuments.TabStop = true;
			this.radStorageDocuments.Text = "Store the data in my user profile";
			this.radStorageDocuments.UseVisualStyleBackColor = true;
			this.radStorageDocuments.CheckedChanged += new System.EventHandler(this.radStorageDocuments_CheckedChanged);
			// 
			// radStoragePortable
			// 
			this.radStoragePortable.AutoSize = true;
			this.radStoragePortable.Cursor = System.Windows.Forms.Cursors.Hand;
			this.radStoragePortable.Dock = System.Windows.Forms.DockStyle.Fill;
			this.radStoragePortable.Location = new System.Drawing.Point(3, 28);
			this.radStoragePortable.Name = "radStoragePortable";
			this.radStoragePortable.Size = new System.Drawing.Size(380, 19);
			this.radStoragePortable.TabIndex = 1;
			this.radStoragePortable.Text = "Store the data in the same folder as the application";
			this.radStoragePortable.UseVisualStyleBackColor = true;
			this.radStoragePortable.CheckedChanged += new System.EventHandler(this.radStoragePortable_CheckedChanged);
			// 
			// lblStorageHint
			// 
			this.lblStorageHint.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblStorageHint.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.lblStorageHint, 2);
			this.lblStorageHint.Location = new System.Drawing.Point(13, 140);
			this.lblStorageHint.Name = "lblStorageHint";
			this.lblStorageHint.Size = new System.Drawing.Size(232, 13);
			this.lblStorageHint.TabIndex = 31;
			this.lblStorageHint.Text = "Select where you want to store Mesen-S\'s data:";
			// 
			// btnOk
			// 
			this.btnOk.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.btnOk.AutoSize = true;
			this.btnOk.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.btnOk.BackColor = System.Drawing.SystemColors.Control;
			this.btnOk.Cursor = System.Windows.Forms.Cursors.Hand;
			this.btnOk.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btnOk.Location = new System.Drawing.Point(333, 440);
			this.btnOk.Name = "btnOk";
			this.btnOk.Size = new System.Drawing.Size(68, 25);
			this.btnOk.TabIndex = 32;
			this.btnOk.Text = "CONFIRM";
			this.btnOk.UseVisualStyleBackColor = false;
			this.btnOk.Click += new System.EventHandler(this.btnOk_Click);
			// 
			// lblInitialSetup
			// 
			this.lblInitialSetup.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblInitialSetup.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.lblInitialSetup, 2);
			this.lblInitialSetup.Location = new System.Drawing.Point(13, 93);
			this.lblInitialSetup.Name = "lblInitialSetup";
			this.lblInitialSetup.Size = new System.Drawing.Size(272, 13);
			this.lblInitialSetup.TabIndex = 33;
			this.lblInitialSetup.Text = "Please take a moment to perform Mesen-S\'s initial setup.";
			// 
			// chkCreateShortcut
			// 
			this.chkCreateShortcut.AutoSize = true;
			this.chkCreateShortcut.Checked = true;
			this.chkCreateShortcut.CheckState = System.Windows.Forms.CheckState.Checked;
			this.tableLayoutPanel1.SetColumnSpan(this.chkCreateShortcut, 2);
			this.chkCreateShortcut.Cursor = System.Windows.Forms.Cursors.Hand;
			this.chkCreateShortcut.Location = new System.Drawing.Point(21, 415);
			this.chkCreateShortcut.Margin = new System.Windows.Forms.Padding(11, 6, 3, 3);
			this.chkCreateShortcut.Name = "chkCreateShortcut";
			this.chkCreateShortcut.Size = new System.Drawing.Size(179, 17);
			this.chkCreateShortcut.TabIndex = 35;
			this.chkCreateShortcut.Text = "Create a shortcut on my desktop";
			this.chkCreateShortcut.UseVisualStyleBackColor = true;
			// 
			// lblCancel
			// 
			this.lblCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblCancel.AutoSize = true;
			this.lblCancel.Cursor = System.Windows.Forms.Cursors.Hand;
			this.lblCancel.ForeColor = System.Drawing.Color.Blue;
			this.lblCancel.Location = new System.Drawing.Point(13, 455);
			this.lblCancel.Name = "lblCancel";
			this.lblCancel.Size = new System.Drawing.Size(40, 13);
			this.lblCancel.TabIndex = 36;
			this.lblCancel.Text = "Cancel";
			this.lblCancel.Click += new System.EventHandler(this.lblCancel_Click);
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 2;
			this.tableLayoutPanel1.SetColumnSpan(this.tableLayoutPanel2, 2);
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Controls.Add(this.lblLocation, 1, 0);
			this.tableLayoutPanel2.Controls.Add(this.lblDataLocation, 0, 0);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(13, 210);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.Padding = new System.Windows.Forms.Padding(0, 5, 0, 0);
			this.tableLayoutPanel2.RowCount = 1;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(388, 39);
			this.tableLayoutPanel2.TabIndex = 38;
			// 
			// lblLocation
			// 
			this.lblLocation.AutoSize = true;
			this.lblLocation.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lblLocation.ForeColor = System.Drawing.SystemColors.ControlText;
			this.lblLocation.Location = new System.Drawing.Point(48, 5);
			this.lblLocation.Name = "lblLocation";
			this.lblLocation.Size = new System.Drawing.Size(19, 13);
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
			// frmConfigWizard
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.SystemColors.Window;
			this.ClientSize = new System.Drawing.Size(420, 484);
			this.Controls.Add(this.panel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
			this.Name = "frmConfigWizard";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
			this.Text = "Mesen - Configuration";
			this.panel1.ResumeLayout(false);
			this.panel2.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picLogo)).EndInit();
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel3.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picPs4)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picXbox)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picWasd)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picArrows)).EndInit();
			this.panel3.ResumeLayout(false);
			this.tableLayoutPanel4.ResumeLayout(false);
			this.tableLayoutPanel4.PerformLayout();
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.PictureBox picLogo;
		private System.Windows.Forms.Label lblMesen;
		private System.Windows.Forms.Label lblConfigWizard;
		private System.Windows.Forms.Label lblStorageLocation;
		private System.Windows.Forms.Label lblInputMappings;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
		private System.Windows.Forms.PictureBox picPs4;
		private System.Windows.Forms.PictureBox picXbox;
		private System.Windows.Forms.PictureBox picWasd;
		private System.Windows.Forms.PictureBox picArrows;
		private System.Windows.Forms.Label lblInputHint;
		private System.Windows.Forms.Panel panel3;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel4;
		private System.Windows.Forms.RadioButton radStorageDocuments;
		private System.Windows.Forms.RadioButton radStoragePortable;
		private System.Windows.Forms.Label lblStorageHint;
		private System.Windows.Forms.Button btnOk;
		private System.Windows.Forms.Label lblInitialSetup;
		private System.Windows.Forms.Label lblMiscOptions;
		private System.Windows.Forms.CheckBox chkCreateShortcut;
		private System.Windows.Forms.Label lblCancel;
		private System.Windows.Forms.Label lblDataLocation;
		private System.Windows.Forms.Label lblLocation;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.CheckBox chkArrows;
		private System.Windows.Forms.CheckBox chkWasd;
		private System.Windows.Forms.CheckBox chkPs4;
		private System.Windows.Forms.CheckBox chkXbox;
	}
}