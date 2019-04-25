namespace Mesen.GUI.Debugger.PpuViewer
{
	partial class ctrlSpriteList
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
			this.chkHideOffscreenSprites = new System.Windows.Forms.CheckBox();
			this.lstSprites = new Mesen.GUI.Controls.DoubleBufferedListView();
			this.colIndex = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colX = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colY = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colSize = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colTile = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colPriority = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colPalette = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colFlags = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.SuspendLayout();
			// 
			// chkHideOffscreenSprites
			// 
			this.chkHideOffscreenSprites.AutoSize = true;
			this.chkHideOffscreenSprites.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.chkHideOffscreenSprites.Location = new System.Drawing.Point(0, 360);
			this.chkHideOffscreenSprites.Name = "chkHideOffscreenSprites";
			this.chkHideOffscreenSprites.Size = new System.Drawing.Size(401, 17);
			this.chkHideOffscreenSprites.TabIndex = 2;
			this.chkHideOffscreenSprites.Text = "Hide off-screen sprites";
			this.chkHideOffscreenSprites.UseVisualStyleBackColor = true;
			this.chkHideOffscreenSprites.Click += new System.EventHandler(this.chkHideOffscreenSprites_Click);
			// 
			// lstSprites
			// 
			this.lstSprites.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colIndex,
            this.colX,
            this.colY,
            this.colSize,
            this.colTile,
            this.colPriority,
            this.colPalette,
            this.colFlags});
			this.lstSprites.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstSprites.FullRowSelect = true;
			this.lstSprites.GridLines = true;
			this.lstSprites.HideSelection = false;
			this.lstSprites.Location = new System.Drawing.Point(0, 0);
			this.lstSprites.MultiSelect = false;
			this.lstSprites.Name = "lstSprites";
			this.lstSprites.Size = new System.Drawing.Size(401, 360);
			this.lstSprites.TabIndex = 1;
			this.lstSprites.UseCompatibleStateImageBehavior = false;
			this.lstSprites.View = System.Windows.Forms.View.Details;
			this.lstSprites.VirtualMode = true;
			this.lstSprites.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.lstSprites_ColumnClick);
			this.lstSprites.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstSprites_RetrieveVirtualItem);
			this.lstSprites.SelectedIndexChanged += new System.EventHandler(this.lstSprites_SelectedIndexChanged);
			// 
			// colIndex
			// 
			this.colIndex.Text = "#";
			this.colIndex.Width = 35;
			// 
			// colX
			// 
			this.colX.Text = "X";
			this.colX.Width = 40;
			// 
			// colY
			// 
			this.colY.Text = "Y";
			this.colY.Width = 40;
			// 
			// colSize
			// 
			this.colSize.Text = "Size";
			this.colSize.Width = 45;
			// 
			// colTile
			// 
			this.colTile.Text = "Tile";
			this.colTile.Width = 40;
			// 
			// colPriority
			// 
			this.colPriority.Text = "Prio";
			this.colPriority.Width = 30;
			// 
			// colPalette
			// 
			this.colPalette.Text = "Pal";
			this.colPalette.Width = 30;
			// 
			// colFlags
			// 
			this.colFlags.Text = "Flags";
			this.colFlags.Width = 40;
			// 
			// ctrlSpriteList
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.lstSprites);
			this.Controls.Add(this.chkHideOffscreenSprites);
			this.Name = "ctrlSpriteList";
			this.Size = new System.Drawing.Size(401, 377);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private GUI.Controls.DoubleBufferedListView lstSprites;
		private System.Windows.Forms.ColumnHeader colIndex;
		private System.Windows.Forms.ColumnHeader colX;
		private System.Windows.Forms.ColumnHeader colY;
		private System.Windows.Forms.ColumnHeader colSize;
		private System.Windows.Forms.ColumnHeader colPriority;
		private System.Windows.Forms.ColumnHeader colPalette;
		private System.Windows.Forms.ColumnHeader colFlags;
		private System.Windows.Forms.ColumnHeader colTile;
		private System.Windows.Forms.CheckBox chkHideOffscreenSprites;
	}
}
