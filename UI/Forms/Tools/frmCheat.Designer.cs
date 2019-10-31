namespace Mesen.GUI.Forms
{
	partial class frmCheat
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
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.lblDescription = new System.Windows.Forms.Label();
			this.txtCheatName = new System.Windows.Forms.TextBox();
			this.grpCode = new System.Windows.Forms.GroupBox();
			this.tlpAdd = new System.Windows.Forms.TableLayoutPanel();
			this.tlpInvalidCodes = new System.Windows.Forms.TableLayoutPanel();
			this.pictureBox2 = new System.Windows.Forms.PictureBox();
			this.lblInvalidCodes = new System.Windows.Forms.Label();
			this.txtCodes = new System.Windows.Forms.TextBox();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.picHint = new System.Windows.Forms.PictureBox();
			this.lblHint = new System.Windows.Forms.Label();
			this.txtRawCodes = new System.Windows.Forms.TextBox();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.radProActionReplay = new System.Windows.Forms.RadioButton();
			this.radGameGenie = new System.Windows.Forms.RadioButton();
			this.chkEnabled = new System.Windows.Forms.CheckBox();
			this.tableLayoutPanel2.SuspendLayout();
			this.grpCode.SuspendLayout();
			this.tlpAdd.SuspendLayout();
			this.tlpInvalidCodes.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).BeginInit();
			this.tableLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picHint)).BeginInit();
			this.tableLayoutPanel3.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 262);
			this.baseConfigPanel.Size = new System.Drawing.Size(410, 29);
			this.baseConfigPanel.TabIndex = 4;
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 2;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel2.Controls.Add(this.lblDescription, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.txtCheatName, 1, 0);
			this.tableLayoutPanel2.Controls.Add(this.grpCode, 0, 2);
			this.tableLayoutPanel2.Controls.Add(this.chkEnabled, 0, 1);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 3;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(410, 262);
			this.tableLayoutPanel2.TabIndex = 4;
			// 
			// lblDescription
			// 
			this.lblDescription.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblDescription.AutoSize = true;
			this.lblDescription.Location = new System.Drawing.Point(3, 6);
			this.lblDescription.Name = "lblDescription";
			this.lblDescription.Size = new System.Drawing.Size(63, 13);
			this.lblDescription.TabIndex = 1;
			this.lblDescription.Text = "Description:";
			// 
			// txtCheatName
			// 
			this.txtCheatName.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtCheatName.Location = new System.Drawing.Point(72, 3);
			this.txtCheatName.MaxLength = 255;
			this.txtCheatName.Name = "txtCheatName";
			this.txtCheatName.Size = new System.Drawing.Size(335, 20);
			this.txtCheatName.TabIndex = 2;
			// 
			// grpCode
			// 
			this.tableLayoutPanel2.SetColumnSpan(this.grpCode, 2);
			this.grpCode.Controls.Add(this.tlpAdd);
			this.grpCode.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpCode.Location = new System.Drawing.Point(3, 52);
			this.grpCode.Name = "grpCode";
			this.grpCode.Size = new System.Drawing.Size(404, 207);
			this.grpCode.TabIndex = 3;
			this.grpCode.TabStop = false;
			this.grpCode.Text = "Codes";
			// 
			// tlpAdd
			// 
			this.tlpAdd.ColumnCount = 2;
			this.tlpAdd.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpAdd.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpAdd.Controls.Add(this.tlpInvalidCodes, 0, 2);
			this.tlpAdd.Controls.Add(this.txtCodes, 0, 1);
			this.tlpAdd.Controls.Add(this.tableLayoutPanel1, 0, 3);
			this.tlpAdd.Controls.Add(this.txtRawCodes, 1, 1);
			this.tlpAdd.Controls.Add(this.tableLayoutPanel3, 0, 0);
			this.tlpAdd.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpAdd.Location = new System.Drawing.Point(3, 16);
			this.tlpAdd.Name = "tlpAdd";
			this.tlpAdd.RowCount = 5;
			this.tlpAdd.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpAdd.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpAdd.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpAdd.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpAdd.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpAdd.Size = new System.Drawing.Size(398, 188);
			this.tlpAdd.TabIndex = 0;
			// 
			// tlpInvalidCodes
			// 
			this.tlpInvalidCodes.ColumnCount = 2;
			this.tlpAdd.SetColumnSpan(this.tlpInvalidCodes, 2);
			this.tlpInvalidCodes.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpInvalidCodes.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpInvalidCodes.Controls.Add(this.pictureBox2, 0, 0);
			this.tlpInvalidCodes.Controls.Add(this.lblInvalidCodes, 1, 0);
			this.tlpInvalidCodes.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpInvalidCodes.Location = new System.Drawing.Point(0, 148);
			this.tlpInvalidCodes.Margin = new System.Windows.Forms.Padding(0);
			this.tlpInvalidCodes.Name = "tlpInvalidCodes";
			this.tlpInvalidCodes.RowCount = 1;
			this.tlpInvalidCodes.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpInvalidCodes.Size = new System.Drawing.Size(398, 20);
			this.tlpInvalidCodes.TabIndex = 7;
			// 
			// pictureBox2
			// 
			this.pictureBox2.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.pictureBox2.Image = global::Mesen.GUI.Properties.Resources.Warning;
			this.pictureBox2.Location = new System.Drawing.Point(0, 1);
			this.pictureBox2.Margin = new System.Windows.Forms.Padding(0);
			this.pictureBox2.Name = "pictureBox2";
			this.pictureBox2.Size = new System.Drawing.Size(17, 17);
			this.pictureBox2.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.pictureBox2.TabIndex = 5;
			this.pictureBox2.TabStop = false;
			// 
			// lblInvalidCodes
			// 
			this.lblInvalidCodes.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblInvalidCodes.AutoSize = true;
			this.lblInvalidCodes.Location = new System.Drawing.Point(20, 3);
			this.lblInvalidCodes.Name = "lblInvalidCodes";
			this.lblInvalidCodes.Size = new System.Drawing.Size(227, 13);
			this.lblInvalidCodes.TabIndex = 4;
			this.lblInvalidCodes.Text = "Some codes do not match the selected format.";
			// 
			// txtCodes
			// 
			this.txtCodes.AcceptsReturn = true;
			this.txtCodes.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtCodes.Location = new System.Drawing.Point(3, 26);
			this.txtCodes.Multiline = true;
			this.txtCodes.Name = "txtCodes";
			this.txtCodes.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.txtCodes.Size = new System.Drawing.Size(193, 119);
			this.txtCodes.TabIndex = 3;
			this.txtCodes.TextChanged += new System.EventHandler(this.txtCodes_TextChanged);
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tlpAdd.SetColumnSpan(this.tableLayoutPanel1, 2);
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.picHint, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblHint, 1, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 168);
			this.tableLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 1;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(398, 20);
			this.tableLayoutPanel1.TabIndex = 5;
			// 
			// picHint
			// 
			this.picHint.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.picHint.Image = global::Mesen.GUI.Properties.Resources.Help;
			this.picHint.Location = new System.Drawing.Point(0, 1);
			this.picHint.Margin = new System.Windows.Forms.Padding(0);
			this.picHint.Name = "picHint";
			this.picHint.Size = new System.Drawing.Size(17, 17);
			this.picHint.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.picHint.TabIndex = 5;
			this.picHint.TabStop = false;
			// 
			// lblHint
			// 
			this.lblHint.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblHint.AutoSize = true;
			this.lblHint.Location = new System.Drawing.Point(20, 3);
			this.lblHint.Name = "lblHint";
			this.lblHint.Size = new System.Drawing.Size(223, 13);
			this.lblHint.TabIndex = 4;
			this.lblHint.Text = "To enter multiple codes, enter 1 code per line.";
			// 
			// txtRawCodes
			// 
			this.txtRawCodes.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtRawCodes.Location = new System.Drawing.Point(202, 26);
			this.txtRawCodes.Multiline = true;
			this.txtRawCodes.Name = "txtRawCodes";
			this.txtRawCodes.ReadOnly = true;
			this.txtRawCodes.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.txtRawCodes.Size = new System.Drawing.Size(193, 119);
			this.txtRawCodes.TabIndex = 8;
			// 
			// tableLayoutPanel3
			// 
			this.tableLayoutPanel3.ColumnCount = 3;
			this.tlpAdd.SetColumnSpan(this.tableLayoutPanel3, 2);
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Controls.Add(this.radProActionReplay, 1, 0);
			this.tableLayoutPanel3.Controls.Add(this.radGameGenie, 0, 0);
			this.tableLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel3.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel3.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel3.Name = "tableLayoutPanel3";
			this.tableLayoutPanel3.RowCount = 1;
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Size = new System.Drawing.Size(398, 23);
			this.tableLayoutPanel3.TabIndex = 9;
			// 
			// radProActionReplay
			// 
			this.radProActionReplay.AutoSize = true;
			this.radProActionReplay.Location = new System.Drawing.Point(153, 3);
			this.radProActionReplay.Name = "radProActionReplay";
			this.radProActionReplay.Size = new System.Drawing.Size(175, 17);
			this.radProActionReplay.TabIndex = 2;
			this.radProActionReplay.Text = "Pro Action Replay (AAAAAAVV)";
			this.radProActionReplay.UseVisualStyleBackColor = true;
			this.radProActionReplay.CheckedChanged += new System.EventHandler(this.radFormat_CheckedChanged);
			// 
			// radGameGenie
			// 
			this.radGameGenie.AutoSize = true;
			this.radGameGenie.Checked = true;
			this.radGameGenie.Location = new System.Drawing.Point(3, 3);
			this.radGameGenie.Name = "radGameGenie";
			this.radGameGenie.Size = new System.Drawing.Size(144, 17);
			this.radGameGenie.TabIndex = 2;
			this.radGameGenie.TabStop = true;
			this.radGameGenie.Text = "Game Genie (0000-0000)";
			this.radGameGenie.UseVisualStyleBackColor = true;
			this.radGameGenie.CheckedChanged += new System.EventHandler(this.radFormat_CheckedChanged);
			// 
			// chkEnabled
			// 
			this.chkEnabled.AutoSize = true;
			this.tableLayoutPanel2.SetColumnSpan(this.chkEnabled, 2);
			this.chkEnabled.Location = new System.Drawing.Point(3, 29);
			this.chkEnabled.Name = "chkEnabled";
			this.chkEnabled.Size = new System.Drawing.Size(96, 17);
			this.chkEnabled.TabIndex = 6;
			this.chkEnabled.Text = "Cheat Enabled";
			this.chkEnabled.UseVisualStyleBackColor = true;
			// 
			// frmCheat
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(410, 291);
			this.Controls.Add(this.tableLayoutPanel2);
			this.MinimumSize = new System.Drawing.Size(426, 330);
			this.Name = "frmCheat";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Edit Cheat";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tableLayoutPanel2, 0);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.grpCode.ResumeLayout(false);
			this.tlpAdd.ResumeLayout(false);
			this.tlpAdd.PerformLayout();
			this.tlpInvalidCodes.ResumeLayout(false);
			this.tlpInvalidCodes.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).EndInit();
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picHint)).EndInit();
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel3.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.Label lblDescription;
		private System.Windows.Forms.TextBox txtCheatName;
		private System.Windows.Forms.GroupBox grpCode;
		private System.Windows.Forms.TableLayoutPanel tlpAdd;
		private System.Windows.Forms.RadioButton radGameGenie;
		private System.Windows.Forms.RadioButton radProActionReplay;
		private System.Windows.Forms.TextBox txtCodes;
		private System.Windows.Forms.CheckBox chkEnabled;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label lblHint;
		private System.Windows.Forms.PictureBox picHint;
		private System.Windows.Forms.TableLayoutPanel tlpInvalidCodes;
		private System.Windows.Forms.PictureBox pictureBox2;
		private System.Windows.Forms.Label lblInvalidCodes;
		private System.Windows.Forms.TextBox txtRawCodes;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
	}
}