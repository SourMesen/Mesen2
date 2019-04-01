using Mesen.GUI.Controls;

namespace Mesen.GUI.Debugger
{
	partial class frmBreakIn
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
			this.radFrames = new System.Windows.Forms.RadioButton();
			this.nudCount = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.radCpuInstructions = new System.Windows.Forms.RadioButton();
			this.radScanlines = new System.Windows.Forms.RadioButton();
			this.lblBreakIn = new System.Windows.Forms.Label();
			this.radPpuCycles = new System.Windows.Forms.RadioButton();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 70);
			this.baseConfigPanel.Size = new System.Drawing.Size(333, 29);
			// 
			// radFrames
			// 
			this.radFrames.AutoSize = true;
			this.radFrames.Location = new System.Drawing.Point(244, 49);
			this.radFrames.Name = "radFrames";
			this.radFrames.Size = new System.Drawing.Size(59, 17);
			this.radFrames.TabIndex = 4;
			this.radFrames.Text = "Frames";
			this.radFrames.UseVisualStyleBackColor = true;
			// 
			// nudCount
			// 
			this.nudCount.DecimalPlaces = 0;
			this.nudCount.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudCount.Location = new System.Drawing.Point(58, 3);
			this.nudCount.Maximum = new decimal(new int[] {
            2000000000,
            0,
            0,
            0});
			this.nudCount.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudCount.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudCount.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudCount.Name = "nudCount";
			this.tableLayoutPanel1.SetRowSpan(this.nudCount, 2);
			this.nudCount.Size = new System.Drawing.Size(70, 21);
			this.nudCount.TabIndex = 3;
			this.nudCount.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			// 
			// radCpuInstructions
			// 
			this.radCpuInstructions.AutoSize = true;
			this.radCpuInstructions.Location = new System.Drawing.Point(134, 3);
			this.radCpuInstructions.Name = "radCpuInstructions";
			this.radCpuInstructions.Size = new System.Drawing.Size(104, 17);
			this.radCpuInstructions.TabIndex = 0;
			this.radCpuInstructions.Text = "CPU Instructions";
			this.radCpuInstructions.UseVisualStyleBackColor = true;
			// 
			// radScanlines
			// 
			this.radScanlines.AutoSize = true;
			this.radScanlines.Location = new System.Drawing.Point(244, 26);
			this.radScanlines.Name = "radScanlines";
			this.radScanlines.Size = new System.Drawing.Size(71, 17);
			this.radScanlines.TabIndex = 3;
			this.radScanlines.Text = "Scanlines";
			this.radScanlines.UseVisualStyleBackColor = true;
			// 
			// lblBreakIn
			// 
			this.lblBreakIn.AutoSize = true;
			this.lblBreakIn.Location = new System.Drawing.Point(3, 5);
			this.lblBreakIn.Margin = new System.Windows.Forms.Padding(3, 5, 3, 0);
			this.lblBreakIn.Name = "lblBreakIn";
			this.tableLayoutPanel1.SetRowSpan(this.lblBreakIn, 2);
			this.lblBreakIn.Size = new System.Drawing.Size(49, 13);
			this.lblBreakIn.TabIndex = 0;
			this.lblBreakIn.Text = "Break in:";
			// 
			// radPpuCycles
			// 
			this.radPpuCycles.AutoSize = true;
			this.radPpuCycles.Location = new System.Drawing.Point(244, 3);
			this.radPpuCycles.Name = "radPpuCycles";
			this.radPpuCycles.Size = new System.Drawing.Size(81, 17);
			this.radPpuCycles.TabIndex = 1;
			this.radPpuCycles.Text = "PPU Cycles";
			this.radPpuCycles.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 4;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 76F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.radPpuCycles, 3, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblBreakIn, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.radScanlines, 3, 1);
			this.tableLayoutPanel1.Controls.Add(this.nudCount, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.radFrames, 3, 2);
			this.tableLayoutPanel1.Controls.Add(this.radCpuInstructions, 2, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 4;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(333, 99);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// frmBreakIn
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(333, 99);
			this.Controls.Add(this.tableLayoutPanel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "frmBreakIn";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Break In...";
			this.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion
		private System.Windows.Forms.RadioButton radFrames;
		private MesenNumericUpDown nudCount;
		private System.Windows.Forms.RadioButton radCpuInstructions;
		private System.Windows.Forms.RadioButton radScanlines;
		private System.Windows.Forms.Label lblBreakIn;
		private System.Windows.Forms.RadioButton radPpuCycles;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
	}
}