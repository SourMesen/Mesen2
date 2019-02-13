using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Forms;
using System.IO;
using Mesen.GUI.Controls;

namespace Mesen.GUI.Controls
{
	public partial class ctrlRiskyOption : BaseControl
	{
		public ctrlRiskyOption()
		{
			InitializeComponent();
			this.lblNotRecommended.Text = ResourceHelper.GetMessage("RiskyOptionHint");
			this.lblNotRecommended.ForeColor = SystemColors.ControlDark;
		}

		private void chkOption_CheckedChanged(object sender, EventArgs e)
		{
			this.lblNotRecommended.ForeColor = this.chkOption.Checked ? Color.Red : SystemColors.ControlDark;
		}

		[Bindable(true)]
		[Browsable(true)]
		[DesignerSerializationVisibility(DesignerSerializationVisibility.Visible)]
		[EditorBrowsable(EditorBrowsableState.Always)]
		public override string Text
		{
			get { return chkOption.Text; }
			set { chkOption.Text = value; }
		}

		public bool Checked
		{
			get { return chkOption.Checked; }
			set { chkOption.Checked = value; }
		}

		protected override Padding DefaultMargin { get { return new Padding(0); } }
	}
}
