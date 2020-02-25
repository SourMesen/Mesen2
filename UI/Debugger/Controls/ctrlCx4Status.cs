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
	public partial class ctrlCx4Status : BaseControl
	{
		public ctrlCx4Status()
		{
			InitializeComponent();
			if(IsDesignMode) {
				return;
			}
		}

		public void UpdateStatus(Cx4State state)
		{
			txtR0.Text = state.Regs[0].ToString("X6");
			txtR1.Text = state.Regs[1].ToString("X6");
			txtR2.Text = state.Regs[2].ToString("X6");
			txtR3.Text = state.Regs[3].ToString("X6");
			txtR4.Text = state.Regs[4].ToString("X6");
			txtR5.Text = state.Regs[5].ToString("X6");
			txtR6.Text = state.Regs[6].ToString("X6");
			txtR7.Text = state.Regs[7].ToString("X6");
			txtR8.Text = state.Regs[8].ToString("X6");
			txtR9.Text = state.Regs[9].ToString("X6");
			txtR10.Text = state.Regs[10].ToString("X6");
			txtR11.Text = state.Regs[11].ToString("X6");
			txtR12.Text = state.Regs[12].ToString("X6");
			txtR13.Text = state.Regs[13].ToString("X6");
			txtR14.Text = state.Regs[14].ToString("X6");
			txtR15.Text = state.Regs[15].ToString("X6");

			txtPB.Text = state.PB.ToString("X4");
			txtP.Text = state.P.ToString("X4");
			txtPC.Text = state.PC.ToString("X2");

			txtA.Text = state.A.ToString("X6");
			txtMAR.Text = state.MemoryAddressReg.ToString("X6");
			txtMDR.Text = state.MemoryDataReg.ToString("X6");
			txtDPR.Text = state.DataPointerReg.ToString("X6");

			int ramBuffer = (state.RamBuffer[2] << 16) | (state.RamBuffer[1] << 8) | state.RamBuffer[0];
			txtRAM.Text = ramBuffer.ToString("X6");
			txtROM.Text = state.RomBuffer.ToString("X6");
			
			txtMult.Text = state.Mult.ToString("X12");

			chkNegative.Checked = state.Negative;
			chkZero.Checked = state.Zero;
			chkOverflow.Checked = state.Overflow;
			chkCarry.Checked = state.Carry;
			chkIrq.Checked = state.IrqFlag;
		}
	}
}
