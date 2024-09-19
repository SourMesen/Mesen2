using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Text;
using System.Text.RegularExpressions;

namespace Mesen.Debugger.StatusViews;

public class WsStatusViewModel : BaseConsoleStatusViewModel
{
	[Reactive] public UInt16 RegAX { get; set; }
	[Reactive] public UInt16 RegBX { get; set; }
	[Reactive] public UInt16 RegCX { get; set; }
	[Reactive] public UInt16 RegDX { get; set; }
	[Reactive] public UInt16 RegFlags { get; set; }

	[Reactive] public UInt16 RegSS { get; set; }
	[Reactive] public UInt16 RegDS { get; set; }
	[Reactive] public UInt16 RegES { get; set; }

	[Reactive] public UInt16 RegCS { get; set; }
	[Reactive] public UInt16 RegIP { get; set; }
	
	[Reactive] public UInt16 RegDI { get; set; }
	[Reactive] public UInt16 RegSI { get; set; }

	[Reactive] public UInt16 RegSP { get; set; }
	[Reactive] public UInt16 RegBP { get; set; }

	[Reactive] public UInt16 Scanline { get; set; }
	[Reactive] public UInt16 Cycle { get; set; }

	[Reactive] public bool FlagCarry { get; set; }
	[Reactive] public bool FlagAuxCarry { get; set; }
	[Reactive] public bool FlagParity { get; set; }
	[Reactive] public bool FlagSign { get; set; }
	[Reactive] public bool FlagZero { get; set; }
	[Reactive] public bool FlagOverflow { get; set; }
	[Reactive] public bool FlagTrap { get; set; }
	[Reactive] public bool FlagIrq { get; set; }
	[Reactive] public bool FlagDirection { get; set; }
	[Reactive] public bool FlagMode { get; set; }

	[Reactive] public bool FlagHalted { get; set; }

	[Reactive] public string StackPreview { get; private set; } = "";

	public WsStatusViewModel()
	{
		this.WhenAnyValue(x => x.FlagZero, x => x.FlagCarry, x => x.FlagSign, x => x.FlagOverflow).Subscribe(x => UpdateFlags());
		this.WhenAnyValue(x => x.FlagParity, x => x.FlagIrq, x => x.FlagTrap, x => x.FlagMode).Subscribe(x => UpdateFlags());
		this.WhenAnyValue(x => x.FlagAuxCarry, x => x.FlagDirection).Subscribe(x => UpdateFlags());

		this.WhenAnyValue(x => x.RegFlags).Subscribe(f => {
			using var delayNotifs = DelayChangeNotifications(); //don't reupdate RegFlags while updating the flags
			FlagCarry = (f & 0x01) != 0;
			FlagParity = (f & 0x04) != 0;
			FlagAuxCarry = (f & 0x10) != 0;
			FlagZero = (f & 0x40) != 0;
			FlagSign = (f & 0x80) != 0;
			FlagTrap = (f & 0x100) != 0;
			FlagIrq = (f & 0x200) != 0;
			FlagDirection = (f & 0x400) != 0;
			FlagOverflow = (f & 0x800) != 0;
			FlagMode = (f & 0x8000) != 0;
		});
	}

	private void UpdateFlags()
	{
		RegFlags = (UInt16)(
			(FlagCarry ? 0x01 : 0) |
			(FlagParity ? 0x04 : 0) |
			(FlagAuxCarry ? 0x10 : 0) |
			(FlagZero ? 0x40 : 0) |
			(FlagSign ? 0x80 : 0) |
			(FlagTrap ? 0x100 : 0) |
			(FlagIrq ? 0x200 : 0) |
			(FlagDirection ? 0x400 : 0) |
			(FlagOverflow ? 0x800 : 0) |
			(FlagMode ? 0x8000 : 0) |
			0x7002
		);
	}

	protected override void InternalUpdateUiState()
	{
		WsCpuState cpu = DebugApi.GetCpuState<WsCpuState>(CpuType.Ws);
		WsPpuState ppu = DebugApi.GetPpuState<WsPpuState>(CpuType.Ws);

		UpdateCycleCount(cpu.CycleCount);

		RegAX = cpu.AX;
		RegBX = cpu.BX;
		RegCX = cpu.CX;
		RegDX = cpu.DX;

		FlagCarry = cpu.Flags.Carry;
		FlagAuxCarry = cpu.Flags.AuxCarry;
		FlagZero = cpu.Flags.Zero;
		FlagSign = cpu.Flags.Sign;
		FlagParity = cpu.Flags.Parity;
		FlagTrap = cpu.Flags.Trap;
		FlagIrq = cpu.Flags.Irq;
		FlagDirection = cpu.Flags.Direction;
		FlagOverflow = cpu.Flags.Overflow;
		FlagMode = cpu.Flags.Mode;

		RegSI = cpu.SI;
		RegDI = cpu.DI;
		
		RegIP = cpu.IP;
		RegCS = cpu.CS;
		
		RegSS = cpu.SS;
		RegDS = cpu.DS;
		RegES = cpu.ES;

		RegSP = cpu.SP;
		RegBP = cpu.BP;

		FlagHalted = cpu.Halted;

		Scanline = ppu.Scanline;
		Cycle = ppu.Cycle;

		StringBuilder sb = new StringBuilder();
		UInt32 stackAddr = (UInt32)((cpu.SS << 4) + cpu.SP);
		byte[] stackValues = DebugApi.GetMemoryValues(MemoryType.WsMemory, stackAddr, stackAddr + 30 * 4 - 1);
		for(int i = 0; i < stackValues.Length; i += 2) {
			UInt16 value = (UInt16)(stackValues[i] | (stackValues[i + 1] << 8));
			sb.Append($"${value:X4} ");
		}
		StackPreview = sb.ToString();
	}

	protected override void InternalUpdateConsoleState()
	{
		WsCpuState cpu = DebugApi.GetCpuState<WsCpuState>(CpuType.Ws);

		cpu.AX = RegAX;
		cpu.BX = RegBX;
		cpu.CX = RegCX;
		cpu.DX = RegDX;

		cpu.Flags.Carry = FlagCarry;
		cpu.Flags.AuxCarry = FlagAuxCarry;
		cpu.Flags.Zero = FlagZero;
		cpu.Flags.Sign = FlagSign;
		cpu.Flags.Parity = FlagParity;
		cpu.Flags.Trap = FlagTrap;
		cpu.Flags.Irq = FlagIrq;
		cpu.Flags.Direction = FlagDirection;
		cpu.Flags.Overflow = FlagOverflow;
		cpu.Flags.Mode = FlagMode;

		cpu.SI = RegSI;
		cpu.DI = RegDI;

		cpu.IP = RegIP;
		cpu.CS = RegCS;

		cpu.SS = RegSS;
		cpu.DS = RegDS;
		cpu.ES = RegES;

		cpu.SP = RegSP;
		cpu.BP = RegBP;

		cpu.Halted = FlagHalted;

		DebugApi.SetCpuState(cpu, CpuType.Ws);
	}
}
