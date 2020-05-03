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
			this.mnuMarkSelectionAs = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuMarkAsCode = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuMarkAsData = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuMarkAsUnidentifiedData = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEditSelectedCode = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuCopySelection = new System.Windows.Forms.ToolStripMenuItem();
			this.sepMarkSelectionAs = new System.Windows.Forms.ToolStripSeparator();
			this.mnuToggleBreakpoint = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAddToWatch = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEditLabel = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEditInMemoryTools = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuGoToLocation = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFindOccurrences = new System.Windows.Forms.ToolStripMenuItem();
			this.sepSwitchView = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSwitchView = new System.Windows.Forms.ToolStripMenuItem();
			this.cboSourceFile = new System.Windows.Forms.ComboBox();
			this.lblSourceFile = new System.Windows.Forms.Label();
			this.tlpMain = new System.Windows.Forms.TableLayoutPanel();
			this.ctxMenu.SuspendLayout();
			this.tlpMain.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlCode
			// 
			this.ctrlCode.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.ctrlCode.CodeHighlightingEnabled = true;
			this.tlpMain.SetColumnSpan(this.ctrlCode, 2);
			this.ctrlCode.ContextMenuStrip = this.ctxMenu;
			this.ctrlCode.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlCode.HideSelection = false;
			this.ctrlCode.Location = new System.Drawing.Point(0, 27);
			this.ctrlCode.Margin = new System.Windows.Forms.Padding(0);
			this.ctrlCode.Name = "ctrlCode";
			this.ctrlCode.ShowCompactPrgAddresses = false;
			this.ctrlCode.ShowContentNotes = false;
			this.ctrlCode.ShowLineNumberNotes = false;
			this.ctrlCode.ShowMemoryValues = false;
			this.ctrlCode.ShowScrollbars = true;
			this.ctrlCode.ShowSingleContentLineNotes = true;
			this.ctrlCode.ShowSingleLineLineNumberNotes = false;
			this.ctrlCode.Size = new System.Drawing.Size(465, 371);
			this.ctrlCode.TabIndex = 0;
			this.ctrlCode.MouseMove += new System.Windows.Forms.MouseEventHandler(this.ctrlCode_MouseMove);
			this.ctrlCode.MouseDown += new System.Windows.Forms.MouseEventHandler(this.ctrlCode_MouseDown);
			this.ctrlCode.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.ctrlCode_MouseDoubleClick);
			this.ctrlCode.MouseLeave += new System.EventHandler(this.ctrlCode_MouseLeave);
			this.ctrlCode.TextZoomChanged += new System.EventHandler(this.ctrlCode_TextZoomChanged);
			// 
			// ctxMenu
			// 
			this.ctxMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuMarkSelectionAs,
            this.mnuEditSelectedCode,
            this.mnuCopySelection,
            this.sepMarkSelectionAs,
            this.mnuToggleBreakpoint,
            this.mnuAddToWatch,
            this.mnuEditLabel,
            this.mnuEditInMemoryTools,
            this.toolStripMenuItem2,
            this.mnuGoToLocation,
            this.mnuFindOccurrences,
            this.sepSwitchView,
            this.mnuSwitchView});
			this.ctxMenu.Name = "ctxMenu";
			this.ctxMenu.Size = new System.Drawing.Size(227, 264);
			this.ctxMenu.Closing += new System.Windows.Forms.ToolStripDropDownClosingEventHandler(this.ctxMenu_Closing);
			this.ctxMenu.Opening += new System.ComponentModel.CancelEventHandler(this.ctxMenu_Opening);
			// 
			// mnuMarkSelectionAs
			// 
			this.mnuMarkSelectionAs.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuMarkAsCode,
            this.mnuMarkAsData,
            this.mnuMarkAsUnidentifiedData});
			this.mnuMarkSelectionAs.Name = "mnuMarkSelectionAs";
			this.mnuMarkSelectionAs.Size = new System.Drawing.Size(226, 22);
			this.mnuMarkSelectionAs.Text = "Mark selection as...";
			// 
			// mnuMarkAsCode
			// 
			this.mnuMarkAsCode.Image = global::Mesen.GUI.Properties.Resources.Accept;
			this.mnuMarkAsCode.Name = "mnuMarkAsCode";
			this.mnuMarkAsCode.Size = new System.Drawing.Size(199, 22);
			this.mnuMarkAsCode.Text = "Verified Code";
			// 
			// mnuMarkAsData
			// 
			this.mnuMarkAsData.Image = global::Mesen.GUI.Properties.Resources.CheatCode;
			this.mnuMarkAsData.Name = "mnuMarkAsData";
			this.mnuMarkAsData.Size = new System.Drawing.Size(199, 22);
			this.mnuMarkAsData.Text = "Verified Data";
			// 
			// mnuMarkAsUnidentifiedData
			// 
			this.mnuMarkAsUnidentifiedData.Image = global::Mesen.GUI.Properties.Resources.Help;
			this.mnuMarkAsUnidentifiedData.Name = "mnuMarkAsUnidentifiedData";
			this.mnuMarkAsUnidentifiedData.Size = new System.Drawing.Size(199, 22);
			this.mnuMarkAsUnidentifiedData.Text = "Unidentified Code/Data";
			// 
			// mnuEditSelectedCode
			// 
			this.mnuEditSelectedCode.Image = global::Mesen.GUI.Properties.Resources.Edit;
			this.mnuEditSelectedCode.Name = "mnuEditSelectedCode";
			this.mnuEditSelectedCode.Size = new System.Drawing.Size(226, 22);
			this.mnuEditSelectedCode.Text = "Edit Selected Code";
			// 
			// mnuCopySelection
			// 
			this.mnuCopySelection.Image = global::Mesen.GUI.Properties.Resources.Copy;
			this.mnuCopySelection.Name = "mnuCopySelection";
			this.mnuCopySelection.Size = new System.Drawing.Size(226, 22);
			this.mnuCopySelection.Text = "Copy Selection";
			// 
			// sepMarkSelectionAs
			// 
			this.sepMarkSelectionAs.Name = "sepMarkSelectionAs";
			this.sepMarkSelectionAs.Size = new System.Drawing.Size(223, 6);
			// 
			// mnuToggleBreakpoint
			// 
			this.mnuToggleBreakpoint.Image = global::Mesen.GUI.Properties.Resources.Breakpoint;
			this.mnuToggleBreakpoint.Name = "mnuToggleBreakpoint";
			this.mnuToggleBreakpoint.Size = new System.Drawing.Size(226, 22);
			this.mnuToggleBreakpoint.Text = "Toggle Breakpoint";
			// 
			// mnuAddToWatch
			// 
			this.mnuAddToWatch.Image = global::Mesen.GUI.Properties.Resources.Add;
			this.mnuAddToWatch.Name = "mnuAddToWatch";
			this.mnuAddToWatch.Size = new System.Drawing.Size(226, 22);
			this.mnuAddToWatch.Text = "Add to Watch";
			this.mnuAddToWatch.Click += new System.EventHandler(this.mnuAddToWatch_Click);
			// 
			// mnuEditLabel
			// 
			this.mnuEditLabel.Image = global::Mesen.GUI.Properties.Resources.EditLabel;
			this.mnuEditLabel.Name = "mnuEditLabel";
			this.mnuEditLabel.Size = new System.Drawing.Size(226, 22);
			this.mnuEditLabel.Text = "Edit Label";
			// 
			// mnuEditInMemoryTools
			// 
			this.mnuEditInMemoryTools.Image = global::Mesen.GUI.Properties.Resources.CheatCode;
			this.mnuEditInMemoryTools.Name = "mnuEditInMemoryTools";
			this.mnuEditInMemoryTools.Size = new System.Drawing.Size(226, 22);
			this.mnuEditInMemoryTools.Text = "Edit in Memory Tools";
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(223, 6);
			// 
			// mnuGoToLocation
			// 
			this.mnuGoToLocation.Name = "mnuGoToLocation";
			this.mnuGoToLocation.ShortcutKeyDisplayString = "Double Click";
			this.mnuGoToLocation.Size = new System.Drawing.Size(226, 22);
			this.mnuGoToLocation.Text = "Go to Location";
			this.mnuGoToLocation.Click += new System.EventHandler(this.mnuGoToLocation_Click);
			// 
			// mnuFindOccurrences
			// 
			this.mnuFindOccurrences.Enabled = false;
			this.mnuFindOccurrences.Image = global::Mesen.GUI.Properties.Resources.Find;
			this.mnuFindOccurrences.Name = "mnuFindOccurrences";
			this.mnuFindOccurrences.Size = new System.Drawing.Size(226, 22);
			this.mnuFindOccurrences.Text = "Find Occurrences";
			this.mnuFindOccurrences.Visible = false;
			// 
			// sepSwitchView
			// 
			this.sepSwitchView.Name = "sepSwitchView";
			this.sepSwitchView.Size = new System.Drawing.Size(223, 6);
			// 
			// mnuSwitchView
			// 
			this.mnuSwitchView.Image = global::Mesen.GUI.Properties.Resources.SwitchView;
			this.mnuSwitchView.Name = "mnuSwitchView";
			this.mnuSwitchView.Size = new System.Drawing.Size(226, 22);
			this.mnuSwitchView.Text = "Switch to Source View";
			// 
			// cboSourceFile
			// 
			this.cboSourceFile.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboSourceFile.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cboSourceFile.FormattingEnabled = true;
			this.cboSourceFile.Location = new System.Drawing.Point(35, 3);
			this.cboSourceFile.Margin = new System.Windows.Forms.Padding(3, 3, 0, 3);
			this.cboSourceFile.Name = "cboSourceFile";
			this.cboSourceFile.Size = new System.Drawing.Size(430, 21);
			this.cboSourceFile.TabIndex = 1;
			this.cboSourceFile.SelectedIndexChanged += new System.EventHandler(this.cboSourceFile_SelectedIndexChanged);
			// 
			// lblSourceFile
			// 
			this.lblSourceFile.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblSourceFile.AutoSize = true;
			this.lblSourceFile.Location = new System.Drawing.Point(3, 7);
			this.lblSourceFile.Name = "lblSourceFile";
			this.lblSourceFile.Size = new System.Drawing.Size(26, 13);
			this.lblSourceFile.TabIndex = 2;
			this.lblSourceFile.Text = "File:";
			// 
			// tlpMain
			// 
			this.tlpMain.ColumnCount = 2;
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpMain.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpMain.Controls.Add(this.lblSourceFile, 0, 0);
			this.tlpMain.Controls.Add(this.cboSourceFile, 1, 0);
			this.tlpMain.Controls.Add(this.ctrlCode, 0, 1);
			this.tlpMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpMain.Location = new System.Drawing.Point(0, 0);
			this.tlpMain.Name = "tlpMain";
			this.tlpMain.RowCount = 2;
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpMain.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpMain.Size = new System.Drawing.Size(465, 398);
			this.tlpMain.TabIndex = 3;
			// 
			// ctrlDisassemblyView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.tlpMain);
			this.Name = "ctrlDisassemblyView";
			this.Size = new System.Drawing.Size(465, 398);
			this.ctxMenu.ResumeLayout(false);
			this.tlpMain.ResumeLayout(false);
			this.tlpMain.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private ctrlScrollableTextbox ctrlCode;
		private System.Windows.Forms.ContextMenuStrip ctxMenu;
		private System.Windows.Forms.ToolStripMenuItem mnuAddToWatch;
		private System.Windows.Forms.ToolStripMenuItem mnuToggleBreakpoint;
		private System.Windows.Forms.ToolStripMenuItem mnuEditLabel;
		private System.Windows.Forms.ToolStripSeparator sepMarkSelectionAs;
		private System.Windows.Forms.ToolStripMenuItem mnuEditInMemoryTools;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuGoToLocation;
		private System.Windows.Forms.ToolStripMenuItem mnuFindOccurrences;
		private System.Windows.Forms.ComboBox cboSourceFile;
		private System.Windows.Forms.ToolStripSeparator sepSwitchView;
		private System.Windows.Forms.ToolStripMenuItem mnuSwitchView;
		private System.Windows.Forms.TableLayoutPanel tlpMain;
		private System.Windows.Forms.Label lblSourceFile;
	  private System.Windows.Forms.ToolStripMenuItem mnuMarkSelectionAs;
	  private System.Windows.Forms.ToolStripMenuItem mnuMarkAsCode;
	  private System.Windows.Forms.ToolStripMenuItem mnuMarkAsData;
	  private System.Windows.Forms.ToolStripMenuItem mnuMarkAsUnidentifiedData;
	  private System.Windows.Forms.ToolStripMenuItem mnuEditSelectedCode;
	  private System.Windows.Forms.ToolStripMenuItem mnuCopySelection;
   }
}
