namespace Mesen.GUI.Debugger
{
	partial class frmEditLabel
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
			this.cboType = new Mesen.GUI.Debugger.Controls.ComboBoxWithSeparator();
			this.flowLayoutPanel3 = new System.Windows.Forms.FlowLayoutPanel();
			this.nudLength = new Mesen.GUI.Controls.MesenNumericUpDown();
			this.lblBytes = new System.Windows.Forms.Label();
			this.lblLength = new System.Windows.Forms.Label();
			this.lblLabel = new System.Windows.Forms.Label();
			this.lblComment = new System.Windows.Forms.Label();
			this.txtComment = new System.Windows.Forms.TextBox();
			this.txtLabel = new System.Windows.Forms.TextBox();
			this.lblRegion = new System.Windows.Forms.Label();
			this.lblAddress = new System.Windows.Forms.Label();
			this.flowLayoutPanel2 = new System.Windows.Forms.FlowLayoutPanel();
			this.lblAddressSign = new System.Windows.Forms.Label();
			this.txtAddress = new System.Windows.Forms.TextBox();
			this.lblRange = new System.Windows.Forms.Label();
			this.tableLayoutPanel1.SuspendLayout();
			this.flowLayoutPanel3.SuspendLayout();
			this.flowLayoutPanel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 242);
			this.baseConfigPanel.Size = new System.Drawing.Size(377, 29);
			this.baseConfigPanel.TabIndex = 4;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.cboType, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.flowLayoutPanel3, 1, 4);
			this.tableLayoutPanel1.Controls.Add(this.lblLength, 0, 4);
			this.tableLayoutPanel1.Controls.Add(this.lblLabel, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.lblComment, 0, 3);
			this.tableLayoutPanel1.Controls.Add(this.txtComment, 1, 3);
			this.tableLayoutPanel1.Controls.Add(this.txtLabel, 1, 2);
			this.tableLayoutPanel1.Controls.Add(this.lblRegion, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblAddress, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.flowLayoutPanel2, 1, 1);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 5;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(377, 242);
			this.tableLayoutPanel1.TabIndex = 2;
			// 
			// cboType
			// 
			this.cboType.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
			this.cboType.FormattingEnabled = true;
			this.cboType.Location = new System.Drawing.Point(63, 3);
			this.cboType.Name = "cboType";
			this.cboType.Size = new System.Drawing.Size(157, 21);
			this.cboType.TabIndex = 14;
			// 
			// flowLayoutPanel3
			// 
			this.flowLayoutPanel3.Controls.Add(this.nudLength);
			this.flowLayoutPanel3.Controls.Add(this.lblBytes);
			this.flowLayoutPanel3.ForeColor = System.Drawing.SystemColors.ControlDarkDark;
			this.flowLayoutPanel3.Location = new System.Drawing.Point(60, 216);
			this.flowLayoutPanel3.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel3.Name = "flowLayoutPanel3";
			this.flowLayoutPanel3.Size = new System.Drawing.Size(200, 26);
			this.flowLayoutPanel3.TabIndex = 9;
			// 
			// nudLength
			// 
			this.nudLength.DecimalPlaces = 0;
			this.nudLength.Increment = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudLength.IsHex = false;
			this.nudLength.Location = new System.Drawing.Point(3, 3);
			this.nudLength.Maximum = new decimal(new int[] {
            65536,
            0,
            0,
            0});
			this.nudLength.MaximumSize = new System.Drawing.Size(10000, 21);
			this.nudLength.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudLength.MinimumSize = new System.Drawing.Size(0, 21);
			this.nudLength.Name = "nudLength";
			this.nudLength.Size = new System.Drawing.Size(52, 21);
			this.nudLength.TabIndex = 9;
			this.nudLength.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudLength.ValueChanged += new System.EventHandler(this.nudLength_ValueChanged);
			// 
			// lblBytes
			// 
			this.lblBytes.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblBytes.AutoSize = true;
			this.lblBytes.Location = new System.Drawing.Point(61, 7);
			this.lblBytes.Name = "lblBytes";
			this.lblBytes.Size = new System.Drawing.Size(32, 13);
			this.lblBytes.TabIndex = 10;
			this.lblBytes.Text = "bytes";
			// 
			// lblLength
			// 
			this.lblLength.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblLength.AutoSize = true;
			this.lblLength.Location = new System.Drawing.Point(3, 222);
			this.lblLength.Name = "lblLength";
			this.lblLength.Size = new System.Drawing.Size(43, 13);
			this.lblLength.TabIndex = 8;
			this.lblLength.Text = "Length:";
			// 
			// lblLabel
			// 
			this.lblLabel.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblLabel.AutoSize = true;
			this.lblLabel.Location = new System.Drawing.Point(3, 59);
			this.lblLabel.Name = "lblLabel";
			this.lblLabel.Size = new System.Drawing.Size(36, 13);
			this.lblLabel.TabIndex = 0;
			this.lblLabel.Text = "Label:";
			// 
			// lblComment
			// 
			this.lblComment.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblComment.AutoSize = true;
			this.lblComment.Location = new System.Drawing.Point(3, 141);
			this.lblComment.Name = "lblComment";
			this.lblComment.Size = new System.Drawing.Size(54, 13);
			this.lblComment.TabIndex = 1;
			this.lblComment.Text = "Comment:";
			// 
			// txtComment
			// 
			this.txtComment.AcceptsReturn = true;
			this.txtComment.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtComment.Location = new System.Drawing.Point(63, 82);
			this.txtComment.Multiline = true;
			this.txtComment.Name = "txtComment";
			this.txtComment.ScrollBars = System.Windows.Forms.ScrollBars.Both;
			this.txtComment.Size = new System.Drawing.Size(311, 131);
			this.txtComment.TabIndex = 3;
			// 
			// txtLabel
			// 
			this.txtLabel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtLabel.Location = new System.Drawing.Point(63, 56);
			this.txtLabel.Name = "txtLabel";
			this.txtLabel.Size = new System.Drawing.Size(311, 20);
			this.txtLabel.TabIndex = 2;
			// 
			// lblRegion
			// 
			this.lblRegion.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRegion.AutoSize = true;
			this.lblRegion.Location = new System.Drawing.Point(3, 7);
			this.lblRegion.Name = "lblRegion";
			this.lblRegion.Size = new System.Drawing.Size(44, 13);
			this.lblRegion.TabIndex = 4;
			this.lblRegion.Text = "Region:";
			// 
			// lblAddress
			// 
			this.lblAddress.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblAddress.AutoSize = true;
			this.lblAddress.Location = new System.Drawing.Point(3, 33);
			this.lblAddress.Name = "lblAddress";
			this.lblAddress.Size = new System.Drawing.Size(48, 13);
			this.lblAddress.TabIndex = 5;
			this.lblAddress.Text = "Address:";
			// 
			// flowLayoutPanel2
			// 
			this.flowLayoutPanel2.Controls.Add(this.lblAddressSign);
			this.flowLayoutPanel2.Controls.Add(this.txtAddress);
			this.flowLayoutPanel2.Controls.Add(this.lblRange);
			this.flowLayoutPanel2.Location = new System.Drawing.Point(60, 27);
			this.flowLayoutPanel2.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel2.Name = "flowLayoutPanel2";
			this.flowLayoutPanel2.Size = new System.Drawing.Size(200, 26);
			this.flowLayoutPanel2.TabIndex = 7;
			// 
			// lblAddressSign
			// 
			this.lblAddressSign.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblAddressSign.AutoSize = true;
			this.lblAddressSign.Location = new System.Drawing.Point(0, 6);
			this.lblAddressSign.Margin = new System.Windows.Forms.Padding(0);
			this.lblAddressSign.Name = "lblAddressSign";
			this.lblAddressSign.Size = new System.Drawing.Size(13, 13);
			this.lblAddressSign.TabIndex = 9;
			this.lblAddressSign.Text = "$";
			// 
			// txtAddress
			// 
			this.txtAddress.Location = new System.Drawing.Point(13, 3);
			this.txtAddress.Margin = new System.Windows.Forms.Padding(0, 3, 3, 3);
			this.txtAddress.Name = "txtAddress";
			this.txtAddress.Size = new System.Drawing.Size(57, 20);
			this.txtAddress.TabIndex = 8;
			// 
			// lblRange
			// 
			this.lblRange.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRange.AutoSize = true;
			this.lblRange.ForeColor = System.Drawing.SystemColors.ControlDarkDark;
			this.lblRange.Location = new System.Drawing.Point(76, 6);
			this.lblRange.Name = "lblRange";
			this.lblRange.Size = new System.Drawing.Size(40, 13);
			this.lblRange.TabIndex = 10;
			this.lblRange.Text = "(range)";
			// 
			// frmEditLabel
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(377, 271);
			this.Controls.Add(this.tableLayoutPanel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "frmEditLabel";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Edit Label";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.flowLayoutPanel3.ResumeLayout(false);
			this.flowLayoutPanel3.PerformLayout();
			this.flowLayoutPanel2.ResumeLayout(false);
			this.flowLayoutPanel2.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.TextBox txtComment;
		private System.Windows.Forms.Label lblLabel;
		private System.Windows.Forms.Label lblComment;
		private System.Windows.Forms.TextBox txtLabel;
		private System.Windows.Forms.Label lblRegion;
		private System.Windows.Forms.Label lblAddress;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel2;
		private System.Windows.Forms.Label lblAddressSign;
		private System.Windows.Forms.TextBox txtAddress;
		private System.Windows.Forms.Label lblRange;
		private System.Windows.Forms.Label lblLength;
		private GUI.Controls.MesenNumericUpDown nudLength;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel3;
		private System.Windows.Forms.Label lblBytes;
		private Controls.ComboBoxWithSeparator cboType;
	}
}