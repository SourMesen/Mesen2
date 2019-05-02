namespace Mesen.GUI.Debugger
{
	partial class frmMemoryTools
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
			if(this._notifListener != null) {
				this._notifListener.Dispose();
				this._notifListener = null;
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
			this.ctrlHexViewer = new Mesen.GUI.Debugger.Controls.ctrlHexViewer();
			this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
			this.lblViewMemoryType = new System.Windows.Forms.Label();
			this.cboMemoryType = new Mesen.GUI.Debugger.Controls.ComboBoxWithSeparator();
			this.menuStrip1 = new Mesen.GUI.Controls.ctrlMesenMenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuImport = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuExport = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuLoadTblFile = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuResetTblMappings = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuClose = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuView = new System.Windows.Forms.ToolStripMenuItem();
			this.highlightToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHighlightExecution = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHighlightWrites = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHightlightReads = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem6 = new System.Windows.Forms.ToolStripSeparator();
			this.fadeSpeedToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFadeSlow = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFadeNormal = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFadeFast = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFadeNever = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem7 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuCustomFadeSpeed = new System.Windows.Forms.ToolStripMenuItem();
			this.dataTypeHighlightingToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHighlightLabelledBytes = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHighlightBreakpoints = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem8 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuHighlightCodeBytes = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHighlightDataBytes = new System.Windows.Forms.ToolStripMenuItem();
			this.fadeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHideUnusedBytes = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHideReadBytes = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHideWrittenBytes = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHideExecutedBytes = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuConfigureColors = new System.Windows.Forms.ToolStripMenuItem();
			this.fontSizeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuIncreaseFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuDecreaseFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuResetFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem12 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSelectFont = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem13 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuHighDensityMode = new System.Windows.Forms.ToolStripMenuItem();
			this.autorefreshSpeedToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefreshLow = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefreshNormal = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRefreshHigh = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRefresh = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem9 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuAutoRefresh = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuShowCharacters = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuShowLabelInfoOnMouseOver = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem10 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuIgnoreRedundantWrites = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEnablePerByteNavigation = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuByteEditingMode = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHighlightCurrentRowColumn = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGoToAll = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGoTo = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem14 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuFind = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFindNext = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuFindPrev = new System.Windows.Forms.ToolStripMenuItem();
			this.tabMain = new System.Windows.Forms.TabControl();
			this.tpgMemoryViewer = new System.Windows.Forms.TabPage();
			this.panel1 = new System.Windows.Forms.Panel();
			this.tpgAccessCounters = new System.Windows.Forms.TabPage();
			this.flowLayoutPanel1.SuspendLayout();
			this.menuStrip1.SuspendLayout();
			this.tabMain.SuspendLayout();
			this.tpgMemoryViewer.SuspendLayout();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlHexViewer
			// 
			this.ctrlHexViewer.BaseFont = new System.Drawing.Font("Consolas", 10F);
			this.ctrlHexViewer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlHexViewer.Location = new System.Drawing.Point(0, 0);
			this.ctrlHexViewer.Margin = new System.Windows.Forms.Padding(0);
			this.ctrlHexViewer.Name = "ctrlHexViewer";
			this.ctrlHexViewer.Size = new System.Drawing.Size(606, 343);
			this.ctrlHexViewer.TabIndex = 0;
			this.ctrlHexViewer.TextZoom = 100;
			this.ctrlHexViewer.RequiredWidthChanged += new System.EventHandler(this.ctrlHexViewer_RequiredWidthChanged);
			this.ctrlHexViewer.ByteMouseHover += new Mesen.GUI.Debugger.Controls.ctrlHexViewer.ByteMouseHoverHandler(this.ctrlHexViewer_ByteMouseHover);
			// 
			// flowLayoutPanel1
			// 
			this.flowLayoutPanel1.AutoSize = true;
			this.flowLayoutPanel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.flowLayoutPanel1.Controls.Add(this.lblViewMemoryType);
			this.flowLayoutPanel1.Controls.Add(this.cboMemoryType);
			this.flowLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.flowLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
			this.flowLayoutPanel1.Name = "flowLayoutPanel1";
			this.flowLayoutPanel1.Size = new System.Drawing.Size(207, 27);
			this.flowLayoutPanel1.TabIndex = 1;
			this.flowLayoutPanel1.WrapContents = false;
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
			// 
			// menuStrip1
			// 
			this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.mnuView,
            this.toolStripMenuItem1});
			this.menuStrip1.Location = new System.Drawing.Point(0, 0);
			this.menuStrip1.Name = "menuStrip1";
			this.menuStrip1.Size = new System.Drawing.Size(614, 24);
			this.menuStrip1.TabIndex = 2;
			this.menuStrip1.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuImport,
            this.mnuExport,
            this.toolStripMenuItem3,
            this.mnuLoadTblFile,
            this.mnuResetTblMappings,
            this.toolStripMenuItem4,
            this.mnuClose});
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
			this.fileToolStripMenuItem.Text = "File";
			// 
			// mnuImport
			// 
			this.mnuImport.Image = global::Mesen.GUI.Properties.Resources.Import;
			this.mnuImport.Name = "mnuImport";
			this.mnuImport.Size = new System.Drawing.Size(181, 22);
			this.mnuImport.Text = "Import";
			this.mnuImport.Click += new System.EventHandler(this.mnuImport_Click);
			// 
			// mnuExport
			// 
			this.mnuExport.Image = global::Mesen.GUI.Properties.Resources.Export;
			this.mnuExport.Name = "mnuExport";
			this.mnuExport.Size = new System.Drawing.Size(181, 22);
			this.mnuExport.Text = "Export";
			this.mnuExport.Click += new System.EventHandler(this.mnuExport_Click);
			// 
			// toolStripMenuItem3
			// 
			this.toolStripMenuItem3.Name = "toolStripMenuItem3";
			this.toolStripMenuItem3.Size = new System.Drawing.Size(178, 6);
			// 
			// mnuLoadTblFile
			// 
			this.mnuLoadTblFile.Name = "mnuLoadTblFile";
			this.mnuLoadTblFile.Size = new System.Drawing.Size(181, 22);
			this.mnuLoadTblFile.Text = "Load TBL file";
			this.mnuLoadTblFile.Click += new System.EventHandler(this.mnuLoadTblFile_Click);
			// 
			// mnuResetTblMappings
			// 
			this.mnuResetTblMappings.Name = "mnuResetTblMappings";
			this.mnuResetTblMappings.Size = new System.Drawing.Size(181, 22);
			this.mnuResetTblMappings.Text = "Reset TBL mappings";
			this.mnuResetTblMappings.Click += new System.EventHandler(this.mnuResetTblMappings_Click);
			// 
			// toolStripMenuItem4
			// 
			this.toolStripMenuItem4.Name = "toolStripMenuItem4";
			this.toolStripMenuItem4.Size = new System.Drawing.Size(178, 6);
			// 
			// mnuClose
			// 
			this.mnuClose.Image = global::Mesen.GUI.Properties.Resources.Exit;
			this.mnuClose.Name = "mnuClose";
			this.mnuClose.Size = new System.Drawing.Size(181, 22);
			this.mnuClose.Text = "Close";
			this.mnuClose.Click += new System.EventHandler(this.mnuClose_Click);
			// 
			// mnuView
			// 
			this.mnuView.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.highlightToolStripMenuItem,
            this.dataTypeHighlightingToolStripMenuItem,
            this.fadeToolStripMenuItem,
            this.toolStripMenuItem5,
            this.mnuConfigureColors,
            this.fontSizeToolStripMenuItem,
            this.autorefreshSpeedToolStripMenuItem,
            this.toolStripMenuItem2,
            this.mnuRefresh,
            this.toolStripMenuItem9,
            this.mnuAutoRefresh,
            this.mnuShowCharacters,
            this.mnuShowLabelInfoOnMouseOver,
            this.toolStripMenuItem10,
            this.mnuIgnoreRedundantWrites,
            this.mnuEnablePerByteNavigation,
            this.mnuByteEditingMode,
            this.mnuHighlightCurrentRowColumn});
			this.mnuView.Name = "mnuView";
			this.mnuView.Size = new System.Drawing.Size(44, 20);
			this.mnuView.Text = "View";
			// 
			// highlightToolStripMenuItem
			// 
			this.highlightToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuHighlightExecution,
            this.mnuHighlightWrites,
            this.mnuHightlightReads,
            this.toolStripMenuItem6,
            this.fadeSpeedToolStripMenuItem});
			this.highlightToolStripMenuItem.Name = "highlightToolStripMenuItem";
			this.highlightToolStripMenuItem.Size = new System.Drawing.Size(256, 22);
			this.highlightToolStripMenuItem.Text = "Memory Access Highlighting";
			// 
			// mnuHighlightExecution
			// 
			this.mnuHighlightExecution.CheckOnClick = true;
			this.mnuHighlightExecution.Name = "mnuHighlightExecution";
			this.mnuHighlightExecution.Size = new System.Drawing.Size(152, 22);
			this.mnuHighlightExecution.Text = "Execution";
			this.mnuHighlightExecution.Click += new System.EventHandler(this.mnuColorProviderOptions_Click);
			// 
			// mnuHighlightWrites
			// 
			this.mnuHighlightWrites.CheckOnClick = true;
			this.mnuHighlightWrites.Name = "mnuHighlightWrites";
			this.mnuHighlightWrites.Size = new System.Drawing.Size(152, 22);
			this.mnuHighlightWrites.Text = "Writes";
			this.mnuHighlightWrites.Click += new System.EventHandler(this.mnuColorProviderOptions_Click);
			// 
			// mnuHightlightReads
			// 
			this.mnuHightlightReads.CheckOnClick = true;
			this.mnuHightlightReads.Name = "mnuHightlightReads";
			this.mnuHightlightReads.Size = new System.Drawing.Size(152, 22);
			this.mnuHightlightReads.Text = "Reads";
			this.mnuHightlightReads.Click += new System.EventHandler(this.mnuColorProviderOptions_Click);
			// 
			// toolStripMenuItem6
			// 
			this.toolStripMenuItem6.Name = "toolStripMenuItem6";
			this.toolStripMenuItem6.Size = new System.Drawing.Size(149, 6);
			// 
			// fadeSpeedToolStripMenuItem
			// 
			this.fadeSpeedToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFadeSlow,
            this.mnuFadeNormal,
            this.mnuFadeFast,
            this.mnuFadeNever,
            this.toolStripMenuItem7,
            this.mnuCustomFadeSpeed});
			this.fadeSpeedToolStripMenuItem.Name = "fadeSpeedToolStripMenuItem";
			this.fadeSpeedToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
			this.fadeSpeedToolStripMenuItem.Text = "Fade speed";
			// 
			// mnuFadeSlow
			// 
			this.mnuFadeSlow.Name = "mnuFadeSlow";
			this.mnuFadeSlow.Size = new System.Drawing.Size(136, 22);
			this.mnuFadeSlow.Text = "Slow";
			this.mnuFadeSlow.Click += new System.EventHandler(this.mnuFadeSpeed_Click);
			// 
			// mnuFadeNormal
			// 
			this.mnuFadeNormal.Checked = true;
			this.mnuFadeNormal.CheckState = System.Windows.Forms.CheckState.Checked;
			this.mnuFadeNormal.Name = "mnuFadeNormal";
			this.mnuFadeNormal.Size = new System.Drawing.Size(136, 22);
			this.mnuFadeNormal.Text = "Normal";
			this.mnuFadeNormal.Click += new System.EventHandler(this.mnuFadeSpeed_Click);
			// 
			// mnuFadeFast
			// 
			this.mnuFadeFast.Name = "mnuFadeFast";
			this.mnuFadeFast.Size = new System.Drawing.Size(136, 22);
			this.mnuFadeFast.Text = "Fast";
			this.mnuFadeFast.Click += new System.EventHandler(this.mnuFadeSpeed_Click);
			// 
			// mnuFadeNever
			// 
			this.mnuFadeNever.Name = "mnuFadeNever";
			this.mnuFadeNever.Size = new System.Drawing.Size(136, 22);
			this.mnuFadeNever.Text = "Do not fade";
			this.mnuFadeNever.Click += new System.EventHandler(this.mnuFadeSpeed_Click);
			// 
			// toolStripMenuItem7
			// 
			this.toolStripMenuItem7.Name = "toolStripMenuItem7";
			this.toolStripMenuItem7.Size = new System.Drawing.Size(133, 6);
			// 
			// mnuCustomFadeSpeed
			// 
			this.mnuCustomFadeSpeed.Name = "mnuCustomFadeSpeed";
			this.mnuCustomFadeSpeed.Size = new System.Drawing.Size(136, 22);
			this.mnuCustomFadeSpeed.Text = "Custom...";
			this.mnuCustomFadeSpeed.Click += new System.EventHandler(this.mnuCustomFadeSpeed_Click);
			// 
			// dataTypeHighlightingToolStripMenuItem
			// 
			this.dataTypeHighlightingToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuHighlightLabelledBytes,
            this.mnuHighlightBreakpoints,
            this.toolStripMenuItem8,
            this.mnuHighlightCodeBytes,
            this.mnuHighlightDataBytes});
			this.dataTypeHighlightingToolStripMenuItem.Name = "dataTypeHighlightingToolStripMenuItem";
			this.dataTypeHighlightingToolStripMenuItem.Size = new System.Drawing.Size(256, 22);
			this.dataTypeHighlightingToolStripMenuItem.Text = "Data Type Highlighting";
			// 
			// mnuHighlightLabelledBytes
			// 
			this.mnuHighlightLabelledBytes.CheckOnClick = true;
			this.mnuHighlightLabelledBytes.Name = "mnuHighlightLabelledBytes";
			this.mnuHighlightLabelledBytes.Size = new System.Drawing.Size(196, 22);
			this.mnuHighlightLabelledBytes.Text = "Labeled bytes";
			this.mnuHighlightLabelledBytes.Click += new System.EventHandler(this.mnuColorProviderOptions_Click);
			// 
			// mnuHighlightBreakpoints
			// 
			this.mnuHighlightBreakpoints.CheckOnClick = true;
			this.mnuHighlightBreakpoints.Name = "mnuHighlightBreakpoints";
			this.mnuHighlightBreakpoints.Size = new System.Drawing.Size(196, 22);
			this.mnuHighlightBreakpoints.Text = "Breakpoints";
			this.mnuHighlightBreakpoints.Click += new System.EventHandler(this.mnuColorProviderOptions_Click);
			// 
			// toolStripMenuItem8
			// 
			this.toolStripMenuItem8.Name = "toolStripMenuItem8";
			this.toolStripMenuItem8.Size = new System.Drawing.Size(193, 6);
			// 
			// mnuHighlightCodeBytes
			// 
			this.mnuHighlightCodeBytes.CheckOnClick = true;
			this.mnuHighlightCodeBytes.Name = "mnuHighlightCodeBytes";
			this.mnuHighlightCodeBytes.Size = new System.Drawing.Size(196, 22);
			this.mnuHighlightCodeBytes.Text = "Code bytes (PRG ROM)";
			this.mnuHighlightCodeBytes.Click += new System.EventHandler(this.mnuColorProviderOptions_Click);
			// 
			// mnuHighlightDataBytes
			// 
			this.mnuHighlightDataBytes.CheckOnClick = true;
			this.mnuHighlightDataBytes.Name = "mnuHighlightDataBytes";
			this.mnuHighlightDataBytes.Size = new System.Drawing.Size(196, 22);
			this.mnuHighlightDataBytes.Text = "Data bytes (PRG ROM)";
			this.mnuHighlightDataBytes.Click += new System.EventHandler(this.mnuColorProviderOptions_Click);
			// 
			// fadeToolStripMenuItem
			// 
			this.fadeToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuHideUnusedBytes,
            this.mnuHideReadBytes,
            this.mnuHideWrittenBytes,
            this.mnuHideExecutedBytes});
			this.fadeToolStripMenuItem.Name = "fadeToolStripMenuItem";
			this.fadeToolStripMenuItem.Size = new System.Drawing.Size(256, 22);
			this.fadeToolStripMenuItem.Text = "De-emphasize";
			// 
			// mnuHideUnusedBytes
			// 
			this.mnuHideUnusedBytes.CheckOnClick = true;
			this.mnuHideUnusedBytes.Name = "mnuHideUnusedBytes";
			this.mnuHideUnusedBytes.Size = new System.Drawing.Size(152, 22);
			this.mnuHideUnusedBytes.Text = "Unused bytes";
			this.mnuHideUnusedBytes.Click += new System.EventHandler(this.mnuColorProviderOptions_Click);
			// 
			// mnuHideReadBytes
			// 
			this.mnuHideReadBytes.CheckOnClick = true;
			this.mnuHideReadBytes.Name = "mnuHideReadBytes";
			this.mnuHideReadBytes.Size = new System.Drawing.Size(152, 22);
			this.mnuHideReadBytes.Text = "Read bytes";
			this.mnuHideReadBytes.Click += new System.EventHandler(this.mnuColorProviderOptions_Click);
			// 
			// mnuHideWrittenBytes
			// 
			this.mnuHideWrittenBytes.CheckOnClick = true;
			this.mnuHideWrittenBytes.Name = "mnuHideWrittenBytes";
			this.mnuHideWrittenBytes.Size = new System.Drawing.Size(152, 22);
			this.mnuHideWrittenBytes.Text = "Written bytes";
			this.mnuHideWrittenBytes.Click += new System.EventHandler(this.mnuColorProviderOptions_Click);
			// 
			// mnuHideExecutedBytes
			// 
			this.mnuHideExecutedBytes.CheckOnClick = true;
			this.mnuHideExecutedBytes.Name = "mnuHideExecutedBytes";
			this.mnuHideExecutedBytes.Size = new System.Drawing.Size(152, 22);
			this.mnuHideExecutedBytes.Text = "Executed bytes";
			this.mnuHideExecutedBytes.Click += new System.EventHandler(this.mnuColorProviderOptions_Click);
			// 
			// toolStripMenuItem5
			// 
			this.toolStripMenuItem5.Name = "toolStripMenuItem5";
			this.toolStripMenuItem5.Size = new System.Drawing.Size(253, 6);
			// 
			// mnuConfigureColors
			// 
			this.mnuConfigureColors.Image = global::Mesen.GUI.Properties.Resources.PipetteSmall;
			this.mnuConfigureColors.Name = "mnuConfigureColors";
			this.mnuConfigureColors.Size = new System.Drawing.Size(256, 22);
			this.mnuConfigureColors.Text = "Configure Colors";
			this.mnuConfigureColors.Click += new System.EventHandler(this.mnuConfigureColors_Click);
			// 
			// fontSizeToolStripMenuItem
			// 
			this.fontSizeToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuIncreaseFontSize,
            this.mnuDecreaseFontSize,
            this.mnuResetFontSize,
            this.toolStripMenuItem12,
            this.mnuSelectFont,
            this.toolStripMenuItem13,
            this.mnuHighDensityMode});
			this.fontSizeToolStripMenuItem.Image = global::Mesen.GUI.Properties.Resources.Font;
			this.fontSizeToolStripMenuItem.Name = "fontSizeToolStripMenuItem";
			this.fontSizeToolStripMenuItem.Size = new System.Drawing.Size(256, 22);
			this.fontSizeToolStripMenuItem.Text = "Font Options";
			// 
			// mnuIncreaseFontSize
			// 
			this.mnuIncreaseFontSize.Name = "mnuIncreaseFontSize";
			this.mnuIncreaseFontSize.ShortcutKeyDisplayString = "";
			this.mnuIncreaseFontSize.Size = new System.Drawing.Size(217, 22);
			this.mnuIncreaseFontSize.Text = "Increase";
			this.mnuIncreaseFontSize.Click += new System.EventHandler(this.mnuIncreaseFontSize_Click);
			// 
			// mnuDecreaseFontSize
			// 
			this.mnuDecreaseFontSize.Name = "mnuDecreaseFontSize";
			this.mnuDecreaseFontSize.ShortcutKeyDisplayString = "";
			this.mnuDecreaseFontSize.Size = new System.Drawing.Size(217, 22);
			this.mnuDecreaseFontSize.Text = "Decrease";
			this.mnuDecreaseFontSize.Click += new System.EventHandler(this.mnuDecreaseFontSize_Click);
			// 
			// mnuResetFontSize
			// 
			this.mnuResetFontSize.Name = "mnuResetFontSize";
			this.mnuResetFontSize.ShortcutKeyDisplayString = "";
			this.mnuResetFontSize.Size = new System.Drawing.Size(217, 22);
			this.mnuResetFontSize.Text = "Reset to Default";
			this.mnuResetFontSize.Click += new System.EventHandler(this.mnuResetFontSize_Click);
			// 
			// toolStripMenuItem12
			// 
			this.toolStripMenuItem12.Name = "toolStripMenuItem12";
			this.toolStripMenuItem12.Size = new System.Drawing.Size(214, 6);
			// 
			// mnuSelectFont
			// 
			this.mnuSelectFont.Name = "mnuSelectFont";
			this.mnuSelectFont.Size = new System.Drawing.Size(217, 22);
			this.mnuSelectFont.Text = "Select Font...";
			this.mnuSelectFont.Click += new System.EventHandler(this.mnuSelectFont_Click);
			// 
			// toolStripMenuItem13
			// 
			this.toolStripMenuItem13.Name = "toolStripMenuItem13";
			this.toolStripMenuItem13.Size = new System.Drawing.Size(214, 6);
			// 
			// mnuHighDensityMode
			// 
			this.mnuHighDensityMode.CheckOnClick = true;
			this.mnuHighDensityMode.Name = "mnuHighDensityMode";
			this.mnuHighDensityMode.Size = new System.Drawing.Size(217, 22);
			this.mnuHighDensityMode.Text = "Use high text density mode";
			this.mnuHighDensityMode.CheckedChanged += new System.EventHandler(this.mnuHighDensityMode_CheckedChanged);
			// 
			// autorefreshSpeedToolStripMenuItem
			// 
			this.autorefreshSpeedToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuAutoRefreshLow,
            this.mnuAutoRefreshNormal,
            this.mnuAutoRefreshHigh});
			this.autorefreshSpeedToolStripMenuItem.Image = global::Mesen.GUI.Properties.Resources.Speed;
			this.autorefreshSpeedToolStripMenuItem.Name = "autorefreshSpeedToolStripMenuItem";
			this.autorefreshSpeedToolStripMenuItem.Size = new System.Drawing.Size(256, 22);
			this.autorefreshSpeedToolStripMenuItem.Text = "Auto-refresh Speed";
			// 
			// mnuAutoRefreshLow
			// 
			this.mnuAutoRefreshLow.Name = "mnuAutoRefreshLow";
			this.mnuAutoRefreshLow.Size = new System.Drawing.Size(159, 22);
			this.mnuAutoRefreshLow.Text = "Low (10 FPS)";
			this.mnuAutoRefreshLow.Click += new System.EventHandler(this.mnuAutoRefreshSpeed_Click);
			// 
			// mnuAutoRefreshNormal
			// 
			this.mnuAutoRefreshNormal.Name = "mnuAutoRefreshNormal";
			this.mnuAutoRefreshNormal.Size = new System.Drawing.Size(159, 22);
			this.mnuAutoRefreshNormal.Text = "Normal (30 FPS)";
			this.mnuAutoRefreshNormal.Click += new System.EventHandler(this.mnuAutoRefreshSpeed_Click);
			// 
			// mnuAutoRefreshHigh
			// 
			this.mnuAutoRefreshHigh.Name = "mnuAutoRefreshHigh";
			this.mnuAutoRefreshHigh.Size = new System.Drawing.Size(159, 22);
			this.mnuAutoRefreshHigh.Text = "High (60 FPS)";
			this.mnuAutoRefreshHigh.Click += new System.EventHandler(this.mnuAutoRefreshSpeed_Click);
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(253, 6);
			// 
			// mnuRefresh
			// 
			this.mnuRefresh.Image = global::Mesen.GUI.Properties.Resources.Refresh;
			this.mnuRefresh.Name = "mnuRefresh";
			this.mnuRefresh.Size = new System.Drawing.Size(256, 22);
			this.mnuRefresh.Text = "Refresh";
			this.mnuRefresh.Click += new System.EventHandler(this.mnuRefresh_Click);
			// 
			// toolStripMenuItem9
			// 
			this.toolStripMenuItem9.Name = "toolStripMenuItem9";
			this.toolStripMenuItem9.Size = new System.Drawing.Size(253, 6);
			// 
			// mnuAutoRefresh
			// 
			this.mnuAutoRefresh.Checked = true;
			this.mnuAutoRefresh.CheckOnClick = true;
			this.mnuAutoRefresh.CheckState = System.Windows.Forms.CheckState.Checked;
			this.mnuAutoRefresh.Name = "mnuAutoRefresh";
			this.mnuAutoRefresh.Size = new System.Drawing.Size(256, 22);
			this.mnuAutoRefresh.Text = "Auto-refresh";
			// 
			// mnuShowCharacters
			// 
			this.mnuShowCharacters.Checked = true;
			this.mnuShowCharacters.CheckOnClick = true;
			this.mnuShowCharacters.CheckState = System.Windows.Forms.CheckState.Checked;
			this.mnuShowCharacters.Name = "mnuShowCharacters";
			this.mnuShowCharacters.Size = new System.Drawing.Size(256, 22);
			this.mnuShowCharacters.Text = "Show characters";
			// 
			// mnuShowLabelInfoOnMouseOver
			// 
			this.mnuShowLabelInfoOnMouseOver.CheckOnClick = true;
			this.mnuShowLabelInfoOnMouseOver.Name = "mnuShowLabelInfoOnMouseOver";
			this.mnuShowLabelInfoOnMouseOver.Size = new System.Drawing.Size(256, 22);
			this.mnuShowLabelInfoOnMouseOver.Text = "Show label tooltip on mouseover";
			// 
			// toolStripMenuItem10
			// 
			this.toolStripMenuItem10.Name = "toolStripMenuItem10";
			this.toolStripMenuItem10.Size = new System.Drawing.Size(253, 6);
			// 
			// mnuIgnoreRedundantWrites
			// 
			this.mnuIgnoreRedundantWrites.CheckOnClick = true;
			this.mnuIgnoreRedundantWrites.Name = "mnuIgnoreRedundantWrites";
			this.mnuIgnoreRedundantWrites.Size = new System.Drawing.Size(256, 22);
			this.mnuIgnoreRedundantWrites.Text = "Ignore writes that do not alter data";
			// 
			// mnuEnablePerByteNavigation
			// 
			this.mnuEnablePerByteNavigation.CheckOnClick = true;
			this.mnuEnablePerByteNavigation.Name = "mnuEnablePerByteNavigation";
			this.mnuEnablePerByteNavigation.Size = new System.Drawing.Size(256, 22);
			this.mnuEnablePerByteNavigation.Text = "Use per-byte left/right navigation";
			this.mnuEnablePerByteNavigation.CheckedChanged += new System.EventHandler(this.mnuEnablePerByteNavigation_CheckedChanged);
			// 
			// mnuByteEditingMode
			// 
			this.mnuByteEditingMode.CheckOnClick = true;
			this.mnuByteEditingMode.Name = "mnuByteEditingMode";
			this.mnuByteEditingMode.Size = new System.Drawing.Size(256, 22);
			this.mnuByteEditingMode.Text = "Use per-byte editing mode";
			this.mnuByteEditingMode.CheckedChanged += new System.EventHandler(this.mnuByteEditingMode_CheckedChanged);
			// 
			// mnuHighlightCurrentRowColumn
			// 
			this.mnuHighlightCurrentRowColumn.CheckOnClick = true;
			this.mnuHighlightCurrentRowColumn.Name = "mnuHighlightCurrentRowColumn";
			this.mnuHighlightCurrentRowColumn.Size = new System.Drawing.Size(256, 22);
			this.mnuHighlightCurrentRowColumn.Text = "Highlight current row/column";
			this.mnuHighlightCurrentRowColumn.CheckedChanged += new System.EventHandler(this.mnuHighlightCurrentRowColumn_CheckedChanged);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuGoToAll,
            this.mnuGoTo,
            this.toolStripMenuItem14,
            this.mnuFind,
            this.mnuFindNext,
            this.mnuFindPrev});
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(54, 20);
			this.toolStripMenuItem1.Text = "Search";
			// 
			// mnuGoToAll
			// 
			this.mnuGoToAll.Name = "mnuGoToAll";
			this.mnuGoToAll.Size = new System.Drawing.Size(152, 22);
			this.mnuGoToAll.Text = "Go to All";
			this.mnuGoToAll.Visible = false;
			this.mnuGoToAll.Click += new System.EventHandler(this.mnuGoToAll_Click);
			// 
			// mnuGoTo
			// 
			this.mnuGoTo.Name = "mnuGoTo";
			this.mnuGoTo.Size = new System.Drawing.Size(152, 22);
			this.mnuGoTo.Text = "Go To...";
			this.mnuGoTo.Click += new System.EventHandler(this.mnuGoTo_Click);
			// 
			// toolStripMenuItem14
			// 
			this.toolStripMenuItem14.Name = "toolStripMenuItem14";
			this.toolStripMenuItem14.Size = new System.Drawing.Size(149, 6);
			// 
			// mnuFind
			// 
			this.mnuFind.Image = global::Mesen.GUI.Properties.Resources.Find;
			this.mnuFind.Name = "mnuFind";
			this.mnuFind.Size = new System.Drawing.Size(152, 22);
			this.mnuFind.Text = "Find...";
			this.mnuFind.Click += new System.EventHandler(this.mnuFind_Click);
			// 
			// mnuFindNext
			// 
			this.mnuFindNext.Image = global::Mesen.GUI.Properties.Resources.NextArrow;
			this.mnuFindNext.Name = "mnuFindNext";
			this.mnuFindNext.Size = new System.Drawing.Size(152, 22);
			this.mnuFindNext.Text = "Find Next";
			this.mnuFindNext.Click += new System.EventHandler(this.mnuFindNext_Click);
			// 
			// mnuFindPrev
			// 
			this.mnuFindPrev.Image = global::Mesen.GUI.Properties.Resources.PreviousArrow;
			this.mnuFindPrev.Name = "mnuFindPrev";
			this.mnuFindPrev.Size = new System.Drawing.Size(152, 22);
			this.mnuFindPrev.Text = "Find Previous";
			this.mnuFindPrev.Click += new System.EventHandler(this.mnuFindPrev_Click);
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tpgMemoryViewer);
			this.tabMain.Controls.Add(this.tpgAccessCounters);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 24);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(614, 369);
			this.tabMain.TabIndex = 4;
			this.tabMain.SelectedIndexChanged += new System.EventHandler(this.tabMain_SelectedIndexChanged);
			// 
			// tpgMemoryViewer
			// 
			this.tpgMemoryViewer.Controls.Add(this.panel1);
			this.tpgMemoryViewer.Location = new System.Drawing.Point(4, 22);
			this.tpgMemoryViewer.Margin = new System.Windows.Forms.Padding(0);
			this.tpgMemoryViewer.Name = "tpgMemoryViewer";
			this.tpgMemoryViewer.Size = new System.Drawing.Size(606, 343);
			this.tpgMemoryViewer.TabIndex = 0;
			this.tpgMemoryViewer.Text = "Memory Viewer";
			this.tpgMemoryViewer.UseVisualStyleBackColor = true;
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.flowLayoutPanel1);
			this.panel1.Controls.Add(this.ctrlHexViewer);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Margin = new System.Windows.Forms.Padding(0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(606, 343);
			this.panel1.TabIndex = 4;
			// 
			// tpgAccessCounters
			// 
			this.tpgAccessCounters.Location = new System.Drawing.Point(4, 22);
			this.tpgAccessCounters.Margin = new System.Windows.Forms.Padding(0);
			this.tpgAccessCounters.Name = "tpgAccessCounters";
			this.tpgAccessCounters.Size = new System.Drawing.Size(606, 343);
			this.tpgAccessCounters.TabIndex = 1;
			this.tpgAccessCounters.Text = "Access Counters";
			this.tpgAccessCounters.UseVisualStyleBackColor = true;
			// 
			// frmMemoryTools
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(614, 393);
			this.Controls.Add(this.tabMain);
			this.Controls.Add(this.menuStrip1);
			this.MainMenuStrip = this.menuStrip1;
			this.MinimumSize = new System.Drawing.Size(429, 337);
			this.Name = "frmMemoryTools";
			this.Text = "Memory Tools";
			this.flowLayoutPanel1.ResumeLayout(false);
			this.flowLayoutPanel1.PerformLayout();
			this.menuStrip1.ResumeLayout(false);
			this.menuStrip1.PerformLayout();
			this.tabMain.ResumeLayout(false);
			this.tpgMemoryViewer.ResumeLayout(false);
			this.panel1.ResumeLayout(false);
			this.panel1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private Controls.ctrlHexViewer ctrlHexViewer;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
		private System.Windows.Forms.Label lblViewMemoryType;
		private Mesen.GUI.Debugger.Controls.ComboBoxWithSeparator cboMemoryType;
		private Mesen.GUI.Controls.ctrlMesenMenuStrip menuStrip1;
		private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuFind;
		private System.Windows.Forms.ToolStripMenuItem mnuFindNext;
		private System.Windows.Forms.ToolStripMenuItem mnuFindPrev;
		private System.Windows.Forms.ToolStripMenuItem mnuGoTo;
		private System.Windows.Forms.ToolStripMenuItem mnuView;
		private System.Windows.Forms.ToolStripMenuItem mnuRefresh;
		private System.Windows.Forms.ToolStripMenuItem fontSizeToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuIncreaseFontSize;
		private System.Windows.Forms.ToolStripMenuItem mnuDecreaseFontSize;
		private System.Windows.Forms.ToolStripMenuItem mnuResetFontSize;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuClose;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoRefresh;
		private System.Windows.Forms.ToolStripMenuItem mnuImport;
		private System.Windows.Forms.ToolStripMenuItem mnuExport;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tpgMemoryViewer;
		private System.Windows.Forms.TabPage tpgAccessCounters;
		private System.Windows.Forms.ToolStripMenuItem mnuLoadTblFile;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
		private System.Windows.Forms.ToolStripMenuItem mnuShowCharacters;
		private System.Windows.Forms.ToolStripMenuItem mnuResetTblMappings;
		private System.Windows.Forms.ToolStripMenuItem highlightToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuHightlightReads;
		private System.Windows.Forms.ToolStripMenuItem mnuHighlightWrites;
		private System.Windows.Forms.ToolStripMenuItem mnuHighlightExecution;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem6;
		private System.Windows.Forms.ToolStripMenuItem fadeSpeedToolStripMenuItem;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem5;
		private System.Windows.Forms.ToolStripMenuItem mnuFadeSlow;
		private System.Windows.Forms.ToolStripMenuItem mnuFadeNormal;
		private System.Windows.Forms.ToolStripMenuItem mnuFadeFast;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem7;
		private System.Windows.Forms.ToolStripMenuItem mnuCustomFadeSpeed;
		private System.Windows.Forms.ToolStripMenuItem fadeToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuHideUnusedBytes;
		private System.Windows.Forms.ToolStripMenuItem mnuHideReadBytes;
		private System.Windows.Forms.ToolStripMenuItem mnuHideWrittenBytes;
		private System.Windows.Forms.ToolStripMenuItem mnuHideExecutedBytes;
		private System.Windows.Forms.ToolStripMenuItem mnuFadeNever;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.ToolStripMenuItem dataTypeHighlightingToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuHighlightCodeBytes;
		private System.Windows.Forms.ToolStripMenuItem mnuHighlightDataBytes;
		private System.Windows.Forms.ToolStripMenuItem mnuConfigureColors;
		private System.Windows.Forms.ToolStripMenuItem mnuShowLabelInfoOnMouseOver;
		private System.Windows.Forms.ToolStripMenuItem mnuHighlightLabelledBytes;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem8;
		private System.Windows.Forms.ToolStripMenuItem autorefreshSpeedToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoRefreshLow;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoRefreshNormal;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoRefreshHigh;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem9;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem10;
		private System.Windows.Forms.ToolStripMenuItem mnuIgnoreRedundantWrites;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem12;
		private System.Windows.Forms.ToolStripMenuItem mnuHighDensityMode;
		private System.Windows.Forms.ToolStripMenuItem mnuEnablePerByteNavigation;
		private System.Windows.Forms.ToolStripMenuItem mnuSelectFont;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem13;
		private System.Windows.Forms.ToolStripMenuItem mnuByteEditingMode;
		private System.Windows.Forms.ToolStripMenuItem mnuGoToAll;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem14;
		private System.Windows.Forms.ToolStripMenuItem mnuHighlightBreakpoints;
		private System.Windows.Forms.ToolStripMenuItem mnuHighlightCurrentRowColumn;
	}
}