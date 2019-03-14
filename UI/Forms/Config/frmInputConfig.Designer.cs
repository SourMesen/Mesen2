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
			this.btnSetupP5 = new System.Windows.Forms.Button();
			this.cboPlayer5 = new System.Windows.Forms.ComboBox();
			this.lblPlayer5 = new System.Windows.Forms.Label();
			this.btnSetupP4 = new System.Windows.Forms.Button();
			this.btnSetupP3 = new System.Windows.Forms.Button();
			this.lblPlayer1 = new System.Windows.Forms.Label();
			this.lblPlayer2 = new System.Windows.Forms.Label();
			this.cboPlayer4 = new System.Windows.Forms.ComboBox();
			this.cboPlayer3 = new System.Windows.Forms.ComboBox();
			this.cboPlayer1 = new System.Windows.Forms.ComboBox();
			this.lblPlayer4 = new System.Windows.Forms.Label();
			this.cboPlayer2 = new System.Windows.Forms.ComboBox();
			this.lblPlayer3 = new System.Windows.Forms.Label();
			this.btnSetupP1 = new System.Windows.Forms.Button();
			this.btnSetupP2 = new System.Windows.Forms.Button();
			this.pnlConflictWarning = new System.Windows.Forms.Panel();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.picWarning = new System.Windows.Forms.PictureBox();
			this.lblKeyBinding = new System.Windows.Forms.Label();
			this.tabMain.SuspendLayout();
			this.tpgControllers.SuspendLayout();
			this.tlpControllers.SuspendLayout();
			this.pnlConflictWarning.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picWarning)).BeginInit();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Size = new System.Drawing.Size(382, 29);
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tpgControllers);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(382, 233);
			this.tabMain.TabIndex = 2;
			// 
			// tpgControllers
			// 
			this.tpgControllers.Controls.Add(this.tlpControllers);
			this.tpgControllers.Location = new System.Drawing.Point(4, 22);
			this.tpgControllers.Name = "tpgControllers";
			this.tpgControllers.Size = new System.Drawing.Size(374, 207);
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
			this.tlpControllers.Controls.Add(this.btnSetupP5, 2, 5);
			this.tlpControllers.Controls.Add(this.cboPlayer5, 1, 5);
			this.tlpControllers.Controls.Add(this.lblPlayer5, 0, 5);
			this.tlpControllers.Controls.Add(this.btnSetupP4, 2, 4);
			this.tlpControllers.Controls.Add(this.btnSetupP3, 2, 3);
			this.tlpControllers.Controls.Add(this.lblPlayer1, 0, 1);
			this.tlpControllers.Controls.Add(this.lblPlayer2, 0, 2);
			this.tlpControllers.Controls.Add(this.cboPlayer4, 1, 4);
			this.tlpControllers.Controls.Add(this.cboPlayer3, 1, 3);
			this.tlpControllers.Controls.Add(this.cboPlayer1, 1, 1);
			this.tlpControllers.Controls.Add(this.lblPlayer4, 0, 4);
			this.tlpControllers.Controls.Add(this.cboPlayer2, 1, 2);
			this.tlpControllers.Controls.Add(this.lblPlayer3, 0, 3);
			this.tlpControllers.Controls.Add(this.btnSetupP1, 2, 1);
			this.tlpControllers.Controls.Add(this.btnSetupP2, 2, 2);
			this.tlpControllers.Controls.Add(this.pnlConflictWarning, 0, 0);
			this.tlpControllers.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpControllers.Location = new System.Drawing.Point(0, 0);
			this.tlpControllers.Name = "tlpControllers";
			this.tlpControllers.RowCount = 6;
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpControllers.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpControllers.Size = new System.Drawing.Size(374, 207);
			this.tlpControllers.TabIndex = 0;
			// 
			// btnSetupP5
			// 
			this.btnSetupP5.AutoSize = true;
			this.btnSetupP5.Location = new System.Drawing.Point(309, 180);
			this.btnSetupP5.Name = "btnSetupP5";
			this.btnSetupP5.Size = new System.Drawing.Size(62, 23);
			this.btnSetupP5.TabIndex = 22;
			this.btnSetupP5.Text = "Setup";
			this.btnSetupP5.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.btnSetupP5.UseVisualStyleBackColor = true;
			this.btnSetupP5.Visible = false;
			// 
			// cboPlayer5
			// 
			this.cboPlayer5.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboPlayer5.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboPlayer5.FormattingEnabled = true;
			this.cboPlayer5.Location = new System.Drawing.Point(57, 180);
			this.cboPlayer5.Name = "cboPlayer5";
			this.cboPlayer5.Size = new System.Drawing.Size(246, 21);
			this.cboPlayer5.TabIndex = 21;
			this.cboPlayer5.Visible = false;
			// 
			// lblPlayer5
			// 
			this.lblPlayer5.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPlayer5.AutoSize = true;
			this.lblPlayer5.Location = new System.Drawing.Point(3, 185);
			this.lblPlayer5.Name = "lblPlayer5";
			this.lblPlayer5.Size = new System.Drawing.Size(48, 13);
			this.lblPlayer5.TabIndex = 20;
			this.lblPlayer5.Text = "Player 5:";
			this.lblPlayer5.Visible = false;
			// 
			// btnSetupP4
			// 
			this.btnSetupP4.AutoSize = true;
			this.btnSetupP4.Location = new System.Drawing.Point(309, 151);
			this.btnSetupP4.Name = "btnSetupP4";
			this.btnSetupP4.Size = new System.Drawing.Size(62, 23);
			this.btnSetupP4.TabIndex = 12;
			this.btnSetupP4.Text = "Setup";
			this.btnSetupP4.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.btnSetupP4.UseVisualStyleBackColor = true;
			this.btnSetupP4.Visible = false;
			this.btnSetupP4.Click += new System.EventHandler(this.btnSetup_Click);
			// 
			// btnSetupP3
			// 
			this.btnSetupP3.AutoSize = true;
			this.btnSetupP3.Location = new System.Drawing.Point(309, 122);
			this.btnSetupP3.Name = "btnSetupP3";
			this.btnSetupP3.Size = new System.Drawing.Size(62, 23);
			this.btnSetupP3.TabIndex = 11;
			this.btnSetupP3.Text = "Setup";
			this.btnSetupP3.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.btnSetupP3.UseVisualStyleBackColor = true;
			this.btnSetupP3.Visible = false;
			this.btnSetupP3.Click += new System.EventHandler(this.btnSetup_Click);
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
			// cboPlayer4
			// 
			this.cboPlayer4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboPlayer4.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboPlayer4.FormattingEnabled = true;
			this.cboPlayer4.Location = new System.Drawing.Point(57, 151);
			this.cboPlayer4.Name = "cboPlayer4";
			this.cboPlayer4.Size = new System.Drawing.Size(246, 21);
			this.cboPlayer4.TabIndex = 8;
			this.cboPlayer4.Visible = false;
			// 
			// cboPlayer3
			// 
			this.cboPlayer3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboPlayer3.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboPlayer3.FormattingEnabled = true;
			this.cboPlayer3.Location = new System.Drawing.Point(57, 122);
			this.cboPlayer3.Name = "cboPlayer3";
			this.cboPlayer3.Size = new System.Drawing.Size(246, 21);
			this.cboPlayer3.TabIndex = 7;
			this.cboPlayer3.Visible = false;
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
			// 
			// lblPlayer4
			// 
			this.lblPlayer4.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPlayer4.AutoSize = true;
			this.lblPlayer4.Location = new System.Drawing.Point(3, 156);
			this.lblPlayer4.Name = "lblPlayer4";
			this.lblPlayer4.Size = new System.Drawing.Size(48, 13);
			this.lblPlayer4.TabIndex = 3;
			this.lblPlayer4.Text = "Player 4:";
			this.lblPlayer4.Visible = false;
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
			// 
			// lblPlayer3
			// 
			this.lblPlayer3.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPlayer3.AutoSize = true;
			this.lblPlayer3.Location = new System.Drawing.Point(3, 127);
			this.lblPlayer3.Name = "lblPlayer3";
			this.lblPlayer3.Size = new System.Drawing.Size(48, 13);
			this.lblPlayer3.TabIndex = 2;
			this.lblPlayer3.Text = "Player 3:";
			this.lblPlayer3.Visible = false;
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
			// frmInputConfig
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(382, 262);
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
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tpgControllers;
		private System.Windows.Forms.TableLayoutPanel tlpControllers;
		private System.Windows.Forms.Button btnSetupP4;
		private System.Windows.Forms.Button btnSetupP3;
		private System.Windows.Forms.Label lblPlayer1;
		private System.Windows.Forms.Label lblPlayer2;
		private System.Windows.Forms.ComboBox cboPlayer4;
		private System.Windows.Forms.ComboBox cboPlayer3;
		private System.Windows.Forms.ComboBox cboPlayer1;
		private System.Windows.Forms.Label lblPlayer4;
		private System.Windows.Forms.ComboBox cboPlayer2;
		private System.Windows.Forms.Label lblPlayer3;
		private System.Windows.Forms.Button btnSetupP1;
		private System.Windows.Forms.Button btnSetupP2;
		private System.Windows.Forms.Panel pnlConflictWarning;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.PictureBox picWarning;
		private System.Windows.Forms.Label lblKeyBinding;
		private System.Windows.Forms.Button btnSetupP5;
		private System.Windows.Forms.ComboBox cboPlayer5;
		private System.Windows.Forms.Label lblPlayer5;
	}
}