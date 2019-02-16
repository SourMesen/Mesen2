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
	public partial class frmFadeSpeed : BaseConfigForm
	{
		public frmFadeSpeed(int initialValue)
		{
			InitializeComponent();
			nudFrameCount.Value = initialValue;
		}

		public int FadeSpeed
		{
			get { return (int)nudFrameCount.Value; }
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);
			nudFrameCount.Focus();
		}
	}
}
