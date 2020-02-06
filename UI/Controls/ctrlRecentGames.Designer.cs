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
			this.baseControl2 = new Mesen.GUI.Controls.BaseControl();
			this.picNextGame = new System.Windows.Forms.PictureBox();
			this.picPrevGame = new System.Windows.Forms.PictureBox();
			this.tlpGrid = new Mesen.GUI.Controls.DBTableLayoutPanel();
			this.tlpTitle = new Mesen.GUI.Controls.DBTableLayoutPanel();
			this.lblScreenTitle = new System.Windows.Forms.Label();
			this.picClose = new System.Windows.Forms.PictureBox();
			this.baseControl1 = new Mesen.GUI.Controls.BaseControl();
			this.tmrInput = new System.Windows.Forms.Timer(this.components);
			this.tlpPreviousState.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picNextGame)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picPrevGame)).BeginInit();
			this.tlpTitle.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picClose)).BeginInit();
			this.SuspendLayout();
			// 
			// tlpPreviousState
			// 
			this.tlpPreviousState.BackColor = System.Drawing.Color.Black;
			this.tlpPreviousState.ColumnCount = 3;
			this.tlpPreviousState.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpPreviousState.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpPreviousState.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpPreviousState.Controls.Add(this.baseControl2, 2, 0);
			this.tlpPreviousState.Controls.Add(this.picNextGame, 2, 1);
			this.tlpPreviousState.Controls.Add(this.picPrevGame, 0, 1);
			this.tlpPreviousState.Controls.Add(this.tlpGrid, 1, 1);
			this.tlpPreviousState.Controls.Add(this.tlpTitle, 1, 0);
			this.tlpPreviousState.Controls.Add(this.baseControl1, 0, 0);
			this.tlpPreviousState.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpPreviousState.Location = new System.Drawing.Point(0, 0);
			this.tlpPreviousState.Name = "tlpPreviousState";
			this.tlpPreviousState.RowCount = 3;
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 10F));
			this.tlpPreviousState.Size = new System.Drawing.Size(272, 236);
			this.tlpPreviousState.TabIndex = 9;
			// 
			// baseControl2
			// 
			this.baseControl2.Location = new System.Drawing.Point(242, 3);
			this.baseControl2.Name = "baseControl2";
			this.baseControl2.Size = new System.Drawing.Size(6, 6);
			this.baseControl2.TabIndex = 18;
			// 
			// picNextGame
			// 
			this.picNextGame.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picNextGame.Dock = System.Windows.Forms.DockStyle.Right;
			this.picNextGame.Image = global::Mesen.GUI.Properties.Resources.MediaPlay;
			this.picNextGame.Location = new System.Drawing.Point(242, 35);
			this.picNextGame.Name = "picNextGame";
			this.picNextGame.Size = new System.Drawing.Size(27, 188);
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
			this.picPrevGame.Location = new System.Drawing.Point(3, 35);
			this.picPrevGame.Name = "picPrevGame";
			this.picPrevGame.Size = new System.Drawing.Size(27, 188);
			this.picPrevGame.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.picPrevGame.TabIndex = 12;
			this.picPrevGame.TabStop = false;
			this.picPrevGame.MouseDown += new System.Windows.Forms.MouseEventHandler(this.picPrevGame_MouseDown);
			// 
			// tlpGrid
			// 
			this.tlpGrid.BackColor = System.Drawing.Color.Black;
			this.tlpGrid.ColumnCount = 2;
			this.tlpGrid.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpGrid.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpGrid.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpGrid.Location = new System.Drawing.Point(33, 32);
			this.tlpGrid.Margin = new System.Windows.Forms.Padding(0);
			this.tlpGrid.Name = "tlpGrid";
			this.tlpGrid.RowCount = 2;
			this.tlpGrid.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpGrid.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tlpGrid.Size = new System.Drawing.Size(206, 194);
			this.tlpGrid.TabIndex = 13;
			// 
			// tlpTitle
			// 
			this.tlpTitle.BackColor = System.Drawing.Color.Black;
			this.tlpTitle.ColumnCount = 2;
			this.tlpTitle.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpTitle.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tlpTitle.Controls.Add(this.lblScreenTitle, 0, 0);
			this.tlpTitle.Controls.Add(this.picClose, 1, 0);
			this.tlpTitle.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpTitle.Location = new System.Drawing.Point(33, 0);
			this.tlpTitle.Margin = new System.Windows.Forms.Padding(0);
			this.tlpTitle.Name = "tlpTitle";
			this.tlpTitle.RowCount = 1;
			this.tlpTitle.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpTitle.Size = new System.Drawing.Size(206, 32);
			this.tlpTitle.TabIndex = 16;
			// 
			// lblScreenTitle
			// 
			this.lblScreenTitle.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.lblScreenTitle.AutoSize = true;
			this.lblScreenTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lblScreenTitle.ForeColor = System.Drawing.Color.White;
			this.lblScreenTitle.Location = new System.Drawing.Point(58, 7);
			this.lblScreenTitle.Margin = new System.Windows.Forms.Padding(35, 0, 3, 0);
			this.lblScreenTitle.Name = "lblScreenTitle";
			this.lblScreenTitle.Size = new System.Drawing.Size(89, 18);
			this.lblScreenTitle.TabIndex = 14;
			this.lblScreenTitle.Text = "Load State";
			this.lblScreenTitle.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// picClose
			// 
			this.picClose.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.picClose.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picClose.Image = global::Mesen.GUI.Properties.Resources.CloseWhite;
			this.picClose.Location = new System.Drawing.Point(174, 0);
			this.picClose.Margin = new System.Windows.Forms.Padding(0);
			this.picClose.Name = "picClose";
			this.picClose.Padding = new System.Windows.Forms.Padding(8);
			this.picClose.Size = new System.Drawing.Size(32, 32);
			this.picClose.TabIndex = 15;
			this.picClose.TabStop = false;
			this.picClose.Click += new System.EventHandler(this.picClose_Click);
			// 
			// baseControl1
			// 
			this.baseControl1.Location = new System.Drawing.Point(3, 3);
			this.baseControl1.Name = "baseControl1";
			this.baseControl1.Size = new System.Drawing.Size(6, 6);
			this.baseControl1.TabIndex = 17;
			// 
			// tmrInput
			// 
			this.tmrInput.Interval = 50;
			this.tmrInput.Tick += new System.EventHandler(this.tmrInput_Tick);
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
			this.tlpTitle.ResumeLayout(false);
			this.tlpTitle.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picClose)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private DBTableLayoutPanel tlpPreviousState;
		private System.Windows.Forms.PictureBox picNextGame;
		private System.Windows.Forms.PictureBox picPrevGame;
		private System.Windows.Forms.Timer tmrInput;
		private DBTableLayoutPanel tlpGrid;
		private System.Windows.Forms.Label lblScreenTitle;
		private System.Windows.Forms.PictureBox picClose;
		private DBTableLayoutPanel tlpTitle;
		private BaseControl baseControl2;
		private BaseControl baseControl1;
   }
}
