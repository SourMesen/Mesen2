namespace Mesen.GUI.Debugger
{
	partial class ctrlEventViewerListView
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
			this.lstEvents = new Mesen.GUI.Controls.DoubleBufferedListView();
			this.colProgramCounter = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colScanline = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colCycle = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colEventType = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colAddress = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colValue = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colDetails = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.ctxMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mnuCopy = new System.Windows.Forms.ToolStripMenuItem();
			this.ctxMenu.SuspendLayout();
			this.SuspendLayout();
			// 
			// lstEvents
			// 
			this.lstEvents.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colProgramCounter,
            this.colScanline,
            this.colCycle,
            this.colEventType,
            this.colAddress,
            this.colValue,
            this.colDetails});
			this.lstEvents.ContextMenuStrip = this.ctxMenu;
			this.lstEvents.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstEvents.FullRowSelect = true;
			this.lstEvents.Location = new System.Drawing.Point(0, 0);
			this.lstEvents.Name = "lstEvents";
			this.lstEvents.Size = new System.Drawing.Size(749, 205);
			this.lstEvents.TabIndex = 0;
			this.lstEvents.UseCompatibleStateImageBehavior = false;
			this.lstEvents.View = System.Windows.Forms.View.Details;
			this.lstEvents.VirtualMode = true;
			this.lstEvents.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.lstEvents_ColumnClick);
			this.lstEvents.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstEvents_RetrieveVirtualItem);
			// 
			// colProgramCounter
			// 
			this.colProgramCounter.Text = "PC";
			this.colProgramCounter.Width = 50;
			// 
			// colScanline
			// 
			this.colScanline.Text = "Scanline";
			// 
			// colCycle
			// 
			this.colCycle.Text = "Cycle";
			this.colCycle.Width = 45;
			// 
			// colEventType
			// 
			this.colEventType.Text = "Type";
			this.colEventType.Width = 135;
			// 
			// colAddress
			// 
			this.colAddress.Text = "Address";
			this.colAddress.Width = 65;
			// 
			// colValue
			// 
			this.colValue.Text = "Value";
			this.colValue.Width = 51;
			// 
			// colDetails
			// 
			this.colDetails.Text = "Details";
			this.colDetails.Width = 289;
			// 
			// ctxMenu
			// 
			this.ctxMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuCopy});
			this.ctxMenu.Name = "ctxMenu";
			this.ctxMenu.Size = new System.Drawing.Size(170, 48);
			// 
			// mnuCopy
			// 
			this.mnuCopy.Image = global::Mesen.GUI.Properties.Resources.Copy;
			this.mnuCopy.Name = "mnuCopy";
			this.mnuCopy.Size = new System.Drawing.Size(169, 22);
			this.mnuCopy.Text = "Copy List Content";
			this.mnuCopy.Click += new System.EventHandler(this.mnuCopy_Click);
			// 
			// ctrlEventViewerListView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.lstEvents);
			this.Name = "ctrlEventViewerListView";
			this.Size = new System.Drawing.Size(749, 205);
			this.ctxMenu.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private GUI.Controls.DoubleBufferedListView lstEvents;
		private System.Windows.Forms.ColumnHeader colScanline;
		private System.Windows.Forms.ColumnHeader colCycle;
		private System.Windows.Forms.ColumnHeader colEventType;
		private System.Windows.Forms.ColumnHeader colDetails;
		private System.Windows.Forms.ColumnHeader colAddress;
		private System.Windows.Forms.ColumnHeader colValue;
		private System.Windows.Forms.ColumnHeader colProgramCounter;
		private System.Windows.Forms.ContextMenuStrip ctxMenu;
		private System.Windows.Forms.ToolStripMenuItem mnuCopy;
	}
}
