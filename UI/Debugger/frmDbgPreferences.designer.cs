namespace Mesen.GUI.Debugger
{
	partial class frmDbgPreferences
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
			this.ctrlDbgShortcutsShared = new Mesen.GUI.Debugger.ctrlDbgShortcuts();
			this.tabMain = new System.Windows.Forms.TabControl();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this.tabPage2 = new System.Windows.Forms.TabPage();
			this.ctrlDbgShortcutsDebugger = new Mesen.GUI.Debugger.ctrlDbgShortcuts();
			this.tabPage3 = new System.Windows.Forms.TabPage();
			this.ctrlDbgShortcutsMemoryViewer = new Mesen.GUI.Debugger.ctrlDbgShortcuts();
			this.tabPage4 = new System.Windows.Forms.TabPage();
			this.ctrlDbgShortcutsScriptWindow = new Mesen.GUI.Debugger.ctrlDbgShortcuts();
			this.btnReset = new System.Windows.Forms.Button();
			this.baseConfigPanel.SuspendLayout();
			this.tabMain.SuspendLayout();
			this.tabPage1.SuspendLayout();
			this.tabPage2.SuspendLayout();
			this.tabPage3.SuspendLayout();
			this.tabPage4.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Controls.Add(this.btnReset);
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 460);
			this.baseConfigPanel.Size = new System.Drawing.Size(605, 29);
			this.baseConfigPanel.Controls.SetChildIndex(this.btnReset, 0);
			// 
			// ctrlDbgShortcutsShared
			// 
			this.ctrlDbgShortcutsShared.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlDbgShortcutsShared.Location = new System.Drawing.Point(3, 3);
			this.ctrlDbgShortcutsShared.Name = "ctrlDbgShortcutsShared";
			this.ctrlDbgShortcutsShared.Size = new System.Drawing.Size(591, 428);
			this.ctrlDbgShortcutsShared.TabIndex = 2;
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tabPage1);
			this.tabMain.Controls.Add(this.tabPage2);
			this.tabMain.Controls.Add(this.tabPage3);
			this.tabMain.Controls.Add(this.tabPage4);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(605, 460);
			this.tabMain.TabIndex = 3;
			// 
			// tabPage1
			// 
			this.tabPage1.Controls.Add(this.ctrlDbgShortcutsShared);
			this.tabPage1.Location = new System.Drawing.Point(4, 22);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage1.Size = new System.Drawing.Size(597, 434);
			this.tabPage1.TabIndex = 0;
			this.tabPage1.Text = "Shared";
			this.tabPage1.UseVisualStyleBackColor = true;
			// 
			// tabPage2
			// 
			this.tabPage2.Controls.Add(this.ctrlDbgShortcutsDebugger);
			this.tabPage2.Location = new System.Drawing.Point(4, 22);
			this.tabPage2.Name = "tabPage2";
			this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage2.Size = new System.Drawing.Size(597, 434);
			this.tabPage2.TabIndex = 1;
			this.tabPage2.Text = "Debugger";
			this.tabPage2.UseVisualStyleBackColor = true;
			// 
			// ctrlDbgShortcutsDebugger
			// 
			this.ctrlDbgShortcutsDebugger.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlDbgShortcutsDebugger.Location = new System.Drawing.Point(3, 3);
			this.ctrlDbgShortcutsDebugger.Name = "ctrlDbgShortcutsDebugger";
			this.ctrlDbgShortcutsDebugger.Size = new System.Drawing.Size(591, 428);
			this.ctrlDbgShortcutsDebugger.TabIndex = 3;
			// 
			// tabPage3
			// 
			this.tabPage3.Controls.Add(this.ctrlDbgShortcutsMemoryViewer);
			this.tabPage3.Location = new System.Drawing.Point(4, 22);
			this.tabPage3.Name = "tabPage3";
			this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage3.Size = new System.Drawing.Size(597, 434);
			this.tabPage3.TabIndex = 2;
			this.tabPage3.Text = "Memory Viewer";
			this.tabPage3.UseVisualStyleBackColor = true;
			// 
			// ctrlDbgShortcutsMemoryViewer
			// 
			this.ctrlDbgShortcutsMemoryViewer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlDbgShortcutsMemoryViewer.Location = new System.Drawing.Point(3, 3);
			this.ctrlDbgShortcutsMemoryViewer.Name = "ctrlDbgShortcutsMemoryViewer";
			this.ctrlDbgShortcutsMemoryViewer.Size = new System.Drawing.Size(591, 428);
			this.ctrlDbgShortcutsMemoryViewer.TabIndex = 3;
			// 
			// tabPage4
			// 
			this.tabPage4.Controls.Add(this.ctrlDbgShortcutsScriptWindow);
			this.tabPage4.Location = new System.Drawing.Point(4, 22);
			this.tabPage4.Name = "tabPage4";
			this.tabPage4.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage4.Size = new System.Drawing.Size(597, 434);
			this.tabPage4.TabIndex = 3;
			this.tabPage4.Text = "Script Window";
			this.tabPage4.UseVisualStyleBackColor = true;
			// 
			// ctrlDbgShortcutsScriptWindow
			// 
			this.ctrlDbgShortcutsScriptWindow.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlDbgShortcutsScriptWindow.Location = new System.Drawing.Point(3, 3);
			this.ctrlDbgShortcutsScriptWindow.Name = "ctrlDbgShortcutsScriptWindow";
			this.ctrlDbgShortcutsScriptWindow.Size = new System.Drawing.Size(591, 428);
			this.ctrlDbgShortcutsScriptWindow.TabIndex = 3;
			// 
			// btnReset
			// 
			this.btnReset.Location = new System.Drawing.Point(4, 3);
			this.btnReset.Name = "btnReset";
			this.btnReset.Size = new System.Drawing.Size(111, 23);
			this.btnReset.TabIndex = 3;
			this.btnReset.Text = "Reset to Defaults";
			this.btnReset.UseVisualStyleBackColor = true;
			this.btnReset.Click += new System.EventHandler(this.btnReset_Click);
			// 
			// frmDbgPreferences
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(605, 489);
			this.Controls.Add(this.tabMain);
			this.Name = "frmDbgPreferences";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Configure Shortcut Keys";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tabMain, 0);
			this.baseConfigPanel.ResumeLayout(false);
			this.tabMain.ResumeLayout(false);
			this.tabPage1.ResumeLayout(false);
			this.tabPage2.ResumeLayout(false);
			this.tabPage3.ResumeLayout(false);
			this.tabPage4.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private ctrlDbgShortcuts ctrlDbgShortcutsShared;
		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage2;
		private ctrlDbgShortcuts ctrlDbgShortcutsDebugger;
		private System.Windows.Forms.TabPage tabPage3;
		private ctrlDbgShortcuts ctrlDbgShortcutsMemoryViewer;
		private System.Windows.Forms.TabPage tabPage4;
		private ctrlDbgShortcuts ctrlDbgShortcutsScriptWindow;
		private System.Windows.Forms.Button btnReset;
	}
}