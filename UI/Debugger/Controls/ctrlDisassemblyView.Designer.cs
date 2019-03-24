namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlDisassemblyView
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
			this.ctrlCode = new Mesen.GUI.Debugger.Controls.ctrlScrollableTextbox();
			this.ctxMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuToggleBreakpoint = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuAddToWatch = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEditLabel = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEditInMemoryTools = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuGoToLocation = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFindOccurrences = new System.Windows.Forms.ToolStripMenuItem();
			this.ctxMenu.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlCode
			// 
			this.ctrlCode.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.ctrlCode.CodeHighlightingEnabled = true;
			this.ctrlCode.ContextMenuStrip = this.ctxMenu;
			this.ctrlCode.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlCode.HideSelection = false;
			this.ctrlCode.Location = new System.Drawing.Point(0, 0);
			this.ctrlCode.Name = "ctrlCode";
			this.ctrlCode.ShowCompactPrgAddresses = false;
			this.ctrlCode.ShowContentNotes = false;
			this.ctrlCode.ShowLineNumberNotes = false;
			this.ctrlCode.ShowMemoryValues = false;
			this.ctrlCode.ShowScrollbars = true;
			this.ctrlCode.ShowSingleContentLineNotes = true;
			this.ctrlCode.ShowSingleLineLineNumberNotes = false;
			this.ctrlCode.Size = new System.Drawing.Size(465, 398);
			this.ctrlCode.TabIndex = 0;
			this.ctrlCode.MouseDown += new System.Windows.Forms.MouseEventHandler(this.ctrlCode_MouseDown);
			// 
			// ctxMenu
			// 
			this.ctxMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuToggleBreakpoint,
            this.toolStripMenuItem1,
            this.mnuAddToWatch,
            this.mnuEditLabel,
            this.mnuEditInMemoryTools,
            this.toolStripMenuItem2,
            this.mnuGoToLocation,
            this.mnuFindOccurrences});
			this.ctxMenu.Name = "ctxMenu";
			this.ctxMenu.Size = new System.Drawing.Size(187, 148);
			// 
			// mnuToggleBreakpoint
			// 
			this.mnuToggleBreakpoint.Image = global::Mesen.GUI.Properties.Resources.Breakpoint;
			this.mnuToggleBreakpoint.Name = "mnuToggleBreakpoint";
			this.mnuToggleBreakpoint.Size = new System.Drawing.Size(186, 22);
			this.mnuToggleBreakpoint.Text = "Toggle Breakpoint";
			this.mnuToggleBreakpoint.Click += new System.EventHandler(this.mnuToggleBreakpoint_Click);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(183, 6);
			// 
			// mnuAddToWatch
			// 
			this.mnuAddToWatch.Enabled = false;
			this.mnuAddToWatch.Name = "mnuAddToWatch";
			this.mnuAddToWatch.Size = new System.Drawing.Size(186, 22);
			this.mnuAddToWatch.Text = "Add to Watch";
			// 
			// mnuEditLabel
			// 
			this.mnuEditLabel.Enabled = false;
			this.mnuEditLabel.Image = global::Mesen.GUI.Properties.Resources.EditLabel;
			this.mnuEditLabel.Name = "mnuEditLabel";
			this.mnuEditLabel.Size = new System.Drawing.Size(186, 22);
			this.mnuEditLabel.Text = "Edit Label";
			// 
			// mnuEditInMemoryTools
			// 
			this.mnuEditInMemoryTools.Enabled = false;
			this.mnuEditInMemoryTools.Image = global::Mesen.GUI.Properties.Resources.CheatCode;
			this.mnuEditInMemoryTools.Name = "mnuEditInMemoryTools";
			this.mnuEditInMemoryTools.Size = new System.Drawing.Size(186, 22);
			this.mnuEditInMemoryTools.Text = "Edit in Memory Tools";
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(183, 6);
			// 
			// mnuGoToLocation
			// 
			this.mnuGoToLocation.Enabled = false;
			this.mnuGoToLocation.Name = "mnuGoToLocation";
			this.mnuGoToLocation.Size = new System.Drawing.Size(186, 22);
			this.mnuGoToLocation.Text = "Go to Location";
			// 
			// mnuFindOccurrences
			// 
			this.mnuFindOccurrences.Enabled = false;
			this.mnuFindOccurrences.Image = global::Mesen.GUI.Properties.Resources.Find;
			this.mnuFindOccurrences.Name = "mnuFindOccurrences";
			this.mnuFindOccurrences.Size = new System.Drawing.Size(186, 22);
			this.mnuFindOccurrences.Text = "Find Occurrences";
			// 
			// ctrlDisassemblyView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.ctrlCode);
			this.Name = "ctrlDisassemblyView";
			this.Size = new System.Drawing.Size(465, 398);
			this.ctxMenu.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private ctrlScrollableTextbox ctrlCode;
		private System.Windows.Forms.ContextMenuStrip ctxMenu;
		private System.Windows.Forms.ToolStripMenuItem mnuAddToWatch;
		private System.Windows.Forms.ToolStripMenuItem mnuToggleBreakpoint;
		private System.Windows.Forms.ToolStripMenuItem mnuEditLabel;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuEditInMemoryTools;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuGoToLocation;
		private System.Windows.Forms.ToolStripMenuItem mnuFindOccurrences;
	}
}
