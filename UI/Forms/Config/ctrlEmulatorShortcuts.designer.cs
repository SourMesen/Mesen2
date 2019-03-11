namespace Mesen.GUI.Forms.Config
{
	partial class ctrlEmulatorShortcuts
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
			this.gridShortcuts = new System.Windows.Forms.DataGridView();
			this.colAction = new System.Windows.Forms.DataGridViewTextBoxColumn();
			this.colBinding1 = new System.Windows.Forms.DataGridViewButtonColumn();
			this.colBinding2 = new System.Windows.Forms.DataGridViewButtonColumn();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.pnlConflictWarning = new System.Windows.Forms.Panel();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.picWarning = new System.Windows.Forms.PictureBox();
			this.lblShortcutWarning = new System.Windows.Forms.Label();
			((System.ComponentModel.ISupportInitialize)(this.gridShortcuts)).BeginInit();
			this.tableLayoutPanel1.SuspendLayout();
			this.pnlConflictWarning.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picWarning)).BeginInit();
			this.SuspendLayout();
			// 
			// gridShortcuts
			// 
			this.gridShortcuts.AllowUserToAddRows = false;
			this.gridShortcuts.AllowUserToDeleteRows = false;
			this.gridShortcuts.AllowUserToResizeColumns = false;
			this.gridShortcuts.AllowUserToResizeRows = false;
			this.gridShortcuts.BackgroundColor = System.Drawing.SystemColors.ControlLightLight;
			this.gridShortcuts.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
			this.gridShortcuts.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.colAction,
            this.colBinding1,
            this.colBinding2});
			this.gridShortcuts.Dock = System.Windows.Forms.DockStyle.Fill;
			this.gridShortcuts.Location = new System.Drawing.Point(0, 32);
			this.gridShortcuts.Margin = new System.Windows.Forms.Padding(0);
			this.gridShortcuts.MultiSelect = false;
			this.gridShortcuts.Name = "gridShortcuts";
			this.gridShortcuts.RowHeadersVisible = false;
			this.gridShortcuts.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.gridShortcuts.Size = new System.Drawing.Size(448, 216);
			this.gridShortcuts.TabIndex = 2;
			this.gridShortcuts.CellMouseDown += new System.Windows.Forms.DataGridViewCellMouseEventHandler(this.gridShortcuts_CellMouseDown);
			// 
			// colAction
			// 
			this.colAction.HeaderText = "Action";
			this.colAction.Name = "colAction";
			this.colAction.ReadOnly = true;
			this.colAction.Resizable = System.Windows.Forms.DataGridViewTriState.False;
			this.colAction.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
			this.colAction.Width = 233;
			// 
			// colBinding1
			// 
			this.colBinding1.HeaderText = "Binding #1";
			this.colBinding1.Name = "colBinding1";
			this.colBinding1.Resizable = System.Windows.Forms.DataGridViewTriState.False;
			this.colBinding1.Width = 110;
			// 
			// colBinding2
			// 
			this.colBinding2.HeaderText = "Binding #2";
			this.colBinding2.Name = "colBinding2";
			this.colBinding2.Resizable = System.Windows.Forms.DataGridViewTriState.False;
			this.colBinding2.Width = 110;
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 1;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.pnlConflictWarning, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.gridShortcuts, 0, 1);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 2;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(448, 248);
			this.tableLayoutPanel1.TabIndex = 3;
			// 
			// pnlConflictWarning
			// 
			this.pnlConflictWarning.BackColor = System.Drawing.Color.WhiteSmoke;
			this.pnlConflictWarning.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.pnlConflictWarning.Controls.Add(this.tableLayoutPanel2);
			this.pnlConflictWarning.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pnlConflictWarning.Location = new System.Drawing.Point(0, 0);
			this.pnlConflictWarning.Margin = new System.Windows.Forms.Padding(0, 0, 0, 2);
			this.pnlConflictWarning.Name = "pnlConflictWarning";
			this.pnlConflictWarning.Size = new System.Drawing.Size(448, 30);
			this.pnlConflictWarning.TabIndex = 20;
			this.pnlConflictWarning.Visible = false;
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 2;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Controls.Add(this.picWarning, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.lblShortcutWarning, 1, 0);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 1;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(446, 28);
			this.tableLayoutPanel2.TabIndex = 0;
			// 
			// picWarning
			// 
			this.picWarning.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.picWarning.Image = global::Mesen.GUI.Properties.Resources.Warning;
			this.picWarning.Location = new System.Drawing.Point(3, 6);
			this.picWarning.Name = "picWarning";
			this.picWarning.Size = new System.Drawing.Size(16, 16);
			this.picWarning.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
			this.picWarning.TabIndex = 0;
			this.picWarning.TabStop = false;
			// 
			// lblShortcutWarning
			// 
			this.lblShortcutWarning.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lblShortcutWarning.Location = new System.Drawing.Point(25, 0);
			this.lblShortcutWarning.Name = "lblShortcutWarning";
			this.lblShortcutWarning.Size = new System.Drawing.Size(418, 28);
			this.lblShortcutWarning.TabIndex = 1;
			this.lblShortcutWarning.Text = "Warning: Your current configuration contains conflicting key bindings. If this is not intentional, please review and correct your key bindings.";
			// 
			// ctrlEmulatorShortcuts
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.tableLayoutPanel1);
			this.Name = "ctrlEmulatorShortcuts";
			this.Size = new System.Drawing.Size(448, 248);
			((System.ComponentModel.ISupportInitialize)(this.gridShortcuts)).EndInit();
			this.tableLayoutPanel1.ResumeLayout(false);
			this.pnlConflictWarning.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picWarning)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion
		private System.Windows.Forms.DataGridView gridShortcuts;
		private System.Windows.Forms.DataGridViewTextBoxColumn colAction;
		private System.Windows.Forms.DataGridViewButtonColumn colBinding1;
		private System.Windows.Forms.DataGridViewButtonColumn colBinding2;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Panel pnlConflictWarning;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.PictureBox picWarning;
		private System.Windows.Forms.Label lblShortcutWarning;
	}
}
