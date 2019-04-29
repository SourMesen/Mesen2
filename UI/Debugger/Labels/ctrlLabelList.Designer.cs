namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlLabelList
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
			this.contextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuAdd = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEdit = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuDelete = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuViewInCpuMemory = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuViewInMemoryType = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuAddBreakpoint = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAddToWatch = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFindOccurrences = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuShowComments = new System.Windows.Forms.ToolStripMenuItem();
			this.lstLabels = new Mesen.GUI.Controls.DoubleBufferedListView();
			this.colFunctionLabel = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colFunctionAddress = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colMemoryAddress = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colComment = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.contextMenu.SuspendLayout();
			this.SuspendLayout();
			// 
			// contextMenu
			// 
			this.contextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuAdd,
            this.mnuEdit,
            this.mnuDelete,
            this.toolStripMenuItem1,
            this.mnuViewInCpuMemory,
            this.mnuViewInMemoryType,
            this.toolStripMenuItem3,
            this.mnuAddBreakpoint,
            this.mnuAddToWatch,
            this.mnuFindOccurrences,
            this.toolStripMenuItem2,
            this.mnuShowComments});
			this.contextMenu.Name = "contextMenu";
			this.contextMenu.Size = new System.Drawing.Size(187, 220);
			// 
			// mnuAdd
			// 
			this.mnuAdd.Image = global::Mesen.GUI.Properties.Resources.Add;
			this.mnuAdd.Name = "mnuAdd";
			this.mnuAdd.Size = new System.Drawing.Size(186, 22);
			this.mnuAdd.Text = "Add";
			this.mnuAdd.Click += new System.EventHandler(this.mnuAdd_Click);
			// 
			// mnuEdit
			// 
			this.mnuEdit.Image = global::Mesen.GUI.Properties.Resources.EditLabel;
			this.mnuEdit.Name = "mnuEdit";
			this.mnuEdit.Size = new System.Drawing.Size(186, 22);
			this.mnuEdit.Text = "Edit";
			this.mnuEdit.Click += new System.EventHandler(this.mnuEdit_Click);
			// 
			// mnuDelete
			// 
			this.mnuDelete.Image = global::Mesen.GUI.Properties.Resources.Close;
			this.mnuDelete.Name = "mnuDelete";
			this.mnuDelete.Size = new System.Drawing.Size(186, 22);
			this.mnuDelete.Text = "Delete";
			this.mnuDelete.Click += new System.EventHandler(this.mnuDelete_Click);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(183, 6);
			// 
			// mnuViewInCpuMemory
			// 
			this.mnuViewInCpuMemory.Image = global::Mesen.GUI.Properties.Resources.Chip;
			this.mnuViewInCpuMemory.Name = "mnuViewInCpuMemory";
			this.mnuViewInCpuMemory.Size = new System.Drawing.Size(186, 22);
			this.mnuViewInCpuMemory.Text = "View in CPU memory";
			this.mnuViewInCpuMemory.Click += new System.EventHandler(this.mnuViewInCpuMemory_Click);
			// 
			// mnuViewInMemoryType
			// 
			this.mnuViewInMemoryType.Image = global::Mesen.GUI.Properties.Resources.CheatCode;
			this.mnuViewInMemoryType.Name = "mnuViewInMemoryType";
			this.mnuViewInMemoryType.Size = new System.Drawing.Size(186, 22);
			this.mnuViewInMemoryType.Text = "View in {0} memory";
			this.mnuViewInMemoryType.Click += new System.EventHandler(this.mnuViewInMemoryType_Click);
			// 
			// toolStripMenuItem3
			// 
			this.toolStripMenuItem3.Name = "toolStripMenuItem3";
			this.toolStripMenuItem3.Size = new System.Drawing.Size(183, 6);
			// 
			// mnuAddBreakpoint
			// 
			this.mnuAddBreakpoint.Image = global::Mesen.GUI.Properties.Resources.Breakpoint;
			this.mnuAddBreakpoint.Name = "mnuAddBreakpoint";
			this.mnuAddBreakpoint.Size = new System.Drawing.Size(186, 22);
			this.mnuAddBreakpoint.Text = "Add breakpoint";
			this.mnuAddBreakpoint.Click += new System.EventHandler(this.mnuAddBreakpoint_Click);
			// 
			// mnuAddToWatch
			// 
			this.mnuAddToWatch.Image = global::Mesen.GUI.Properties.Resources.Add;
			this.mnuAddToWatch.Name = "mnuAddToWatch";
			this.mnuAddToWatch.Size = new System.Drawing.Size(186, 22);
			this.mnuAddToWatch.Text = "Add to watch";
			this.mnuAddToWatch.Click += new System.EventHandler(this.mnuAddToWatch_Click);
			// 
			// mnuFindOccurrences
			// 
			this.mnuFindOccurrences.Image = global::Mesen.GUI.Properties.Resources.Find;
			this.mnuFindOccurrences.Name = "mnuFindOccurrences";
			this.mnuFindOccurrences.Size = new System.Drawing.Size(186, 22);
			this.mnuFindOccurrences.Text = "Find Occurrences";
			this.mnuFindOccurrences.Visible = false;
			this.mnuFindOccurrences.Click += new System.EventHandler(this.mnuFindOccurrences_Click);
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(183, 6);
			// 
			// mnuShowComments
			// 
			this.mnuShowComments.CheckOnClick = true;
			this.mnuShowComments.Name = "mnuShowComments";
			this.mnuShowComments.Size = new System.Drawing.Size(186, 22);
			this.mnuShowComments.Text = "Show Comments";
			this.mnuShowComments.Click += new System.EventHandler(this.mnuShowComments_Click);
			// 
			// lstLabels
			// 
			this.lstLabels.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colFunctionLabel,
            this.colFunctionAddress,
            this.colMemoryAddress,
            this.colComment});
			this.lstLabels.ContextMenuStrip = this.contextMenu;
			this.lstLabels.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstLabels.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lstLabels.FullRowSelect = true;
			this.lstLabels.GridLines = true;
			this.lstLabels.HideSelection = false;
			this.lstLabels.Location = new System.Drawing.Point(0, 0);
			this.lstLabels.Name = "lstLabels";
			this.lstLabels.Size = new System.Drawing.Size(341, 112);
			this.lstLabels.TabIndex = 2;
			this.lstLabels.UseCompatibleStateImageBehavior = false;
			this.lstLabels.View = System.Windows.Forms.View.Details;
			this.lstLabels.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.lstLabels_ColumnClick);
			this.lstLabels.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstLabels_RetrieveVirtualItem);
			this.lstLabels.SearchForVirtualItem += new System.Windows.Forms.SearchForVirtualItemEventHandler(this.lstLabels_SearchForVirtualItem);
			this.lstLabels.SelectedIndexChanged += new System.EventHandler(this.lstLabels_SelectedIndexChanged);
			this.lstLabels.DoubleClick += new System.EventHandler(this.lstLabels_DoubleClick);
			// 
			// colFunctionLabel
			// 
			this.colFunctionLabel.Text = "Label";
			this.colFunctionLabel.Width = 133;
			// 
			// colFunctionAddress
			// 
			this.colFunctionAddress.Text = "CPU Addr";
			// 
			// colMemoryAddress
			// 
			this.colMemoryAddress.Text = "ROM Addr";
			this.colMemoryAddress.Width = 84;
			// 
			// colComment
			// 
			this.colComment.Text = "Comment";
			// 
			// ctrlLabelList
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.lstLabels);
			this.Name = "ctrlLabelList";
			this.Size = new System.Drawing.Size(341, 112);
			this.contextMenu.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private Mesen.GUI.Controls.DoubleBufferedListView lstLabels;
		private System.Windows.Forms.ColumnHeader colFunctionLabel;
		private System.Windows.Forms.ColumnHeader colFunctionAddress;
		private System.Windows.Forms.ColumnHeader colMemoryAddress;
		private System.Windows.Forms.ContextMenuStrip contextMenu;
		private System.Windows.Forms.ToolStripMenuItem mnuDelete;
		private System.Windows.Forms.ToolStripMenuItem mnuAdd;
		private System.Windows.Forms.ToolStripMenuItem mnuEdit;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuFindOccurrences;
		private System.Windows.Forms.ToolStripMenuItem mnuAddBreakpoint;
		private System.Windows.Forms.ToolStripMenuItem mnuAddToWatch;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ColumnHeader colComment;
		private System.Windows.Forms.ToolStripMenuItem mnuShowComments;
		private System.Windows.Forms.ToolStripMenuItem mnuViewInCpuMemory;
		private System.Windows.Forms.ToolStripMenuItem mnuViewInMemoryType;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
	}
}
