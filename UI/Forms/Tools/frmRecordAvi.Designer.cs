namespace Mesen.GUI.Forms
{
	partial class frmRecordAvi
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
			this.lblCodec = new System.Windows.Forms.Label();
			this.lblAviFile = new System.Windows.Forms.Label();
			this.txtFilename = new System.Windows.Forms.TextBox();
			this.btnBrowse = new System.Windows.Forms.Button();
			this.cboVideoCodec = new System.Windows.Forms.ComboBox();
			this.lblCompressionLevel = new System.Windows.Forms.Label();
			this.lblLowCompression = new System.Windows.Forms.Label();
			this.panel1 = new System.Windows.Forms.Panel();
			this.trkCompressionLevel = new System.Windows.Forms.TrackBar();
			this.lblHighCompression = new System.Windows.Forms.Label();
			this.tlpCompressionLevel = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel1.SuspendLayout();
			this.panel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.trkCompressionLevel)).BeginInit();
			this.tlpCompressionLevel.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 99);
			this.baseConfigPanel.Size = new System.Drawing.Size(397, 29);
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 3;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.lblCodec, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.lblAviFile, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtFilename, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.btnBrowse, 2, 0);
			this.tableLayoutPanel1.Controls.Add(this.cboVideoCodec, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.lblCompressionLevel, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.tlpCompressionLevel, 1, 2);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 4;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(397, 128);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// lblCodec
			// 
			this.lblCodec.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblCodec.AutoSize = true;
			this.lblCodec.Location = new System.Drawing.Point(3, 36);
			this.lblCodec.Name = "lblCodec";
			this.lblCodec.Size = new System.Drawing.Size(71, 13);
			this.lblCodec.TabIndex = 5;
			this.lblCodec.Text = "Video Codec:";
			// 
			// lblAviFile
			// 
			this.lblAviFile.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblAviFile.AutoSize = true;
			this.lblAviFile.Location = new System.Drawing.Point(3, 8);
			this.lblAviFile.Name = "lblAviFile";
			this.lblAviFile.Size = new System.Drawing.Size(47, 13);
			this.lblAviFile.TabIndex = 0;
			this.lblAviFile.Text = "Save to:";
			// 
			// txtFilename
			// 
			this.txtFilename.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtFilename.Location = new System.Drawing.Point(108, 3);
			this.txtFilename.Name = "txtFilename";
			this.txtFilename.ReadOnly = true;
			this.txtFilename.Size = new System.Drawing.Size(205, 20);
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
			// cboVideoCodec
			// 
			this.cboVideoCodec.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboVideoCodec.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboVideoCodec.FormattingEnabled = true;
			this.cboVideoCodec.Location = new System.Drawing.Point(108, 32);
			this.cboVideoCodec.Name = "cboVideoCodec";
			this.cboVideoCodec.Size = new System.Drawing.Size(205, 21);
			this.cboVideoCodec.TabIndex = 4;
			this.cboVideoCodec.SelectedIndexChanged += new System.EventHandler(this.cboVideoCodec_SelectedIndexChanged);
			// 
			// lblCompressionLevel
			// 
			this.lblCompressionLevel.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblCompressionLevel.AutoSize = true;
			this.lblCompressionLevel.Location = new System.Drawing.Point(3, 67);
			this.lblCompressionLevel.Name = "lblCompressionLevel";
			this.lblCompressionLevel.Size = new System.Drawing.Size(99, 13);
			this.lblCompressionLevel.TabIndex = 6;
			this.lblCompressionLevel.Text = "Compression Level:";
			// 
			// lblLowCompression
			// 
			this.lblLowCompression.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblLowCompression.AutoSize = true;
			this.lblLowCompression.Location = new System.Drawing.Point(3, 4);
			this.lblLowCompression.Margin = new System.Windows.Forms.Padding(3, 0, 0, 0);
			this.lblLowCompression.Name = "lblLowCompression";
			this.lblLowCompression.Size = new System.Drawing.Size(30, 26);
			this.lblLowCompression.TabIndex = 9;
			this.lblLowCompression.Text = "low\r\n(fast)";
			this.lblLowCompression.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.trkCompressionLevel);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(36, 3);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(138, 29);
			this.panel1.TabIndex = 9;
			// 
			// trkCompressionLevel
			// 
			this.trkCompressionLevel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trkCompressionLevel.Location = new System.Drawing.Point(0, 0);
			this.trkCompressionLevel.Maximum = 9;
			this.trkCompressionLevel.Minimum = 1;
			this.trkCompressionLevel.Name = "trkCompressionLevel";
			this.trkCompressionLevel.Size = new System.Drawing.Size(138, 29);
			this.trkCompressionLevel.TabIndex = 7;
			this.trkCompressionLevel.Value = 1;
			// 
			// lblHighCompression
			// 
			this.lblHighCompression.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblHighCompression.AutoSize = true;
			this.lblHighCompression.Location = new System.Drawing.Point(177, 4);
			this.lblHighCompression.Margin = new System.Windows.Forms.Padding(0);
			this.lblHighCompression.Name = "lblHighCompression";
			this.lblHighCompression.Size = new System.Drawing.Size(34, 26);
			this.lblHighCompression.TabIndex = 10;
			this.lblHighCompression.Text = "high\r\n(slow)";
			this.lblHighCompression.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// tlpCompressionLevel
			// 
			this.tlpCompressionLevel.ColumnCount = 3;
			this.tlpCompressionLevel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpCompressionLevel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpCompressionLevel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpCompressionLevel.Controls.Add(this.lblHighCompression, 2, 0);
			this.tlpCompressionLevel.Controls.Add(this.panel1, 1, 0);
			this.tlpCompressionLevel.Controls.Add(this.lblLowCompression, 0, 0);
			this.tlpCompressionLevel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpCompressionLevel.Location = new System.Drawing.Point(105, 56);
			this.tlpCompressionLevel.Margin = new System.Windows.Forms.Padding(0, 0, 0, 0);
			this.tlpCompressionLevel.Name = "tlpCompressionLevel";
			this.tlpCompressionLevel.RowCount = 1;
			this.tlpCompressionLevel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpCompressionLevel.Size = new System.Drawing.Size(211, 35);
			this.tlpCompressionLevel.TabIndex = 9;
			// 
			// frmRecordAvi
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(397, 128);
			this.Controls.Add(this.tableLayoutPanel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "frmRecordAvi";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Video Recording Options";
			this.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.panel1.ResumeLayout(false);
			this.panel1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.trkCompressionLevel)).EndInit();
			this.tlpCompressionLevel.ResumeLayout(false);
			this.tlpCompressionLevel.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label lblAviFile;
		private System.Windows.Forms.TextBox txtFilename;
		private System.Windows.Forms.Button btnBrowse;
		private System.Windows.Forms.ComboBox cboVideoCodec;
		private System.Windows.Forms.Label lblCodec;
		private System.Windows.Forms.Label lblCompressionLevel;
		private System.Windows.Forms.Label lblLowCompression;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.TrackBar trkCompressionLevel;
		private System.Windows.Forms.Label lblHighCompression;
		private System.Windows.Forms.TableLayoutPanel tlpCompressionLevel;
	}
}