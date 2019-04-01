namespace Mesen.GUI.Controls
{
	partial class ctrlRiskyOption
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
			this.chkOption = new System.Windows.Forms.CheckBox();
			this.lblNotRecommended = new System.Windows.Forms.Label();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.tableLayoutPanel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// chkOption
			// 
			this.chkOption.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.chkOption.AutoSize = true;
			this.chkOption.Location = new System.Drawing.Point(3, 3);
			this.chkOption.Margin = new System.Windows.Forms.Padding(3, 3, 0, 3);
			this.chkOption.Name = "chkOption";
			this.chkOption.Size = new System.Drawing.Size(86, 15);
			this.chkOption.TabIndex = 0;
			this.chkOption.Text = "Option name";
			this.chkOption.UseVisualStyleBackColor = true;
			this.chkOption.CheckedChanged += new System.EventHandler(this.chkOption_CheckedChanged);
			// 
			// lblNotRecommended
			// 
			this.lblNotRecommended.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblNotRecommended.AutoSize = true;
			this.lblNotRecommended.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lblNotRecommended.Location = new System.Drawing.Point(89, 3);
			this.lblNotRecommended.Margin = new System.Windows.Forms.Padding(0, 0, 0, 2);
			this.lblNotRecommended.Name = "lblNotRecommended";
			this.lblNotRecommended.Size = new System.Drawing.Size(98, 13);
			this.lblNotRecommended.TabIndex = 1;
			this.lblNotRecommended.Text = "(not recommended)";
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 3;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Controls.Add(this.lblNotRecommended, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.chkOption, 0, 0);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 1;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 226F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(193, 21);
			this.tableLayoutPanel1.TabIndex = 1;
			// 
			// ctrlRiskyOption
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.tableLayoutPanel1);
			this.Name = "ctrlRiskyOption";
			this.Size = new System.Drawing.Size(193, 21);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion
		private System.Windows.Forms.CheckBox chkOption;
		private System.Windows.Forms.Label lblNotRecommended;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
	}
}
