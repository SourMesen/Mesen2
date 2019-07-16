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
			this.picPreviousState = new Mesen.GUI.Controls.GamePreviewBox();
			this.lblGameName = new System.Windows.Forms.Label();
			this.lblSaveDate = new System.Windows.Forms.Label();
			this.picNextGame = new System.Windows.Forms.PictureBox();
			this.picPrevGame = new System.Windows.Forms.PictureBox();
			this.tmrInput = new System.Windows.Forms.Timer(this.components);
			this.tlpPreviousState.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picPreviousState)).BeginInit();
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
			this.tlpPreviousState.Controls.Add(this.picPreviousState, 1, 1);
			this.tlpPreviousState.Controls.Add(this.lblGameName, 1, 2);
			this.tlpPreviousState.Controls.Add(this.lblSaveDate, 1, 3);
			this.tlpPreviousState.Controls.Add(this.picNextGame, 2, 1);
			this.tlpPreviousState.Controls.Add(this.picPrevGame, 0, 1);
			this.tlpPreviousState.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tlpPreviousState.Location = new System.Drawing.Point(0, 0);
			this.tlpPreviousState.Name = "tlpPreviousState";
			this.tlpPreviousState.RowCount = 6;
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 10F));
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 5F));
			this.tlpPreviousState.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tlpPreviousState.Size = new System.Drawing.Size(272, 107);
			this.tlpPreviousState.TabIndex = 9;
			// 
			// picPreviousState
			// 
			this.picPreviousState.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.picPreviousState.BackColor = System.Drawing.Color.Black;
			this.picPreviousState.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picPreviousState.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
			this.picPreviousState.Location = new System.Drawing.Point(115, 10);
			this.picPreviousState.Margin = new System.Windows.Forms.Padding(0);
			this.picPreviousState.Name = "picPreviousState";
			this.picPreviousState.Size = new System.Drawing.Size(42, 42);
			this.picPreviousState.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
			this.picPreviousState.TabIndex = 7;
			this.picPreviousState.TabStop = false;
			this.picPreviousState.Visible = false;
			this.picPreviousState.Click += new System.EventHandler(this.picPreviousState_Click);
			// 
			// lblGameName
			// 
			this.lblGameName.AutoEllipsis = true;
			this.lblGameName.BackColor = System.Drawing.Color.Transparent;
			this.lblGameName.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lblGameName.ForeColor = System.Drawing.Color.White;
			this.lblGameName.Location = new System.Drawing.Point(36, 62);
			this.lblGameName.Name = "lblGameName";
			this.lblGameName.Size = new System.Drawing.Size(200, 16);
			this.lblGameName.TabIndex = 9;
			this.lblGameName.Text = "Game Name";
			this.lblGameName.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// lblSaveDate
			// 
			this.lblSaveDate.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.lblSaveDate.AutoSize = true;
			this.lblSaveDate.BackColor = System.Drawing.Color.Transparent;
			this.lblSaveDate.ForeColor = System.Drawing.Color.White;
			this.lblSaveDate.Location = new System.Drawing.Point(121, 78);
			this.lblSaveDate.Name = "lblSaveDate";
			this.lblSaveDate.Size = new System.Drawing.Size(30, 13);
			this.lblSaveDate.TabIndex = 10;
			this.lblSaveDate.Text = "Date";
			// 
			// picNextGame
			// 
			this.picNextGame.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picNextGame.Dock = System.Windows.Forms.DockStyle.Right;
			this.picNextGame.Image = global::Mesen.GUI.Properties.Resources.MediaPlay;
			this.picNextGame.Location = new System.Drawing.Point(242, 13);
			this.picNextGame.Name = "picNextGame";
			this.picNextGame.Size = new System.Drawing.Size(27, 46);
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
			this.picPrevGame.Size = new System.Drawing.Size(27, 46);
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
			// ctrlRecentGames
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.Transparent;
			this.Controls.Add(this.tlpPreviousState);
			this.Name = "ctrlRecentGames";
			this.Size = new System.Drawing.Size(272, 107);
			this.tlpPreviousState.ResumeLayout(false);
			this.tlpPreviousState.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picPreviousState)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picNextGame)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picPrevGame)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private DBTableLayoutPanel tlpPreviousState;
		private GamePreviewBox picPreviousState;
		private System.Windows.Forms.Label lblGameName;
		private System.Windows.Forms.Label lblSaveDate;
		private System.Windows.Forms.PictureBox picNextGame;
		private System.Windows.Forms.PictureBox picPrevGame;
		private System.Windows.Forms.Timer tmrInput;
	}
}
