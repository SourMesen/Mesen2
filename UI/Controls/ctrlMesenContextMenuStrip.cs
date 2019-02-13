using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Controls
{
	public class ctrlMesenContextMenuStrip : ContextMenuStrip
	{
		public ctrlMesenContextMenuStrip() : base()
		{
		}

		public ctrlMesenContextMenuStrip(IContainer container) : base(container)
		{
		}

		internal bool ProcessCommandKey(ref Message msg, Keys keyData)
		{
			return this.ProcessCmdKey(ref msg, keyData);
		}
	}
}
