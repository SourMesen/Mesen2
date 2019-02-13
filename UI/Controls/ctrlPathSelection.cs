using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Controls;

namespace Mesen.GUI.Forms.Config
{
	public partial class ctrlPathSelection : BaseControl
	{
		public ctrlPathSelection()
		{
			InitializeComponent();

			txtDisabledPath.ReadOnly = true;
			txtDisabledPath.Visible = false;
		}

		public string DisabledText
		{
			get { return txtDisabledPath.Text; }
			set { txtDisabledPath.Text = value; }
		}

		public override string Text
		{
			get { return txtPath.Text; }
			set { txtPath.Text = value; }
		}

		public new bool Enabled
		{
			get { return !txtPath.ReadOnly; }
			set
			{
				txtPath.Visible = value;
				txtDisabledPath.Visible = !value;
				tlpPath.ColumnStyles[0].SizeType = value ? SizeType.Percent : SizeType.Absolute;
				tlpPath.ColumnStyles[1].SizeType = value ? SizeType.Absolute : SizeType.Percent;
				tlpPath.ColumnStyles[0].Width = value ? 100F : 0F;
				tlpPath.ColumnStyles[1].Width = value ? 0F : 100F;
				btnBrowse.Visible = value;
			}
		}

		private void btnBrowse_Click(object sender, EventArgs e)
		{
			FolderBrowserDialog fbd = new FolderBrowserDialog();
			if(fbd.ShowDialog() == DialogResult.OK) {
				txtPath.Text = fbd.SelectedPath;
			}
		}
	}
}
