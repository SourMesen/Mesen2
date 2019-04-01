using Mesen.GUI.Controls;

namespace Mesen.GUI.Debugger
{
	partial class frmBreakOn
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
			this.nudCount = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblBreakOn = new System.Windows.Forms.Label();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 29);
			this.baseConfigPanel.Size = new System.Drawing.Size(186, 29);
			// 
			// nudCount
			// 
			this.nudCount.DecimalPlaces = 0;
			this.nudCount.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudCount.Location = new System.Drawing.Point(104, 3);
			this.nudCount.Maximum = new decimal(new int[] {
            999,
            0,
            0,
            0});
			this.nudCount.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudCount.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
			this.nudCount.Name = "nudCount";
			this.tableLayoutPanel1.SetRowSpan(this.nudCount, 2);
			this.nudCount.Size = new System.Drawing.Size(70, 20);
			this.nudCount.TabIndex = 3;
			this.nudCount.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			// 
			// lblBreakOn
			// 
			this.lblBreakOn.AutoSize = true;
			this.lblBreakOn.Location = new System.Drawing.Point(3, 5);
			this.lblBreakOn.Margin = new System.Windows.Forms.Padding(3, 5, 3, 0);
			this.lblBreakOn.Name = "lblBreakOn";
			this.tableLayoutPanel1.SetRowSpan(this.lblBreakOn, 2);
			this.lblBreakOn.Size = new System.Drawing.Size(95, 13);
			this.lblBreakOn.TabIndex = 0;
			this.lblBreakOn.Text = "Break on scanline:";
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 4;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 76F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.lblBreakOn, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.nudCount, 1, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 4;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(186, 58);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// frmBreakOn
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(186, 58);
			this.Controls.Add(this.tableLayoutPanel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "frmBreakOn";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Break On...";
			this.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion
		private MesenNumericUpDown nudCount;
		private System.Windows.Forms.Label lblBreakOn;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
	}
}