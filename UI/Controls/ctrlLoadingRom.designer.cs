namespace Mesen.GUI.Controls
{
	partial class ctrlLoadingRom
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
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.pbLoading = new System.Windows.Forms.ProgressBar();
			this.lblExtractingFile = new System.Windows.Forms.Label();
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 3;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 200F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.Controls.Add(this.pbLoading, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.lblExtractingFile, 0, 2);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 4;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(261, 107);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// pbLoading
			// 
			this.pbLoading.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pbLoading.Location = new System.Drawing.Point(33, 35);
			this.pbLoading.Name = "pbLoading";
			this.pbLoading.Size = new System.Drawing.Size(194, 23);
			this.pbLoading.Style = System.Windows.Forms.ProgressBarStyle.Marquee;
			this.pbLoading.TabIndex = 0;
			// 
			// lblExtractingFile
			// 
			this.lblExtractingFile.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.lblExtractingFile.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.lblExtractingFile, 3);
			this.lblExtractingFile.ForeColor = System.Drawing.Color.White;
			this.lblExtractingFile.Location = new System.Drawing.Point(61, 61);
			this.lblExtractingFile.Name = "lblExtractingFile";
			this.lblExtractingFile.Size = new System.Drawing.Size(138, 13);
			this.lblExtractingFile.TabIndex = 1;
			this.lblExtractingFile.Text = "Extracting file, please wait...";
			// 
			// ctrlLoadingRom
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.Black;
			this.Controls.Add(this.tableLayoutPanel1);
			this.Name = "ctrlLoadingRom";
			this.Size = new System.Drawing.Size(261, 107);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.ProgressBar pbLoading;
		private System.Windows.Forms.Label lblExtractingFile;
	}
}
