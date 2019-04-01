namespace Mesen.GUI.Forms.Config
{
	partial class frmCopyFiles
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
			this.components = new System.ComponentModel.Container();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.pbProgress = new System.Windows.Forms.ProgressBar();
			this.lblCopying = new System.Windows.Forms.Label();
			this.lblTarget = new System.Windows.Forms.Label();
			this.tmrProgress = new System.Windows.Forms.Timer(this.components);
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.lblTarget, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.pbProgress, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.lblCopying, 0, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.Size = new System.Drawing.Size(268, 64);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// pbProgress
			// 
			this.tableLayoutPanel1.SetColumnSpan(this.pbProgress, 2);
			this.pbProgress.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pbProgress.Location = new System.Drawing.Point(3, 38);
			this.pbProgress.Name = "pbProgress";
			this.pbProgress.Size = new System.Drawing.Size(262, 23);
			this.pbProgress.TabIndex = 0;
			// 
			// lblCopying
			// 
			this.lblCopying.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblCopying.AutoSize = true;
			this.lblCopying.Location = new System.Drawing.Point(3, 11);
			this.lblCopying.Name = "lblCopying";
			this.lblCopying.Size = new System.Drawing.Size(48, 13);
			this.lblCopying.TabIndex = 1;
			this.lblCopying.Text = "Copying:";
			// 
			// lblTarget
			// 
			this.lblTarget.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblTarget.AutoSize = true;
			this.lblTarget.Location = new System.Drawing.Point(57, 11);
			this.lblTarget.Name = "lblTarget";
			this.lblTarget.Size = new System.Drawing.Size(19, 13);
			this.lblTarget.TabIndex = 2;
			this.lblTarget.Text = "....";
			// 
			// tmrProgress
			// 
			this.tmrProgress.Tick += new System.EventHandler(this.tmrProgress_Tick);
			// 
			// frmCopyFiles
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(268, 64);
			this.ControlBox = false;
			this.Controls.Add(this.tableLayoutPanel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "frmCopyFiles";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Please wait...";
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label lblTarget;
		private System.Windows.Forms.ProgressBar pbProgress;
		private System.Windows.Forms.Label lblCopying;
		private System.Windows.Forms.Timer tmrProgress;
	}
}