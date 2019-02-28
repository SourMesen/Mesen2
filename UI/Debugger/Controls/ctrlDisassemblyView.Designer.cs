namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlDisassemblyView
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
			this.ctrlCode = new Mesen.GUI.Debugger.Controls.ctrlScrollableTextbox();
			this.SuspendLayout();
			// 
			// ctrlCode
			// 
			this.ctrlCode.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.ctrlCode.CodeHighlightingEnabled = true;
			this.ctrlCode.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctrlCode.HideSelection = false;
			this.ctrlCode.Location = new System.Drawing.Point(0, 0);
			this.ctrlCode.Name = "ctrlCode";
			this.ctrlCode.ShowCompactPrgAddresses = false;
			this.ctrlCode.ShowContentNotes = false;
			this.ctrlCode.ShowLineNumberNotes = false;
			this.ctrlCode.ShowMemoryValues = false;
			this.ctrlCode.ShowScrollbars = true;
			this.ctrlCode.ShowSingleContentLineNotes = true;
			this.ctrlCode.ShowSingleLineLineNumberNotes = false;
			this.ctrlCode.Size = new System.Drawing.Size(465, 398);
			this.ctrlCode.TabIndex = 0;
			// 
			// ctrlDisassemblyView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.ctrlCode);
			this.Name = "ctrlDisassemblyView";
			this.Size = new System.Drawing.Size(465, 398);
			this.ResumeLayout(false);

		}

		#endregion

		private ctrlScrollableTextbox ctrlCode;
	}
}
