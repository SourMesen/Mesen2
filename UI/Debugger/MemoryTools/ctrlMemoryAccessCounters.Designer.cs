namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlMemoryAccessCounters
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
			this.components = new System.ComponentModel.Container();
			this.lblViewMemoryType = new System.Windows.Forms.Label();
			this.cboMemoryType = new Mesen.GUI.Debugger.Controls.ctrlMemoryType();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.btnReset = new System.Windows.Forms.Button();
			this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
			this.picWatchHelp = new System.Windows.Forms.PictureBox();
			this.lblHint = new System.Windows.Forms.Label();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.lstCounters = new Mesen.GUI.Controls.DoubleBufferedListView();
			this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader9 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader7 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader8 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.toolTip = new System.Windows.Forms.ToolTip(this.components);
			this.tableLayoutPanel1.SuspendLayout();
			this.tableLayoutPanel3.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picWatchHelp)).BeginInit();
			this.tableLayoutPanel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// lblViewMemoryType
			// 
			this.lblViewMemoryType.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblViewMemoryType.AutoSize = true;
			this.lblViewMemoryType.Location = new System.Drawing.Point(3, 7);
			this.lblViewMemoryType.Name = "lblViewMemoryType";
			this.lblViewMemoryType.Size = new System.Drawing.Size(33, 13);
			this.lblViewMemoryType.TabIndex = 0;
			this.lblViewMemoryType.Text = "View:";
			// 
			// cboMemoryType
			// 
			this.cboMemoryType.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
			this.cboMemoryType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboMemoryType.FormattingEnabled = true;
			this.cboMemoryType.Location = new System.Drawing.Point(42, 3);
			this.cboMemoryType.Name = "cboMemoryType";
			this.cboMemoryType.Size = new System.Drawing.Size(162, 21);
			this.cboMemoryType.TabIndex = 1;
			this.cboMemoryType.SelectedIndexChanged += new System.EventHandler(this.cboMemoryType_SelectedIndexChanged);
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 2;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.btnReset, 1, 2);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel3, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.lstCounters, 0, 1);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 3;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(641, 343);
			this.tableLayoutPanel1.TabIndex = 3;
			// 
			// btnReset
			// 
			this.btnReset.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.btnReset.Location = new System.Drawing.Point(566, 320);
			this.btnReset.Margin = new System.Windows.Forms.Padding(0);
			this.btnReset.Name = "btnReset";
			this.btnReset.Size = new System.Drawing.Size(75, 23);
			this.btnReset.TabIndex = 5;
			this.btnReset.Text = "Reset";
			this.btnReset.UseVisualStyleBackColor = true;
			this.btnReset.Click += new System.EventHandler(this.btnReset_Click);
			// 
			// tableLayoutPanel3
			// 
			this.tableLayoutPanel3.ColumnCount = 2;
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Controls.Add(this.picWatchHelp, 0, 0);
			this.tableLayoutPanel3.Controls.Add(this.lblHint, 1, 0);
			this.tableLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel3.Location = new System.Drawing.Point(0, 320);
			this.tableLayoutPanel3.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel3.Name = "tableLayoutPanel3";
			this.tableLayoutPanel3.RowCount = 2;
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel3.Size = new System.Drawing.Size(566, 23);
			this.tableLayoutPanel3.TabIndex = 2;
			// 
			// picWatchHelp
			// 
			this.picWatchHelp.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.picWatchHelp.Image = global::Mesen.GUI.Properties.Resources.Warning;
			this.picWatchHelp.Location = new System.Drawing.Point(3, 3);
			this.picWatchHelp.Name = "picWatchHelp";
			this.picWatchHelp.Size = new System.Drawing.Size(16, 16);
			this.picWatchHelp.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.picWatchHelp.TabIndex = 2;
			this.picWatchHelp.TabStop = false;
			// 
			// lblHint
			// 
			this.lblHint.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblHint.AutoSize = true;
			this.lblHint.Location = new System.Drawing.Point(25, 4);
			this.lblHint.Name = "lblHint";
			this.lblHint.Size = new System.Drawing.Size(530, 13);
			this.lblHint.TabIndex = 0;
			this.lblHint.Text = "Uninitialized read column is only accurate if the debugger was active when the ga" +
    "me was loaded/power cycled";
			this.lblHint.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 2;
			this.tableLayoutPanel1.SetColumnSpan(this.tableLayoutPanel2, 2);
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Controls.Add(this.cboMemoryType, 1, 0);
			this.tableLayoutPanel2.Controls.Add(this.lblViewMemoryType, 0, 0);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel2.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 1;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(641, 27);
			this.tableLayoutPanel2.TabIndex = 6;
			// 
			// lstCounters
			// 
			this.lstCounters.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader9,
            this.columnHeader2,
            this.columnHeader6,
            this.columnHeader3,
            this.columnHeader7,
            this.columnHeader4,
            this.columnHeader8,
            this.columnHeader5});
			this.tableLayoutPanel1.SetColumnSpan(this.lstCounters, 2);
			this.lstCounters.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstCounters.FullRowSelect = true;
			this.lstCounters.Location = new System.Drawing.Point(0, 27);
			this.lstCounters.Margin = new System.Windows.Forms.Padding(0);
			this.lstCounters.Name = "lstCounters";
			this.lstCounters.Size = new System.Drawing.Size(641, 293);
			this.lstCounters.TabIndex = 7;
			this.lstCounters.UseCompatibleStateImageBehavior = false;
			this.lstCounters.View = System.Windows.Forms.View.Details;
			this.lstCounters.VirtualMode = true;
			this.lstCounters.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.lstCounters_ColumnClick);
			this.lstCounters.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstCounters_RetrieveVirtualItem);
			// 
			// columnHeader1
			// 
			this.columnHeader1.Text = "Address";
			// 
			// columnHeader9
			// 
			this.columnHeader9.Text = "Value";
			this.columnHeader9.Width = 40;
			// 
			// columnHeader2
			// 
			this.columnHeader2.Text = "Reads";
			this.columnHeader2.Width = 70;
			// 
			// columnHeader6
			// 
			this.columnHeader6.Text = "Last Read";
			this.columnHeader6.Width = 70;
			// 
			// columnHeader3
			// 
			this.columnHeader3.Text = "Writes";
			this.columnHeader3.Width = 70;
			// 
			// columnHeader7
			// 
			this.columnHeader7.Text = "Last Write";
			this.columnHeader7.Width = 70;
			// 
			// columnHeader4
			// 
			this.columnHeader4.Text = "Executes";
			this.columnHeader4.Width = 70;
			// 
			// columnHeader8
			// 
			this.columnHeader8.Text = "Last Exec";
			this.columnHeader8.Width = 70;
			// 
			// columnHeader5
			// 
			this.columnHeader5.Text = "Uninit Read";
			this.columnHeader5.Width = 70;
			// 
			// toolTip
			// 
			this.toolTip.AutomaticDelay = 0;
			this.toolTip.AutoPopDelay = 32700;
			this.toolTip.InitialDelay = 10;
			this.toolTip.ReshowDelay = 10;
			// 
			// ctrlMemoryAccessCounters
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.tableLayoutPanel1);
			this.Margin = new System.Windows.Forms.Padding(0);
			this.Name = "ctrlMemoryAccessCounters";
			this.Size = new System.Drawing.Size(641, 343);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel3.ResumeLayout(false);
			this.tableLayoutPanel3.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picWatchHelp)).EndInit();
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion
		private System.Windows.Forms.Label lblViewMemoryType;
		private ctrlMemoryType cboMemoryType;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Button btnReset;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
		private System.Windows.Forms.ToolTip toolTip;
	  private Mesen.GUI.Controls.DoubleBufferedListView lstCounters;
	  private System.Windows.Forms.ColumnHeader columnHeader1;
	  private System.Windows.Forms.ColumnHeader columnHeader2;
	  private System.Windows.Forms.ColumnHeader columnHeader3;
	  private System.Windows.Forms.ColumnHeader columnHeader4;
	  private System.Windows.Forms.ColumnHeader columnHeader5;
		private System.Windows.Forms.ColumnHeader columnHeader6;
		private System.Windows.Forms.ColumnHeader columnHeader7;
		private System.Windows.Forms.ColumnHeader columnHeader8;
		private System.Windows.Forms.Label lblHint;
		private System.Windows.Forms.PictureBox picWatchHelp;
		private System.Windows.Forms.ColumnHeader columnHeader9;
	}
}
