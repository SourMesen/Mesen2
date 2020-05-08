namespace Mesen.GUI.Debugger
{
	partial class frmIntegrationSettings
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
			this.grpSPC = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.chkSpcComments = new System.Windows.Forms.CheckBox();
			this.chkSpcRam = new System.Windows.Forms.CheckBox();
			this.grpCpu = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.chkCpuComments = new System.Windows.Forms.CheckBox();
			this.chkCpuWorkRam = new System.Windows.Forms.CheckBox();
			this.chkCpuPrgRom = new System.Windows.Forms.CheckBox();
			this.chkCpuSaveRam = new System.Windows.Forms.CheckBox();
			this.grpGeneral = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel4 = new System.Windows.Forms.TableLayoutPanel();
			this.chkResetLabelsOnImport = new System.Windows.Forms.CheckBox();
			this.chkAutoLoadFiles = new System.Windows.Forms.CheckBox();
			this.tableLayoutPanel1.SuspendLayout();
			this.grpSPC.SuspendLayout();
			this.tableLayoutPanel3.SuspendLayout();
			this.grpCpu.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.grpGeneral.SuspendLayout();
			this.tableLayoutPanel4.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 196);
			this.baseConfigPanel.Size = new System.Drawing.Size(463, 29);
			this.baseConfigPanel.TabIndex = 4;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.Controls.Add(this.grpSPC, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.grpCpu, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.grpGeneral, 0, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(463, 196);
			this.tableLayoutPanel1.TabIndex = 2;
			// 
			// grpSPC
			// 
			this.grpSPC.Controls.Add(this.tableLayoutPanel3);
			this.grpSPC.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpSPC.Location = new System.Drawing.Point(234, 75);
			this.grpSPC.Name = "grpSPC";
			this.grpSPC.Size = new System.Drawing.Size(226, 118);
			this.grpSPC.TabIndex = 1;
			this.grpSPC.TabStop = false;
			this.grpSPC.Text = "SPC700";
			// 
			// tableLayoutPanel3
			// 
			this.tableLayoutPanel3.ColumnCount = 1;
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Controls.Add(this.chkSpcComments, 0, 1);
			this.tableLayoutPanel3.Controls.Add(this.chkSpcRam, 0, 0);
			this.tableLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel3.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel3.Name = "tableLayoutPanel3";
			this.tableLayoutPanel3.RowCount = 3;
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel3.Size = new System.Drawing.Size(220, 99);
			this.tableLayoutPanel3.TabIndex = 0;
			// 
			// chkSpcComments
			// 
			this.chkSpcComments.AutoSize = true;
			this.chkSpcComments.Location = new System.Drawing.Point(3, 26);
			this.chkSpcComments.Name = "chkSpcComments";
			this.chkSpcComments.Size = new System.Drawing.Size(106, 17);
			this.chkSpcComments.TabIndex = 2;
			this.chkSpcComments.Text = "Import comments";
			this.chkSpcComments.UseVisualStyleBackColor = true;
			// 
			// chkSpcRam
			// 
			this.chkSpcRam.AutoSize = true;
			this.chkSpcRam.Location = new System.Drawing.Point(3, 3);
			this.chkSpcRam.Name = "chkSpcRam";
			this.chkSpcRam.Size = new System.Drawing.Size(136, 17);
			this.chkSpcRam.TabIndex = 0;
			this.chkSpcRam.Text = "Import SPC RAM labels";
			this.chkSpcRam.UseVisualStyleBackColor = true;
			// 
			// grpCpu
			// 
			this.grpCpu.Controls.Add(this.tableLayoutPanel2);
			this.grpCpu.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpCpu.Location = new System.Drawing.Point(3, 75);
			this.grpCpu.Name = "grpCpu";
			this.grpCpu.Size = new System.Drawing.Size(225, 118);
			this.grpCpu.TabIndex = 0;
			this.grpCpu.TabStop = false;
			this.grpCpu.Text = "CPU (65816)";
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 1;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Controls.Add(this.chkCpuComments, 0, 3);
			this.tableLayoutPanel2.Controls.Add(this.chkCpuWorkRam, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkCpuPrgRom, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkCpuSaveRam, 0, 2);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 5;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(219, 99);
			this.tableLayoutPanel2.TabIndex = 0;
			// 
			// chkCpuComments
			// 
			this.chkCpuComments.AutoSize = true;
			this.chkCpuComments.Location = new System.Drawing.Point(3, 72);
			this.chkCpuComments.Name = "chkCpuComments";
			this.chkCpuComments.Size = new System.Drawing.Size(106, 17);
			this.chkCpuComments.TabIndex = 2;
			this.chkCpuComments.Text = "Import comments";
			this.chkCpuComments.UseVisualStyleBackColor = true;
			// 
			// chkCpuWorkRam
			// 
			this.chkCpuWorkRam.AutoSize = true;
			this.chkCpuWorkRam.Location = new System.Drawing.Point(3, 26);
			this.chkCpuWorkRam.Name = "chkCpuWorkRam";
			this.chkCpuWorkRam.Size = new System.Drawing.Size(141, 17);
			this.chkCpuWorkRam.TabIndex = 3;
			this.chkCpuWorkRam.Text = "Import Work RAM labels";
			this.chkCpuWorkRam.UseVisualStyleBackColor = true;
			// 
			// chkCpuPrgRom
			// 
			this.chkCpuPrgRom.AutoSize = true;
			this.chkCpuPrgRom.Location = new System.Drawing.Point(3, 3);
			this.chkCpuPrgRom.Name = "chkCpuPrgRom";
			this.chkCpuPrgRom.Size = new System.Drawing.Size(139, 17);
			this.chkCpuPrgRom.TabIndex = 1;
			this.chkCpuPrgRom.Text = "Import PRG ROM labels";
			this.chkCpuPrgRom.UseVisualStyleBackColor = true;
			// 
			// chkCpuSaveRam
			// 
			this.chkCpuSaveRam.AutoSize = true;
			this.chkCpuSaveRam.Location = new System.Drawing.Point(3, 49);
			this.chkCpuSaveRam.Name = "chkCpuSaveRam";
			this.chkCpuSaveRam.Size = new System.Drawing.Size(140, 17);
			this.chkCpuSaveRam.TabIndex = 4;
			this.chkCpuSaveRam.Text = "Import Save RAM labels";
			this.chkCpuSaveRam.UseVisualStyleBackColor = true;
			// 
			// grpGeneral
			// 
			this.tableLayoutPanel1.SetColumnSpan(this.grpGeneral, 2);
			this.grpGeneral.Controls.Add(this.tableLayoutPanel4);
			this.grpGeneral.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpGeneral.Location = new System.Drawing.Point(3, 3);
			this.grpGeneral.Name = "grpGeneral";
			this.grpGeneral.Size = new System.Drawing.Size(457, 66);
			this.grpGeneral.TabIndex = 2;
			this.grpGeneral.TabStop = false;
			this.grpGeneral.Text = "General Settings";
			// 
			// tableLayoutPanel4
			// 
			this.tableLayoutPanel4.ColumnCount = 1;
			this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Controls.Add(this.chkResetLabelsOnImport, 0, 1);
			this.tableLayoutPanel4.Controls.Add(this.chkAutoLoadFiles, 0, 0);
			this.tableLayoutPanel4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel4.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel4.Name = "tableLayoutPanel4";
			this.tableLayoutPanel4.RowCount = 3;
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel4.Size = new System.Drawing.Size(451, 47);
			this.tableLayoutPanel4.TabIndex = 0;
			// 
			// chkResetLabelsOnImport
			// 
			this.chkResetLabelsOnImport.AutoSize = true;
			this.chkResetLabelsOnImport.Location = new System.Drawing.Point(3, 26);
			this.chkResetLabelsOnImport.Name = "chkResetLabelsOnImport";
			this.chkResetLabelsOnImport.Size = new System.Drawing.Size(369, 17);
			this.chkResetLabelsOnImport.TabIndex = 2;
			this.chkResetLabelsOnImport.Text = "Reset workspace labels to their default state before importing symbol files";
			this.chkResetLabelsOnImport.UseVisualStyleBackColor = true;
			// 
			// chkAutoLoadFiles
			// 
			this.chkAutoLoadFiles.AutoSize = true;
			this.chkAutoLoadFiles.Location = new System.Drawing.Point(3, 3);
			this.chkAutoLoadFiles.Name = "chkAutoLoadFiles";
			this.chkAutoLoadFiles.Size = new System.Drawing.Size(433, 17);
			this.chkAutoLoadFiles.TabIndex = 3;
			this.chkAutoLoadFiles.Text = "Automatically load DBG/MSL debug symbols when debugger opens or on power cycle";
			this.chkAutoLoadFiles.UseVisualStyleBackColor = true;
			// 
			// frmIntegrationSettings
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(463, 225);
			this.Controls.Add(this.tableLayoutPanel1);
			this.Name = "frmIntegrationSettings";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "CC65/CA65 Integration Settings (DBG files)";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.grpSPC.ResumeLayout(false);
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel3.PerformLayout();
			this.grpCpu.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.grpGeneral.ResumeLayout(false);
			this.tableLayoutPanel4.ResumeLayout(false);
			this.tableLayoutPanel4.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.GroupBox grpCpu;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.CheckBox chkCpuComments;
		private System.Windows.Forms.CheckBox chkCpuPrgRom;
		private System.Windows.Forms.GroupBox grpGeneral;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel4;
		private System.Windows.Forms.CheckBox chkResetLabelsOnImport;
		private System.Windows.Forms.CheckBox chkCpuWorkRam;
		private System.Windows.Forms.CheckBox chkCpuSaveRam;
		private System.Windows.Forms.GroupBox grpSPC;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
		private System.Windows.Forms.CheckBox chkSpcComments;
		private System.Windows.Forms.CheckBox chkSpcRam;
		private System.Windows.Forms.CheckBox chkAutoLoadFiles;
	}
}