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
	public partial class frmInfoTooltip : TooltipForm
	{
		private Dictionary<string, string> _values;
		private int _showOnLeftOffset;

		protected override bool ShowWithoutActivation
		{
			get { return true; }
		}

		public frmInfoTooltip(Form parent, Dictionary<string, string> values, int showOnLeftOffset = 0)
		{
			_showOnLeftOffset = showOnLeftOffset;
			_parentForm = parent;
			_values = values;
			InitializeComponent();
			this.TopLevel = false;
			this.Parent = _parentForm;
			_parentForm.Controls.Add(this);
		}

		protected override void OnLoad(EventArgs e)
		{
			tlpMain.SuspendLayout();

			TableLayoutPanel tlpLabels = new TableLayoutPanel();
			tlpLabels.SuspendLayout();
			tlpLabels.AutoSize = true;
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
				if(_showOnLeftOffset == 0) {
					lbl.Size = new Size(maxLabelWidth, 10);
				} else {
					lbl.Size = new Size(500, 10);
				}
				tlpLabels.Controls.Add(lbl, 1, i);

				i++;
			}

			tlpLabels.ResumeLayout();
			tlpMain.ResumeLayout();

			base.OnLoad(e);

			this.Width = this.tlpMain.Width;
			if(this.Location.X + this.Width > _parentForm.ClientSize.Width) {
				if(_showOnLeftOffset > 0) {
					this.Left -= this.Width + _showOnLeftOffset * 2;
				} else {
					int maxWidth = Math.Max(10, _parentForm.ClientSize.Width - this.Location.X - 10);
					this.tlpMain.MaximumSize = new Size(maxWidth, _parentForm.ClientSize.Height - 10);
					this.MaximumSize = new Size(maxWidth, _parentForm.ClientSize.Height - 10);
				}
			}
			this.Height = this.tlpMain.Height;
			this.BringToFront();

			panel.BackColor = SystemColors.Info;
		}
	}
}
