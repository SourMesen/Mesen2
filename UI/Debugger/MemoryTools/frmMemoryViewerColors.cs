using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmMemoryToolsColors : BaseConfigForm
	{
		public frmMemoryToolsColors()
		{
			InitializeComponent();

			Entity = ConfigManager.Config.Debug.HexEditor;

			AddBinding(nameof(HexEditorInfo.ReadColor), picRead);
			AddBinding(nameof(HexEditorInfo.WriteColor), picWrite);
			AddBinding(nameof(HexEditorInfo.ExecColor), picExecute);
			AddBinding(nameof(HexEditorInfo.LabelledByteColor), picLabelledByte);
			AddBinding(nameof(HexEditorInfo.CodeByteColor), picCodeByte);
			AddBinding(nameof(HexEditorInfo.DataByteColor), picDataByte);
		}

		private void btnReset_Click(object sender, EventArgs e)
		{
			picRead.BackColor = Color.Blue;
			picWrite.BackColor = Color.Red;
			picExecute.BackColor = Color.Green;
			picLabelledByte.BackColor = Color.LightPink;
			picCodeByte.BackColor = Color.DarkSeaGreen;
			picDataByte.BackColor = Color.LightSteelBlue;
		}
	}
}
