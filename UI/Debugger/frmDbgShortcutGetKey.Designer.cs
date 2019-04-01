namespace Mesen.GUI.Debugger
{
	partial class frmDbgShortcutGetKey
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
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.lblSetKeyMessage = new System.Windows.Forms.Label();
			this.lblCurrentKeys = new System.Windows.Forms.Label();
			this.groupBox1.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// groupBox1
			// 
			this.groupBox1.AutoSize = true;
			this.groupBox1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.groupBox1.Controls.Add(this.tableLayoutPanel1);
			this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.groupBox1.Location = new System.Drawing.Point(3, 0);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(391, 105);
			this.groupBox1.TabIndex = 0;
			this.groupBox1.TabStop = false;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.AutoSize = true;
			this.tableLayoutPanel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.tableLayoutPanel1.ColumnCount = 1;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.lblSetKeyMessage, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblCurrentKeys, 0, 1);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(385, 86);
			this.tableLayoutPanel1.TabIndex = 2;
			// 
			// lblSetKeyMessage
			// 
			this.lblSetKeyMessage.AutoSize = true;
			this.lblSetKeyMessage.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lblSetKeyMessage.Location = new System.Drawing.Point(3, 0);
			this.lblSetKeyMessage.Name = "lblSetKeyMessage";
			this.lblSetKeyMessage.Size = new System.Drawing.Size(379, 43);
			this.lblSetKeyMessage.TabIndex = 0;
			this.lblSetKeyMessage.Text = "Press any key on your keyboard to set a new binding.";
			this.lblSetKeyMessage.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// lblCurrentKeys
			// 
			this.lblCurrentKeys.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lblCurrentKeys.Location = new System.Drawing.Point(3, 43);
			this.lblCurrentKeys.Name = "lblCurrentKeys";
			this.lblCurrentKeys.Size = new System.Drawing.Size(379, 43);
			this.lblCurrentKeys.TabIndex = 1;
			this.lblCurrentKeys.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// frmDbgShortcutGetKey
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(397, 108);
			this.Controls.Add(this.groupBox1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "frmDbgShortcutGetKey";
			this.Padding = new System.Windows.Forms.Padding(3, 0, 3, 3);
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Set key binding...";
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Label lblSetKeyMessage;
		private System.Windows.Forms.Label lblCurrentKeys;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
	}
}