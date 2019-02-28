namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlScrollableTextbox
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
			this.vScrollBar = new Mesen.GUI.Debugger.Controls.ctrlCodeScrollbar();
			this.panelSearch = new System.Windows.Forms.Panel();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.picCloseSearch = new System.Windows.Forms.PictureBox();
			this.picSearchNext = new System.Windows.Forms.PictureBox();
			this.picSearchPrevious = new System.Windows.Forms.PictureBox();
			this.cboSearch = new System.Windows.Forms.ComboBox();
			this.hScrollBar = new System.Windows.Forms.HScrollBar();
			this.ctrlTextbox = new Mesen.GUI.Debugger.Controls.ctrlTextbox();
			this.panelSearch.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picCloseSearch)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picSearchNext)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picSearchPrevious)).BeginInit();
			this.SuspendLayout();
			// 
			// vScrollBar
			// 
			this.vScrollBar.ColorProvider = null;
			this.vScrollBar.Dock = System.Windows.Forms.DockStyle.Right;
			this.vScrollBar.LargeChange = 20;
			this.vScrollBar.Location = new System.Drawing.Point(490, 0);
			this.vScrollBar.Maximum = 1;
			this.vScrollBar.MaximumSize = new System.Drawing.Size(18, 0);
			this.vScrollBar.MinimumSize = new System.Drawing.Size(18, 0);
			this.vScrollBar.Name = "vScrollBar";
			this.vScrollBar.Size = new System.Drawing.Size(18, 141);
			this.vScrollBar.TabIndex = 0;
			this.vScrollBar.Value = 0;
			this.vScrollBar.VisibleLineCount = 1;
			// 
			// panelSearch
			// 
			this.panelSearch.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.panelSearch.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.panelSearch.Controls.Add(this.tableLayoutPanel1);
			this.panelSearch.Location = new System.Drawing.Point(266, -1);
			this.panelSearch.Name = "panelSearch";
			this.panelSearch.Size = new System.Drawing.Size(222, 28);
			this.panelSearch.TabIndex = 2;
			this.panelSearch.Visible = false;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 4;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.picCloseSearch, 3, 0);
			this.tableLayoutPanel1.Controls.Add(this.picSearchNext, 2, 0);
			this.tableLayoutPanel1.Controls.Add(this.picSearchPrevious, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.cboSearch, 0, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 1;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(220, 26);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// picCloseSearch
			// 
			this.picCloseSearch.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.picCloseSearch.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picCloseSearch.Image = global::Mesen.GUI.Properties.Resources.Close;
			this.picCloseSearch.Location = new System.Drawing.Point(201, 5);
			this.picCloseSearch.Name = "picCloseSearch";
			this.picCloseSearch.Size = new System.Drawing.Size(16, 16);
			this.picCloseSearch.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
			this.picCloseSearch.TabIndex = 3;
			this.picCloseSearch.TabStop = false;
			this.picCloseSearch.Click += new System.EventHandler(this.picCloseSearch_Click);
			// 
			// picSearchNext
			// 
			this.picSearchNext.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.picSearchNext.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picSearchNext.Image = global::Mesen.GUI.Properties.Resources.NextArrow;
			this.picSearchNext.Location = new System.Drawing.Point(179, 5);
			this.picSearchNext.Name = "picSearchNext";
			this.picSearchNext.Size = new System.Drawing.Size(16, 16);
			this.picSearchNext.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
			this.picSearchNext.TabIndex = 2;
			this.picSearchNext.TabStop = false;
			this.picSearchNext.MouseUp += new System.Windows.Forms.MouseEventHandler(this.picSearchNext_MouseUp);
			// 
			// picSearchPrevious
			// 
			this.picSearchPrevious.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.picSearchPrevious.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picSearchPrevious.Image = global::Mesen.GUI.Properties.Resources.PreviousArrow;
			this.picSearchPrevious.Location = new System.Drawing.Point(157, 5);
			this.picSearchPrevious.Name = "picSearchPrevious";
			this.picSearchPrevious.Size = new System.Drawing.Size(16, 16);
			this.picSearchPrevious.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
			this.picSearchPrevious.TabIndex = 1;
			this.picSearchPrevious.TabStop = false;
			this.picSearchPrevious.MouseUp += new System.Windows.Forms.MouseEventHandler(this.picSearchPrevious_MouseUp);
			// 
			// cboSearch
			// 
			this.cboSearch.Dock = System.Windows.Forms.DockStyle.Fill;
			this.cboSearch.FormattingEnabled = true;
			this.cboSearch.Location = new System.Drawing.Point(3, 3);
			this.cboSearch.Name = "cboSearch";
			this.cboSearch.Size = new System.Drawing.Size(148, 21);
			this.cboSearch.TabIndex = 4;
			this.cboSearch.TextUpdate += new System.EventHandler(this.cboSearch_TextUpdate);
			this.cboSearch.KeyDown += new System.Windows.Forms.KeyEventHandler(this.cboSearch_KeyDown);
			// 
			// hScrollBar
			// 
			this.hScrollBar.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.hScrollBar.Location = new System.Drawing.Point(0, 141);
			this.hScrollBar.Name = "hScrollBar";
			this.hScrollBar.Size = new System.Drawing.Size(508, 18);
			this.hScrollBar.TabIndex = 3;
			// 
			// ctrlTextbox
			// 
			this.ctrlTextbox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlTextbox.HideSelection = false;
			this.ctrlTextbox.HorizontalScrollWidth = 0;
			this.ctrlTextbox.Location = new System.Drawing.Point(0, 0);
			this.ctrlTextbox.Name = "ctrlTextbox";
			this.ctrlTextbox.ShowCompactPrgAddresses = false;
			this.ctrlTextbox.ShowByteCode = false;
			this.ctrlTextbox.ShowAbsoluteAddreses = false;
			this.ctrlTextbox.ShowLineNumbers = true;
			this.ctrlTextbox.ShowMemoryValues = false;
			this.ctrlTextbox.ShowSingleContentLineNotes = true;
			this.ctrlTextbox.ShowSingleLineLineNumberNotes = false;
			this.ctrlTextbox.Size = new System.Drawing.Size(490, 141);
			this.ctrlTextbox.StyleProvider = null;
			this.ctrlTextbox.TabIndex = 1;
			// 
			// ctrlScrollableTextbox
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.Controls.Add(this.panelSearch);
			this.Controls.Add(this.ctrlTextbox);
			this.Controls.Add(this.vScrollBar);
			this.Controls.Add(this.hScrollBar);
			this.Name = "ctrlScrollableTextbox";
			this.Size = new System.Drawing.Size(508, 159);
			this.panelSearch.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picCloseSearch)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picSearchNext)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picSearchPrevious)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private ctrlCodeScrollbar vScrollBar;
		private ctrlTextbox ctrlTextbox;
		private System.Windows.Forms.Panel panelSearch;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.PictureBox picSearchPrevious;
		private System.Windows.Forms.PictureBox picSearchNext;
		private System.Windows.Forms.PictureBox picCloseSearch;
		private System.Windows.Forms.ComboBox cboSearch;
		private System.Windows.Forms.HScrollBar hScrollBar;
	}
}
