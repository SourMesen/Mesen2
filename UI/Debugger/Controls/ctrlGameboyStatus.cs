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
	public partial class ctrlGameboyStatus : BaseControl
	{
		private EntityBinder _cpuBinder = new EntityBinder();
		private EntityBinder _ppuBinder = new EntityBinder();
		private GbState _lastState;

		public ctrlGameboyStatus()
		{
			InitializeComponent();
			if(IsDesignMode) {
				return;
			}

			_cpuBinder.Entity = new GbCpuState();
			_cpuBinder.AddBinding(nameof(GbCpuState.A), txtA);
			_cpuBinder.AddBinding(nameof(GbCpuState.B), txtB);
			_cpuBinder.AddBinding(nameof(GbCpuState.C), txtC);
			_cpuBinder.AddBinding(nameof(GbCpuState.D), txtD);
			_cpuBinder.AddBinding(nameof(GbCpuState.E), txtE);
			_cpuBinder.AddBinding(nameof(GbCpuState.Flags), txtF);
			_cpuBinder.AddBinding(nameof(GbCpuState.PC), txtPC);
			_cpuBinder.AddBinding(nameof(GbCpuState.SP), txtSP);

			_cpuBinder.AddBinding(nameof(GbCpuState.Halted), chkHalted);
			_cpuBinder.AddBinding(nameof(GbCpuState.IME), chkIme);
			_cpuBinder.AddBinding(nameof(GbCpuState.CycleCount), txtCycleCount, eNumberFormat.Decimal);

			_ppuBinder.Entity = new GbPpuState();
			_ppuBinder.AddBinding(nameof(GbPpuState.Cycle), txtCycle, eNumberFormat.Decimal);
			_ppuBinder.AddBinding(nameof(GbPpuState.Scanline), txtScanline, eNumberFormat.Decimal);
		}

		public void UpdateStatus(GbState state)
		{
			_lastState = state;

			_cpuBinder.Entity = state.Cpu;
			_cpuBinder.UpdateUI();

			txtHL.Text = ((state.Cpu.H << 8) | state.Cpu.L).ToString("X4");

			_ppuBinder.Entity = state.Ppu;
			_ppuBinder.UpdateUI();

			UpdateCpuFlags();
			UpdateStack();
		}

		private void UpdateCpuFlags()
		{
			GameboyFlags flags = (GameboyFlags)_lastState.Cpu.Flags;
			chkNegative.Checked = flags.HasFlag(GameboyFlags.AddSub);
			chkHalfCarry.Checked = flags.HasFlag(GameboyFlags.HalfCarry);
			chkZero.Checked = flags.HasFlag(GameboyFlags.Zero);
			chkCarry.Checked = flags.HasFlag(GameboyFlags.Carry);
		}

		private void UpdateStack()
		{
			StringBuilder sb = new StringBuilder();
			for(UInt32 i = (uint)_lastState.Cpu.SP + 1; (i & 0xFF) != 0; i++) {
				sb.Append("$");
				sb.Append(DebugApi.GetMemoryValue(SnesMemoryType.GameboyMemory, i).ToString("X2"));
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
