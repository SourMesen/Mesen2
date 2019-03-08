namespace Mesen.GUI.Debugger
{
	partial class frmEventViewer
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

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.ctrlPpuView = new Mesen.GUI.Debugger.ctrlEventViewerPpuView();
			this.SuspendLayout();
			// 
			// ctrlPpuView
			// 
			this.ctrlPpuView.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlPpuView.Location = new System.Drawing.Point(0, 0);
			this.ctrlPpuView.Name = "ctrlPpuView";
			this.ctrlPpuView.Size = new System.Drawing.Size(945, 531);
			this.ctrlPpuView.TabIndex = 0;
			// 
			// frmEventViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(945, 531);
			this.Controls.Add(this.ctrlPpuView);
			this.Name = "frmEventViewer";
			this.Text = "Event Viewer";
			this.ResumeLayout(false);

		}

		#endregion

		private ctrlEventViewerPpuView ctrlPpuView;
	}
}