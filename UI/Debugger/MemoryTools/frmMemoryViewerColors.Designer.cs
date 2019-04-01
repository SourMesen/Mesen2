namespace Mesen.GUI.Debugger
{
	partial class frmMemoryToolsColors
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
			this.lblCodeByte = new System.Windows.Forms.Label();
			this.lblWrite = new System.Windows.Forms.Label();
			this.picCodeByte = new ctrlColorPicker();
			this.picWrite = new ctrlColorPicker();
			this.picDataByte = new ctrlColorPicker();
			this.lblDataByte = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.picLabelledByte = new ctrlColorPicker();
			this.lblRead = new System.Windows.Forms.Label();
			this.lblExecute = new System.Windows.Forms.Label();
			this.picRead = new ctrlColorPicker();
			this.picExecute = new ctrlColorPicker();
			this.btnReset = new System.Windows.Forms.Button();
			this.baseConfigPanel.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picCodeByte)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picWrite)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picDataByte)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picLabelledByte)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picRead)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picExecute)).BeginInit();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Controls.Add(this.btnReset);
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 128);
			this.baseConfigPanel.Size = new System.Drawing.Size(453, 29);
			this.baseConfigPanel.Controls.SetChildIndex(this.btnReset, 0);
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 8;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33334F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33334F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.lblCodeByte, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.lblWrite, 3, 0);
			this.tableLayoutPanel1.Controls.Add(this.picCodeByte, 1, 2);
			this.tableLayoutPanel1.Controls.Add(this.picWrite, 4, 0);
			this.tableLayoutPanel1.Controls.Add(this.picDataByte, 4, 2);
			this.tableLayoutPanel1.Controls.Add(this.lblDataByte, 3, 2);
			this.tableLayoutPanel1.Controls.Add(this.label1, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.picLabelledByte, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.lblRead, 6, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblExecute, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.picRead, 7, 0);
			this.tableLayoutPanel1.Controls.Add(this.picExecute, 1, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 7;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(453, 157);
			this.tableLayoutPanel1.TabIndex = 2;
			// 
			// lblCodeByte
			// 
			this.lblCodeByte.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblCodeByte.AutoSize = true;
			this.lblCodeByte.Location = new System.Drawing.Point(3, 88);
			this.lblCodeByte.Name = "lblCodeByte";
			this.lblCodeByte.Size = new System.Drawing.Size(59, 13);
			this.lblCodeByte.TabIndex = 2;
			this.lblCodeByte.Text = "Code Byte:";
			// 
			// lblWrite
			// 
			this.lblWrite.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblWrite.AutoSize = true;
			this.lblWrite.Location = new System.Drawing.Point(160, 12);
			this.lblWrite.Name = "lblWrite";
			this.lblWrite.Size = new System.Drawing.Size(35, 13);
			this.lblWrite.TabIndex = 10;
			this.lblWrite.Text = "Write:";
			// 
			// picCodeByte
			// 
			this.picCodeByte.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picCodeByte.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picCodeByte.Location = new System.Drawing.Point(102, 79);
			this.picCodeByte.Name = "picCodeByte";
			this.picCodeByte.Size = new System.Drawing.Size(32, 32);
			this.picCodeByte.TabIndex = 6;
			this.picCodeByte.TabStop = false;
			// 
			// picWrite
			// 
			this.picWrite.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picWrite.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picWrite.Location = new System.Drawing.Point(259, 3);
			this.picWrite.Name = "picWrite";
			this.picWrite.Size = new System.Drawing.Size(32, 32);
			this.picWrite.TabIndex = 8;
			this.picWrite.TabStop = false;
			// 
			// picDataByte
			// 
			this.picDataByte.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picDataByte.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picDataByte.Location = new System.Drawing.Point(259, 79);
			this.picDataByte.Name = "picDataByte";
			this.picDataByte.Size = new System.Drawing.Size(32, 32);
			this.picDataByte.TabIndex = 12;
			this.picDataByte.TabStop = false;
			// 
			// lblDataByte
			// 
			this.lblDataByte.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblDataByte.AutoSize = true;
			this.lblDataByte.Location = new System.Drawing.Point(160, 88);
			this.lblDataByte.Name = "lblDataByte";
			this.lblDataByte.Size = new System.Drawing.Size(57, 13);
			this.lblDataByte.TabIndex = 1;
			this.lblDataByte.Text = "Data Byte:";
			// 
			// label1
			// 
			this.label1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(3, 50);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(74, 13);
			this.label1.TabIndex = 14;
			this.label1.Text = "Labelled Byte:";
			// 
			// picLabelledByte
			// 
			this.picLabelledByte.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picLabelledByte.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picLabelledByte.Location = new System.Drawing.Point(102, 41);
			this.picLabelledByte.Name = "picLabelledByte";
			this.picLabelledByte.Size = new System.Drawing.Size(32, 32);
			this.picLabelledByte.TabIndex = 15;
			this.picLabelledByte.TabStop = false;
			// 
			// lblRead
			// 
			this.lblRead.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRead.AutoSize = true;
			this.lblRead.Location = new System.Drawing.Point(317, 12);
			this.lblRead.Name = "lblRead";
			this.lblRead.Size = new System.Drawing.Size(36, 13);
			this.lblRead.TabIndex = 0;
			this.lblRead.Text = "Read:";
			// 
			// lblExecute
			// 
			this.lblExecute.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblExecute.AutoSize = true;
			this.lblExecute.Location = new System.Drawing.Point(3, 12);
			this.lblExecute.Name = "lblExecute";
			this.lblExecute.Size = new System.Drawing.Size(49, 13);
			this.lblExecute.TabIndex = 11;
			this.lblExecute.Text = "Execute:";
			// 
			// picRead
			// 
			this.picRead.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picRead.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picRead.Location = new System.Drawing.Point(416, 3);
			this.picRead.Name = "picRead";
			this.picRead.Size = new System.Drawing.Size(32, 32);
			this.picRead.TabIndex = 5;
			this.picRead.TabStop = false;
			// 
			// picExecute
			// 
			this.picExecute.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picExecute.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picExecute.Location = new System.Drawing.Point(102, 3);
			this.picExecute.Name = "picExecute";
			this.picExecute.Size = new System.Drawing.Size(32, 32);
			this.picExecute.TabIndex = 9;
			this.picExecute.TabStop = false;
			// 
			// btnReset
			// 
			this.btnReset.Location = new System.Drawing.Point(3, 3);
			this.btnReset.Name = "btnReset";
			this.btnReset.Size = new System.Drawing.Size(102, 23);
			this.btnReset.TabIndex = 3;
			this.btnReset.Text = "Use default colors";
			this.btnReset.UseVisualStyleBackColor = true;
			this.btnReset.Click += new System.EventHandler(this.btnReset_Click);
			// 
			// frmMemoryToolsColors
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(453, 157);
			this.Controls.Add(this.tableLayoutPanel1);
			this.Name = "frmMemoryToolsColors";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Configure Colors...";
			this.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.baseConfigPanel.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picCodeByte)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picWrite)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picDataByte)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picLabelledByte)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picRead)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picExecute)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label lblCodeByte;
		private System.Windows.Forms.Label lblRead;
		private System.Windows.Forms.Label lblDataByte;
		private ctrlColorPicker picExecute;
		private ctrlColorPicker picWrite;
		private ctrlColorPicker picCodeByte;
		private ctrlColorPicker picRead;
		private System.Windows.Forms.Label lblWrite;
		private System.Windows.Forms.Label lblExecute;
		private ctrlColorPicker picDataByte;
		private System.Windows.Forms.Button btnReset;
		private System.Windows.Forms.Label label1;
		private ctrlColorPicker picLabelledByte;
	}
}