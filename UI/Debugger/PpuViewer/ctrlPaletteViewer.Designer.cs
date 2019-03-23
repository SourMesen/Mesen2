namespace Mesen.GUI.Debugger
{
	partial class ctrlPaletteViewer
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
			this.picPalette = new Mesen.GUI.Controls.ctrlMesenPictureBox();
			((System.ComponentModel.ISupportInitialize)(this.picPalette)).BeginInit();
			this.SuspendLayout();
			// 
			// picPalette
			// 
			this.picPalette.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
			this.picPalette.Location = new System.Drawing.Point(0, 0);
			this.picPalette.MinimumSize = new System.Drawing.Size(256, 256);
			this.picPalette.Name = "picPalette";
			this.picPalette.Size = new System.Drawing.Size(256, 256);
			this.picPalette.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
			this.picPalette.TabIndex = 1;
			this.picPalette.TabStop = false;
			this.picPalette.MouseClick += new System.Windows.Forms.MouseEventHandler(this.picPalette_MouseClick);
			// 
			// ctrlPaletteViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.picPalette);
			this.Name = "ctrlPaletteViewer";
			this.Size = new System.Drawing.Size(256, 256);
			((System.ComponentModel.ISupportInitialize)(this.picPalette)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private GUI.Controls.ctrlMesenPictureBox picPalette;
	}
}
