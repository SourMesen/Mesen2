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
	public partial class frmDebuggerColors : BaseConfigForm
	{
		public frmDebuggerColors()
		{
			InitializeComponent();

			Entity = ConfigManager.Config.Debug.Debugger;
			AddBinding(nameof(DebuggerInfo.CodeActiveStatementColor), picActiveStatement);
			AddBinding(nameof(DebuggerInfo.CodeAddressColor), picAddress);
			AddBinding(nameof(DebuggerInfo.CodeCommentColor), picComment);
			AddBinding(nameof(DebuggerInfo.CodeEffectiveAddressColor), picEffectiveAddress);
			AddBinding(nameof(DebuggerInfo.CodeExecBreakpointColor), picExecBreakpoint);
			AddBinding(nameof(DebuggerInfo.CodeImmediateColor), picImmediate);
			AddBinding(nameof(DebuggerInfo.CodeLabelDefinitionColor), picLabelDefinition);
			AddBinding(nameof(DebuggerInfo.CodeOpcodeColor), picOpcode);
			AddBinding(nameof(DebuggerInfo.CodeReadBreakpointColor), picReadBreakpoint);
			AddBinding(nameof(DebuggerInfo.CodeUnexecutedCodeColor), picUnexecutedCode);
			AddBinding(nameof(DebuggerInfo.CodeUnidentifiedDataColor), picUnidentifiedData);
			AddBinding(nameof(DebuggerInfo.CodeVerifiedDataColor), picVerifiedData);
			AddBinding(nameof(DebuggerInfo.CodeWriteBreakpointColor), picWriteBreakpoint);
		}

		private void btnReset_Click(object sender, EventArgs e)
		{
			picVerifiedData.BackColor = Color.FromArgb(255, 252, 236);
			picUnidentifiedData.BackColor = Color.FromArgb(255, 242, 242);
			picUnexecutedCode.BackColor = Color.FromArgb(225, 244, 228);

			picExecBreakpoint.BackColor = Color.FromArgb(140, 40, 40);
			picWriteBreakpoint.BackColor = Color.FromArgb(40, 120, 80);
			picReadBreakpoint.BackColor = Color.FromArgb(40, 40, 200);
			picActiveStatement.BackColor = Color.Yellow;
			picEffectiveAddress.BackColor = Color.SteelBlue;

			picOpcode.BackColor = Color.FromArgb(22, 37, 37);
			picLabelDefinition.BackColor = Color.Blue;
			picImmediate.BackColor = Color.Chocolate;
			picAddress.BackColor = Color.DarkRed;
			picComment.BackColor = Color.Green;
		}
	}
}
