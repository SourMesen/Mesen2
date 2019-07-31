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
	public partial class ctrlGsuStatus : BaseControl
	{
		public ctrlGsuStatus()
		{
			InitializeComponent();
			if(IsDesignMode) {
				return;
			}
		}

		public void UpdateStatus(GsuState state)
		{
			txtR0.Text = state.R[0].ToString("X4");
			txtR1.Text = state.R[1].ToString("X4");
			txtR2.Text = state.R[2].ToString("X4");
			txtR3.Text = state.R[3].ToString("X4");
			txtR4.Text = state.R[4].ToString("X4");
			txtR5.Text = state.R[5].ToString("X4");
			txtR6.Text = state.R[6].ToString("X4");
			txtR7.Text = state.R[7].ToString("X4");
			txtR8.Text = state.R[8].ToString("X4");
			txtR9.Text = state.R[9].ToString("X4");
			txtR10.Text = state.R[10].ToString("X4");
			txtR11.Text = state.R[11].ToString("X4");
			txtR12.Text = state.R[12].ToString("X4");
			txtR13.Text = state.R[13].ToString("X4");
			txtR14.Text = state.R[14].ToString("X4");
			txtR15.Text = state.R[15].ToString("X4");

			txtProgramBank.Text = state.ProgramBank.ToString("X2");
			txtRamBank.Text = state.RamBank.ToString("X2");
			txtRomBank.Text = state.RomBank.ToString("X2");

			txtSrc.Text = state.SrcReg.ToString();
			txtDest.Text = state.DestReg.ToString();

			chkAlt1.Checked = state.SFR.Alt1;
			chkAlt2.Checked = state.SFR.Alt2;
			chkPrefix.Checked = state.SFR.Prefix;
			chkIrq.Checked = state.SFR.Irq;
			chkRunning.Checked = state.SFR.Running;
			chkNegative.Checked = state.SFR.Sign;
			chkZero.Checked = state.SFR.Zero;
			chkOverflow.Checked = state.SFR.Overflow;
			chkCarry.Checked = state.SFR.Carry;
			chkRomRead.Checked = state.SFR.RomReadPending;

			txtSFR.Text = GetFlags(ref state).ToString("X4");
		}

		private UInt16 GetFlags(ref GsuState state)
		{
			return (UInt16)(
				(state.SFR.Zero ? 2 : 0) |
				(state.SFR.Carry ? 4 : 0) |
				(state.SFR.Sign ? 8 : 0) |
				(state.SFR.Overflow ? 0x10 : 0) |
				(state.SFR.Running ? 0x20 : 0) |
				(state.SFR.RomReadPending ? 0x40 : 0) |
				(state.SFR.Alt1 ? 0x100 : 0) |
				(state.SFR.Alt2 ? 0x200 : 0) |
				(state.SFR.ImmLow ? 0x400 : 0) |
				(state.SFR.ImmHigh ? 0x800 : 0) |
				(state.SFR.Prefix ? 0x1000 : 0) |
				(state.SFR.Irq ? 0x8000 : 0)
			);
		}
	}
}
