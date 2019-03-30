namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlScanlineCycleSelect
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
			this.flpRefreshTiming = new System.Windows.Forms.FlowLayoutPanel();
			this.lblShowFrameAt = new System.Windows.Forms.Label();
			this.nudScanline = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblCycle = new System.Windows.Forms.Label();
			this.nudCycle = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.btnReset = new System.Windows.Forms.Button();
			this.flpRefreshTiming.SuspendLayout();
			this.SuspendLayout();
			// 
			// flpRefreshTiming
			// 
			this.flpRefreshTiming.Controls.Add(this.lblShowFrameAt);
			this.flpRefreshTiming.Controls.Add(this.nudScanline);
			this.flpRefreshTiming.Controls.Add(this.lblCycle);
			this.flpRefreshTiming.Controls.Add(this.nudCycle);
			this.flpRefreshTiming.Controls.Add(this.btnReset);
			this.flpRefreshTiming.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flpRefreshTiming.Location = new System.Drawing.Point(0, 0);
			this.flpRefreshTiming.Name = "flpRefreshTiming";
			this.flpRefreshTiming.Size = new System.Drawing.Size(317, 28);
			this.flpRefreshTiming.TabIndex = 5;
			// 
			// lblShowFrameAt
			// 
			this.lblShowFrameAt.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblShowFrameAt.AutoSize = true;
			this.lblShowFrameAt.Location = new System.Drawing.Point(3, 8);
			this.lblShowFrameAt.Name = "lblShowFrameAt";
			this.lblShowFrameAt.Size = new System.Drawing.Size(101, 13);
			this.lblShowFrameAt.TabIndex = 0;
			this.lblShowFrameAt.Text = "Refresh on scanline";
			// 
			// nudScanline
			// 
			this.nudScanline.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.nudScanline.DecimalPlaces = 0;
			this.nudScanline.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudScanline.Location = new System.Drawing.Point(110, 4);
			this.nudScanline.Maximum = new decimal(new int[] {
            312,
            0,
            0,
            0});
			this.nudScanline.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudScanline.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudScanline.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudScanline.Name = "nudScanline";
			this.nudScanline.Size = new System.Drawing.Size(42, 21);
			this.nudScanline.TabIndex = 5;
			this.nudScanline.Value = new decimal(new int[] {
            241,
            0,
            0,
            0});
			this.nudScanline.ValueChanged += new System.EventHandler(this.nudScanlineCycle_ValueChanged);
			// 
			// lblCycle
			// 
			this.lblCycle.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblCycle.AutoSize = true;
			this.lblCycle.Location = new System.Drawing.Point(158, 8);
			this.lblCycle.Name = "lblCycle";
			this.lblCycle.Size = new System.Drawing.Size(53, 13);
			this.lblCycle.TabIndex = 5;
			this.lblCycle.Text = "and cycle";
			// 
			// nudCycle
			// 
			this.nudCycle.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.nudCycle.DecimalPlaces = 0;
			this.nudCycle.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudCycle.Location = new System.Drawing.Point(217, 4);
			this.nudCycle.Maximum = new decimal(new int[] {
            340,
            0,
            0,
            0});
			this.nudCycle.MaximumSize = new System.Drawing.Size(10000, 20);
			this.nudCycle.Minimum = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudCycle.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudCycle.Name = "nudCycle";
			this.nudCycle.Size = new System.Drawing.Size(42, 21);
			this.nudCycle.TabIndex = 6;
			this.nudCycle.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
			this.nudCycle.ValueChanged += new System.EventHandler(this.nudScanlineCycle_ValueChanged);
			// 
			// btnReset
			// 
			this.btnReset.Location = new System.Drawing.Point(265, 3);
			this.btnReset.Name = "btnReset";
			this.btnReset.Size = new System.Drawing.Size(49, 23);
			this.btnReset.TabIndex = 7;
			this.btnReset.Text = "Reset";
			this.btnReset.UseVisualStyleBackColor = true;
			this.btnReset.Click += new System.EventHandler(this.btnReset_Click);
			// 
			// ctrlScanlineCycleSelect
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.flpRefreshTiming);
			this.Name = "ctrlScanlineCycleSelect";
			this.Size = new System.Drawing.Size(317, 28);
			this.flpRefreshTiming.ResumeLayout(false);
			this.flpRefreshTiming.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.FlowLayoutPanel flpRefreshTiming;
		private System.Windows.Forms.Label lblShowFrameAt;
		private GUI.Controls.MesenNumericUpDown nudScanline;
		private System.Windows.Forms.Label lblCycle;
		private GUI.Controls.MesenNumericUpDown nudCycle;
		private System.Windows.Forms.Button btnReset;
	}
}
