namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlSpcStatus
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
			this.grpSpc = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.label1 = new System.Windows.Forms.Label();
			this.lblA = new System.Windows.Forms.Label();
			this.txtA = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.txtX = new System.Windows.Forms.TextBox();
			this.txtY = new System.Windows.Forms.TextBox();
			this.label7 = new System.Windows.Forms.Label();
			this.txtP = new System.Windows.Forms.TextBox();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.chkHalfCarry = new System.Windows.Forms.CheckBox();
			this.chkInterrupt = new System.Windows.Forms.CheckBox();
			this.chkCarry = new System.Windows.Forms.CheckBox();
			this.chkZero = new System.Windows.Forms.CheckBox();
			this.chkBreak = new System.Windows.Forms.CheckBox();
			this.chkPage = new System.Windows.Forms.CheckBox();
			this.chkNegative = new System.Windows.Forms.CheckBox();
			this.chkOverflow = new System.Windows.Forms.CheckBox();
			this.txtStack = new System.Windows.Forms.TextBox();
			this.label6 = new System.Windows.Forms.Label();
			this.txtPC = new System.Windows.Forms.TextBox();
			this.label5 = new System.Windows.Forms.Label();
			this.txtS = new System.Windows.Forms.TextBox();
			this.grpSpc.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// grpSpc
			// 
			this.grpSpc.Controls.Add(this.tableLayoutPanel1);
			this.grpSpc.Dock = System.Windows.Forms.DockStyle.Top;
			this.grpSpc.Location = new System.Drawing.Point(0, 0);
			this.grpSpc.Name = "grpSpc";
			this.grpSpc.Size = new System.Drawing.Size(342, 120);
			this.grpSpc.TabIndex = 0;
			this.grpSpc.TabStop = false;
			this.grpSpc.Text = "SPC";
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 11;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
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
			this.tableLayoutPanel1.Controls.Add(this.label1, 2, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblA, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtA, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.label2, 4, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtX, 3, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtY, 5, 0);
			this.tableLayoutPanel1.Controls.Add(this.label7, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.txtP, 1, 2);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 2, 2);
			this.tableLayoutPanel1.Controls.Add(this.txtStack, 8, 2);
			this.tableLayoutPanel1.Controls.Add(this.label6, 6, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtPC, 7, 0);
			this.tableLayoutPanel1.Controls.Add(this.label5, 8, 0);
			this.tableLayoutPanel1.Controls.Add(this.txtS, 9, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 5;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.Size = new System.Drawing.Size(336, 101);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// label1
			// 
			this.label1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(57, 6);
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
			this.txtA.Location = new System.Drawing.Point(26, 3);
			this.txtA.Name = "txtA";
			this.txtA.Size = new System.Drawing.Size(25, 20);
			this.txtA.TabIndex = 1;
			this.txtA.Text = "DDDD";
			// 
			// label2
			// 
			this.label2.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(111, 6);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(17, 13);
			this.label2.TabIndex = 3;
			this.label2.Text = "Y:";
			// 
			// txtX
			// 
			this.txtX.Location = new System.Drawing.Point(80, 3);
			this.txtX.Name = "txtX";
			this.txtX.Size = new System.Drawing.Size(25, 20);
			this.txtX.TabIndex = 4;
			// 
			// txtY
			// 
			this.txtY.Location = new System.Drawing.Point(134, 3);
			this.txtY.Name = "txtY";
			this.txtY.Size = new System.Drawing.Size(25, 20);
			this.txtY.TabIndex = 5;
			// 
			// label7
			// 
			this.label7.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label7.AutoSize = true;
			this.label7.Location = new System.Drawing.Point(3, 32);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(17, 13);
			this.label7.TabIndex = 14;
			this.label7.Text = "P:";
			// 
			// txtP
			// 
			this.txtP.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.txtP.Location = new System.Drawing.Point(26, 29);
			this.txtP.Name = "txtP";
			this.txtP.Size = new System.Drawing.Size(25, 20);
			this.txtP.TabIndex = 15;
			this.txtP.Text = "DD";
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 5;
			this.tableLayoutPanel1.SetColumnSpan(this.tableLayoutPanel2, 6);
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 40F));
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel2.Controls.Add(this.chkHalfCarry, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkInterrupt, 1, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkCarry, 3, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkZero, 2, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkBreak, 3, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkPage, 2, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkNegative, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkOverflow, 1, 0);
			this.tableLayoutPanel2.Location = new System.Drawing.Point(57, 29);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 2;
			this.tableLayoutPanel1.SetRowSpan(this.tableLayoutPanel2, 2);
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.Size = new System.Drawing.Size(159, 46);
			this.tableLayoutPanel2.TabIndex = 16;
			// 
			// chkHalfCarry
			// 
			this.chkHalfCarry.AutoSize = true;
			this.chkHalfCarry.Location = new System.Drawing.Point(3, 26);
			this.chkHalfCarry.Name = "chkHalfCarry";
			this.chkHalfCarry.Size = new System.Drawing.Size(34, 17);
			this.chkHalfCarry.TabIndex = 25;
			this.chkHalfCarry.Text = "H";
			this.chkHalfCarry.UseVisualStyleBackColor = true;
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
			this.chkCarry.Location = new System.Drawing.Point(121, 26);
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
			// chkBreak
			// 
			this.chkBreak.AutoSize = true;
			this.chkBreak.Location = new System.Drawing.Point(121, 3);
			this.chkBreak.Name = "chkBreak";
			this.chkBreak.Size = new System.Drawing.Size(33, 17);
			this.chkBreak.TabIndex = 18;
			this.chkBreak.Text = "B";
			this.chkBreak.UseVisualStyleBackColor = true;
			// 
			// chkPage
			// 
			this.chkPage.AutoSize = true;
			this.chkPage.Location = new System.Drawing.Point(82, 3);
			this.chkPage.Name = "chkPage";
			this.chkPage.Size = new System.Drawing.Size(33, 17);
			this.chkPage.TabIndex = 19;
			this.chkPage.Text = "P";
			this.chkPage.UseVisualStyleBackColor = true;
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
			// txtStack
			// 
			this.txtStack.BackColor = System.Drawing.SystemColors.Window;
			this.tableLayoutPanel1.SetColumnSpan(this.txtStack, 2);
			this.txtStack.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtStack.Location = new System.Drawing.Point(241, 29);
			this.txtStack.Multiline = true;
			this.txtStack.Name = "txtStack";
			this.txtStack.ReadOnly = true;
			this.tableLayoutPanel1.SetRowSpan(this.txtStack, 3);
			this.txtStack.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.txtStack.Size = new System.Drawing.Size(92, 69);
			this.txtStack.TabIndex = 23;
			// 
			// label6
			// 
			this.label6.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(165, 6);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(24, 13);
			this.label6.TabIndex = 12;
			this.label6.Text = "PC:";
			// 
			// txtPC
			// 
			this.txtPC.Location = new System.Drawing.Point(195, 3);
			this.txtPC.Name = "txtPC";
			this.txtPC.Size = new System.Drawing.Size(40, 20);
			this.txtPC.TabIndex = 13;
			this.txtPC.Text = "DDDD";
			// 
			// label5
			// 
			this.label5.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(241, 6);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(17, 13);
			this.label5.TabIndex = 10;
			this.label5.Text = "S:";
			// 
			// txtS
			// 
			this.txtS.Location = new System.Drawing.Point(264, 3);
			this.txtS.Name = "txtS";
			this.txtS.Size = new System.Drawing.Size(34, 20);
			this.txtS.TabIndex = 11;
			// 
			// ctrlSpcStatus
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.grpSpc);
			this.Name = "ctrlSpcStatus";
			this.Size = new System.Drawing.Size(342, 120);
			this.grpSpc.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.GroupBox grpSpc;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label lblA;
		private System.Windows.Forms.TextBox txtA;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox txtX;
		private System.Windows.Forms.TextBox txtY;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.TextBox txtS;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.TextBox txtPC;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.TextBox txtP;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.CheckBox chkHalfCarry;
		private System.Windows.Forms.CheckBox chkInterrupt;
		private System.Windows.Forms.CheckBox chkCarry;
		private System.Windows.Forms.CheckBox chkZero;
		private System.Windows.Forms.CheckBox chkBreak;
		private System.Windows.Forms.CheckBox chkPage;
		private System.Windows.Forms.CheckBox chkNegative;
		private System.Windows.Forms.CheckBox chkOverflow;
		private System.Windows.Forms.TextBox txtStack;
	}
}
