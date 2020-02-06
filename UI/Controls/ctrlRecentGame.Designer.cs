namespace Mesen.GUI.Controls
{
	partial class ctrlRecentGame
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
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.picPreviousState = new Mesen.GUI.Controls.GamePreviewBox();
			this.lblGameName = new System.Windows.Forms.Label();
			this.lblSaveDate = new System.Windows.Forms.Label();
			this.tableLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picPreviousState)).BeginInit();
			this.SuspendLayout();
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 1;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.Controls.Add(this.picPreviousState, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblGameName, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.lblSaveDate, 0, 2);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 3;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.Size = new System.Drawing.Size(158, 93);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// picPreviousState
			// 
			this.picPreviousState.BackColor = System.Drawing.Color.Black;
			this.picPreviousState.Cursor = System.Windows.Forms.Cursors.Hand;
			this.picPreviousState.Dock = System.Windows.Forms.DockStyle.Fill;
			this.picPreviousState.Highlight = false;
			this.picPreviousState.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
			this.picPreviousState.Location = new System.Drawing.Point(0, 0);
			this.picPreviousState.Margin = new System.Windows.Forms.Padding(0);
			this.picPreviousState.Name = "picPreviousState";
			this.picPreviousState.Size = new System.Drawing.Size(158, 64);
			this.picPreviousState.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
			this.picPreviousState.TabIndex = 11;
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
			this.lblGameName.Location = new System.Drawing.Point(3, 64);
			this.lblGameName.Name = "lblGameName";
			this.lblGameName.Size = new System.Drawing.Size(152, 16);
			this.lblGameName.TabIndex = 12;
			this.lblGameName.Text = "Game Name";
			this.lblGameName.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// lblSaveDate
			// 
			this.lblSaveDate.BackColor = System.Drawing.Color.Transparent;
			this.lblSaveDate.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lblSaveDate.ForeColor = System.Drawing.Color.White;
			this.lblSaveDate.Location = new System.Drawing.Point(3, 80);
			this.lblSaveDate.Name = "lblSaveDate";
			this.lblSaveDate.Size = new System.Drawing.Size(152, 13);
			this.lblSaveDate.TabIndex = 13;
			this.lblSaveDate.Text = "Date";
			this.lblSaveDate.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// ctrlRecentGame
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.Black;
			this.Controls.Add(this.tableLayoutPanel1);
			this.Name = "ctrlRecentGame";
			this.Size = new System.Drawing.Size(158, 93);
			this.tableLayoutPanel1.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.picPreviousState)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private GamePreviewBox picPreviousState;
		private System.Windows.Forms.Label lblGameName;
		private System.Windows.Forms.Label lblSaveDate;
	}
}
