namespace Mesen.GUI.Updates
{
	partial class frmDownloadProgress
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
			this.progressDownload = new System.Windows.Forms.ProgressBar();
			this.lblFilename = new System.Windows.Forms.Label();
			this.tmrStart = new System.Windows.Forms.Timer(this.components);
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// progressDownload
			// 
			this.progressDownload.Dock = System.Windows.Forms.DockStyle.Top;
			this.progressDownload.Location = new System.Drawing.Point(3, 38);
			this.progressDownload.Name = "progressDownload";
			this.progressDownload.Size = new System.Drawing.Size(328, 23);
			this.progressDownload.Style = System.Windows.Forms.ProgressBarStyle.Continuous;
			this.progressDownload.TabIndex = 0;
			// 
			// lblFilename
			// 
			this.lblFilename.AutoSize = true;
			this.lblFilename.Location = new System.Drawing.Point(3, 5);
			this.lblFilename.Margin = new System.Windows.Forms.Padding(3, 5, 3, 0);
			this.lblFilename.Name = "lblFilename";
			this.lblFilename.Size = new System.Drawing.Size(100, 13);
			this.lblFilename.TabIndex = 1;
			this.lblFilename.Text = "Downloading file: ...";
			// 
			// tmrStart
			// 
			this.tmrStart.Tick += new System.EventHandler(this.tmrStart_Tick);
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 1;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.lblFilename, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.progressDownload, 0, 1);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 35F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.Size = new System.Drawing.Size(334, 67);
			this.tableLayoutPanel1.TabIndex = 2;
			// 
			// frmDownloadProgress
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(334, 67);
			this.Controls.Add(this.tableLayoutPanel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.MaximumSize = new System.Drawing.Size(350, 500);
			this.Name = "frmDownloadProgress";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
			this.Text = "Downloading...";
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.ProgressBar progressDownload;
		private System.Windows.Forms.Label lblFilename;
		private System.Windows.Forms.Timer tmrStart;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
	}
}