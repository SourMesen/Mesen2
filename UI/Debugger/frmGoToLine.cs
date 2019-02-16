using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmGoToLine : BaseConfigForm
	{
		public frmGoToLine(GoToAddress address, int charLimit)
		{
			InitializeComponent();

			Entity = address;
			AddBinding("Address", txtAddress);

			txtAddress.MaxLength = charLimit;
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);
			txtAddress.Focus();
		}
	}

	public class GoToAddress
	{
		public UInt32 Address;
	}
}
