using Mesen.GUI.Controls;
namespace Mesen.GUI.Forms
{
	partial class frmCheatList
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
			this.components = new System.ComponentModel.Container();
			this.tabMain = new System.Windows.Forms.TabControl();
			this.tabCheats = new System.Windows.Forms.TabPage();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.lstCheats = new Mesen.GUI.Controls.MyListView();
			this.colCheatName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colCodes = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.contextMenuCheats = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuAddCheat = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEditCheat = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuDeleteCheat = new System.Windows.Forms.ToolStripMenuItem();
			this.tsCheatActions = new Mesen.GUI.Controls.ctrlMesenToolStrip();
			this.btnAddCheat = new System.Windows.Forms.ToolStripButton();
			this.btnEditCheat = new System.Windows.Forms.ToolStripButton();
			this.btnDeleteCheat = new System.Windows.Forms.ToolStripButton();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.chkDisableCheats = new System.Windows.Forms.CheckBox();
			this.colEnabled = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.baseConfigPanel.SuspendLayout();
			this.tabMain.SuspendLayout();
			this.tabCheats.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			this.contextMenuCheats.SuspendLayout();
			this.tsCheatActions.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// baseConfigPanel
			// 
			this.baseConfigPanel.Controls.Add(this.chkDisableCheats);
			this.baseConfigPanel.Location = new System.Drawing.Point(0, 324);
			this.baseConfigPanel.Size = new System.Drawing.Size(446, 29);
			this.baseConfigPanel.TabIndex = 4;
			this.baseConfigPanel.Controls.SetChildIndex(this.chkDisableCheats, 0);
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tabCheats);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 0);
			this.tabMain.Margin = new System.Windows.Forms.Padding(0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(446, 324);
			this.tabMain.TabIndex = 0;
			// 
			// tabCheats
			// 
			this.tabCheats.Controls.Add(this.tableLayoutPanel1);
			this.tabCheats.Location = new System.Drawing.Point(4, 22);
			this.tabCheats.Name = "tabCheats";
			this.tabCheats.Padding = new System.Windows.Forms.Padding(3);
			this.tabCheats.Size = new System.Drawing.Size(438, 298);
			this.tabCheats.TabIndex = 0;
			this.tabCheats.Text = "Cheats";
			this.tabCheats.UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 1;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.Controls.Add(this.lstCheats, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.tsCheatActions, 0, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 3);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(432, 292);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// lstCheats
			// 
			this.lstCheats.CheckBoxes = true;
			this.lstCheats.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colEnabled,
            this.colCheatName,
            this.colCodes});
			this.lstCheats.ContextMenuStrip = this.contextMenuCheats;
			this.lstCheats.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstCheats.FullRowSelect = true;
			this.lstCheats.GridLines = true;
			this.lstCheats.HideSelection = false;
			this.lstCheats.Location = new System.Drawing.Point(0, 26);
			this.lstCheats.Margin = new System.Windows.Forms.Padding(0, 3, 0, 0);
			this.lstCheats.Name = "lstCheats";
			this.lstCheats.Size = new System.Drawing.Size(432, 266);
			this.lstCheats.TabIndex = 1;
			this.lstCheats.UseCompatibleStateImageBehavior = false;
			this.lstCheats.View = System.Windows.Forms.View.Details;
			this.lstCheats.SelectedIndexChanged += new System.EventHandler(this.lstCheats_SelectedIndexChanged);
			this.lstCheats.DoubleClick += new System.EventHandler(this.lstCheats_DoubleClick);
			// 
			// colCheatName
			// 
			this.colCheatName.Name = "colCheatName";
			this.colCheatName.Text = "Cheat Name";
			this.colCheatName.Width = 250;
			// 
			// colCodes
			// 
			this.colCodes.Name = "colCodes";
			this.colCodes.Text = "Codes";
			this.colCodes.Width = 304;
			// 
			// contextMenuCheats
			// 
			this.contextMenuCheats.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuAddCheat,
            this.mnuEditCheat,
            this.mnuDeleteCheat});
			this.contextMenuCheats.Name = "contextMenuCheats";
			this.contextMenuCheats.Size = new System.Drawing.Size(160, 70);
			// 
			// mnuAddCheat
			// 
			this.mnuAddCheat.Image = global::Mesen.GUI.Properties.Resources.Add;
			this.mnuAddCheat.Name = "mnuAddCheat";
			this.mnuAddCheat.ShortcutKeys = System.Windows.Forms.Keys.Insert;
			this.mnuAddCheat.Size = new System.Drawing.Size(159, 22);
			this.mnuAddCheat.Text = "Add cheat...";
			this.mnuAddCheat.Click += new System.EventHandler(this.mnuAddCheat_Click);
			// 
			// mnuEditCheat
			// 
			this.mnuEditCheat.Image = global::Mesen.GUI.Properties.Resources.Edit;
			this.mnuEditCheat.Name = "mnuEditCheat";
			this.mnuEditCheat.ShortcutKeyDisplayString = "Dbl-Click";
			this.mnuEditCheat.Size = new System.Drawing.Size(159, 22);
			this.mnuEditCheat.Text = "Edit";
			this.mnuEditCheat.Click += new System.EventHandler(this.mnuEditCheat_Click);
			// 
			// mnuDeleteCheat
			// 
			this.mnuDeleteCheat.Enabled = false;
			this.mnuDeleteCheat.Image = global::Mesen.GUI.Properties.Resources.Close;
			this.mnuDeleteCheat.Name = "mnuDeleteCheat";
			this.mnuDeleteCheat.ShortcutKeys = System.Windows.Forms.Keys.Delete;
			this.mnuDeleteCheat.Size = new System.Drawing.Size(159, 22);
			this.mnuDeleteCheat.Text = "Delete";
			this.mnuDeleteCheat.Click += new System.EventHandler(this.btnDeleteCheat_Click);
			// 
			// tsCheatActions
			// 
			this.tsCheatActions.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.btnAddCheat,
            this.btnEditCheat,
            this.btnDeleteCheat});
			this.tsCheatActions.LayoutStyle = System.Windows.Forms.ToolStripLayoutStyle.Flow;
			this.tsCheatActions.Location = new System.Drawing.Point(0, 0);
			this.tsCheatActions.Name = "tsCheatActions";
			this.tsCheatActions.Size = new System.Drawing.Size(432, 23);
			this.tsCheatActions.TabIndex = 6;
			this.tsCheatActions.Text = "toolStrip1";
			// 
			// btnAddCheat
			// 
			this.btnAddCheat.Image = global::Mesen.GUI.Properties.Resources.Add;
			this.btnAddCheat.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.btnAddCheat.Name = "btnAddCheat";
			this.btnAddCheat.Size = new System.Drawing.Size(83, 20);
			this.btnAddCheat.Text = "Add Cheat";
			this.btnAddCheat.Click += new System.EventHandler(this.mnuAddCheat_Click);
			// 
			// btnEditCheat
			// 
			this.btnEditCheat.Image = global::Mesen.GUI.Properties.Resources.Edit;
			this.btnEditCheat.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.btnEditCheat.Name = "btnEditCheat";
			this.btnEditCheat.Size = new System.Drawing.Size(47, 20);
			this.btnEditCheat.Text = "Edit";
			this.btnEditCheat.Click += new System.EventHandler(this.mnuEditCheat_Click);
			// 
			// btnDeleteCheat
			// 
			this.btnDeleteCheat.Image = global::Mesen.GUI.Properties.Resources.Close;
			this.btnDeleteCheat.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.btnDeleteCheat.Name = "btnDeleteCheat";
			this.btnDeleteCheat.Size = new System.Drawing.Size(60, 20);
			this.btnDeleteCheat.Text = "Delete";
			this.btnDeleteCheat.Click += new System.EventHandler(this.btnDeleteCheat_Click);
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 1;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Controls.Add(this.tabMain, 0, 0);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 2;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.Size = new System.Drawing.Size(446, 324);
			this.tableLayoutPanel2.TabIndex = 2;
			// 
			// chkDisableCheats
			// 
			this.chkDisableCheats.AutoSize = true;
			this.chkDisableCheats.Location = new System.Drawing.Point(7, 7);
			this.chkDisableCheats.Name = "chkDisableCheats";
			this.chkDisableCheats.Size = new System.Drawing.Size(109, 17);
			this.chkDisableCheats.TabIndex = 3;
			this.chkDisableCheats.Text = "Disable all cheats";
			this.chkDisableCheats.UseVisualStyleBackColor = true;
			this.chkDisableCheats.CheckedChanged += new System.EventHandler(this.chkDisableCheats_CheckedChanged);
			// 
			// colEnabled
			// 
			this.colEnabled.Text = "";
			this.colEnabled.Width = 21;
			// 
			// frmCheatList
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(446, 353);
			this.Controls.Add(this.tableLayoutPanel2);
			this.MinimumSize = new System.Drawing.Size(415, 316);
			this.Name = "frmCheatList";
			this.ShowInTaskbar = true;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Cheats";
			this.Controls.SetChildIndex(this.baseConfigPanel, 0);
			this.Controls.SetChildIndex(this.tableLayoutPanel2, 0);
			this.baseConfigPanel.ResumeLayout(false);
			this.baseConfigPanel.PerformLayout();
			this.tabMain.ResumeLayout(false);
			this.tabCheats.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.contextMenuCheats.ResumeLayout(false);
			this.tsCheatActions.ResumeLayout(false);
			this.tsCheatActions.PerformLayout();
			this.tableLayoutPanel2.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage tabCheats;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private MyListView lstCheats;
		private System.Windows.Forms.ColumnHeader colCheatName;
		private System.Windows.Forms.ContextMenuStrip contextMenuCheats;
		private System.Windows.Forms.ToolStripMenuItem mnuAddCheat;
		private System.Windows.Forms.ToolStripMenuItem mnuDeleteCheat;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.CheckBox chkDisableCheats;
		private ctrlMesenToolStrip tsCheatActions;
		private System.Windows.Forms.ToolStripButton btnAddCheat;
		private System.Windows.Forms.ColumnHeader colCodes;
		private System.Windows.Forms.ToolStripMenuItem mnuEditCheat;
		private System.Windows.Forms.ToolStripButton btnEditCheat;
		private System.Windows.Forms.ToolStripButton btnDeleteCheat;
		private System.Windows.Forms.ColumnHeader colEnabled;
	}
}