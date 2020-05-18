namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlGameboyStatus
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
			this.lblA = new System.Windows.Forms.Label();
			this.txtA = new System.Windows.Forms.TextBox();
			this.txtF = new System.Windows.Forms.TextBox();
			this.txtStack = new System.Windows.Forms.TextBox();
			this.txtPC = new System.Windows.Forms.TextBox();
			this.label5 = new System.Windows.Forms.Label();
			this.txtB = new System.Windows.Forms.TextBox();
			this.txtC = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.label8 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.txtE = new System.Windows.Forms.TextBox();
			this.txtD = new System.Windows.Forms.TextBox();
			this.txtSP = new System.Windows.Forms.TextBox();
			this.label6 = new System.Windows.Forms.Label();
			this.label7 = new System.Windows.Forms.Label();
			this.txtHL = new System.Windows.Forms.TextBox();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.chkCarry = new System.Windows.Forms.CheckBox();
			this.chkNegative = new System.Windows.Forms.CheckBox();
			this.chkZero = new System.Windows.Forms.CheckBox();
			this.chkHalfCarry = new System.Windows.Forms.CheckBox();
			this.chkIme = new System.Windows.Forms.CheckBox();
			this.chkHalted = new System.Windows.Forms.CheckBox();
			this.label12 = new System.Windows.Forms.Label();
			this.txtCycleCount = new System.Windows.Forms.TextBox();
			this.grpPpu = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.txtScanline = new System.Windows.Forms.TextBox();
			this.label9 = new System.Windows.Forms.Label();
			this.label10 = new System.Windows.Forms.Label();
			this.txtCycle = new System.Windows.Forms.TextBox();
			this.grpCpu.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.grpPpu.SuspendLayout();
			this.tableLayoutPanel3.SuspendLayout();
			this.SuspendLayout();
			// 
			// grpCpu
			// 
			this.grpCpu.Controls.Add(this.tableLayoutPanel1);
			this.grpCpu.Dock = System.Windows.Forms.DockStyle.Top;
			this.grpCpu.Location = new System.Drawing.Point(0, 0);
			this.grpCpu.Name = "grpCpu";
			this.grpCpu.Size = new System.Drawing.Size(342, 141);
			this.grpCpu.TabIndex = 0;
			this.grpCpu.TabStop = false;
			this.grpCpu.Text = "CPU";
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 10;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.Controls.Add(this.lblA, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtA, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtF, 3, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtStack, 7, 1);
			this.tableLayoutPanel1.Controls.Add(this.label5, 7, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtB, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.txtC, 3, 1);
			this.tableLayoutPanel1.Controls.Add(this.label1, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.label8, 2, 0);
			this.tableLayoutPanel1.Controls.Add(this.label2, 2, 1);
			this.tableLayoutPanel1.Controls.Add(this.label3, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.label4, 2, 2);
			this.tableLayoutPanel1.Controls.Add(this.txtE, 3, 2);
			this.tableLayoutPanel1.Controls.Add(this.txtD, 1, 2);
			this.tableLayoutPanel1.Controls.Add(this.txtSP, 8, 0);
			this.tableLayoutPanel1.Controls.Add(this.label7, 4, 1);
			this.tableLayoutPanel1.Controls.Add(this.txtHL, 5, 1);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 0, 3);
			this.tableLayoutPanel1.Controls.Add(this.label6, 4, 2);
			this.tableLayoutPanel1.Controls.Add(this.label12, 4, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtPC, 5, 2);
			this.tableLayoutPanel1.Controls.Add(this.txtCycleCount, 5, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 7;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.Size = new System.Drawing.Size(336, 122);
			this.tableLayoutPanel1.TabIndex = 0;
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
			this.txtA.Size = new System.Drawing.Size(25, 20);
			this.txtA.TabIndex = 1;
			this.txtA.Text = "DDDD";
			// 
			// txtF
			// 
			this.txtF.Location = new System.Drawing.Point(81, 3);
			this.txtF.Name = "txtF";
			this.txtF.Size = new System.Drawing.Size(25, 20);
			this.txtF.TabIndex = 4;
			// 
			// txtStack
			// 
			this.txtStack.BackColor = System.Drawing.SystemColors.Window;
			this.tableLayoutPanel1.SetColumnSpan(this.txtStack, 2);
			this.txtStack.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtStack.Location = new System.Drawing.Point(244, 29);
			this.txtStack.Multiline = true;
			this.txtStack.Name = "txtStack";
			this.txtStack.ReadOnly = true;
			this.tableLayoutPanel1.SetRowSpan(this.txtStack, 3);
			this.txtStack.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.txtStack.Size = new System.Drawing.Size(88, 69);
			this.txtStack.TabIndex = 23;
			// 
			// txtPC
			// 
			this.txtPC.Location = new System.Drawing.Point(154, 55);
			this.txtPC.Name = "txtPC";
			this.txtPC.Size = new System.Drawing.Size(40, 20);
			this.txtPC.TabIndex = 13;
			this.txtPC.Text = "DDDD";
			// 
			// label5
			// 
			this.label5.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(244, 6);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(24, 13);
			this.label5.TabIndex = 10;
			this.label5.Text = "SP:";
			// 
			// txtB
			// 
			this.txtB.Location = new System.Drawing.Point(27, 29);
			this.txtB.Name = "txtB";
			this.txtB.Size = new System.Drawing.Size(25, 20);
			this.txtB.TabIndex = 25;
			this.txtB.Text = "DDDD";
			// 
			// txtC
			// 
			this.txtC.Location = new System.Drawing.Point(81, 29);
			this.txtC.Name = "txtC";
			this.txtC.Size = new System.Drawing.Size(25, 20);
			this.txtC.TabIndex = 26;
			this.txtC.Text = "DDDD";
			// 
			// label1
			// 
			this.label1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(3, 32);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(17, 13);
			this.label1.TabIndex = 2;
			this.label1.Text = "B:";
			// 
			// label8
			// 
			this.label8.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label8.AutoSize = true;
			this.label8.Location = new System.Drawing.Point(58, 6);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(16, 13);
			this.label8.TabIndex = 29;
			this.label8.Text = "F:";
			// 
			// label2
			// 
			this.label2.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(58, 32);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(17, 13);
			this.label2.TabIndex = 3;
			this.label2.Text = "C:";
			// 
			// label3
			// 
			this.label3.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(3, 58);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(18, 13);
			this.label3.TabIndex = 24;
			this.label3.Text = "D:";
			// 
			// label4
			// 
			this.label4.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(58, 58);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(17, 13);
			this.label4.TabIndex = 28;
			this.label4.Text = "E:";
			// 
			// txtE
			// 
			this.txtE.Location = new System.Drawing.Point(81, 55);
			this.txtE.Name = "txtE";
			this.txtE.Size = new System.Drawing.Size(25, 20);
			this.txtE.TabIndex = 27;
			this.txtE.Text = "DDDD";
			// 
			// txtD
			// 
			this.txtD.Location = new System.Drawing.Point(27, 55);
			this.txtD.Name = "txtD";
			this.txtD.Size = new System.Drawing.Size(25, 20);
			this.txtD.TabIndex = 5;
			// 
			// txtSP
			// 
			this.txtSP.Location = new System.Drawing.Point(274, 3);
			this.txtSP.Name = "txtSP";
			this.txtSP.Size = new System.Drawing.Size(40, 20);
			this.txtSP.TabIndex = 11;
			// 
			// label6
			// 
			this.label6.Anchor = System.Windows.Forms.AnchorStyles.Right;
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(124, 58);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(24, 13);
			this.label6.TabIndex = 12;
			this.label6.Text = "PC:";
			// 
			// label7
			// 
			this.label7.Anchor = System.Windows.Forms.AnchorStyles.Right;
			this.label7.AutoSize = true;
			this.label7.Location = new System.Drawing.Point(124, 32);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(24, 13);
			this.label7.TabIndex = 14;
			this.label7.Text = "HL:";
			// 
			// txtHL
			// 
			this.txtHL.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.txtHL.Location = new System.Drawing.Point(154, 29);
			this.txtHL.Name = "txtHL";
			this.txtHL.Size = new System.Drawing.Size(40, 20);
			this.txtHL.TabIndex = 15;
			this.txtHL.Text = "DD";
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 4;
			this.tableLayoutPanel1.SetColumnSpan(this.tableLayoutPanel2, 5);
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel2.Controls.Add(this.chkCarry, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkHalfCarry, 1, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkIme, 2, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkNegative, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkZero, 1, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkHalted, 2, 0);
			this.tableLayoutPanel2.Location = new System.Drawing.Point(0, 78);
			this.tableLayoutPanel2.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 2;
			this.tableLayoutPanel1.SetRowSpan(this.tableLayoutPanel2, 2);
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(135, 44);
			this.tableLayoutPanel2.TabIndex = 16;
			// 
			// chkCarry
			// 
			this.chkCarry.AutoSize = true;
			this.chkCarry.Location = new System.Drawing.Point(3, 3);
			this.chkCarry.Name = "chkCarry";
			this.chkCarry.Size = new System.Drawing.Size(33, 17);
			this.chkCarry.TabIndex = 23;
			this.chkCarry.Text = "C";
			this.chkCarry.UseVisualStyleBackColor = true;
			// 
			// chkNegative
			// 
			this.chkNegative.AutoSize = true;
			this.chkNegative.Location = new System.Drawing.Point(3, 26);
			this.chkNegative.Name = "chkNegative";
			this.chkNegative.Size = new System.Drawing.Size(34, 15);
			this.chkNegative.TabIndex = 17;
			this.chkNegative.Text = "N";
			this.chkNegative.UseVisualStyleBackColor = true;
			// 
			// chkZero
			// 
			this.chkZero.AutoSize = true;
			this.chkZero.Location = new System.Drawing.Point(43, 26);
			this.chkZero.Name = "chkZero";
			this.chkZero.Size = new System.Drawing.Size(33, 15);
			this.chkZero.TabIndex = 21;
			this.chkZero.Text = "Z";
			this.chkZero.UseVisualStyleBackColor = true;
			// 
			// chkHalfCarry
			// 
			this.chkHalfCarry.AutoSize = true;
			this.chkHalfCarry.Location = new System.Drawing.Point(43, 3);
			this.chkHalfCarry.Name = "chkHalfCarry";
			this.chkHalfCarry.Size = new System.Drawing.Size(34, 17);
			this.chkHalfCarry.TabIndex = 25;
			this.chkHalfCarry.Text = "H";
			this.chkHalfCarry.UseVisualStyleBackColor = true;
			// 
			// chkIme
			// 
			this.chkIme.AutoSize = true;
			this.chkIme.Location = new System.Drawing.Point(83, 26);
			this.chkIme.Name = "chkIme";
			this.chkIme.Size = new System.Drawing.Size(45, 15);
			this.chkIme.TabIndex = 26;
			this.chkIme.Text = "IME";
			this.chkIme.UseVisualStyleBackColor = true;
			// 
			// chkHalted
			// 
			this.chkHalted.AutoSize = true;
			this.chkHalted.Location = new System.Drawing.Point(83, 3);
			this.chkHalted.Name = "chkHalted";
			this.chkHalted.Size = new System.Drawing.Size(45, 17);
			this.chkHalted.TabIndex = 27;
			this.chkHalted.Text = "Halt";
			this.chkHalted.UseVisualStyleBackColor = true;
			// 
			// label12
			// 
			this.label12.Anchor = System.Windows.Forms.AnchorStyles.Right;
			this.label12.AutoSize = true;
			this.label12.Location = new System.Drawing.Point(112, 6);
			this.label12.Name = "label12";
			this.label12.Size = new System.Drawing.Size(36, 13);
			this.label12.TabIndex = 31;
			this.label12.Text = "Cycle:";
			// 
			// txtCycleCount
			// 
			this.tableLayoutPanel1.SetColumnSpan(this.txtCycleCount, 2);
			this.txtCycleCount.Location = new System.Drawing.Point(154, 3);
			this.txtCycleCount.Name = "txtCycleCount";
			this.txtCycleCount.Size = new System.Drawing.Size(84, 20);
			this.txtCycleCount.TabIndex = 32;
			this.txtCycleCount.Text = "DDDD";
			// 
			// grpPpu
			// 
			this.grpPpu.Controls.Add(this.tableLayoutPanel3);
			this.grpPpu.Dock = System.Windows.Forms.DockStyle.Top;
			this.grpPpu.Location = new System.Drawing.Point(0, 141);
			this.grpPpu.Name = "grpPpu";
			this.grpPpu.Size = new System.Drawing.Size(342, 47);
			this.grpPpu.TabIndex = 2;
			this.grpPpu.TabStop = false;
			this.grpPpu.Text = "PPU";
			// 
			// tableLayoutPanel3
			// 
			this.tableLayoutPanel3.ColumnCount = 6;
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Controls.Add(this.txtScanline, 0, 0);
			this.tableLayoutPanel3.Controls.Add(this.label9, 0, 0);
			this.tableLayoutPanel3.Controls.Add(this.label10, 2, 0);
			this.tableLayoutPanel3.Controls.Add(this.txtCycle, 3, 0);
			this.tableLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel3.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel3.Name = "tableLayoutPanel3";
			this.tableLayoutPanel3.RowCount = 2;
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Size = new System.Drawing.Size(336, 28);
			this.tableLayoutPanel3.TabIndex = 0;
			// 
			// txtScanline
			// 
			this.txtScanline.Location = new System.Drawing.Point(60, 3);
			this.txtScanline.Name = "txtScanline";
			this.txtScanline.Size = new System.Drawing.Size(33, 20);
			this.txtScanline.TabIndex = 3;
			this.txtScanline.Text = "555";
			// 
			// label9
			// 
			this.label9.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label9.AutoSize = true;
			this.label9.Location = new System.Drawing.Point(3, 6);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(51, 13);
			this.label9.TabIndex = 1;
			this.label9.Text = "Scanline:";
			// 
			// label10
			// 
			this.label10.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label10.AutoSize = true;
			this.label10.Location = new System.Drawing.Point(99, 6);
			this.label10.Name = "label10";
			this.label10.Size = new System.Drawing.Size(36, 13);
			this.label10.TabIndex = 2;
			this.label10.Text = "Cycle:";
			// 
			// txtCycle
			// 
			this.txtCycle.Location = new System.Drawing.Point(141, 3);
			this.txtCycle.Name = "txtCycle";
			this.txtCycle.Size = new System.Drawing.Size(33, 20);
			this.txtCycle.TabIndex = 4;
			this.txtCycle.Text = "555";
			// 
			// ctrlGameboyStatus
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.grpPpu);
			this.Controls.Add(this.grpCpu);
			this.Name = "ctrlGameboyStatus";
			this.Size = new System.Drawing.Size(342, 187);
			this.grpCpu.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.grpPpu.ResumeLayout(false);
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel3.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.GroupBox grpCpu;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label lblA;
		private System.Windows.Forms.TextBox txtA;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox txtF;
		private System.Windows.Forms.TextBox txtD;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.TextBox txtSP;
		private System.Windows.Forms.TextBox txtPC;
		private System.Windows.Forms.TextBox txtHL;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.CheckBox chkHalfCarry;
		private System.Windows.Forms.CheckBox chkCarry;
		private System.Windows.Forms.CheckBox chkZero;
		private System.Windows.Forms.CheckBox chkNegative;
		private System.Windows.Forms.TextBox txtStack;
	  private System.Windows.Forms.TextBox txtB;
	  private System.Windows.Forms.TextBox txtC;
	  private System.Windows.Forms.Label label8;
	  private System.Windows.Forms.TextBox txtE;
	  private System.Windows.Forms.Label label4;
	  private System.Windows.Forms.Label label3;
	  private System.Windows.Forms.Label label6;
	  private System.Windows.Forms.Label label7;
	  private System.Windows.Forms.GroupBox grpPpu;
	  private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
	  private System.Windows.Forms.TextBox txtScanline;
	  private System.Windows.Forms.Label label9;
	  private System.Windows.Forms.Label label10;
	  private System.Windows.Forms.TextBox txtCycle;
		private System.Windows.Forms.CheckBox chkIme;
		private System.Windows.Forms.CheckBox chkHalted;
		private System.Windows.Forms.Label label12;
		private System.Windows.Forms.TextBox txtCycleCount;
	}
}
