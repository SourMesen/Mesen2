namespace Mesen.GUI.Debugger
{
	partial class frmScript
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
			this.components = new System.ComponentModel.Container();
			this.mnuMain = new Mesen.GUI.Controls.ctrlMesenMenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuNewScript = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuOpen = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuSave = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuSaveAs = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuBuiltInScripts = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRecentScripts = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuClose = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuView = new System.Windows.Forms.ToolStripMenuItem();
			this.fontSizeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuIncreaseFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuDecreaseFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuResetFontSize = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem6 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSelectFont = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuShowLogWindow = new System.Windows.Forms.ToolStripMenuItem();
			this.scriptToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRun = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuStop = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem7 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSetScriptTimeout = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSaveBeforeRun = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoRestart = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoReload = new System.Windows.Forms.ToolStripMenuItem();
			this.onStartupToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuBlankWindow = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuTutorialScript = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAutoLoadLastScript = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHelp = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuApiReference = new System.Windows.Forms.ToolStripMenuItem();
			this.tsToolbar = new Mesen.GUI.Controls.ctrlMesenToolStrip();
			this.txtScriptContent = new FastColoredTextBoxNS.FastColoredTextBox();
			this.contextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuCopy = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuCut = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPaste = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSelectAll = new System.Windows.Forms.ToolStripMenuItem();
			this.ctrlSplit = new Mesen.GUI.Controls.ctrlSplitContainer();
			this.txtLog = new System.Windows.Forms.TextBox();
			this.tmrLog = new System.Windows.Forms.Timer(this.components);
			this.statusStrip1 = new System.Windows.Forms.StatusStrip();
			this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
			this.lblScriptActive = new System.Windows.Forms.ToolStripStatusLabel();
			this.mnuMain.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.txtScriptContent)).BeginInit();
			this.contextMenu.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.ctrlSplit)).BeginInit();
			this.ctrlSplit.Panel1.SuspendLayout();
			this.ctrlSplit.Panel2.SuspendLayout();
			this.ctrlSplit.SuspendLayout();
			this.statusStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// mnuMain
			// 
			this.mnuMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.mnuView,
            this.scriptToolStripMenuItem,
            this.mnuHelp});
			this.mnuMain.Location = new System.Drawing.Point(0, 0);
			this.mnuMain.Name = "mnuMain";
			this.mnuMain.Size = new System.Drawing.Size(965, 24);
			this.mnuMain.TabIndex = 0;
			this.mnuMain.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuNewScript,
            this.mnuOpen,
            this.mnuSave,
            this.mnuSaveAs,
            this.toolStripMenuItem1,
            this.mnuBuiltInScripts,
            this.mnuRecentScripts,
            this.toolStripMenuItem2,
            this.mnuClose});
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
			this.fileToolStripMenuItem.Text = "File";
			// 
			// mnuNewScript
			// 
			this.mnuNewScript.Image = global::Mesen.GUI.Properties.Resources.Script;
			this.mnuNewScript.Name = "mnuNewScript";
			this.mnuNewScript.Size = new System.Drawing.Size(178, 22);
			this.mnuNewScript.Text = "New Script Window";
			this.mnuNewScript.Click += new System.EventHandler(this.mnuNewScript_Click);
			// 
			// mnuOpen
			// 
			this.mnuOpen.Image = global::Mesen.GUI.Properties.Resources.Folder;
			this.mnuOpen.Name = "mnuOpen";
			this.mnuOpen.Size = new System.Drawing.Size(178, 22);
			this.mnuOpen.Text = "Open";
			this.mnuOpen.Click += new System.EventHandler(this.mnuOpen_Click);
			// 
			// mnuSave
			// 
			this.mnuSave.Image = global::Mesen.GUI.Properties.Resources.SaveFloppy;
			this.mnuSave.Name = "mnuSave";
			this.mnuSave.Size = new System.Drawing.Size(178, 22);
			this.mnuSave.Text = "Save";
			this.mnuSave.Click += new System.EventHandler(this.mnuSave_Click);
			// 
			// mnuSaveAs
			// 
			this.mnuSaveAs.Name = "mnuSaveAs";
			this.mnuSaveAs.Size = new System.Drawing.Size(178, 22);
			this.mnuSaveAs.Text = "Save as...";
			this.mnuSaveAs.Click += new System.EventHandler(this.mnuSaveAs_Click);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(175, 6);
			// 
			// mnuBuiltInScripts
			// 
			this.mnuBuiltInScripts.Image = global::Mesen.GUI.Properties.Resources.LogWindow;
			this.mnuBuiltInScripts.Name = "mnuBuiltInScripts";
			this.mnuBuiltInScripts.Size = new System.Drawing.Size(178, 22);
			this.mnuBuiltInScripts.Text = "Built-in Scripts";
			// 
			// mnuRecentScripts
			// 
			this.mnuRecentScripts.Name = "mnuRecentScripts";
			this.mnuRecentScripts.Size = new System.Drawing.Size(178, 22);
			this.mnuRecentScripts.Text = "Recent Scripts";
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(175, 6);
			// 
			// mnuClose
			// 
			this.mnuClose.Image = global::Mesen.GUI.Properties.Resources.Exit;
			this.mnuClose.Name = "mnuClose";
			this.mnuClose.Size = new System.Drawing.Size(178, 22);
			this.mnuClose.Text = "Close";
			this.mnuClose.Click += new System.EventHandler(this.mnuClose_Click);
			// 
			// mnuView
			// 
			this.mnuView.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fontSizeToolStripMenuItem,
            this.toolStripMenuItem4,
            this.mnuShowLogWindow});
			this.mnuView.Name = "mnuView";
			this.mnuView.Size = new System.Drawing.Size(44, 20);
			this.mnuView.Text = "View";
			this.mnuView.DropDownOpening += new System.EventHandler(this.mnuView_DropDownOpening);
			// 
			// fontSizeToolStripMenuItem
			// 
			this.fontSizeToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuIncreaseFontSize,
            this.mnuDecreaseFontSize,
            this.mnuResetFontSize,
            this.toolStripMenuItem6,
            this.mnuSelectFont});
			this.fontSizeToolStripMenuItem.Image = global::Mesen.GUI.Properties.Resources.Font;
			this.fontSizeToolStripMenuItem.Name = "fontSizeToolStripMenuItem";
			this.fontSizeToolStripMenuItem.Size = new System.Drawing.Size(173, 22);
			this.fontSizeToolStripMenuItem.Text = "Font Options";
			// 
			// mnuIncreaseFontSize
			// 
			this.mnuIncreaseFontSize.Name = "mnuIncreaseFontSize";
			this.mnuIncreaseFontSize.ShortcutKeyDisplayString = "";
			this.mnuIncreaseFontSize.Size = new System.Drawing.Size(157, 22);
			this.mnuIncreaseFontSize.Text = "Increase Size";
			this.mnuIncreaseFontSize.Click += new System.EventHandler(this.mnuIncreaseFontSize_Click);
			// 
			// mnuDecreaseFontSize
			// 
			this.mnuDecreaseFontSize.Name = "mnuDecreaseFontSize";
			this.mnuDecreaseFontSize.ShortcutKeyDisplayString = "";
			this.mnuDecreaseFontSize.Size = new System.Drawing.Size(157, 22);
			this.mnuDecreaseFontSize.Text = "Decrease Size";
			this.mnuDecreaseFontSize.Click += new System.EventHandler(this.mnuDecreaseFontSize_Click);
			// 
			// mnuResetFontSize
			// 
			this.mnuResetFontSize.Name = "mnuResetFontSize";
			this.mnuResetFontSize.ShortcutKeyDisplayString = "";
			this.mnuResetFontSize.Size = new System.Drawing.Size(157, 22);
			this.mnuResetFontSize.Text = "Reset to Default";
			this.mnuResetFontSize.Click += new System.EventHandler(this.mnuResetFontSize_Click);
			// 
			// toolStripMenuItem6
			// 
			this.toolStripMenuItem6.Name = "toolStripMenuItem6";
			this.toolStripMenuItem6.Size = new System.Drawing.Size(154, 6);
			// 
			// mnuSelectFont
			// 
			this.mnuSelectFont.Name = "mnuSelectFont";
			this.mnuSelectFont.Size = new System.Drawing.Size(157, 22);
			this.mnuSelectFont.Text = "Select Font...";
			this.mnuSelectFont.Click += new System.EventHandler(this.mnuSelectFont_Click);
			// 
			// toolStripMenuItem4
			// 
			this.toolStripMenuItem4.Name = "toolStripMenuItem4";
			this.toolStripMenuItem4.Size = new System.Drawing.Size(170, 6);
			// 
			// mnuShowLogWindow
			// 
			this.mnuShowLogWindow.CheckOnClick = true;
			this.mnuShowLogWindow.Name = "mnuShowLogWindow";
			this.mnuShowLogWindow.Size = new System.Drawing.Size(173, 22);
			this.mnuShowLogWindow.Text = "Show Log Window";
			this.mnuShowLogWindow.Click += new System.EventHandler(this.mnuShowLogWindow_Click);
			// 
			// scriptToolStripMenuItem
			// 
			this.scriptToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuRun,
            this.mnuStop,
            this.toolStripMenuItem7,
            this.mnuSetScriptTimeout,
            this.toolStripMenuItem3,
            this.mnuSaveBeforeRun,
            this.mnuAutoRestart,
            this.mnuAutoReload,
            this.onStartupToolStripMenuItem});
			this.scriptToolStripMenuItem.Name = "scriptToolStripMenuItem";
			this.scriptToolStripMenuItem.Size = new System.Drawing.Size(49, 20);
			this.scriptToolStripMenuItem.Text = "Script";
			// 
			// mnuRun
			// 
			this.mnuRun.Image = global::Mesen.GUI.Properties.Resources.MediaPlay;
			this.mnuRun.Name = "mnuRun";
			this.mnuRun.Size = new System.Drawing.Size(258, 22);
			this.mnuRun.Text = "Run";
			this.mnuRun.Click += new System.EventHandler(this.mnuRun_Click);
			// 
			// mnuStop
			// 
			this.mnuStop.Image = global::Mesen.GUI.Properties.Resources.MediaStop;
			this.mnuStop.Name = "mnuStop";
			this.mnuStop.ShortcutKeyDisplayString = "";
			this.mnuStop.Size = new System.Drawing.Size(258, 22);
			this.mnuStop.Text = "Stop";
			this.mnuStop.Click += new System.EventHandler(this.mnuStop_Click);
			// 
			// toolStripMenuItem7
			// 
			this.toolStripMenuItem7.Name = "toolStripMenuItem7";
			this.toolStripMenuItem7.Size = new System.Drawing.Size(255, 6);
			this.toolStripMenuItem7.Visible = false;
			// 
			// mnuSetScriptTimeout
			// 
			this.mnuSetScriptTimeout.Image = global::Mesen.GUI.Properties.Resources.Speed;
			this.mnuSetScriptTimeout.Name = "mnuSetScriptTimeout";
			this.mnuSetScriptTimeout.Size = new System.Drawing.Size(258, 22);
			this.mnuSetScriptTimeout.Text = "Set Script Timeout...";
			this.mnuSetScriptTimeout.Visible = false;
			this.mnuSetScriptTimeout.Click += new System.EventHandler(this.mnuSetScriptTimeout_Click);
			// 
			// toolStripMenuItem3
			// 
			this.toolStripMenuItem3.Name = "toolStripMenuItem3";
			this.toolStripMenuItem3.Size = new System.Drawing.Size(255, 6);
			// 
			// mnuSaveBeforeRun
			// 
			this.mnuSaveBeforeRun.CheckOnClick = true;
			this.mnuSaveBeforeRun.Name = "mnuSaveBeforeRun";
			this.mnuSaveBeforeRun.Size = new System.Drawing.Size(258, 22);
			this.mnuSaveBeforeRun.Text = "Auto-save before running";
			// 
			// mnuAutoRestart
			// 
			this.mnuAutoRestart.CheckOnClick = true;
			this.mnuAutoRestart.Name = "mnuAutoRestart";
			this.mnuAutoRestart.Size = new System.Drawing.Size(258, 22);
			this.mnuAutoRestart.Text = "Auto-restart after reload";
			// 
			// mnuAutoReload
			// 
			this.mnuAutoReload.CheckOnClick = true;
			this.mnuAutoReload.Name = "mnuAutoReload";
			this.mnuAutoReload.Size = new System.Drawing.Size(258, 22);
			this.mnuAutoReload.Text = "Auto-reload when file changes";
			// 
			// onStartupToolStripMenuItem
			// 
			this.onStartupToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuBlankWindow,
            this.mnuTutorialScript,
            this.mnuAutoLoadLastScript});
			this.onStartupToolStripMenuItem.Name = "onStartupToolStripMenuItem";
			this.onStartupToolStripMenuItem.Size = new System.Drawing.Size(258, 22);
			this.onStartupToolStripMenuItem.Text = "When opening the script window...";
			// 
			// mnuBlankWindow
			// 
			this.mnuBlankWindow.Name = "mnuBlankWindow";
			this.mnuBlankWindow.Size = new System.Drawing.Size(227, 22);
			this.mnuBlankWindow.Text = "Display a blank code window";
			this.mnuBlankWindow.Click += new System.EventHandler(this.mnuBlankWindow_Click);
			// 
			// mnuTutorialScript
			// 
			this.mnuTutorialScript.Name = "mnuTutorialScript";
			this.mnuTutorialScript.Size = new System.Drawing.Size(227, 22);
			this.mnuTutorialScript.Text = "Display the tutorial script";
			this.mnuTutorialScript.Click += new System.EventHandler(this.mnuTutorialScript_Click);
			// 
			// mnuAutoLoadLastScript
			// 
			this.mnuAutoLoadLastScript.Name = "mnuAutoLoadLastScript";
			this.mnuAutoLoadLastScript.Size = new System.Drawing.Size(227, 22);
			this.mnuAutoLoadLastScript.Text = "Load the last script loaded";
			this.mnuAutoLoadLastScript.Click += new System.EventHandler(this.mnuAutoLoadLastScript_Click);
			// 
			// mnuHelp
			// 
			this.mnuHelp.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuApiReference});
			this.mnuHelp.Name = "mnuHelp";
			this.mnuHelp.Size = new System.Drawing.Size(44, 20);
			this.mnuHelp.Text = "Help";
			this.mnuHelp.Visible = false;
			// 
			// mnuApiReference
			// 
			this.mnuApiReference.Image = global::Mesen.GUI.Properties.Resources.Exclamation;
			this.mnuApiReference.Name = "mnuApiReference";
			this.mnuApiReference.Size = new System.Drawing.Size(147, 22);
			this.mnuApiReference.Text = "API Reference";
			this.mnuApiReference.Click += new System.EventHandler(this.mnuApiReference_Click);
			// 
			// tsToolbar
			// 
			this.tsToolbar.Location = new System.Drawing.Point(0, 24);
			this.tsToolbar.Name = "tsToolbar";
			this.tsToolbar.RenderMode = System.Windows.Forms.ToolStripRenderMode.Professional;
			this.tsToolbar.Size = new System.Drawing.Size(965, 25);
			this.tsToolbar.TabIndex = 1;
			this.tsToolbar.Text = "toolStrip1";
			// 
			// txtScriptContent
			// 
			this.txtScriptContent.AutoCompleteBracketsList = new char[] {
        '(',
        ')',
        '{',
        '}',
        '[',
        ']',
        '\"',
        '\"',
        '\'',
        '\''};
			this.txtScriptContent.AutoIndentChars = false;
			this.txtScriptContent.AutoIndentCharsPatterns = "\r\n^\\s*[\\w\\.]+(\\s\\w+)?\\s*(?<range>=)\\s*(?<range>.+)\r\n";
			this.txtScriptContent.AutoIndentExistingLines = false;
			this.txtScriptContent.AutoScrollMinSize = new System.Drawing.Size(43, 14);
			this.txtScriptContent.BackBrush = null;
			this.txtScriptContent.BracketsHighlightStrategy = FastColoredTextBoxNS.BracketsHighlightStrategy.Strategy2;
			this.txtScriptContent.CharHeight = 14;
			this.txtScriptContent.CharWidth = 8;
			this.txtScriptContent.CommentPrefix = "--";
			this.txtScriptContent.ContextMenuStrip = this.contextMenu;
			this.txtScriptContent.CurrentLineColor = System.Drawing.Color.Gainsboro;
			this.txtScriptContent.Cursor = System.Windows.Forms.Cursors.IBeam;
			this.txtScriptContent.DisabledColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))));
			this.txtScriptContent.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtScriptContent.IsReplaceMode = false;
			this.txtScriptContent.Language = FastColoredTextBoxNS.Language.Lua;
			this.txtScriptContent.LeftBracket = '(';
			this.txtScriptContent.LeftBracket2 = '{';
			this.txtScriptContent.Location = new System.Drawing.Point(0, 0);
			this.txtScriptContent.Name = "txtScriptContent";
			this.txtScriptContent.Paddings = new System.Windows.Forms.Padding(0);
			this.txtScriptContent.ReservedCountOfLineNumberChars = 3;
			this.txtScriptContent.RightBracket = ')';
			this.txtScriptContent.RightBracket2 = '}';
			this.txtScriptContent.SelectionColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(255)))));
			this.txtScriptContent.Size = new System.Drawing.Size(965, 421);
			this.txtScriptContent.TabIndex = 3;
			this.txtScriptContent.TabLength = 2;
			this.txtScriptContent.Zoom = 100;
			// 
			// contextMenu
			// 
			this.contextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuCopy,
            this.mnuCut,
            this.mnuPaste,
            this.toolStripMenuItem5,
            this.mnuSelectAll});
			this.contextMenu.Name = "contextMenu";
			this.contextMenu.Size = new System.Drawing.Size(123, 98);
			// 
			// mnuCopy
			// 
			this.mnuCopy.Image = global::Mesen.GUI.Properties.Resources.Copy;
			this.mnuCopy.Name = "mnuCopy";
			this.mnuCopy.Size = new System.Drawing.Size(122, 22);
			this.mnuCopy.Text = "Copy";
			this.mnuCopy.Click += new System.EventHandler(this.mnuCopy_Click);
			// 
			// mnuCut
			// 
			this.mnuCut.Image = global::Mesen.GUI.Properties.Resources.Cut;
			this.mnuCut.Name = "mnuCut";
			this.mnuCut.Size = new System.Drawing.Size(122, 22);
			this.mnuCut.Text = "Cut";
			this.mnuCut.Click += new System.EventHandler(this.mnuCut_Click);
			// 
			// mnuPaste
			// 
			this.mnuPaste.Image = global::Mesen.GUI.Properties.Resources.Paste;
			this.mnuPaste.Name = "mnuPaste";
			this.mnuPaste.Size = new System.Drawing.Size(122, 22);
			this.mnuPaste.Text = "Paste";
			this.mnuPaste.Click += new System.EventHandler(this.mnuPaste_Click);
			// 
			// toolStripMenuItem5
			// 
			this.toolStripMenuItem5.Name = "toolStripMenuItem5";
			this.toolStripMenuItem5.Size = new System.Drawing.Size(119, 6);
			// 
			// mnuSelectAll
			// 
			this.mnuSelectAll.Image = global::Mesen.GUI.Properties.Resources.SelectAll;
			this.mnuSelectAll.Name = "mnuSelectAll";
			this.mnuSelectAll.Size = new System.Drawing.Size(122, 22);
			this.mnuSelectAll.Text = "Select All";
			this.mnuSelectAll.Click += new System.EventHandler(this.mnuSelectAll_Click);
			// 
			// ctrlSplit
			// 
			this.ctrlSplit.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlSplit.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
			this.ctrlSplit.HidePanel2 = false;
			this.ctrlSplit.Location = new System.Drawing.Point(0, 49);
			this.ctrlSplit.Name = "ctrlSplit";
			this.ctrlSplit.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// ctrlSplit.Panel1
			// 
			this.ctrlSplit.Panel1.Controls.Add(this.txtScriptContent);
			this.ctrlSplit.Panel1.Padding = new System.Windows.Forms.Padding(0, 0, 0, 2);
			this.ctrlSplit.Panel1MinSize = 200;
			// 
			// ctrlSplit.Panel2
			// 
			this.ctrlSplit.Panel2.Controls.Add(this.txtLog);
			this.ctrlSplit.Panel2.Padding = new System.Windows.Forms.Padding(0, 2, 0, 0);
			this.ctrlSplit.Size = new System.Drawing.Size(965, 516);
			this.ctrlSplit.SplitterDistance = 423;
			this.ctrlSplit.TabIndex = 4;
			// 
			// txtLog
			// 
			this.txtLog.BackColor = System.Drawing.SystemColors.ControlLightLight;
			this.txtLog.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtLog.Location = new System.Drawing.Point(0, 2);
			this.txtLog.Multiline = true;
			this.txtLog.Name = "txtLog";
			this.txtLog.ReadOnly = true;
			this.txtLog.ScrollBars = System.Windows.Forms.ScrollBars.Both;
			this.txtLog.Size = new System.Drawing.Size(965, 87);
			this.txtLog.TabIndex = 0;
			// 
			// tmrLog
			// 
			this.tmrLog.Enabled = true;
			this.tmrLog.Interval = 200;
			this.tmrLog.Tick += new System.EventHandler(this.tmrLog_Tick);
			// 
			// statusStrip1
			// 
			this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.lblScriptActive});
			this.statusStrip1.Location = new System.Drawing.Point(0, 565);
			this.statusStrip1.Name = "statusStrip1";
			this.statusStrip1.Size = new System.Drawing.Size(965, 22);
			this.statusStrip1.TabIndex = 6;
			this.statusStrip1.Text = "statusStrip1";
			// 
			// toolStripStatusLabel1
			// 
			this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
			this.toolStripStatusLabel1.Size = new System.Drawing.Size(950, 17);
			this.toolStripStatusLabel1.Spring = true;
			// 
			// lblScriptActive
			// 
			this.lblScriptActive.ForeColor = System.Drawing.Color.Green;
			this.lblScriptActive.Image = global::Mesen.GUI.Properties.Resources.MediaPlay;
			this.lblScriptActive.Name = "lblScriptActive";
			this.lblScriptActive.Size = new System.Drawing.Size(109, 17);
			this.lblScriptActive.Text = "Script is running";
			this.lblScriptActive.Visible = false;
			// 
			// frmScript
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(965, 587);
			this.Controls.Add(this.ctrlSplit);
			this.Controls.Add(this.tsToolbar);
			this.Controls.Add(this.mnuMain);
			this.Controls.Add(this.statusStrip1);
			this.MainMenuStrip = this.mnuMain;
			this.MinimumSize = new System.Drawing.Size(441, 444);
			this.Name = "frmScript";
			this.Text = "Script Window";
			this.Controls.SetChildIndex(this.statusStrip1, 0);
			this.Controls.SetChildIndex(this.mnuMain, 0);
			this.Controls.SetChildIndex(this.tsToolbar, 0);
			this.Controls.SetChildIndex(this.ctrlSplit, 0);
			this.mnuMain.ResumeLayout(false);
			this.mnuMain.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.txtScriptContent)).EndInit();
			this.contextMenu.ResumeLayout(false);
			this.ctrlSplit.Panel1.ResumeLayout(false);
			this.ctrlSplit.Panel2.ResumeLayout(false);
			this.ctrlSplit.Panel2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.ctrlSplit)).EndInit();
			this.ctrlSplit.ResumeLayout(false);
			this.statusStrip1.ResumeLayout(false);
			this.statusStrip1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private Mesen.GUI.Controls.ctrlMesenMenuStrip mnuMain;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuOpen;
		private Mesen.GUI.Controls.ctrlMesenToolStrip tsToolbar;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuClose;
		private System.Windows.Forms.ToolStripMenuItem scriptToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuRun;
		private FastColoredTextBoxNS.FastColoredTextBox txtScriptContent;
		private System.Windows.Forms.ToolStripMenuItem mnuSave;
		private System.Windows.Forms.ToolStripMenuItem mnuRecentScripts;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuStop;
		private GUI.Controls.ctrlSplitContainer ctrlSplit;
		private System.Windows.Forms.TextBox txtLog;
		private System.Windows.Forms.Timer tmrLog;
		private System.Windows.Forms.StatusStrip statusStrip1;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
		private System.Windows.Forms.ToolStripStatusLabel lblScriptActive;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
		private System.Windows.Forms.ToolStripMenuItem mnuSaveBeforeRun;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoReload;
		private System.Windows.Forms.ToolStripMenuItem mnuView;
		private System.Windows.Forms.ToolStripMenuItem mnuShowLogWindow;
		private System.Windows.Forms.ContextMenuStrip contextMenu;
		private System.Windows.Forms.ToolStripMenuItem mnuCopy;
		private System.Windows.Forms.ToolStripMenuItem mnuCut;
		private System.Windows.Forms.ToolStripMenuItem mnuPaste;
		private System.Windows.Forms.ToolStripMenuItem fontSizeToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuIncreaseFontSize;
		private System.Windows.Forms.ToolStripMenuItem mnuDecreaseFontSize;
		private System.Windows.Forms.ToolStripMenuItem mnuResetFontSize;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
		private System.Windows.Forms.ToolStripMenuItem mnuNewScript;
		private System.Windows.Forms.ToolStripMenuItem mnuSaveAs;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem5;
		private System.Windows.Forms.ToolStripMenuItem mnuSelectAll;
		private System.Windows.Forms.ToolStripMenuItem onStartupToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem mnuBlankWindow;
		private System.Windows.Forms.ToolStripMenuItem mnuTutorialScript;
		private System.Windows.Forms.ToolStripMenuItem mnuAutoLoadLastScript;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem6;
		private System.Windows.Forms.ToolStripMenuItem mnuSelectFont;
		private System.Windows.Forms.ToolStripMenuItem mnuBuiltInScripts;
		private System.Windows.Forms.ToolStripMenuItem mnuHelp;
		private System.Windows.Forms.ToolStripMenuItem mnuApiReference;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem7;
		private System.Windows.Forms.ToolStripMenuItem mnuSetScriptTimeout;
	  private System.Windows.Forms.ToolStripMenuItem mnuAutoRestart;
   }
}