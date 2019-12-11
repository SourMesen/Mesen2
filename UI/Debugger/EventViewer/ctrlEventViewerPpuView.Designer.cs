using Mesen.GUI.Controls;

namespace Mesen.GUI.Debugger
{
	partial class ctrlEventViewerPpuView
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
			this.tmrOverlay = new System.Windows.Forms.Timer(this.components);
			this.picViewer = new Mesen.GUI.Debugger.PpuViewer.ctrlImagePanel();
			this.SuspendLayout();
			// 
			// tmrOverlay
			// 
			this.tmrOverlay.Interval = 50;
			this.tmrOverlay.Tick += new System.EventHandler(this.tmrOverlay_Tick);
			// 
			// picViewer
			// 
			this.picViewer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.picViewer.GridSizeX = 0;
			this.picViewer.GridSizeY = 0;
			this.picViewer.Image = null;
			this.picViewer.ImageScale = 1;
			this.picViewer.ImageSize = new System.Drawing.Size(0, 0);
			this.picViewer.Location = new System.Drawing.Point(0, 0);
			this.picViewer.Name = "picViewer";
			this.picViewer.Overlay = new System.Drawing.Rectangle(0, 0, 0, 0);
			this.picViewer.Selection = new System.Drawing.Rectangle(0, 0, 0, 0);
			this.picViewer.SelectionWrapPosition = 0;
			this.picViewer.Size = new System.Drawing.Size(947, 529);
			this.picViewer.TabIndex = 3;
			this.picViewer.MouseLeave += new System.EventHandler(this.picPicture_MouseLeave);
			this.picViewer.MouseMove += new System.Windows.Forms.MouseEventHandler(this.picPicture_MouseMove);
			// 
			// ctrlEventViewerPpuView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.picViewer);
			this.Name = "ctrlEventViewerPpuView";
			this.Size = new System.Drawing.Size(947, 529);
			this.ResumeLayout(false);

		}

		#endregion
		private System.Windows.Forms.Timer tmrOverlay;
		private PpuViewer.ctrlImagePanel picViewer;
	}
}
