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
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlScanlineCycleSelect
			// 
			this.ctrlScanlineCycleSelect.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.ctrlScanlineCycleSelect.Location = new System.Drawing.Point(0, 263);
			this.ctrlScanlineCycleSelect.Name = "ctrlScanlineCycleSelect";
			this.ctrlScanlineCycleSelect.Size = new System.Drawing.Size(485, 28);
			this.ctrlScanlineCycleSelect.TabIndex = 5;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.ctrlPaletteViewer, 0, 1);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(485, 263);
			this.tableLayoutPanel1.TabIndex = 7;
			// 
			// ctrlPaletteViewer1
			// 
			this.ctrlPaletteViewer.Location = new System.Drawing.Point(3, 3);
			this.ctrlPaletteViewer.Name = "ctrlPaletteViewer";
			this.ctrlPaletteViewer.Size = new System.Drawing.Size(256, 256);
			this.ctrlPaletteViewer.TabIndex = 0;
			// 
			// frmPaletteViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(485, 291);
			this.Controls.Add(this.tableLayoutPanel1);
			this.Controls.Add(this.ctrlScanlineCycleSelect);
			this.Name = "frmPaletteViewer";
			this.Text = "Palette Viewer";
			this.tableLayoutPanel1.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion
		private Controls.ctrlScanlineCycleSelect ctrlScanlineCycleSelect;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private ctrlPaletteViewer ctrlPaletteViewer;
	}
}