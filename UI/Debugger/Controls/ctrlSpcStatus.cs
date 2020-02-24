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
	public partial class ctrlSpcStatus : BaseControl
	{
		private EntityBinder _binder = new EntityBinder();
		private SpcState _lastState;

		public ctrlSpcStatus()
		{
			InitializeComponent();
			if(IsDesignMode) {
				return;
			}

			_binder.Entity = new SpcState();
			_binder.AddBinding(nameof(SpcState.A), txtA);
			_binder.AddBinding(nameof(SpcState.X), txtX);
			_binder.AddBinding(nameof(SpcState.Y), txtY);
			_binder.AddBinding(nameof(SpcState.PC), txtPC);
			_binder.AddBinding(nameof(SpcState.SP), txtS);
			_binder.AddBinding(nameof(SpcState.PS), txtP);
		}

		public void UpdateStatus(SpcState state)
		{
			_lastState = state;

			_binder.Entity = state;
			_binder.UpdateUI();

			UpdateCpuFlags();
			UpdateStack();
		}

		private void UpdateCpuFlags()
		{
			SpcFlags flags = _lastState.PS;
			chkNegative.Checked = flags.HasFlag(SpcFlags.Negative);
			chkOverflow.Checked = flags.HasFlag(SpcFlags.Overflow);
			chkPage.Checked = flags.HasFlag(SpcFlags.DirectPage);
			chkBreak.Checked = flags.HasFlag(SpcFlags.Break);
			chkHalfCarry.Checked = flags.HasFlag(SpcFlags.HalfCarry);
			chkInterrupt.Checked = flags.HasFlag(SpcFlags.IrqEnable);
			chkZero.Checked = flags.HasFlag(SpcFlags.Zero);
			chkCarry.Checked = flags.HasFlag(SpcFlags.Carry);
		}

		private void UpdateStack()
		{
			StringBuilder sb = new StringBuilder();
			for(UInt32 i = (uint)_lastState.SP + 1; (i & 0xFF) != 0; i++) {
				sb.Append("$");
				sb.Append(DebugApi.GetMemoryValue(SnesMemoryType.SpcMemory, 0x100 | i).ToString("X2"));
				sb.Append(", ");
			}
			string stack = sb.ToString();
			if(stack.Length > 2) {
				stack = stack.Substring(0, stack.Length - 2);
			}
			txtStack.Text = stack;
		}
	}
}
