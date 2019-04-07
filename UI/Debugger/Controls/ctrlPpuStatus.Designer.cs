namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlPpuStatus
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
			this.grpPpu = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.txtScanline = new System.Windows.Forms.TextBox();
			this.label8 = new System.Windows.Forms.Label();
			this.label9 = new System.Windows.Forms.Label();
			this.txtCycle = new System.Windows.Forms.TextBox();
			this.label10 = new System.Windows.Forms.Label();
			this.txtHClocks = new System.Windows.Forms.TextBox();
			this.grpPpu.SuspendLayout();
			this.tableLayoutPanel3.SuspendLayout();
			this.SuspendLayout();
			// 
			// grpPpu
			// 
			this.grpPpu.Controls.Add(this.tableLayoutPanel3);
			this.grpPpu.Dock = System.Windows.Forms.DockStyle.Top;
			this.grpPpu.Location = new System.Drawing.Point(0, 0);
			this.grpPpu.Name = "grpPpu";
			this.grpPpu.Size = new System.Drawing.Size(342, 47);
			this.grpPpu.TabIndex = 1;
			this.grpPpu.TabStop = false;
			this.grpPpu.Text = "PPU";
			// 
			// tableLayoutPanel3
			// 
			this.tableLayoutPanel3.ColumnCount = 6;
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Controls.Add(this.txtScanline, 0, 0);
			this.tableLayoutPanel3.Controls.Add(this.label8, 0, 0);
			this.tableLayoutPanel3.Controls.Add(this.label9, 2, 0);
			this.tableLayoutPanel3.Controls.Add(this.txtCycle, 3, 0);
			this.tableLayoutPanel3.Controls.Add(this.label10, 4, 0);
			this.tableLayoutPanel3.Controls.Add(this.txtHClocks, 5, 0);
			this.tableLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel3.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel3.Name = "tableLayoutPanel3";
			this.tableLayoutPanel3.RowCount = 2;
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Size = new System.Drawing.Size(336, 28);
			this.tableLayoutPanel3.TabIndex = 0;
			// 
			// txtScanline
			// 
			this.txtScanline.Location = new System.Drawing.Point(60, 3);
			this.txtScanline.Name = "txtScanline";
			this.txtScanline.Size = new System.Drawing.Size(33, 20);
			this.txtScanline.TabIndex = 3;
			this.txtScanline.Text = "555";
			// 
			// label8
			// 
			this.label8.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label8.AutoSize = true;
			this.label8.Location = new System.Drawing.Point(3, 6);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(51, 13);
			this.label8.TabIndex = 1;
			this.label8.Text = "Scanline:";
			// 
			// label9
			// 
			this.label9.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label9.AutoSize = true;
			this.label9.Location = new System.Drawing.Point(99, 6);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(36, 13);
			this.label9.TabIndex = 2;
			this.label9.Text = "Cycle:";
			// 
			// txtCycle
			// 
			this.txtCycle.Location = new System.Drawing.Point(141, 3);
			this.txtCycle.Name = "txtCycle";
			this.txtCycle.Size = new System.Drawing.Size(33, 20);
			this.txtCycle.TabIndex = 4;
			this.txtCycle.Text = "555";
			// 
			// label10
			// 
			this.label10.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label10.AutoSize = true;
			this.label10.Location = new System.Drawing.Point(180, 6);
			this.label10.Name = "label10";
			this.label10.Size = new System.Drawing.Size(53, 13);
			this.label10.TabIndex = 5;
			this.label10.Text = "H Clocks:";
			// 
			// txtHClocks
			// 
			this.txtHClocks.Location = new System.Drawing.Point(239, 3);
			this.txtHClocks.Name = "txtHClocks";
			this.txtHClocks.Size = new System.Drawing.Size(33, 20);
			this.txtHClocks.TabIndex = 6;
			this.txtHClocks.Text = "555";
			// 
			// ctrlPpuStatus
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.grpPpu);
			this.Name = "ctrlPpuStatus";
			this.Size = new System.Drawing.Size(342, 47);
			this.grpPpu.ResumeLayout(false);
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel3.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion
		private System.Windows.Forms.GroupBox grpPpu;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
		private System.Windows.Forms.TextBox txtScanline;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.Label label9;
		private System.Windows.Forms.TextBox txtCycle;
		private System.Windows.Forms.Label label10;
		private System.Windows.Forms.TextBox txtHClocks;
	}
}
