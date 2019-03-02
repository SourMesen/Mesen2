namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlBreakpoints
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
			BreakpointManager.BreakpointsChanged -= BreakpointManager_OnBreakpointChanged;
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
			this.mnuBreakpoints = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuAddBreakpoint = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEditBreakpoint = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRemoveBreakpoint = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuGoToLocation = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuShowLabels = new System.Windows.Forms.ToolStripMenuItem();
			this.lstBreakpoints = new Mesen.GUI.Controls.MyListView();
			this.colEnabled = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colMarker = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colType = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colAddress = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colCondition = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.mnuBreakpoints.SuspendLayout();
			this.SuspendLayout();
			// 
			// mnuBreakpoints
			// 
			this.mnuBreakpoints.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuAddBreakpoint,
            this.mnuEditBreakpoint,
            this.mnuRemoveBreakpoint,
            this.toolStripMenuItem1,
            this.mnuGoToLocation,
            this.toolStripMenuItem2,
            this.mnuShowLabels});
			this.mnuBreakpoints.Name = "mnuBreakpoints";
			this.mnuBreakpoints.Size = new System.Drawing.Size(150, 126);
			// 
			// mnuAddBreakpoint
			// 
			this.mnuAddBreakpoint.Image = global::Mesen.GUI.Properties.Resources.Add;
			this.mnuAddBreakpoint.Name = "mnuAddBreakpoint";
			this.mnuAddBreakpoint.Size = new System.Drawing.Size(149, 22);
			this.mnuAddBreakpoint.Text = "Add...";
			this.mnuAddBreakpoint.Click += new System.EventHandler(this.mnuAddBreakpoint_Click);
			// 
			// mnuEditBreakpoint
			// 
			this.mnuEditBreakpoint.Image = global::Mesen.GUI.Properties.Resources.Edit;
			this.mnuEditBreakpoint.Name = "mnuEditBreakpoint";
			this.mnuEditBreakpoint.Size = new System.Drawing.Size(149, 22);
			this.mnuEditBreakpoint.Text = "Edit";
			this.mnuEditBreakpoint.Click += new System.EventHandler(this.mnuEditBreakpoint_Click);
			// 
			// mnuRemoveBreakpoint
			// 
			this.mnuRemoveBreakpoint.Image = global::Mesen.GUI.Properties.Resources.Close;
			this.mnuRemoveBreakpoint.Name = "mnuRemoveBreakpoint";
			this.mnuRemoveBreakpoint.Size = new System.Drawing.Size(149, 22);
			this.mnuRemoveBreakpoint.Text = "Remove";
			this.mnuRemoveBreakpoint.Click += new System.EventHandler(this.mnuRemoveBreakpoint_Click);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(146, 6);
			// 
			// mnuGoToLocation
			// 
			this.mnuGoToLocation.Name = "mnuGoToLocation";
			this.mnuGoToLocation.Size = new System.Drawing.Size(149, 22);
			this.mnuGoToLocation.Text = "Go to location";
			this.mnuGoToLocation.Click += new System.EventHandler(this.mnuGoToLocation_Click);
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(146, 6);
			// 
			// mnuShowLabels
			// 
			this.mnuShowLabels.CheckOnClick = true;
			this.mnuShowLabels.Name = "mnuShowLabels";
			this.mnuShowLabels.Size = new System.Drawing.Size(149, 22);
			this.mnuShowLabels.Text = "Show Labels";
			// 
			// lstBreakpoints
			// 
			this.lstBreakpoints.CheckBoxes = true;
			this.lstBreakpoints.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colEnabled,
            this.colMarker,
            this.colType,
            this.colAddress,
            this.colCondition});
			this.lstBreakpoints.ContextMenuStrip = this.mnuBreakpoints;
			this.lstBreakpoints.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstBreakpoints.FullRowSelect = true;
			this.lstBreakpoints.GridLines = true;
			this.lstBreakpoints.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
			this.lstBreakpoints.Location = new System.Drawing.Point(0, 0);
			this.lstBreakpoints.Name = "lstBreakpoints";
			this.lstBreakpoints.Size = new System.Drawing.Size(393, 101);
			this.lstBreakpoints.TabIndex = 7;
			this.lstBreakpoints.UseCompatibleStateImageBehavior = false;
			this.lstBreakpoints.View = System.Windows.Forms.View.Details;
			this.lstBreakpoints.SelectedIndexChanged += new System.EventHandler(this.lstBreakpoints_SelectedIndexChanged);
			this.lstBreakpoints.DoubleClick += new System.EventHandler(this.lstBreakpoints_DoubleClick);
			this.lstBreakpoints.MouseDown += new System.Windows.Forms.MouseEventHandler(this.lstBreakpoints_MouseDown);
			// 
			// colEnabled
			// 
			this.colEnabled.Text = "";
			this.colEnabled.Width = 21;
			// 
			// colMarker
			// 
			this.colMarker.Text = "M";
			this.colMarker.Width = 25;
			// 
			// colType
			// 
			this.colType.Text = "Type";
			this.colType.Width = 87;
			// 
			// colAddress
			// 
			this.colAddress.Text = "Address";
			this.colAddress.Width = 108;
			// 
			// colCondition
			// 
			this.colCondition.Text = "Condition";
			this.colCondition.Width = 142;
			// 
			// ctrlBreakpoints
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.lstBreakpoints);
			this.Name = "ctrlBreakpoints";
			this.Size = new System.Drawing.Size(393, 101);
			this.mnuBreakpoints.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private Mesen.GUI.Controls.MyListView lstBreakpoints;
		private System.Windows.Forms.ColumnHeader colType;
		private System.Windows.Forms.ColumnHeader colAddress;
		private System.Windows.Forms.ContextMenuStrip mnuBreakpoints;
		private System.Windows.Forms.ToolStripMenuItem mnuAddBreakpoint;
		private System.Windows.Forms.ToolStripMenuItem mnuRemoveBreakpoint;
		private System.Windows.Forms.ColumnHeader colEnabled;
		private System.Windows.Forms.ColumnHeader colCondition;
		private System.Windows.Forms.ToolStripMenuItem mnuGoToLocation;
		private System.Windows.Forms.ToolStripMenuItem mnuEditBreakpoint;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuShowLabels;
		private System.Windows.Forms.ColumnHeader colMarker;
	}
}
