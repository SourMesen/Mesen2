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
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlNecDspStatus : BaseControl
	{
		public ctrlNecDspStatus()
		{
			InitializeComponent();
			if(IsDesignMode) {
				return;
			}
		}

		public void UpdateStatus(NecDspState state)
		{
			txtTR.Text = state.TR.ToString("X4");
			txtTRB.Text = state.TRB.ToString("X4");
			txtPC.Text = state.PC.ToString("X4");
			txtSP.Text = state.SP.ToString("X2");
			txtRP.Text = state.RP.ToString("X4");
			txtDP.Text = state.DP.ToString("X4");
			txtDR.Text = state.DR.ToString("X4");
			txtSR.Text = state.SR.ToString("X4");
			txtK.Text = state.K.ToString("X4");
			txtL.Text = state.L.ToString("X4");
			txtM.Text = state.M.ToString("X4");
			txtN.Text = state.N.ToString("X4");
			txtA.Text = state.A.ToString("X4");
			txtB.Text = state.B.ToString("X4");
			
			chkCarryA.Checked = state.FlagsA.Carry;
			chkZeroA.Checked = state.FlagsA.Zero;
			chkOverflow0A.Checked = state.FlagsA.Overflow0;
			chkOverflow1A.Checked = state.FlagsA.Overflow1;
			chkSign0A.Checked = state.FlagsA.Sign0;
			chkSign1A.Checked = state.FlagsA.Sign1;

			chkCarryB.Checked = state.FlagsB.Carry;
			chkZeroB.Checked = state.FlagsB.Zero;
			chkOverflow0B.Checked = state.FlagsB.Overflow0;
			chkOverflow1B.Checked = state.FlagsB.Overflow1;
			chkSign0B.Checked = state.FlagsB.Sign0;
			chkSign1B.Checked = state.FlagsB.Sign1;
		}
	}
}
