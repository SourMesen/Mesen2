namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlProfiler
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
			this.btnReset = new System.Windows.Forms.Button();
			this.lstFunctions = new Mesen.GUI.Controls.DoubleBufferedListView();
			this.colFunction = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colExclusiveTimePercent = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader7 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.btnReset, 1, 2);
			this.tableLayoutPanel1.Controls.Add(this.lstFunctions, 0, 1);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 3;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.Size = new System.Drawing.Size(673, 307);
			this.tableLayoutPanel1.TabIndex = 3;
			// 
			// btnReset
			// 
			this.btnReset.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.btnReset.Location = new System.Drawing.Point(595, 281);
			this.btnReset.Name = "btnReset";
			this.btnReset.Size = new System.Drawing.Size(75, 23);
			this.btnReset.TabIndex = 5;
			this.btnReset.Text = "Reset Counts";
			this.btnReset.UseVisualStyleBackColor = true;
			this.btnReset.Click += new System.EventHandler(this.btnReset_Click);
			// 
			// lstFunctions
			// 
			this.lstFunctions.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colFunction,
            this.columnHeader4,
            this.columnHeader3,
            this.columnHeader5,
            this.columnHeader2,
            this.colExclusiveTimePercent,
            this.columnHeader1,
            this.columnHeader6,
            this.columnHeader7});
			this.tableLayoutPanel1.SetColumnSpan(this.lstFunctions, 2);
			this.lstFunctions.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstFunctions.FullRowSelect = true;
			this.lstFunctions.GridLines = true;
			this.lstFunctions.HideSelection = false;
			this.lstFunctions.Location = new System.Drawing.Point(0, 0);
			this.lstFunctions.Margin = new System.Windows.Forms.Padding(0);
			this.lstFunctions.Name = "lstFunctions";
			this.lstFunctions.Size = new System.Drawing.Size(673, 278);
			this.lstFunctions.TabIndex = 7;
			this.lstFunctions.UseCompatibleStateImageBehavior = false;
			this.lstFunctions.View = System.Windows.Forms.View.Details;
			this.lstFunctions.VirtualMode = true;
			this.lstFunctions.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.lstFunctions_ColumnClick);
			this.lstFunctions.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstFunctions_RetrieveVirtualItem);
			this.lstFunctions.DoubleClick += new System.EventHandler(this.lstFunctions_DoubleClick);
			// 
			// colFunction
			// 
			this.colFunction.Text = "Function (Entry Addr)";
			this.colFunction.Width = 120;
			// 
			// columnHeader4
			// 
			this.columnHeader4.Text = "Call Count";
			// 
			// columnHeader3
			// 
			this.columnHeader3.Text = "Inclusive Time (Cyc)";
			this.columnHeader3.Width = 79;
			// 
			// columnHeader5
			// 
			this.columnHeader5.Text = "Inclusive Time (%)";
			this.columnHeader5.Width = 70;
			// 
			// columnHeader2
			// 
			this.columnHeader2.Text = "Exclusive Time (Cyc)";
			this.columnHeader2.Width = 72;
			// 
			// colExclusiveTimePercent
			// 
			this.colExclusiveTimePercent.Text = "Exclusive Time (%)";
			this.colExclusiveTimePercent.Width = 57;
			// 
			// columnHeader1
			// 
			this.columnHeader1.Text = "Avg. Cycles";
			// 
			// columnHeader6
			// 
			this.columnHeader6.Text = "Min. Cycles";
			// 
			// columnHeader7
			// 
			this.columnHeader7.Text = "Max. Cycles";
			// 
			// ctrlProfiler
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.tableLayoutPanel1);
			this.Margin = new System.Windows.Forms.Padding(0);
			this.Name = "ctrlProfiler";
			this.Size = new System.Drawing.Size(673, 307);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Button btnReset;
		private Mesen.GUI.Controls.DoubleBufferedListView lstFunctions;
		private System.Windows.Forms.ColumnHeader colFunction;
		private System.Windows.Forms.ColumnHeader columnHeader4;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ColumnHeader columnHeader5;
		private System.Windows.Forms.ColumnHeader colExclusiveTimePercent;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader6;
		private System.Windows.Forms.ColumnHeader columnHeader7;
	}
}
