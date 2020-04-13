namespace Mesen.GUI.Debugger
{
	partial class frmPaletteViewer
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
			this.ctrlScanlineCycleSelect = new Mesen.GUI.Debugger.Controls.ctrlScanlineCycleSelect();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.ctrlPaletteViewer = new Mesen.GUI.Debugger.ctrlPaletteViewer();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.txtR = new System.Windows.Forms.TextBox();
			this.txtValue = new System.Windows.Forms.TextBox();
			this.lblIndex = new System.Windows.Forms.Label();
			this.lblValue = new System.Windows.Forms.Label();
			this.lblR = new System.Windows.Forms.Label();
			this.lblG = new System.Windows.Forms.Label();
			this.lblB = new System.Windows.Forms.Label();
			this.lblRgb = new System.Windows.Forms.Label();
			this.txtIndex = new System.Windows.Forms.TextBox();
			this.txtG = new System.Windows.Forms.TextBox();
			this.txtB = new System.Windows.Forms.TextBox();
			this.txtRgb = new System.Windows.Forms.TextBox();
			this.tableLayoutPanel1.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlScanlineCycleSelect
			// 
			this.ctrlScanlineCycleSelect.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.ctrlScanlineCycleSelect.Location = new System.Drawing.Point(0, 275);
			this.ctrlScanlineCycleSelect.Name = "ctrlScanlineCycleSelect";
			this.ctrlScanlineCycleSelect.Size = new System.Drawing.Size(398, 28);
			this.ctrlScanlineCycleSelect.TabIndex = 5;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.ctrlPaletteViewer, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 1, 1);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(398, 275);
			this.tableLayoutPanel1.TabIndex = 7;
			// 
			// ctrlPaletteViewer
			// 
			this.ctrlPaletteViewer.Location = new System.Drawing.Point(3, 3);
			this.ctrlPaletteViewer.Name = "ctrlPaletteViewer";
			this.ctrlPaletteViewer.PaletteScale = 16;
			this.ctrlPaletteViewer.SelectedPalette = 0;
			this.ctrlPaletteViewer.SelectionMode = Mesen.GUI.Debugger.PaletteSelectionMode.SingleColor;
			this.ctrlPaletteViewer.Size = new System.Drawing.Size(256, 256);
			this.ctrlPaletteViewer.TabIndex = 0;
			this.ctrlPaletteViewer.TabStop = false;
			this.ctrlPaletteViewer.SelectionChanged += new Mesen.GUI.Debugger.ctrlPaletteViewer.SelectionChangedHandler(this.ctrlPaletteViewer_SelectionChanged);
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 2;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Controls.Add(this.txtR, 1, 2);
			this.tableLayoutPanel2.Controls.Add(this.txtValue, 1, 1);
			this.tableLayoutPanel2.Controls.Add(this.lblIndex, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.lblValue, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.lblR, 0, 2);
			this.tableLayoutPanel2.Controls.Add(this.lblG, 0, 3);
			this.tableLayoutPanel2.Controls.Add(this.lblB, 0, 4);
			this.tableLayoutPanel2.Controls.Add(this.lblRgb, 0, 5);
			this.tableLayoutPanel2.Controls.Add(this.txtIndex, 1, 0);
			this.tableLayoutPanel2.Controls.Add(this.txtG, 1, 3);
			this.tableLayoutPanel2.Controls.Add(this.txtB, 1, 4);
			this.tableLayoutPanel2.Controls.Add(this.txtRgb, 1, 5);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(265, 3);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 7;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(136, 269);
			this.tableLayoutPanel2.TabIndex = 1;
			// 
			// txtR
			// 
			this.txtR.Location = new System.Drawing.Point(71, 55);
			this.txtR.Name = "txtR";
			this.txtR.ReadOnly = true;
			this.txtR.Size = new System.Drawing.Size(42, 20);
			this.txtR.TabIndex = 11;
			// 
			// txtValue
			// 
			this.txtValue.Location = new System.Drawing.Point(71, 29);
			this.txtValue.Name = "txtValue";
			this.txtValue.ReadOnly = true;
			this.txtValue.Size = new System.Drawing.Size(42, 20);
			this.txtValue.TabIndex = 7;
			// 
			// lblIndex
			// 
			this.lblIndex.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblIndex.AutoSize = true;
			this.lblIndex.Location = new System.Drawing.Point(3, 6);
			this.lblIndex.Name = "lblIndex";
			this.lblIndex.Size = new System.Drawing.Size(36, 13);
			this.lblIndex.TabIndex = 0;
			this.lblIndex.Text = "Index:";
			// 
			// lblValue
			// 
			this.lblValue.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblValue.AutoSize = true;
			this.lblValue.Location = new System.Drawing.Point(3, 32);
			this.lblValue.Name = "lblValue";
			this.lblValue.Size = new System.Drawing.Size(37, 13);
			this.lblValue.TabIndex = 1;
			this.lblValue.Text = "Value:";
			// 
			// lblR
			// 
			this.lblR.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblR.AutoSize = true;
			this.lblR.Location = new System.Drawing.Point(3, 58);
			this.lblR.Name = "lblR";
			this.lblR.Size = new System.Drawing.Size(18, 13);
			this.lblR.TabIndex = 2;
			this.lblR.Text = "R:";
			// 
			// lblG
			// 
			this.lblG.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblG.AutoSize = true;
			this.lblG.Location = new System.Drawing.Point(3, 84);
			this.lblG.Name = "lblG";
			this.lblG.Size = new System.Drawing.Size(18, 13);
			this.lblG.TabIndex = 3;
			this.lblG.Text = "G:";
			// 
			// lblB
			// 
			this.lblB.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblB.AutoSize = true;
			this.lblB.Location = new System.Drawing.Point(3, 110);
			this.lblB.Name = "lblB";
			this.lblB.Size = new System.Drawing.Size(17, 13);
			this.lblB.TabIndex = 4;
			this.lblB.Text = "B:";
			// 
			// lblRgb
			// 
			this.lblRgb.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblRgb.AutoSize = true;
			this.lblRgb.Location = new System.Drawing.Point(3, 136);
			this.lblRgb.Name = "lblRgb";
			this.lblRgb.Size = new System.Drawing.Size(62, 13);
			this.lblRgb.TabIndex = 5;
			this.lblRgb.Text = "24-bit RGB:";
			// 
			// txtIndex
			// 
			this.txtIndex.Location = new System.Drawing.Point(71, 3);
			this.txtIndex.Name = "txtIndex";
			this.txtIndex.ReadOnly = true;
			this.txtIndex.Size = new System.Drawing.Size(42, 20);
			this.txtIndex.TabIndex = 6;
			// 
			// txtG
			// 
			this.txtG.Location = new System.Drawing.Point(71, 81);
			this.txtG.Name = "txtG";
			this.txtG.ReadOnly = true;
			this.txtG.Size = new System.Drawing.Size(42, 20);
			this.txtG.TabIndex = 8;
			// 
			// txtB
			// 
			this.txtB.Location = new System.Drawing.Point(71, 107);
			this.txtB.Name = "txtB";
			this.txtB.ReadOnly = true;
			this.txtB.Size = new System.Drawing.Size(42, 20);
			this.txtB.TabIndex = 9;
			// 
			// txtRgb
			// 
			this.txtRgb.Location = new System.Drawing.Point(71, 133);
			this.txtRgb.Name = "txtRgb";
			this.txtRgb.ReadOnly = true;
			this.txtRgb.Size = new System.Drawing.Size(56, 20);
			this.txtRgb.TabIndex = 10;
			// 
			// frmPaletteViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(398, 303);
			this.Controls.Add(this.tableLayoutPanel1);
			this.Controls.Add(this.ctrlScanlineCycleSelect);
			this.Name = "frmPaletteViewer";
			this.Text = "Palette Viewer";
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion
		private Controls.ctrlScanlineCycleSelect ctrlScanlineCycleSelect;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private ctrlPaletteViewer ctrlPaletteViewer;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.TextBox txtR;
		private System.Windows.Forms.TextBox txtValue;
		private System.Windows.Forms.Label lblIndex;
		private System.Windows.Forms.Label lblValue;
		private System.Windows.Forms.Label lblR;
		private System.Windows.Forms.Label lblG;
		private System.Windows.Forms.Label lblB;
		private System.Windows.Forms.Label lblRgb;
		private System.Windows.Forms.TextBox txtIndex;
		private System.Windows.Forms.TextBox txtG;
		private System.Windows.Forms.TextBox txtB;
		private System.Windows.Forms.TextBox txtRgb;
	}
}