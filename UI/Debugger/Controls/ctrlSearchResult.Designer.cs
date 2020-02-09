namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlSearchResult
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
			this.tableLayoutPanel4 = new System.Windows.Forms.TableLayoutPanel();
			this.lblCpu = new System.Windows.Forms.Label();
			this.lblRelativeAddress = new System.Windows.Forms.Label();
			this.lblLocation = new System.Windows.Forms.Label();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.lblAbsoluteAddress = new System.Windows.Forms.Label();
			this.lblMemoryType = new System.Windows.Forms.Label();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.picWarning = new System.Windows.Forms.PictureBox();
			this.lblLabelName = new System.Windows.Forms.Label();
			this.picType = new System.Windows.Forms.PictureBox();
			this.tableLayoutPanel1.SuspendLayout();
			this.tableLayoutPanel4.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.tableLayoutPanel3.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picWarning)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picType)).BeginInit();
			this.SuspendLayout();
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 160F));
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel4, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.lblLocation, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel3, 0, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 3;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 17F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 17F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(274, 36);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// tableLayoutPanel4
			// 
			this.tableLayoutPanel4.ColumnCount = 2;
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel4.Controls.Add(this.lblCpu, 0, 0);
			this.tableLayoutPanel4.Controls.Add(this.lblRelativeAddress, 1, 0);
			this.tableLayoutPanel4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel4.Location = new System.Drawing.Point(114, 17);
			this.tableLayoutPanel4.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel4.Name = "tableLayoutPanel4";
			this.tableLayoutPanel4.RowCount = 1;
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Size = new System.Drawing.Size(160, 17);
			this.tableLayoutPanel4.TabIndex = 6;
			// 
			// lblCpu
			// 
			this.lblCpu.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.lblCpu.AutoSize = true;
			this.lblCpu.Font = new System.Drawing.Font("Microsoft Sans Serif", 6.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lblCpu.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblCpu.Location = new System.Drawing.Point(68, 0);
			this.lblCpu.Name = "lblCpu";
			this.lblCpu.Padding = new System.Windows.Forms.Padding(0, 3, 0, 0);
			this.lblCpu.Size = new System.Drawing.Size(25, 15);
			this.lblCpu.TabIndex = 4;
			this.lblCpu.Text = "CPU";
			// 
			// lblRelativeAddress
			// 
			this.lblRelativeAddress.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.lblRelativeAddress.AutoSize = true;
			this.lblRelativeAddress.Location = new System.Drawing.Point(99, 2);
			this.lblRelativeAddress.Name = "lblRelativeAddress";
			this.lblRelativeAddress.Padding = new System.Windows.Forms.Padding(0, 0, 0, 2);
			this.lblRelativeAddress.Size = new System.Drawing.Size(58, 15);
			this.lblRelativeAddress.TabIndex = 2;
			this.lblRelativeAddress.Text = "$FFFF:$00";
			// 
			// lblLocation
			// 
			this.lblLocation.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.lblLocation.AutoSize = true;
			this.lblLocation.Location = new System.Drawing.Point(3, 19);
			this.lblLocation.Name = "lblLocation";
			this.lblLocation.Padding = new System.Windows.Forms.Padding(0, 0, 0, 2);
			this.lblLocation.Size = new System.Drawing.Size(71, 15);
			this.lblLocation.TabIndex = 1;
			this.lblLocation.Text = "Filename.asm";
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 2;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.Controls.Add(this.lblAbsoluteAddress, 1, 0);
			this.tableLayoutPanel2.Controls.Add(this.lblMemoryType, 0, 0);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(114, 0);
			this.tableLayoutPanel2.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 1;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(160, 17);
			this.tableLayoutPanel2.TabIndex = 4;
			// 
			// lblAbsoluteAddress
			// 
			this.lblAbsoluteAddress.Anchor = System.Windows.Forms.AnchorStyles.Right;
			this.lblAbsoluteAddress.AutoSize = true;
			this.lblAbsoluteAddress.Location = new System.Drawing.Point(93, 1);
			this.lblAbsoluteAddress.Name = "lblAbsoluteAddress";
			this.lblAbsoluteAddress.Padding = new System.Windows.Forms.Padding(0, 2, 0, 0);
			this.lblAbsoluteAddress.Size = new System.Drawing.Size(64, 15);
			this.lblAbsoluteAddress.TabIndex = 3;
			this.lblAbsoluteAddress.Text = "$12345:$55";
			// 
			// lblMemoryType
			// 
			this.lblMemoryType.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.lblMemoryType.AutoSize = true;
			this.lblMemoryType.Font = new System.Drawing.Font("Microsoft Sans Serif", 6.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lblMemoryType.ForeColor = System.Drawing.SystemColors.GrayText;
			this.lblMemoryType.Location = new System.Drawing.Point(11, 0);
			this.lblMemoryType.Name = "lblMemoryType";
			this.lblMemoryType.Padding = new System.Windows.Forms.Padding(0, 3, 0, 0);
			this.lblMemoryType.Size = new System.Drawing.Size(76, 15);
			this.lblMemoryType.TabIndex = 4;
			this.lblMemoryType.Text = "NES RAM (2 KB)";
			// 
			// tableLayoutPanel3
			// 
			this.tableLayoutPanel3.ColumnCount = 3;
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Controls.Add(this.picWarning, 0, 0);
			this.tableLayoutPanel3.Controls.Add(this.lblLabelName, 2, 0);
			this.tableLayoutPanel3.Controls.Add(this.picType, 1, 0);
			this.tableLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel3.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel3.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel3.Name = "tableLayoutPanel3";
			this.tableLayoutPanel3.RowCount = 1;
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.Size = new System.Drawing.Size(114, 17);
			this.tableLayoutPanel3.TabIndex = 5;
			// 
			// picWarning
			// 
			this.picWarning.Image = global::Mesen.GUI.Properties.Resources.Warning;
			this.picWarning.Location = new System.Drawing.Point(1, 0);
			this.picWarning.Margin = new System.Windows.Forms.Padding(1, 0, 0, 0);
			this.picWarning.Name = "picWarning";
			this.picWarning.Size = new System.Drawing.Size(16, 16);
			this.picWarning.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
			this.picWarning.TabIndex = 2;
			this.picWarning.TabStop = false;
			// 
			// lblLabelName
			// 
			this.lblLabelName.AutoSize = true;
			this.lblLabelName.Location = new System.Drawing.Point(37, 0);
			this.lblLabelName.Name = "lblLabelName";
			this.lblLabelName.Padding = new System.Windows.Forms.Padding(0, 2, 0, 0);
			this.lblLabelName.Size = new System.Drawing.Size(69, 28);
			this.lblLabelName.TabIndex = 0;
			this.lblLabelName.Text = "MyLabelName";
			// 
			// picType
			// 
			this.picType.Location = new System.Drawing.Point(18, 0);
			this.picType.Margin = new System.Windows.Forms.Padding(1, 0, 0, 0);
			this.picType.Name = "picType";
			this.picType.Size = new System.Drawing.Size(16, 16);
			this.picType.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
			this.picType.TabIndex = 1;
			this.picType.TabStop = false;
			// 
			// ctrlSearchResult
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.tableLayoutPanel1);
			this.Margin = new System.Windows.Forms.Padding(0, 0, 0, 1);
			this.Name = "ctrlSearchResult";
			this.Size = new System.Drawing.Size(274, 36);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.tableLayoutPanel4.ResumeLayout(false);
			this.tableLayoutPanel4.PerformLayout();
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel3.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picWarning)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picType)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label lblLabelName;
		private System.Windows.Forms.Label lblLocation;
		private System.Windows.Forms.Label lblRelativeAddress;
		private System.Windows.Forms.Label lblAbsoluteAddress;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.Label lblMemoryType;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
		private System.Windows.Forms.PictureBox picType;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel4;
		private System.Windows.Forms.Label lblCpu;
		private System.Windows.Forms.PictureBox picWarning;
	}
}
