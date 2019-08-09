namespace Mesen.GUI.Forms
{
	partial class frmRecordMovie
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
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.lblSaveTo = new System.Windows.Forms.Label();
			this.txtFilename = new System.Windows.Forms.TextBox();
			this.btnBrowse = new System.Windows.Forms.Button();
			this.txtAuthor = new System.Windows.Forms.TextBox();
			this.lblRecordFrom = new System.Windows.Forms.Label();
			this.lblAuthor = new System.Windows.Forms.Label();
			this.lblDescription = new System.Windows.Forms.Label();
			this.cboRecordFrom = new System.Windows.Forms.ComboBox();
			this.txtDescription = new System.Windows.Forms.TextBox();
			this.lblMovieInformation = new System.Windows.Forms.Label();
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 202);
			this.baseConfigPanel.Size = new System.Drawing.Size(397, 29);
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 3;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.lblSaveTo, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtFilename, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.btnBrowse, 2, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtAuthor, 1, 3);
			this.tableLayoutPanel1.Controls.Add(this.lblRecordFrom, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.lblAuthor, 0, 3);
			this.tableLayoutPanel1.Controls.Add(this.lblDescription, 0, 4);
			this.tableLayoutPanel1.Controls.Add(this.cboRecordFrom, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.txtDescription, 1, 4);
			this.tableLayoutPanel1.Controls.Add(this.lblMovieInformation, 0, 2);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 6;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 25F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(397, 231);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// lblSaveTo
			// 
			this.lblSaveTo.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblSaveTo.AutoSize = true;
			this.lblSaveTo.Location = new System.Drawing.Point(3, 8);
			this.lblSaveTo.Name = "lblSaveTo";
			this.lblSaveTo.Size = new System.Drawing.Size(47, 13);
			this.lblSaveTo.TabIndex = 0;
			this.lblSaveTo.Text = "Save to:";
			// 
			// txtFilename
			// 
			this.txtFilename.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtFilename.Location = new System.Drawing.Point(82, 3);
			this.txtFilename.MaxLength = 1999;
			this.txtFilename.Name = "txtFilename";
			this.txtFilename.ReadOnly = true;
			this.txtFilename.Size = new System.Drawing.Size(231, 20);
			this.txtFilename.TabIndex = 1;
			// 
			// btnBrowse
			// 
			this.btnBrowse.Location = new System.Drawing.Point(319, 3);
			this.btnBrowse.Name = "btnBrowse";
			this.btnBrowse.Size = new System.Drawing.Size(75, 23);
			this.btnBrowse.TabIndex = 2;
			this.btnBrowse.Text = "Browse...";
			this.btnBrowse.UseVisualStyleBackColor = true;
			this.btnBrowse.Click += new System.EventHandler(this.btnBrowse_Click);
			// 
			// txtAuthor
			// 
			this.tableLayoutPanel1.SetColumnSpan(this.txtAuthor, 2);
			this.txtAuthor.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtAuthor.Location = new System.Drawing.Point(82, 84);
			this.txtAuthor.MaxLength = 249;
			this.txtAuthor.Name = "txtAuthor";
			this.txtAuthor.Size = new System.Drawing.Size(312, 20);
			this.txtAuthor.TabIndex = 12;
			// 
			// lblRecordFrom
			// 
			this.lblRecordFrom.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRecordFrom.AutoSize = true;
			this.lblRecordFrom.Location = new System.Drawing.Point(3, 36);
			this.lblRecordFrom.Name = "lblRecordFrom";
			this.lblRecordFrom.Size = new System.Drawing.Size(68, 13);
			this.lblRecordFrom.TabIndex = 6;
			this.lblRecordFrom.Text = "Record from:";
			// 
			// lblAuthor
			// 
			this.lblAuthor.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblAuthor.AutoSize = true;
			this.lblAuthor.Location = new System.Drawing.Point(13, 87);
			this.lblAuthor.Margin = new System.Windows.Forms.Padding(13, 0, 3, 0);
			this.lblAuthor.Name = "lblAuthor";
			this.lblAuthor.Size = new System.Drawing.Size(41, 13);
			this.lblAuthor.TabIndex = 5;
			this.lblAuthor.Text = "Author:";
			// 
			// lblDescription
			// 
			this.lblDescription.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblDescription.AutoSize = true;
			this.lblDescription.Location = new System.Drawing.Point(13, 146);
			this.lblDescription.Margin = new System.Windows.Forms.Padding(13, 0, 3, 0);
			this.lblDescription.Name = "lblDescription";
			this.lblDescription.Size = new System.Drawing.Size(63, 13);
			this.lblDescription.TabIndex = 11;
			this.lblDescription.Text = "Description:";
			// 
			// cboRecordFrom
			// 
			this.cboRecordFrom.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboRecordFrom.FormattingEnabled = true;
			this.cboRecordFrom.Location = new System.Drawing.Point(82, 32);
			this.cboRecordFrom.Name = "cboRecordFrom";
			this.cboRecordFrom.Size = new System.Drawing.Size(231, 21);
			this.cboRecordFrom.TabIndex = 13;
			// 
			// txtDescription
			// 
			this.txtDescription.AcceptsReturn = true;
			this.tableLayoutPanel1.SetColumnSpan(this.txtDescription, 2);
			this.txtDescription.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtDescription.Location = new System.Drawing.Point(82, 110);
			this.txtDescription.MaxLength = 9999;
			this.txtDescription.Multiline = true;
			this.txtDescription.Name = "txtDescription";
			this.txtDescription.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.txtDescription.Size = new System.Drawing.Size(312, 86);
			this.txtDescription.TabIndex = 10;
			// 
			// lblMovieInformation
			// 
			this.lblMovieInformation.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblMovieInformation.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.lblMovieInformation, 2);
			this.lblMovieInformation.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblMovieInformation.Location = new System.Drawing.Point(3, 65);
			this.lblMovieInformation.Name = "lblMovieInformation";
			this.lblMovieInformation.Padding = new System.Windows.Forms.Padding(0, 0, 0, 3);
			this.lblMovieInformation.Size = new System.Drawing.Size(139, 16);
			this.lblMovieInformation.TabIndex = 24;
			this.lblMovieInformation.Text = "Movie Information (Optional)";
			// 
			// frmRecordMovie
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(397, 231);
			this.Controls.Add(this.tableLayoutPanel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "frmRecordMovie";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Movie Recording Options";
			this.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label lblSaveTo;
		private System.Windows.Forms.TextBox txtFilename;
		private System.Windows.Forms.Button btnBrowse;
		private System.Windows.Forms.Label lblAuthor;
		private System.Windows.Forms.Label lblRecordFrom;
		private System.Windows.Forms.TextBox txtDescription;
		private System.Windows.Forms.Label lblDescription;
		private System.Windows.Forms.TextBox txtAuthor;
		private System.Windows.Forms.ComboBox cboRecordFrom;
		private System.Windows.Forms.Label lblMovieInformation;
	}
}