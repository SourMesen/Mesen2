using Mesen.GUI.Controls;

namespace Mesen.GUI.Forms.NetPlay
{
	partial class frmServerConfig
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
			this.tlpMain = new System.Windows.Forms.TableLayoutPanel();
			this.txtPort = new System.Windows.Forms.TextBox();
			this.lblPort = new System.Windows.Forms.Label();
			this.lblServerName = new System.Windows.Forms.Label();
			this.txtServerName = new System.Windows.Forms.TextBox();
			this.lblPassword = new System.Windows.Forms.Label();
			this.txtPassword = new System.Windows.Forms.TextBox();
			this.tlpMain.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 98);
			this.baseConfigPanel.Size = new System.Drawing.Size(302, 29);
			// 
			// tlpMain
			// 
			this.tlpMain.ColumnCount = 2;
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpMain.Controls.Add(this.txtPort, 1, 1);
			this.tlpMain.Controls.Add(this.lblPort, 0, 1);
			this.tlpMain.Controls.Add(this.lblServerName, 0, 0);
			this.tlpMain.Controls.Add(this.txtServerName, 1, 0);
			this.tlpMain.Controls.Add(this.lblPassword, 0, 2);
			this.tlpMain.Controls.Add(this.txtPassword, 1, 2);
			this.tlpMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpMain.Location = new System.Drawing.Point(0, 0);
			this.tlpMain.Name = "tlpMain";
			this.tlpMain.RowCount = 4;
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpMain.Size = new System.Drawing.Size(302, 98);
			this.tlpMain.TabIndex = 1;
			// 
			// txtPort
			// 
			this.txtPort.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtPort.Location = new System.Drawing.Point(79, 29);
			this.txtPort.Name = "txtPort";
			this.txtPort.Size = new System.Drawing.Size(220, 20);
			this.txtPort.TabIndex = 13;
			this.txtPort.TextChanged += new System.EventHandler(this.Field_ValueChanged);
			// 
			// lblPort
			// 
			this.lblPort.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPort.AutoSize = true;
			this.lblPort.Location = new System.Drawing.Point(3, 32);
			this.lblPort.Name = "lblPort";
			this.lblPort.Size = new System.Drawing.Size(29, 13);
			this.lblPort.TabIndex = 12;
			this.lblPort.Text = "Port:";
			// 
			// lblServerName
			// 
			this.lblServerName.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblServerName.AutoSize = true;
			this.lblServerName.Location = new System.Drawing.Point(3, 6);
			this.lblServerName.Name = "lblServerName";
			this.lblServerName.Size = new System.Drawing.Size(70, 13);
			this.lblServerName.TabIndex = 3;
			this.lblServerName.Text = "Server name:";
			// 
			// txtServerName
			// 
			this.txtServerName.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtServerName.Location = new System.Drawing.Point(79, 3);
			this.txtServerName.Name = "txtServerName";
			this.txtServerName.Size = new System.Drawing.Size(220, 20);
			this.txtServerName.TabIndex = 5;
			this.txtServerName.TextChanged += new System.EventHandler(this.Field_ValueChanged);
			// 
			// lblPassword
			// 
			this.lblPassword.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPassword.AutoSize = true;
			this.lblPassword.Location = new System.Drawing.Point(3, 58);
			this.lblPassword.Name = "lblPassword";
			this.lblPassword.Size = new System.Drawing.Size(56, 13);
			this.lblPassword.TabIndex = 9;
			this.lblPassword.Text = "Password:";
			// 
			// txtPassword
			// 
			this.txtPassword.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtPassword.Location = new System.Drawing.Point(79, 55);
			this.txtPassword.Name = "txtPassword";
			this.txtPassword.Size = new System.Drawing.Size(220, 20);
			this.txtPassword.TabIndex = 10;
			this.txtPassword.UseSystemPasswordChar = true;
			this.txtPassword.TextChanged += new System.EventHandler(this.Field_ValueChanged);
			// 
			// frmServerConfig
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(302, 127);
			this.Controls.Add(this.tlpMain);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "frmServerConfig";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Server Configuration";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tlpMain, 0);
			this.tlpMain.ResumeLayout(false);
			this.tlpMain.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tlpMain;
		private System.Windows.Forms.Label lblServerName;
		private System.Windows.Forms.TextBox txtServerName;
		private System.Windows.Forms.Label lblPassword;
		private System.Windows.Forms.TextBox txtPassword;
		private System.Windows.Forms.Label lblPort;
		private System.Windows.Forms.TextBox txtPort;
	}
}