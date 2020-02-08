namespace Mesen.GUI.Debugger
{
	partial class frmProfiler
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
			_refreshManager?.Dispose();
			_notifListener?.Dispose();
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.ctrlProfiler = new Mesen.GUI.Debugger.Controls.ctrlProfiler();
			this.tabMain = new System.Windows.Forms.TabControl();
			this.tpgCpu = new System.Windows.Forms.TabPage();
			this.tpgSpc = new System.Windows.Forms.TabPage();
			this.ctrlProfilerSpc = new Mesen.GUI.Debugger.Controls.ctrlProfiler();
			this.tabMain.SuspendLayout();
			this.tpgCpu.SuspendLayout();
			this.tpgSpc.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlProfiler
			// 
			this.ctrlProfiler.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlProfiler.Location = new System.Drawing.Point(0, 0);
			this.ctrlProfiler.Margin = new System.Windows.Forms.Padding(0);
			this.ctrlProfiler.Name = "ctrlProfiler";
			this.ctrlProfiler.Size = new System.Drawing.Size(657, 359);
			this.ctrlProfiler.TabIndex = 1;
			// 
			// tabMain
			// 
			this.tabMain.Controls.Add(this.tpgCpu);
			this.tabMain.Controls.Add(this.tpgSpc);
			this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabMain.Location = new System.Drawing.Point(0, 0);
			this.tabMain.Name = "tabMain";
			this.tabMain.SelectedIndex = 0;
			this.tabMain.Size = new System.Drawing.Size(665, 385);
			this.tabMain.TabIndex = 2;
			this.tabMain.SelectedIndexChanged += new System.EventHandler(this.tabMain_SelectedIndexChanged);
			// 
			// tpgCpu
			// 
			this.tpgCpu.Controls.Add(this.ctrlProfiler);
			this.tpgCpu.Location = new System.Drawing.Point(4, 22);
			this.tpgCpu.Margin = new System.Windows.Forms.Padding(0);
			this.tpgCpu.Name = "tpgCpu";
			this.tpgCpu.Size = new System.Drawing.Size(657, 359);
			this.tpgCpu.TabIndex = 0;
			this.tpgCpu.Text = "CPU";
			this.tpgCpu.UseVisualStyleBackColor = true;
			// 
			// tpgSpc
			// 
			this.tpgSpc.Controls.Add(this.ctrlProfilerSpc);
			this.tpgSpc.Location = new System.Drawing.Point(4, 22);
			this.tpgSpc.Margin = new System.Windows.Forms.Padding(0);
			this.tpgSpc.Name = "tpgSpc";
			this.tpgSpc.Size = new System.Drawing.Size(657, 359);
			this.tpgSpc.TabIndex = 1;
			this.tpgSpc.Text = "SPC";
			this.tpgSpc.UseVisualStyleBackColor = true;
			// 
			// ctrlProfilerSpc
			// 
			this.ctrlProfilerSpc.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlProfilerSpc.Location = new System.Drawing.Point(0, 0);
			this.ctrlProfilerSpc.Margin = new System.Windows.Forms.Padding(0);
			this.ctrlProfilerSpc.Name = "ctrlProfilerSpc";
			this.ctrlProfilerSpc.Size = new System.Drawing.Size(657, 359);
			this.ctrlProfilerSpc.TabIndex = 2;
			// 
			// frmProfiler
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(665, 385);
			this.Controls.Add(this.tabMain);
			this.Name = "frmProfiler";
			this.Text = "Performance Profiler";
			this.Controls.SetChildIndex(this.tabMain, 0);
			this.tabMain.ResumeLayout(false);
			this.tpgCpu.ResumeLayout(false);
			this.tpgSpc.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private Controls.ctrlProfiler ctrlProfiler;
	  private System.Windows.Forms.TabControl tabMain;
	  private System.Windows.Forms.TabPage tpgCpu;
	  private System.Windows.Forms.TabPage tpgSpc;
	  private Controls.ctrlProfiler ctrlProfilerSpc;
   }
}