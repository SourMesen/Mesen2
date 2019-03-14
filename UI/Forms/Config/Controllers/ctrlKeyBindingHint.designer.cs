namespace Mesen.GUI.Forms.Config
{
	partial class ctrlKeyBindingHint
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.pnlHint = new System.Windows.Forms.Panel();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.lblHint = new System.Windows.Forms.Label();
			this.picHint = new System.Windows.Forms.PictureBox();
			this.pnlHint.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picHint)).BeginInit();
			this.SuspendLayout();
			// 
			// pnlHint
			// 
			this.pnlHint.BackColor = System.Drawing.Color.WhiteSmoke;
			this.pnlHint.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.pnlHint.Controls.Add(this.tableLayoutPanel1);
			this.pnlHint.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pnlHint.Location = new System.Drawing.Point(3, 0);
			this.pnlHint.Name = "pnlHint";
			this.pnlHint.Size = new System.Drawing.Size(441, 31);
			this.pnlHint.TabIndex = 6;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.lblHint, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.picHint, 0, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 1;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(439, 29);
			this.tableLayoutPanel1.TabIndex = 1;
			// 
			// lblHint
			// 
			this.lblHint.AutoSize = true;
			this.lblHint.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lblHint.Location = new System.Drawing.Point(25, 0);
			this.lblHint.Name = "lblHint";
			this.lblHint.Size = new System.Drawing.Size(411, 29);
			this.lblHint.TabIndex = 1;
			this.lblHint.Text = "Tabs with an icon contain key bindings for this player.\r\nEach button can be mappe" +
    "d to up to 4 different keyboard keys or gamepad buttons.";
			// 
			// picHint
			// 
			this.picHint.BackgroundImage = global::Mesen.GUI.Properties.Resources.Help;
			this.picHint.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
			this.picHint.Location = new System.Drawing.Point(3, 5);
			this.picHint.Margin = new System.Windows.Forms.Padding(3, 5, 3, 3);
			this.picHint.Name = "picHint";
			this.picHint.Size = new System.Drawing.Size(16, 16);
			this.picHint.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
			this.picHint.TabIndex = 0;
			this.picHint.TabStop = false;
			// 
			// ctrlKeyBindingHint
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.pnlHint);
			this.Name = "ctrlKeyBindingHint";
			this.Padding = new System.Windows.Forms.Padding(3, 0, 3, 0);
			this.Size = new System.Drawing.Size(447, 31);
			this.pnlHint.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picHint)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Panel pnlHint;
		private System.Windows.Forms.PictureBox picHint;
		private System.Windows.Forms.Label lblHint;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
	}
}
