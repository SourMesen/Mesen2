namespace Mesen.GUI.Debugger.PpuViewer
{
	partial class ctrlImagePanel
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
			this.ctrlPanel = new Mesen.GUI.Debugger.Controls.ctrlPanel();
			this.ctrlImageViewer = new Mesen.GUI.Debugger.PpuViewer.ctrlImageViewer();
			this.ctrlPanel.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlPanel
			// 
			this.ctrlPanel.AutoScroll = true;
			this.ctrlPanel.Controls.Add(this.ctrlImageViewer);
			this.ctrlPanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlPanel.Location = new System.Drawing.Point(0, 0);
			this.ctrlPanel.Name = "ctrlPanel";
			this.ctrlPanel.Size = new System.Drawing.Size(327, 326);
			this.ctrlPanel.TabIndex = 0;
			// 
			// ctrlImageViewer
			// 
			this.ctrlImageViewer.Image = null;
			this.ctrlImageViewer.ImageScale = 1;
			this.ctrlImageViewer.Location = new System.Drawing.Point(0, 0);
			this.ctrlImageViewer.Name = "ctrlImageViewer";
			this.ctrlImageViewer.Selection = new System.Drawing.Rectangle(0, 0, 0, 0);
			this.ctrlImageViewer.Size = new System.Drawing.Size(327, 326);
			this.ctrlImageViewer.TabIndex = 0;
			this.ctrlImageViewer.Text = "ctrlImageViewer";
			// 
			// ctrlImagePanel
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.ctrlPanel);
			this.Name = "ctrlImagePanel";
			this.Size = new System.Drawing.Size(327, 326);
			this.ctrlPanel.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private Controls.ctrlPanel ctrlPanel;
		private ctrlImageViewer ctrlImageViewer;
	}
}
