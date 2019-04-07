namespace Mesen.GUI.Debugger
{
	partial class ctrlWatch
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
			if(_watchManager != null) {
				_watchManager.WatchChanged -= WatchManager_WatchChanged;
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
			System.Windows.Forms.ListViewItem listViewItem1 = new System.Windows.Forms.ListViewItem("");
			this.lstWatch = new Mesen.GUI.Controls.WatchListView();
			this.colName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colValue = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.contextMenuWatch = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuRemoveWatch = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuEditInMemoryViewer = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuViewInDisassembly = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuMoveUp = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuMoveDown = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuDecimalDisplay = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHexDisplay = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuBinaryDisplay = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRowDisplayFormat = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRowBinary = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem8 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRowHex1 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRowHex2 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRowHex3 = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem6 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRowSigned1 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRowSigned2 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRowSigned3 = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem7 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRowUnsigned = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem9 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRowClearFormat = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuImport = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuExport = new System.Windows.Forms.ToolStripMenuItem();
			this.txtEdit = new System.Windows.Forms.TextBox();
			this.contextMenuWatch.SuspendLayout();
			this.SuspendLayout();
			// 
			// lstWatch
			// 
			this.lstWatch.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colName,
            this.colValue});
			this.lstWatch.ContextMenuStrip = this.contextMenuWatch;
			this.lstWatch.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstWatch.FullRowSelect = true;
			this.lstWatch.GridLines = true;
			this.lstWatch.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
			this.lstWatch.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem1});
			this.lstWatch.Location = new System.Drawing.Point(0, 0);
			this.lstWatch.Name = "lstWatch";
			this.lstWatch.Size = new System.Drawing.Size(378, 112);
			this.lstWatch.TabIndex = 6;
			this.lstWatch.UseCompatibleStateImageBehavior = false;
			this.lstWatch.View = System.Windows.Forms.View.Details;
			this.lstWatch.OnMoveUpDown += new Mesen.GUI.Controls.WatchListView.MoveUpDownHandler(this.lstWatch_OnMoveUpDown);
			this.lstWatch.SelectedIndexChanged += new System.EventHandler(this.lstWatch_SelectedIndexChanged);
			this.lstWatch.Click += new System.EventHandler(this.lstWatch_Click);
			this.lstWatch.DoubleClick += new System.EventHandler(this.lstWatch_DoubleClick);
			this.lstWatch.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.lstWatch_KeyPress);
			// 
			// colName
			// 
			this.colName.Text = "Name";
			this.colName.Width = 180;
			// 
			// colValue
			// 
			this.colValue.Text = "Value";
			this.colValue.Width = 110;
			// 
			// contextMenuWatch
			// 
			this.contextMenuWatch.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuRemoveWatch,
            this.toolStripMenuItem1,
            this.mnuEditInMemoryViewer,
            this.mnuViewInDisassembly,
            this.toolStripMenuItem4,
            this.mnuMoveUp,
            this.mnuMoveDown,
            this.toolStripMenuItem2,
            this.mnuRowDisplayFormat,
            this.toolStripMenuItem3,
            this.mnuDecimalDisplay,
            this.mnuHexDisplay,
            this.mnuBinaryDisplay,
            this.toolStripMenuItem5,
            this.mnuImport,
            this.mnuExport});
			this.contextMenuWatch.Name = "contextMenuWatch";
			this.contextMenuWatch.Size = new System.Drawing.Size(194, 298);
			this.contextMenuWatch.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuWatch_Opening);
			// 
			// mnuRemoveWatch
			// 
			this.mnuRemoveWatch.Image = global::Mesen.GUI.Properties.Resources.Close;
			this.mnuRemoveWatch.Name = "mnuRemoveWatch";
			this.mnuRemoveWatch.Size = new System.Drawing.Size(193, 22);
			this.mnuRemoveWatch.Text = "Remove";
			this.mnuRemoveWatch.Click += new System.EventHandler(this.mnuRemoveWatch_Click);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(190, 6);
			// 
			// mnuEditInMemoryViewer
			// 
			this.mnuEditInMemoryViewer.Image = global::Mesen.GUI.Properties.Resources.CheatCode;
			this.mnuEditInMemoryViewer.Name = "mnuEditInMemoryViewer";
			this.mnuEditInMemoryViewer.Size = new System.Drawing.Size(193, 22);
			this.mnuEditInMemoryViewer.Text = "Edit in Memory Viewer";
			this.mnuEditInMemoryViewer.Click += new System.EventHandler(this.mnuEditInMemoryViewer_Click);
			// 
			// mnuViewInDisassembly
			// 
			this.mnuViewInDisassembly.Image = global::Mesen.GUI.Properties.Resources.Debugger;
			this.mnuViewInDisassembly.Name = "mnuViewInDisassembly";
			this.mnuViewInDisassembly.Size = new System.Drawing.Size(193, 22);
			this.mnuViewInDisassembly.Text = "View in disassembly";
			this.mnuViewInDisassembly.Click += new System.EventHandler(this.mnuViewInDisassembly_Click);
			// 
			// toolStripMenuItem4
			// 
			this.toolStripMenuItem4.Name = "toolStripMenuItem4";
			this.toolStripMenuItem4.Size = new System.Drawing.Size(190, 6);
			// 
			// mnuMoveUp
			// 
			this.mnuMoveUp.Image = global::Mesen.GUI.Properties.Resources.MoveUp;
			this.mnuMoveUp.Name = "mnuMoveUp";
			this.mnuMoveUp.Size = new System.Drawing.Size(193, 22);
			this.mnuMoveUp.Text = "Move up";
			this.mnuMoveUp.Click += new System.EventHandler(this.mnuMoveUp_Click);
			// 
			// mnuMoveDown
			// 
			this.mnuMoveDown.Image = global::Mesen.GUI.Properties.Resources.MoveDown;
			this.mnuMoveDown.Name = "mnuMoveDown";
			this.mnuMoveDown.Size = new System.Drawing.Size(193, 22);
			this.mnuMoveDown.Text = "Move down";
			this.mnuMoveDown.Click += new System.EventHandler(this.mnuMoveDown_Click);
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(190, 6);
			// 
			// mnuDecimalDisplay
			// 
			this.mnuDecimalDisplay.Name = "mnuDecimalDisplay";
			this.mnuDecimalDisplay.Size = new System.Drawing.Size(193, 22);
			this.mnuDecimalDisplay.Text = "Decimal Display";
			this.mnuDecimalDisplay.Click += new System.EventHandler(this.mnuDecimalDisplay_Click);
			// 
			// mnuHexDisplay
			// 
			this.mnuHexDisplay.Checked = true;
			this.mnuHexDisplay.CheckState = System.Windows.Forms.CheckState.Checked;
			this.mnuHexDisplay.Name = "mnuHexDisplay";
			this.mnuHexDisplay.Size = new System.Drawing.Size(193, 22);
			this.mnuHexDisplay.Text = "Hexadecimal Display";
			this.mnuHexDisplay.Click += new System.EventHandler(this.mnuHexDisplay_Click);
			// 
			// mnuBinaryDisplay
			// 
			this.mnuBinaryDisplay.Name = "mnuBinaryDisplay";
			this.mnuBinaryDisplay.Size = new System.Drawing.Size(193, 22);
			this.mnuBinaryDisplay.Text = "Binary Display";
			this.mnuBinaryDisplay.Click += new System.EventHandler(this.mnuBinaryDisplay_Click);
			// 
			// toolStripMenuItem5
			// 
			this.toolStripMenuItem5.Name = "toolStripMenuItem5";
			this.toolStripMenuItem5.Size = new System.Drawing.Size(190, 6);
			// 
			// mnuRowDisplayFormat
			// 
			this.mnuRowDisplayFormat.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuRowBinary,
            this.toolStripMenuItem8,
            this.mnuRowHex1,
            this.mnuRowHex2,
            this.mnuRowHex3,
            this.toolStripMenuItem6,
            this.mnuRowSigned1,
            this.mnuRowSigned2,
            this.mnuRowSigned3,
            this.toolStripMenuItem7,
            this.mnuRowUnsigned,
            this.toolStripMenuItem9,
            this.mnuRowClearFormat});
			this.mnuRowDisplayFormat.Name = "mnuRowDisplayFormat";
			this.mnuRowDisplayFormat.Size = new System.Drawing.Size(193, 22);
			this.mnuRowDisplayFormat.Text = "Row Display Format";
			// 
			// mnuRowBinary
			// 
			this.mnuRowBinary.Name = "mnuRowBinary";
			this.mnuRowBinary.Size = new System.Drawing.Size(197, 22);
			this.mnuRowBinary.Text = "Binary";
			this.mnuRowBinary.Click += new System.EventHandler(this.mnuRowBinary_Click);
			// 
			// toolStripMenuItem8
			// 
			this.toolStripMenuItem8.Name = "toolStripMenuItem8";
			this.toolStripMenuItem8.Size = new System.Drawing.Size(194, 6);
			// 
			// mnuRowHex1
			// 
			this.mnuRowHex1.Name = "mnuRowHex1";
			this.mnuRowHex1.Size = new System.Drawing.Size(197, 22);
			this.mnuRowHex1.Text = "Hexadecimal (8-bit)";
			this.mnuRowHex1.Click += new System.EventHandler(this.mnuRowHex1_Click);
			// 
			// mnuRowHex2
			// 
			this.mnuRowHex2.Name = "mnuRowHex2";
			this.mnuRowHex2.Size = new System.Drawing.Size(197, 22);
			this.mnuRowHex2.Text = "Hexadecimal (16-bit)";
			this.mnuRowHex2.Click += new System.EventHandler(this.mnuRowHex2_Click);
			// 
			// mnuRowHex3
			// 
			this.mnuRowHex3.Name = "mnuRowHex3";
			this.mnuRowHex3.Size = new System.Drawing.Size(197, 22);
			this.mnuRowHex3.Text = "Hexadecimal (24-bit)";
			this.mnuRowHex3.Click += new System.EventHandler(this.mnuRowHex3_Click);
			// 
			// toolStripMenuItem6
			// 
			this.toolStripMenuItem6.Name = "toolStripMenuItem6";
			this.toolStripMenuItem6.Size = new System.Drawing.Size(194, 6);
			// 
			// mnuRowSigned1
			// 
			this.mnuRowSigned1.Name = "mnuRowSigned1";
			this.mnuRowSigned1.Size = new System.Drawing.Size(197, 22);
			this.mnuRowSigned1.Text = "Signed decimal (8-bit)";
			this.mnuRowSigned1.Click += new System.EventHandler(this.mnuRowSigned1_Click);
			// 
			// mnuRowSigned2
			// 
			this.mnuRowSigned2.Name = "mnuRowSigned2";
			this.mnuRowSigned2.Size = new System.Drawing.Size(197, 22);
			this.mnuRowSigned2.Text = "Signed decimal (16-bit)";
			this.mnuRowSigned2.Click += new System.EventHandler(this.mnuRowSigned2_Click);
			// 
			// mnuRowSigned3
			// 
			this.mnuRowSigned3.Name = "mnuRowSigned3";
			this.mnuRowSigned3.Size = new System.Drawing.Size(197, 22);
			this.mnuRowSigned3.Text = "Signed decimal (24-bit)";
			this.mnuRowSigned3.Click += new System.EventHandler(this.mnuRowSigned3_Click);
			// 
			// toolStripMenuItem7
			// 
			this.toolStripMenuItem7.Name = "toolStripMenuItem7";
			this.toolStripMenuItem7.Size = new System.Drawing.Size(194, 6);
			// 
			// mnuRowUnsigned
			// 
			this.mnuRowUnsigned.Name = "mnuRowUnsigned";
			this.mnuRowUnsigned.Size = new System.Drawing.Size(197, 22);
			this.mnuRowUnsigned.Text = "Unsigned decimal";
			this.mnuRowUnsigned.Click += new System.EventHandler(this.mnuRowUnsigned_Click);
			// 
			// toolStripMenuItem9
			// 
			this.toolStripMenuItem9.Name = "toolStripMenuItem9";
			this.toolStripMenuItem9.Size = new System.Drawing.Size(194, 6);
			// 
			// mnuRowClearFormat
			// 
			this.mnuRowClearFormat.Image = global::Mesen.GUI.Properties.Resources.Close;
			this.mnuRowClearFormat.Name = "mnuRowClearFormat";
			this.mnuRowClearFormat.Size = new System.Drawing.Size(197, 22);
			this.mnuRowClearFormat.Text = "Clear";
			this.mnuRowClearFormat.Click += new System.EventHandler(this.mnuRowClearFormat_Click);
			// 
			// toolStripMenuItem3
			// 
			this.toolStripMenuItem3.Name = "toolStripMenuItem3";
			this.toolStripMenuItem3.Size = new System.Drawing.Size(190, 6);
			// 
			// mnuImport
			// 
			this.mnuImport.Image = global::Mesen.GUI.Properties.Resources.Import;
			this.mnuImport.Name = "mnuImport";
			this.mnuImport.Size = new System.Drawing.Size(193, 22);
			this.mnuImport.Text = "Import...";
			this.mnuImport.Click += new System.EventHandler(this.mnuImport_Click);
			// 
			// mnuExport
			// 
			this.mnuExport.Image = global::Mesen.GUI.Properties.Resources.Export;
			this.mnuExport.Name = "mnuExport";
			this.mnuExport.Size = new System.Drawing.Size(193, 22);
			this.mnuExport.Text = "Export...";
			this.mnuExport.Click += new System.EventHandler(this.mnuExport_Click);
			// 
			// txtEdit
			// 
			this.txtEdit.AcceptsReturn = true;
			this.txtEdit.Location = new System.Drawing.Point(3, 24);
			this.txtEdit.Name = "txtEdit";
			this.txtEdit.Size = new System.Drawing.Size(177, 20);
			this.txtEdit.TabIndex = 7;
			this.txtEdit.Visible = false;
			this.txtEdit.Leave += new System.EventHandler(this.txtEdit_Leave);
			// 
			// ctrlWatch
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.txtEdit);
			this.Controls.Add(this.lstWatch);
			this.Name = "ctrlWatch";
			this.Size = new System.Drawing.Size(378, 112);
			this.contextMenuWatch.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private Mesen.GUI.Controls.WatchListView lstWatch;
		private System.Windows.Forms.ColumnHeader colName;
		private System.Windows.Forms.ColumnHeader colValue;
		private System.Windows.Forms.ContextMenuStrip contextMenuWatch;
		private System.Windows.Forms.ToolStripMenuItem mnuRemoveWatch;
		private System.Windows.Forms.ToolStripMenuItem mnuHexDisplay;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuEditInMemoryViewer;
		private System.Windows.Forms.ToolStripMenuItem mnuViewInDisassembly;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.TextBox txtEdit;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
		private System.Windows.Forms.ToolStripMenuItem mnuMoveUp;
		private System.Windows.Forms.ToolStripMenuItem mnuMoveDown;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
		private System.Windows.Forms.ToolStripMenuItem mnuImport;
		private System.Windows.Forms.ToolStripMenuItem mnuExport;
		private System.Windows.Forms.ToolStripMenuItem mnuDecimalDisplay;
		private System.Windows.Forms.ToolStripMenuItem mnuBinaryDisplay;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem5;
		private System.Windows.Forms.ToolStripMenuItem mnuRowDisplayFormat;
		private System.Windows.Forms.ToolStripMenuItem mnuRowSigned1;
		private System.Windows.Forms.ToolStripMenuItem mnuRowSigned2;
		private System.Windows.Forms.ToolStripMenuItem mnuRowSigned3;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem6;
		private System.Windows.Forms.ToolStripMenuItem mnuRowHex1;
		private System.Windows.Forms.ToolStripMenuItem mnuRowHex2;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem7;
		private System.Windows.Forms.ToolStripMenuItem mnuRowUnsigned;
		private System.Windows.Forms.ToolStripMenuItem mnuRowBinary;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem8;
		private System.Windows.Forms.ToolStripMenuItem mnuRowHex3;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem9;
		private System.Windows.Forms.ToolStripMenuItem mnuRowClearFormat;
	}
}
