namespace Mesen.GUI.Controls
{
	partial class ctrlRecentGames
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
			this.tlpPreviousState = new Mesen.GUI.Controls.DBTableLayoutPanel();
			this.picNextGame = new System.Windows.Forms.PictureBox();
			this.picPrevGame = new System.Windows.Forms.PictureBox();
			this.tmrInput = new System.Windows.Forms.Timer(this.components);
			this.tlpGrid = new DBTableLayoutPanel();
			this.tlpPreviousState.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picNextGame)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picPrevGame)).BeginInit();
			this.SuspendLayout();
			// 
			// tlpPreviousState
			// 
			this.tlpPreviousState.BackColor = System.Drawing.Color.Black;
			this.tlpPreviousState.ColumnCount = 3;
			this.tlpPreviousState.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpPreviousState.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpPreviousState.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpPreviousState.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpPreviousState.Controls.Add(this.picNextGame, 2, 1);
			this.tlpPreviousState.Controls.Add(this.picPrevGame, 0, 1);
			this.tlpPreviousState.Controls.Add(this.tlpGrid, 1, 1);
			this.tlpPreviousState.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpPreviousState.Location = new System.Drawing.Point(0, 0);
			this.tlpPreviousState.Name = "tlpPreviousState";
			this.tlpPreviousState.RowCount = 3;
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 10F));
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tlpPreviousState.Size = new System.Drawing.Size(272, 236);
			this.tlpPreviousState.TabIndex = 9;
			// 
			// picNextGame
			// 
			this.picNextGame.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picNextGame.Dock = System.Windows.Forms.DockStyle.Right;
			this.picNextGame.Image = global::Mesen.GUI.Properties.Resources.MediaPlay;
			this.picNextGame.Location = new System.Drawing.Point(242, 13);
			this.picNextGame.Name = "picNextGame";
			this.picNextGame.Size = new System.Drawing.Size(27, 200);
			this.picNextGame.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.picNextGame.TabIndex = 11;
			this.picNextGame.TabStop = false;
			this.picNextGame.MouseDown += new System.Windows.Forms.MouseEventHandler(this.picNextGame_MouseDown);
			// 
			// picPrevGame
			// 
			this.picPrevGame.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picPrevGame.Dock = System.Windows.Forms.DockStyle.Left;
			this.picPrevGame.Image = global::Mesen.GUI.Properties.Resources.MediaPlay;
			this.picPrevGame.Location = new System.Drawing.Point(3, 13);
			this.picPrevGame.Name = "picPrevGame";
			this.picPrevGame.Size = new System.Drawing.Size(27, 200);
			this.picPrevGame.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.picPrevGame.TabIndex = 12;
			this.picPrevGame.TabStop = false;
			this.picPrevGame.MouseDown += new System.Windows.Forms.MouseEventHandler(this.picPrevGame_MouseDown);
			// 
			// tmrInput
			// 
			this.tmrInput.Interval = 50;
			this.tmrInput.Tick += new System.EventHandler(this.tmrInput_Tick);
			// 
			// tlpGrid
			// 
			this.tlpGrid.ColumnCount = 2;
			this.tlpGrid.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpGrid.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpGrid.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpGrid.Location = new System.Drawing.Point(33, 10);
			this.tlpGrid.Margin = new System.Windows.Forms.Padding(0, 0, 0, 0);
			this.tlpGrid.Name = "tlpGrid";
			this.tlpGrid.RowCount = 2;
			this.tlpGrid.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpGrid.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpGrid.Size = new System.Drawing.Size(206, 206);
			this.tlpGrid.TabIndex = 13;
			// 
			// ctrlRecentGames
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.Transparent;
			this.Controls.Add(this.tlpPreviousState);
			this.Name = "ctrlRecentGames";
			this.Size = new System.Drawing.Size(272, 236);
			this.tlpPreviousState.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.picNextGame)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picPrevGame)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private DBTableLayoutPanel tlpPreviousState;
		private System.Windows.Forms.PictureBox picNextGame;
		private System.Windows.Forms.PictureBox picPrevGame;
		private System.Windows.Forms.Timer tmrInput;
		private DBTableLayoutPanel tlpGrid;
	}
}
