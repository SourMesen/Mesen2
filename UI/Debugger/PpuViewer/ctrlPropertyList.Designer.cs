namespace Mesen.GUI.Debugger
{
	partial class ctrlPropertyList
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
			this.lstProperties = new Mesen.GUI.Controls.DoubleBufferedListView();
			this.colAddress = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colValue = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.colValueHex = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.SuspendLayout();
			// 
			// lstProperties
			// 
			this.lstProperties.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colAddress,
            this.colName,
            this.colValue,
            this.colValueHex});
			this.lstProperties.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstProperties.FullRowSelect = true;
			this.lstProperties.GridLines = true;
			this.lstProperties.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
			this.lstProperties.HideSelection = false;
			this.lstProperties.Location = new System.Drawing.Point(0, 0);
			this.lstProperties.Name = "lstProperties";
			this.lstProperties.Size = new System.Drawing.Size(424, 292);
			this.lstProperties.TabIndex = 1;
			this.lstProperties.UseCompatibleStateImageBehavior = false;
			this.lstProperties.View = System.Windows.Forms.View.Details;
			this.lstProperties.VirtualMode = true;
			this.lstProperties.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.lstProperties_RetrieveVirtualItem);
			// 
			// colAddress
			// 
			this.colAddress.Text = "Address";
			this.colAddress.Width = 125;
			// 
			// colName
			// 
			this.colName.Text = "Name";
			this.colName.Width = 114;
			// 
			// colValue
			// 
			this.colValue.Text = "Value";
			this.colValue.Width = 79;
			// 
			// colValueHex
			// 
			this.colValueHex.Text = "Value (Hex)";
			this.colValueHex.Width = 90;
			// 
			// ctrlPropertyList
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.lstProperties);
			this.Name = "ctrlPropertyList";
			this.Size = new System.Drawing.Size(424, 292);
			this.ResumeLayout(false);

		}

		#endregion

		private GUI.Controls.DoubleBufferedListView lstProperties;
		private System.Windows.Forms.ColumnHeader colName;
		private System.Windows.Forms.ColumnHeader colValue;
		private System.Windows.Forms.ColumnHeader colAddress;
		private System.Windows.Forms.ColumnHeader colValueHex;
	}
}
