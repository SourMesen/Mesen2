namespace Mesen.GUI.Forms.Config
{
	partial class frmInputConfig
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(frmInputConfig));
			this.tabMain = new System.Windows.Forms.TabControl();
			this.tpgControllers = new System.Windows.Forms.TabPage();
			this.tlpControllers = new System.Windows.Forms.TableLayoutPanel();
			this.lblPlayer1 = new System.Windows.Forms.Label();
			this.lblPlayer2 = new System.Windows.Forms.Label();
			this.cboPlayer1 = new System.Windows.Forms.ComboBox();
			this.cboPlayer2 = new System.Windows.Forms.ComboBox();
			this.btnSetupP1 = new System.Windows.Forms.Button();
			this.btnSetupP2 = new System.Windows.Forms.Button();
			this.pnlConflictWarning = new System.Windows.Forms.Panel();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.picWarning = new System.Windows.Forms.PictureBox();
			this.lblKeyBinding = new System.Windows.Forms.Label();
			this.grpMultitap = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.cboMultitap3 = new System.Windows.Forms.ComboBox();
			this.btnSetupMultitap3 = new System.Windows.Forms.Button();
			this.btnSetupMultitap1 = new System.Windows.Forms.Button();
			this.btnSetupMultitap2 = new System.Windows.Forms.Button();
			this.cboMultitap2 = new System.Windows.Forms.ComboBox();
			this.btnSetupMultitap4 = new System.Windows.Forms.Button();
			this.cboMultitap1 = new System.Windows.Forms.ComboBox();
			this.cboMultitap4 = new System.Windows.Forms.ComboBox();
			this.lblMultitap4 = new System.Windows.Forms.Label();
			this.lblMultitap3 = new System.Windows.Forms.Label();
			this.lblMultitap2 = new System.Windows.Forms.Label();
			this.lblMultitap1 = new System.Windows.Forms.Label();
			this.tabMain.SuspendLayout();
			this.tpgControllers.SuspendLayout();
			this.tlpControllers.SuspendLayout();
			this.pnlConflictWarning.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picWarning)).BeginInit();
			this.grpMultitap.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 286);
			this.baseConfigPanel.Size = new System.Drawing.Size(382, 29);
			this.baseConfigPanel.TabIndex = 4;
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tpgControllers);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(382, 286);
			this.tabMain.TabIndex = 2;
			// 
			// tpgControllers
			// 
			this.tpgControllers.Controls.Add(this.tlpControllers);
			this.tpgControllers.Location = new System.Drawing.Point(4, 22);
			this.tpgControllers.Name = "tpgControllers";
			this.tpgControllers.Size = new System.Drawing.Size(374, 260);
			this.tpgControllers.TabIndex = 3;
			this.tpgControllers.Text = "Controllers";
			this.tpgControllers.UseVisualStyleBackColor = true;
			// 
			// tlpControllers
			// 
			this.tlpControllers.ColumnCount = 3;
			this.tlpControllers.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpControllers.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpControllers.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpControllers.Controls.Add(this.lblPlayer1, 0, 1);
			this.tlpControllers.Controls.Add(this.lblPlayer2, 0, 2);
			this.tlpControllers.Controls.Add(this.cboPlayer1, 1, 1);
			this.tlpControllers.Controls.Add(this.cboPlayer2, 1, 2);
			this.tlpControllers.Controls.Add(this.btnSetupP1, 2, 1);
			this.tlpControllers.Controls.Add(this.btnSetupP2, 2, 2);
			this.tlpControllers.Controls.Add(this.pnlConflictWarning, 0, 0);
			this.tlpControllers.Controls.Add(this.grpMultitap, 0, 3);
			this.tlpControllers.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpControllers.Location = new System.Drawing.Point(0, 0);
			this.tlpControllers.Name = "tlpControllers";
			this.tlpControllers.RowCount = 5;
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpControllers.Size = new System.Drawing.Size(374, 260);
			this.tlpControllers.TabIndex = 0;
			// 
			// lblPlayer1
			// 
			this.lblPlayer1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPlayer1.AutoSize = true;
			this.lblPlayer1.Location = new System.Drawing.Point(3, 69);
			this.lblPlayer1.Name = "lblPlayer1";
			this.lblPlayer1.Size = new System.Drawing.Size(48, 13);
			this.lblPlayer1.TabIndex = 0;
			this.lblPlayer1.Text = "Player 1:";
			// 
			// lblPlayer2
			// 
			this.lblPlayer2.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPlayer2.AutoSize = true;
			this.lblPlayer2.Location = new System.Drawing.Point(3, 98);
			this.lblPlayer2.Name = "lblPlayer2";
			this.lblPlayer2.Size = new System.Drawing.Size(48, 13);
			this.lblPlayer2.TabIndex = 1;
			this.lblPlayer2.Text = "Player 2:";
			// 
			// cboPlayer1
			// 
			this.cboPlayer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboPlayer1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboPlayer1.FormattingEnabled = true;
			this.cboPlayer1.Location = new System.Drawing.Point(57, 64);
			this.cboPlayer1.Name = "cboPlayer1";
			this.cboPlayer1.Size = new System.Drawing.Size(246, 21);
			this.cboPlayer1.TabIndex = 4;
			this.cboPlayer1.SelectionChangeCommitted += new System.EventHandler(this.cboPlayer1_SelectionChangeCommitted);
			// 
			// cboPlayer2
			// 
			this.cboPlayer2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboPlayer2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboPlayer2.FormattingEnabled = true;
			this.cboPlayer2.Location = new System.Drawing.Point(57, 93);
			this.cboPlayer2.Name = "cboPlayer2";
			this.cboPlayer2.Size = new System.Drawing.Size(246, 21);
			this.cboPlayer2.TabIndex = 6;
			this.cboPlayer2.SelectionChangeCommitted += new System.EventHandler(this.cboPlayer2_SelectionChangeCommitted);
			// 
			// btnSetupP1
			// 
			this.btnSetupP1.AutoSize = true;
			this.btnSetupP1.Location = new System.Drawing.Point(309, 64);
			this.btnSetupP1.Name = "btnSetupP1";
			this.btnSetupP1.Size = new System.Drawing.Size(62, 23);
			this.btnSetupP1.TabIndex = 9;
			this.btnSetupP1.Text = "Setup";
			this.btnSetupP1.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.btnSetupP1.UseVisualStyleBackColor = true;
			this.btnSetupP1.Click += new System.EventHandler(this.btnSetup_Click);
			// 
			// btnSetupP2
			// 
			this.btnSetupP2.AutoSize = true;
			this.btnSetupP2.Location = new System.Drawing.Point(309, 93);
			this.btnSetupP2.Name = "btnSetupP2";
			this.btnSetupP2.Size = new System.Drawing.Size(62, 23);
			this.btnSetupP2.TabIndex = 10;
			this.btnSetupP2.Text = "Setup";
			this.btnSetupP2.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.btnSetupP2.UseVisualStyleBackColor = true;
			this.btnSetupP2.Click += new System.EventHandler(this.btnSetup_Click);
			// 
			// pnlConflictWarning
			// 
			this.pnlConflictWarning.BackColor = System.Drawing.Color.WhiteSmoke;
			this.pnlConflictWarning.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.tlpControllers.SetColumnSpan(this.pnlConflictWarning, 3);
			this.pnlConflictWarning.Controls.Add(this.tableLayoutPanel1);
			this.pnlConflictWarning.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pnlConflictWarning.Location = new System.Drawing.Point(3, 3);
			this.pnlConflictWarning.Name = "pnlConflictWarning";
			this.pnlConflictWarning.Size = new System.Drawing.Size(368, 55);
			this.pnlConflictWarning.TabIndex = 19;
			this.pnlConflictWarning.Visible = false;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.picWarning, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblKeyBinding, 1, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 1;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(366, 53);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// picWarning
			// 
			this.picWarning.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.picWarning.Image = global::Mesen.GUI.Properties.Resources.Warning;
			this.picWarning.Location = new System.Drawing.Point(3, 18);
			this.picWarning.Name = "picWarning";
			this.picWarning.Size = new System.Drawing.Size(16, 16);
			this.picWarning.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
			this.picWarning.TabIndex = 0;
			this.picWarning.TabStop = false;
			// 
			// lblKeyBinding
			// 
			this.lblKeyBinding.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lblKeyBinding.Location = new System.Drawing.Point(25, 0);
			this.lblKeyBinding.Name = "lblKeyBinding";
			this.lblKeyBinding.Size = new System.Drawing.Size(338, 53);
			this.lblKeyBinding.TabIndex = 1;
			this.lblKeyBinding.Text = resources.GetString("lblKeyBinding.Text");
			// 
			// grpMultitap
			// 
			this.tlpControllers.SetColumnSpan(this.grpMultitap, 3);
			this.grpMultitap.Controls.Add(this.tableLayoutPanel2);
			this.grpMultitap.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpMultitap.Location = new System.Drawing.Point(3, 122);
			this.grpMultitap.Name = "grpMultitap";
			this.grpMultitap.Padding = new System.Windows.Forms.Padding(0);
			this.grpMultitap.Size = new System.Drawing.Size(368, 133);
			this.grpMultitap.TabIndex = 26;
			this.grpMultitap.TabStop = false;
			this.grpMultitap.Text = "Multitap Configuration";
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 3;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.Controls.Add(this.cboMultitap3, 1, 2);
			this.tableLayoutPanel2.Controls.Add(this.btnSetupMultitap3, 2, 2);
			this.tableLayoutPanel2.Controls.Add(this.btnSetupMultitap1, 2, 0);
			this.tableLayoutPanel2.Controls.Add(this.btnSetupMultitap2, 2, 1);
			this.tableLayoutPanel2.Controls.Add(this.cboMultitap2, 1, 1);
			this.tableLayoutPanel2.Controls.Add(this.btnSetupMultitap4, 2, 3);
			this.tableLayoutPanel2.Controls.Add(this.cboMultitap1, 1, 0);
			this.tableLayoutPanel2.Controls.Add(this.cboMultitap4, 1, 3);
			this.tableLayoutPanel2.Controls.Add(this.lblMultitap4, 0, 3);
			this.tableLayoutPanel2.Controls.Add(this.lblMultitap3, 0, 2);
			this.tableLayoutPanel2.Controls.Add(this.lblMultitap2, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.lblMultitap1, 0, 0);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(0, 13);
			this.tableLayoutPanel2.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 5;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(368, 120);
			this.tableLayoutPanel2.TabIndex = 0;
			// 
			// cboMultitap3
			// 
			this.cboMultitap3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboMultitap3.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboMultitap3.FormattingEnabled = true;
			this.cboMultitap3.Location = new System.Drawing.Point(57, 61);
			this.cboMultitap3.Name = "cboMultitap3";
			this.cboMultitap3.Size = new System.Drawing.Size(240, 21);
			this.cboMultitap3.TabIndex = 21;
			// 
			// btnSetupMultitap3
			// 
			this.btnSetupMultitap3.AutoSize = true;
			this.btnSetupMultitap3.Location = new System.Drawing.Point(303, 61);
			this.btnSetupMultitap3.Name = "btnSetupMultitap3";
			this.btnSetupMultitap3.Size = new System.Drawing.Size(62, 23);
			this.btnSetupMultitap3.TabIndex = 22;
			this.btnSetupMultitap3.Text = "Setup";
			this.btnSetupMultitap3.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.btnSetupMultitap3.UseVisualStyleBackColor = true;
			this.btnSetupMultitap3.Click += new System.EventHandler(this.btnSetup_Click);
			// 
			// btnSetupMultitap1
			// 
			this.btnSetupMultitap1.AutoSize = true;
			this.btnSetupMultitap1.Location = new System.Drawing.Point(303, 3);
			this.btnSetupMultitap1.Name = "btnSetupMultitap1";
			this.btnSetupMultitap1.Size = new System.Drawing.Size(62, 23);
			this.btnSetupMultitap1.TabIndex = 11;
			this.btnSetupMultitap1.Text = "Setup";
			this.btnSetupMultitap1.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.btnSetupMultitap1.UseVisualStyleBackColor = true;
			this.btnSetupMultitap1.Click += new System.EventHandler(this.btnSetup_Click);
			// 
			// btnSetupMultitap2
			// 
			this.btnSetupMultitap2.AutoSize = true;
			this.btnSetupMultitap2.Location = new System.Drawing.Point(303, 32);
			this.btnSetupMultitap2.Name = "btnSetupMultitap2";
			this.btnSetupMultitap2.Size = new System.Drawing.Size(62, 23);
			this.btnSetupMultitap2.TabIndex = 12;
			this.btnSetupMultitap2.Text = "Setup";
			this.btnSetupMultitap2.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.btnSetupMultitap2.UseVisualStyleBackColor = true;
			this.btnSetupMultitap2.Click += new System.EventHandler(this.btnSetup_Click);
			// 
			// cboMultitap2
			// 
			this.cboMultitap2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboMultitap2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboMultitap2.FormattingEnabled = true;
			this.cboMultitap2.Location = new System.Drawing.Point(57, 32);
			this.cboMultitap2.Name = "cboMultitap2";
			this.cboMultitap2.Size = new System.Drawing.Size(240, 21);
			this.cboMultitap2.TabIndex = 8;
			// 
			// btnSetupMultitap4
			// 
			this.btnSetupMultitap4.AutoSize = true;
			this.btnSetupMultitap4.Location = new System.Drawing.Point(303, 90);
			this.btnSetupMultitap4.Name = "btnSetupMultitap4";
			this.btnSetupMultitap4.Size = new System.Drawing.Size(62, 23);
			this.btnSetupMultitap4.TabIndex = 25;
			this.btnSetupMultitap4.Text = "Setup";
			this.btnSetupMultitap4.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.btnSetupMultitap4.UseVisualStyleBackColor = true;
			this.btnSetupMultitap4.Click += new System.EventHandler(this.btnSetup_Click);
			// 
			// cboMultitap1
			// 
			this.cboMultitap1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboMultitap1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboMultitap1.FormattingEnabled = true;
			this.cboMultitap1.Location = new System.Drawing.Point(57, 3);
			this.cboMultitap1.Name = "cboMultitap1";
			this.cboMultitap1.Size = new System.Drawing.Size(240, 21);
			this.cboMultitap1.TabIndex = 7;
			// 
			// cboMultitap4
			// 
			this.cboMultitap4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboMultitap4.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboMultitap4.FormattingEnabled = true;
			this.cboMultitap4.Location = new System.Drawing.Point(57, 90);
			this.cboMultitap4.Name = "cboMultitap4";
			this.cboMultitap4.Size = new System.Drawing.Size(240, 21);
			this.cboMultitap4.TabIndex = 24;
			// 
			// lblMultitap4
			// 
			this.lblMultitap4.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblMultitap4.AutoSize = true;
			this.lblMultitap4.Location = new System.Drawing.Point(3, 95);
			this.lblMultitap4.Name = "lblMultitap4";
			this.lblMultitap4.Size = new System.Drawing.Size(48, 13);
			this.lblMultitap4.TabIndex = 23;
			this.lblMultitap4.Text = "Player 4:";
			// 
			// lblMultitap3
			// 
			this.lblMultitap3.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblMultitap3.AutoSize = true;
			this.lblMultitap3.Location = new System.Drawing.Point(3, 66);
			this.lblMultitap3.Name = "lblMultitap3";
			this.lblMultitap3.Size = new System.Drawing.Size(48, 13);
			this.lblMultitap3.TabIndex = 20;
			this.lblMultitap3.Text = "Player 3:";
			// 
			// lblMultitap2
			// 
			this.lblMultitap2.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblMultitap2.AutoSize = true;
			this.lblMultitap2.Location = new System.Drawing.Point(3, 37);
			this.lblMultitap2.Name = "lblMultitap2";
			this.lblMultitap2.Size = new System.Drawing.Size(48, 13);
			this.lblMultitap2.TabIndex = 3;
			this.lblMultitap2.Text = "Player 3:";
			// 
			// lblMultitap1
			// 
			this.lblMultitap1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblMultitap1.AutoSize = true;
			this.lblMultitap1.Location = new System.Drawing.Point(3, 8);
			this.lblMultitap1.Name = "lblMultitap1";
			this.lblMultitap1.Size = new System.Drawing.Size(48, 13);
			this.lblMultitap1.TabIndex = 2;
			this.lblMultitap1.Text = "Player 2:";
			// 
			// frmInputConfig
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(382, 315);
			this.Controls.Add(this.tabMain);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "frmInputConfig";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Input Config";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tabMain, 0);
			this.tabMain.ResumeLayout(false);
			this.tpgControllers.ResumeLayout(false);
			this.tlpControllers.ResumeLayout(false);
			this.tlpControllers.PerformLayout();
			this.pnlConflictWarning.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picWarning)).EndInit();
			this.grpMultitap.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tpgControllers;
		private System.Windows.Forms.TableLayoutPanel tlpControllers;
		private System.Windows.Forms.Button btnSetupMultitap2;
		private System.Windows.Forms.Button btnSetupMultitap1;
		private System.Windows.Forms.Label lblPlayer1;
		private System.Windows.Forms.Label lblPlayer2;
		private System.Windows.Forms.ComboBox cboMultitap2;
		private System.Windows.Forms.ComboBox cboMultitap1;
		private System.Windows.Forms.ComboBox cboPlayer1;
		private System.Windows.Forms.Label lblMultitap2;
		private System.Windows.Forms.ComboBox cboPlayer2;
		private System.Windows.Forms.Label lblMultitap1;
		private System.Windows.Forms.Button btnSetupP1;
		private System.Windows.Forms.Button btnSetupP2;
		private System.Windows.Forms.Panel pnlConflictWarning;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.PictureBox picWarning;
		private System.Windows.Forms.Label lblKeyBinding;
		private System.Windows.Forms.Button btnSetupMultitap3;
		private System.Windows.Forms.ComboBox cboMultitap3;
		private System.Windows.Forms.Label lblMultitap3;
		private System.Windows.Forms.Label lblMultitap4;
		private System.Windows.Forms.ComboBox cboMultitap4;
		private System.Windows.Forms.Button btnSetupMultitap4;
		private System.Windows.Forms.GroupBox grpMultitap;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
	}
}