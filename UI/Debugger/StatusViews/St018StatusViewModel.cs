using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Text;

namespace Mesen.Debugger.StatusViews;

public class St018StatusViewModel : BaseConsoleStatusViewModel
{
	[Reactive] public UInt32 Reg0 { get; set; }
	[Reactive] public UInt32 Reg1 { get; set; }
	[Reactive] public UInt32 Reg2 { get; set; }
	[Reactive] public UInt32 Reg3 { get; set; }
	[Reactive] public UInt32 Reg4 { get; set; }
	[Reactive] public UInt32 Reg5 { get; set; }
	[Reactive] public UInt32 Reg6 { get; set; }
	[Reactive] public UInt32 Reg7 { get; set; }
	[Reactive] public UInt32 Reg8 { get; set; }
	[Reactive] public UInt32 Reg9 { get; set; }
	[Reactive] public UInt32 Reg10 { get; set; }
	[Reactive] public UInt32 Reg11 { get; set; }
	[Reactive] public UInt32 Reg12 { get; set; }
	[Reactive] public UInt32 Reg13 { get; set; }
	[Reactive] public UInt32 Reg14 { get; set; }
	[Reactive] public UInt32 Reg15 { get; set; }

	[Reactive] public UInt32 RegCpsr { get; set; }

	[Reactive] public ArmV3CpuMode Mode { get; set; }
	[Reactive] public string ModeString { get; set; } = ArmV3CpuMode.User.ToString();

	[Reactive] public bool FlagZero { get; set; }
	[Reactive] public bool FlagCarry { get; set; }
	[Reactive] public bool FlagNegative { get; set; }
	[Reactive] public bool FlagOverflow { get; set; }
	
	[Reactive] public bool FlagThumb { get; set; }
	[Reactive] public bool FlagIrqDisable { get; set; }
	[Reactive] public bool FlagFiqDisable { get; set; }

	[Reactive] public string StackPreview { get; set; } = "";

	public St018StatusViewModel()
	{
		this.WhenAnyValue(x => x.FlagZero, x => x.FlagCarry, x => x.FlagNegative, x => x.FlagOverflow).Subscribe(x => UpdateFlags());
		this.WhenAnyValue(x => x.FlagIrqDisable, x => x.FlagFiqDisable).Subscribe(x => UpdateFlags());
	}

	private void UpdateFlags()
	{
		RegCpsr = (UInt32)(
			(FlagNegative ? (1 << 31) : 0) |
			(FlagZero ? (1 << 30) : 0) |
			(FlagCarry ? (1 << 29) : 0) |
			(FlagOverflow ? (1 << 28) : 0) |

			(FlagIrqDisable ? (1 << 7) : 0) |
			(FlagFiqDisable ? (1 << 6) : 0) |
			(byte)Mode
		);
	}

	protected override void InternalUpdateUiState()
	{
		ArmV3CpuState cpu = DebugApi.GetCpuState<ArmV3CpuState>(CpuType.St018);
		UpdateCycleCount(cpu.CycleCount);

		Reg0 = cpu.R[0];
		Reg1 = cpu.R[1];
		Reg2 = cpu.R[2];
		Reg3 = cpu.R[3];
		Reg4 = cpu.R[4];
		Reg5 = cpu.R[5];
		Reg6 = cpu.R[6];
		Reg7 = cpu.R[7];
		Reg8 = cpu.R[8];
		Reg9 = cpu.R[9];
		Reg10 = cpu.R[10];
		Reg11 = cpu.R[11];
		Reg12 = cpu.R[12];
		Reg13 = cpu.R[13];
		Reg14 = cpu.R[14];
		Reg15 = cpu.R[15];

		FlagCarry = cpu.CPSR.Carry;
		FlagZero = cpu.CPSR.Zero;
		FlagNegative = cpu.CPSR.Negative;
		FlagOverflow = cpu.CPSR.Overflow;
		FlagIrqDisable = cpu.CPSR.IrqDisable;
		FlagFiqDisable = cpu.CPSR.FiqDisable;

		Mode = cpu.CPSR.Mode;
		if(cpu.CPSR.Mode == ArmV3CpuMode.Irq || cpu.CPSR.Mode == ArmV3CpuMode.Fiq) {
			ModeString = cpu.CPSR.Mode.ToString().ToUpper();
		} else {
			ModeString = cpu.CPSR.Mode.ToString();
		}

		StringBuilder sb = new StringBuilder();
		if(cpu.R[13] >= 0xE0000000) {
			//Patch to show work ram data directly (since APIs don't work well with addresses over 2^31)
			uint start = (cpu.R[13] - 0xE0000000) & 0x3FFF;
			uint end = (cpu.R[13] - 0xE0000000 + 30 * 4 - 1) & 0x3FFF;
			if(end < start) {
				end = 0x3FFF;
			}
			byte[] stackValues = DebugApi.GetMemoryValues(MemoryType.St018WorkRam, start, end);
			for(int i = 0; i < stackValues.Length; i += 4) {
				UInt32 value = (uint)stackValues[i] | (uint)(stackValues[i + 1] << 8) | (uint)(stackValues[i + 2] << 16) | (uint)(stackValues[i + 3] << 24);
				sb.Append($"${value:X8} ");
			}
		}
		StackPreview = sb.ToString();
	}

	protected override void InternalUpdateConsoleState()
	{
		ArmV3CpuState cpu = DebugApi.GetCpuState<ArmV3CpuState>(CpuType.St018);

		cpu.R[0] = Reg0;
		cpu.R[1] = Reg1;
		cpu.R[2] = Reg2;
		cpu.R[3] = Reg3;
		cpu.R[4] = Reg4;
		cpu.R[5] = Reg5;
		cpu.R[6] = Reg6;
		cpu.R[7] = Reg7;
		cpu.R[8] = Reg8;
		cpu.R[9] = Reg9;
		cpu.R[10] = Reg10;
		cpu.R[11] = Reg11;
		cpu.R[12] = Reg12;
		cpu.R[13] = Reg13;
		cpu.R[14] = Reg14;
		cpu.R[15] = Reg15;

		cpu.CPSR.Carry = FlagCarry;
		cpu.CPSR.Zero = FlagZero;
		cpu.CPSR.Negative = FlagNegative;
		cpu.CPSR.Overflow = FlagOverflow;
		cpu.CPSR.IrqDisable = FlagIrqDisable;
		cpu.CPSR.FiqDisable = FlagFiqDisable;

		DebugApi.SetCpuState(cpu, CpuType.St018);
	}
}
