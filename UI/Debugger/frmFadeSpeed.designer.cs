using Mesen.GUI.Controls;

namespace Mesen.GUI.Debugger
{
	partial class frmFadeSpeed
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
			this.lblAddress = new System.Windows.Forms.Label();
			this.lblFrames = new System.Windows.Forms.Label();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.nudFrameCount = new MesenNumericUpDown();
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 28);
			this.baseConfigPanel.Size = new System.Drawing.Size(226, 29);
			// 
			// lblAddress
			// 
			this.lblAddress.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblAddress.AutoSize = true;
			this.lblAddress.Location = new System.Drawing.Point(3, 6);
			this.lblAddress.Name = "lblAddress";
			this.lblAddress.Size = new System.Drawing.Size(55, 13);
			this.lblAddress.TabIndex = 0;
			this.lblAddress.Text = "Fade after";
			// 
			// lblFrames
			// 
			this.lblFrames.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblFrames.AutoSize = true;
			this.lblFrames.Location = new System.Drawing.Point(149, 6);
			this.lblFrames.Name = "lblFrames";
			this.lblFrames.Size = new System.Drawing.Size(38, 13);
			this.lblFrames.TabIndex = 2;
			this.lblFrames.Text = "frames";
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 3;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 80F));
			this.tableLayoutPanel1.Controls.Add(this.lblFrames, 2, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblAddress, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.nudFrameCount, 1, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(226, 57);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// nudFrameCount
			// 
			this.nudFrameCount.Dock = System.Windows.Forms.DockStyle.Fill;
			this.nudFrameCount.Location = new System.Drawing.Point(64, 3);
			this.nudFrameCount.Maximum = new decimal(new int[] {
            99999,
            0,
            0,
            0});
			this.nudFrameCount.Name = "nudFrameCount";
			this.nudFrameCount.Size = new System.Drawing.Size(79, 20);
			this.nudFrameCount.TabIndex = 3;
			this.nudFrameCount.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			// 
			// frmFadeSpeed
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(226, 57);
			this.Controls.Add(this.tableLayoutPanel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "frmFadeSpeed";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Fade speed...";
			this.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Label lblAddress;
		private System.Windows.Forms.Label lblFrames;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private MesenNumericUpDown nudFrameCount;
	}
}