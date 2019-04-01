namespace Mesen.GUI.Debugger.Controls
{
	partial class ctrlCallstack
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
			this.lstCallstack = new Mesen.GUI.Controls.DoubleBufferedListView();
			this.colFunctionAddress = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colStackAddr = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.SuspendLayout();
			// 
			// lstCallstack
			// 
			this.lstCallstack.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colFunctionAddress,
            this.colStackAddr});
			this.lstCallstack.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstCallstack.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lstCallstack.FullRowSelect = true;
			this.lstCallstack.GridLines = true;
			this.lstCallstack.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
			this.lstCallstack.HideSelection = false;
			this.lstCallstack.Location = new System.Drawing.Point(0, 0);
			this.lstCallstack.MultiSelect = false;
			this.lstCallstack.Name = "lstCallstack";
			this.lstCallstack.Size = new System.Drawing.Size(330, 125);
			this.lstCallstack.TabIndex = 1;
			this.lstCallstack.UseCompatibleStateImageBehavior = false;
			this.lstCallstack.View = System.Windows.Forms.View.Details;
			this.lstCallstack.DoubleClick += new System.EventHandler(this.lstCallstack_DoubleClick);
			// 
			// colFunctionAddress
			// 
			this.colFunctionAddress.Text = "Function (Entry Address)";
			this.colFunctionAddress.Width = 174;
			// 
			// colStackAddr
			// 
			this.colStackAddr.Text = "PC Address";
			this.colStackAddr.Width = 88;
			// 
			// ctrlCallstack
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.lstCallstack);
			this.Name = "ctrlCallstack";
			this.Size = new System.Drawing.Size(330, 125);
			this.ResumeLayout(false);

		}

		#endregion

		private Mesen.GUI.Controls.DoubleBufferedListView lstCallstack;
		private System.Windows.Forms.ColumnHeader colFunctionAddress;
		private System.Windows.Forms.ColumnHeader colStackAddr;
	}
}
