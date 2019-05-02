using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Controls;
using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Controls;

namespace Mesen.GUI.Debugger
{
	public partial class ctrlTooltip : UserControl
	{
		private Dictionary<string, string> _values;
		private Form _parentForm;

		public ctrlTooltip()
		{
			InitializeComponent();
		}

		public void SetTooltip(Point location, Dictionary<string, string> values)
		{
			_parentForm = this.FindForm();
			_values = values;
			location.Offset(5, 5);
			Location = location;

			UpdateContentLayout();

			this.Visible = true;
		}

		private void UpdateContentLayout()
		{
			tlpMain.SuspendLayout();

			TableLayoutPanel tlpLabels = new TableLayoutPanel();
			tlpLabels.SuspendLayout();
			tlpLabels.AutoSize = true;
			this.Size = new Size(1, 1);
			tlpMain.Size = new Size(1, 1);
			tlpMain.Controls.Clear();
			tlpMain.Controls.Add(tlpLabels, 0, 0);
			int i = 0;
			int maxLabelWidth = (_parentForm.ClientSize.Width - this.Location.X - 150);
			foreach(KeyValuePair<string, string> kvp in _values) {
				tlpLabels.RowStyles.Add(new RowStyle());
				Label lbl = new Label();
				lbl.Margin = new Padding(2, 3, 2, 2);
				lbl.Text = kvp.Key + ":";
				lbl.Font = new Font(lbl.Font, FontStyle.Bold);
				lbl.AutoSize = true;
				tlpLabels.Controls.Add(lbl, 0, i);

				lbl = new ctrlAutoGrowLabel();
				lbl.Font = new Font(BaseControl.MonospaceFontFamily, 10);
				lbl.Margin = new Padding(2);
				lbl.Text = kvp.Value;
				lbl.Size = new Size(500, 10);
				tlpLabels.Controls.Add(lbl, 1, i);

				i++;
			}

			tlpLabels.ResumeLayout();
			tlpMain.ResumeLayout();

			this.Width = this.tlpMain.Width;
			if(this.Left + this.Width > _parentForm.ClientSize.Width) {
				this.Left = this.Left - this.Width - 5;
			}

			this.Height = this.tlpMain.Height;
			if(this.Height + this.Top > _parentForm.ClientSize.Height) {
				this.Top = _parentForm.ClientSize.Height - this.Height - 5;
			}

			this.BringToFront();

			panel.BackColor = SystemColors.Info;
		}
	}
}
