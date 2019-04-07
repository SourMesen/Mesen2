namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlCpuStatus
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
			this.grpCpu = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.label1 = new System.Windows.Forms.Label();
			this.lblA = new System.Windows.Forms.Label();
			this.txtA = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.txtX = new System.Windows.Forms.TextBox();
			this.txtY = new System.Windows.Forms.TextBox();
			this.label3 = new System.Windows.Forms.Label();
			this.txtD = new System.Windows.Forms.TextBox();
			this.txtDB = new System.Windows.Forms.TextBox();
			this.label6 = new System.Windows.Forms.Label();
			this.txtPC = new System.Windows.Forms.TextBox();
			this.label7 = new System.Windows.Forms.Label();
			this.txtP = new System.Windows.Forms.TextBox();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.chkDecimal = new System.Windows.Forms.CheckBox();
			this.chkInterrupt = new System.Windows.Forms.CheckBox();
			this.chkCarry = new System.Windows.Forms.CheckBox();
			this.chkZero = new System.Windows.Forms.CheckBox();
			this.chkIndex = new System.Windows.Forms.CheckBox();
			this.chkMemory = new System.Windows.Forms.CheckBox();
			this.chkNegative = new System.Windows.Forms.CheckBox();
			this.chkOverflow = new System.Windows.Forms.CheckBox();
			this.chkEmulation = new System.Windows.Forms.CheckBox();
			this.label4 = new System.Windows.Forms.Label();
			this.txtS = new System.Windows.Forms.TextBox();
			this.label5 = new System.Windows.Forms.Label();
			this.txtStack = new System.Windows.Forms.TextBox();
			this.chkNmi = new System.Windows.Forms.CheckBox();
			this.chkIrq = new System.Windows.Forms.CheckBox();
			this.grpCpu.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// grpCpu
			// 
			this.grpCpu.Controls.Add(this.tableLayoutPanel1);
			this.grpCpu.Dock = System.Windows.Forms.DockStyle.Top;
			this.grpCpu.Location = new System.Drawing.Point(0, 0);
			this.grpCpu.Name = "grpCpu";
			this.grpCpu.Size = new System.Drawing.Size(342, 146);
			this.grpCpu.TabIndex = 0;
			this.grpCpu.TabStop = false;
			this.grpCpu.Text = "CPU";
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 8;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.label1, 2, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblA, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtA, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.label2, 4, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtX, 3, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtY, 5, 0);
			this.tableLayoutPanel1.Controls.Add(this.label3, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.txtD, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.txtDB, 3, 1);
			this.tableLayoutPanel1.Controls.Add(this.label6, 6, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtPC, 7, 0);
			this.tableLayoutPanel1.Controls.Add(this.label7, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.txtP, 1, 2);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 2, 2);
			this.tableLayoutPanel1.Controls.Add(this.chkEmulation, 1, 3);
			this.tableLayoutPanel1.Controls.Add(this.label4, 2, 1);
			this.tableLayoutPanel1.Controls.Add(this.txtS, 7, 1);
			this.tableLayoutPanel1.Controls.Add(this.label5, 6, 1);
			this.tableLayoutPanel1.Controls.Add(this.txtStack, 6, 2);
			this.tableLayoutPanel1.Controls.Add(this.chkNmi, 1, 4);
			this.tableLayoutPanel1.Controls.Add(this.chkIrq, 2, 4);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 5;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.Size = new System.Drawing.Size(336, 127);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// label1
			// 
			this.label1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(79, 6);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(17, 13);
			this.label1.TabIndex = 2;
			this.label1.Text = "X:";
			// 
			// lblA
			// 
			this.lblA.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblA.AutoSize = true;
			this.lblA.Location = new System.Drawing.Point(3, 6);
			this.lblA.Name = "lblA";
			this.lblA.Size = new System.Drawing.Size(17, 13);
			this.lblA.TabIndex = 0;
			this.lblA.Text = "A:";
			// 
			// txtA
			// 
			this.txtA.Location = new System.Drawing.Point(27, 3);
			this.txtA.Name = "txtA";
			this.txtA.Size = new System.Drawing.Size(42, 20);
			this.txtA.TabIndex = 1;
			this.txtA.Text = "DDDD";
			// 
			// label2
			// 
			this.label2.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(158, 6);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(17, 13);
			this.label2.TabIndex = 3;
			this.label2.Text = "Y:";
			// 
			// txtX
			// 
			this.txtX.Location = new System.Drawing.Point(110, 3);
			this.txtX.Name = "txtX";
			this.txtX.Size = new System.Drawing.Size(42, 20);
			this.txtX.TabIndex = 4;
			// 
			// txtY
			// 
			this.txtY.Location = new System.Drawing.Point(181, 3);
			this.txtY.Name = "txtY";
			this.txtY.Size = new System.Drawing.Size(42, 20);
			this.txtY.TabIndex = 5;
			// 
			// label3
			// 
			this.label3.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(3, 32);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(18, 13);
			this.label3.TabIndex = 6;
			this.label3.Text = "D:";
			// 
			// txtD
			// 
			this.txtD.Location = new System.Drawing.Point(27, 29);
			this.txtD.Name = "txtD";
			this.txtD.Size = new System.Drawing.Size(42, 20);
			this.txtD.TabIndex = 7;
			this.txtD.Text = "FFFF";
			// 
			// txtDB
			// 
			this.txtDB.Location = new System.Drawing.Point(110, 29);
			this.txtDB.Name = "txtDB";
			this.txtDB.Size = new System.Drawing.Size(25, 20);
			this.txtDB.TabIndex = 9;
			// 
			// label6
			// 
			this.label6.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(244, 6);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(24, 13);
			this.label6.TabIndex = 12;
			this.label6.Text = "PC:";
			// 
			// txtPC
			// 
			this.txtPC.Location = new System.Drawing.Point(274, 3);
			this.txtPC.Name = "txtPC";
			this.txtPC.Size = new System.Drawing.Size(56, 20);
			this.txtPC.TabIndex = 13;
			this.txtPC.Text = "DDDDDD";
			// 
			// label7
			// 
			this.label7.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label7.AutoSize = true;
			this.label7.Location = new System.Drawing.Point(3, 58);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(17, 13);
			this.label7.TabIndex = 14;
			this.label7.Text = "P:";
			// 
			// txtP
			// 
			this.txtP.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.txtP.Location = new System.Drawing.Point(27, 55);
			this.txtP.Name = "txtP";
			this.txtP.Size = new System.Drawing.Size(25, 20);
			this.txtP.TabIndex = 15;
			this.txtP.Text = "DD";
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 5;
			this.tableLayoutPanel1.SetColumnSpan(this.tableLayoutPanel2, 4);
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 40F));
			this.tableLayoutPanel2.Controls.Add(this.chkDecimal, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkInterrupt, 1, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkCarry, 3, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkZero, 2, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkIndex, 3, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkMemory, 2, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkNegative, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkOverflow, 1, 0);
			this.tableLayoutPanel2.Location = new System.Drawing.Point(79, 55);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 2;
			this.tableLayoutPanel1.SetRowSpan(this.tableLayoutPanel2, 2);
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.Size = new System.Drawing.Size(159, 46);
			this.tableLayoutPanel2.TabIndex = 16;
			// 
			// chkDecimal
			// 
			this.chkDecimal.AutoSize = true;
			this.chkDecimal.Location = new System.Drawing.Point(3, 26);
			this.chkDecimal.Name = "chkDecimal";
			this.chkDecimal.Size = new System.Drawing.Size(34, 17);
			this.chkDecimal.TabIndex = 25;
			this.chkDecimal.Text = "D";
			this.chkDecimal.UseVisualStyleBackColor = true;
			// 
			// chkInterrupt
			// 
			this.chkInterrupt.AutoSize = true;
			this.chkInterrupt.Location = new System.Drawing.Point(43, 26);
			this.chkInterrupt.Name = "chkInterrupt";
			this.chkInterrupt.Size = new System.Drawing.Size(29, 17);
			this.chkInterrupt.TabIndex = 24;
			this.chkInterrupt.Text = "I";
			this.chkInterrupt.UseVisualStyleBackColor = true;
			// 
			// chkCarry
			// 
			this.chkCarry.AutoSize = true;
			this.chkCarry.Location = new System.Drawing.Point(123, 26);
			this.chkCarry.Name = "chkCarry";
			this.chkCarry.Size = new System.Drawing.Size(33, 17);
			this.chkCarry.TabIndex = 23;
			this.chkCarry.Text = "C";
			this.chkCarry.UseVisualStyleBackColor = true;
			// 
			// chkZero
			// 
			this.chkZero.AutoSize = true;
			this.chkZero.Location = new System.Drawing.Point(82, 26);
			this.chkZero.Name = "chkZero";
			this.chkZero.Size = new System.Drawing.Size(33, 17);
			this.chkZero.TabIndex = 21;
			this.chkZero.Text = "Z";
			this.chkZero.UseVisualStyleBackColor = true;
			// 
			// chkIndex
			// 
			this.chkIndex.AutoSize = true;
			this.chkIndex.Location = new System.Drawing.Point(123, 3);
			this.chkIndex.Name = "chkIndex";
			this.chkIndex.Size = new System.Drawing.Size(33, 17);
			this.chkIndex.TabIndex = 18;
			this.chkIndex.Text = "X";
			this.chkIndex.UseVisualStyleBackColor = true;
			// 
			// chkMemory
			// 
			this.chkMemory.AutoSize = true;
			this.chkMemory.Location = new System.Drawing.Point(82, 3);
			this.chkMemory.Name = "chkMemory";
			this.chkMemory.Size = new System.Drawing.Size(35, 17);
			this.chkMemory.TabIndex = 19;
			this.chkMemory.Text = "M";
			this.chkMemory.UseVisualStyleBackColor = true;
			// 
			// chkNegative
			// 
			this.chkNegative.AutoSize = true;
			this.chkNegative.Location = new System.Drawing.Point(3, 3);
			this.chkNegative.Name = "chkNegative";
			this.chkNegative.Size = new System.Drawing.Size(34, 17);
			this.chkNegative.TabIndex = 17;
			this.chkNegative.Text = "N";
			this.chkNegative.UseVisualStyleBackColor = true;
			// 
			// chkOverflow
			// 
			this.chkOverflow.AutoSize = true;
			this.chkOverflow.Location = new System.Drawing.Point(43, 3);
			this.chkOverflow.Name = "chkOverflow";
			this.chkOverflow.Size = new System.Drawing.Size(33, 17);
			this.chkOverflow.TabIndex = 20;
			this.chkOverflow.Text = "V";
			this.chkOverflow.UseVisualStyleBackColor = true;
			// 
			// chkEmulation
			// 
			this.chkEmulation.AutoSize = true;
			this.chkEmulation.Location = new System.Drawing.Point(27, 81);
			this.chkEmulation.Name = "chkEmulation";
			this.chkEmulation.Size = new System.Drawing.Size(33, 17);
			this.chkEmulation.TabIndex = 22;
			this.chkEmulation.Text = "E";
			this.chkEmulation.UseVisualStyleBackColor = true;
			// 
			// label4
			// 
			this.label4.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(79, 32);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(25, 13);
			this.label4.TabIndex = 8;
			this.label4.Text = "DB:";
			// 
			// txtS
			// 
			this.txtS.Location = new System.Drawing.Point(274, 29);
			this.txtS.Name = "txtS";
			this.txtS.Size = new System.Drawing.Size(42, 20);
			this.txtS.TabIndex = 11;
			// 
			// label5
			// 
			this.label5.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(244, 32);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(17, 13);
			this.label5.TabIndex = 10;
			this.label5.Text = "S:";
			// 
			// txtStack
			// 
			this.txtStack.BackColor = System.Drawing.SystemColors.Window;
			this.tableLayoutPanel1.SetColumnSpan(this.txtStack, 2);
			this.txtStack.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtStack.Location = new System.Drawing.Point(244, 55);
			this.txtStack.Multiline = true;
			this.txtStack.Name = "txtStack";
			this.txtStack.ReadOnly = true;
			this.tableLayoutPanel1.SetRowSpan(this.txtStack, 3);
			this.txtStack.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.txtStack.Size = new System.Drawing.Size(89, 69);
			this.txtStack.TabIndex = 23;
			// 
			// chkNmi
			// 
			this.chkNmi.AutoSize = true;
			this.chkNmi.Location = new System.Drawing.Point(27, 107);
			this.chkNmi.Name = "chkNmi";
			this.chkNmi.Size = new System.Drawing.Size(46, 17);
			this.chkNmi.TabIndex = 27;
			this.chkNmi.Text = "NMI";
			this.chkNmi.UseVisualStyleBackColor = true;
			// 
			// chkIrq
			// 
			this.chkIrq.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.chkIrq, 2);
			this.chkIrq.Location = new System.Drawing.Point(79, 107);
			this.chkIrq.Name = "chkIrq";
			this.chkIrq.Size = new System.Drawing.Size(45, 17);
			this.chkIrq.TabIndex = 26;
			this.chkIrq.Text = "IRQ";
			this.chkIrq.UseVisualStyleBackColor = true;
			// 
			// ctrlCpuStatus
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.grpCpu);
			this.Name = "ctrlCpuStatus";
			this.Size = new System.Drawing.Size(342, 146);
			this.grpCpu.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.GroupBox grpCpu;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label lblA;
		private System.Windows.Forms.TextBox txtA;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox txtX;
		private System.Windows.Forms.TextBox txtY;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.TextBox txtD;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.TextBox txtDB;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.TextBox txtS;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.TextBox txtPC;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.TextBox txtP;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.CheckBox chkEmulation;
		private System.Windows.Forms.CheckBox chkDecimal;
		private System.Windows.Forms.CheckBox chkInterrupt;
		private System.Windows.Forms.CheckBox chkCarry;
		private System.Windows.Forms.CheckBox chkZero;
		private System.Windows.Forms.CheckBox chkIndex;
		private System.Windows.Forms.CheckBox chkMemory;
		private System.Windows.Forms.CheckBox chkNegative;
		private System.Windows.Forms.CheckBox chkOverflow;
		private System.Windows.Forms.TextBox txtStack;
		private System.Windows.Forms.CheckBox chkNmi;
		private System.Windows.Forms.CheckBox chkIrq;
	}
}
