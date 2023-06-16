using Avalonia.Collections;
using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Text;

namespace Mesen.Debugger.StatusViews
{
	public class GsuStatusViewModel : BaseConsoleStatusViewModel
	{
		[Reactive] public UInt16 Reg0 { get; set; }
		[Reactive] public UInt16 Reg1 { get; set; }
		[Reactive] public UInt16 Reg2 { get; set; }
		[Reactive] public UInt16 Reg3 { get; set; }
		[Reactive] public UInt16 Reg4 { get; set; }
		[Reactive] public UInt16 Reg5 { get; set; }
		[Reactive] public UInt16 Reg6 { get; set; }
		[Reactive] public UInt16 Reg7 { get; set; }
		[Reactive] public UInt16 Reg8 { get; set; }
		[Reactive] public UInt16 Reg9 { get; set; }
		[Reactive] public UInt16 Reg10 { get; set; }
		[Reactive] public UInt16 Reg11 { get; set; }
		[Reactive] public UInt16 Reg12 { get; set; }
		[Reactive] public UInt16 Reg13 { get; set; }
		[Reactive] public UInt16 Reg14 { get; set; }
		[Reactive] public UInt16 Reg15 { get; set; }

		[Reactive] public UInt16 RegSfr { get; set; }
		[Reactive] public UInt16 RamAddrCache { get; set; }

		[Reactive] public byte RegSrc { get; set; }
		[Reactive] public byte RegDst { get; set; }
		[Reactive] public byte RegColor { get; set; }
		[Reactive] public byte RegPor { get; set;}

		[Reactive] public byte RegPbr { get; set; }
		[Reactive] public byte RomBank { get; set; }
		[Reactive] public byte RamBank { get; set; }

		[Reactive] public bool FlagZero { get; set; }
		[Reactive] public bool FlagCarry { get; set; }
		[Reactive] public bool FlagSign { get; set; }
		[Reactive] public bool FlagOverflow { get; set; }
		
		[Reactive] public bool FlagAlt1 { get; set; }
		[Reactive] public bool FlagAlt2 { get; set; }
		[Reactive] public bool FlagIrq { get; set; }
		[Reactive] public bool FlagRomReadPending { get; set; }

		[Reactive] public bool FlagRunning { get; set; }
		[Reactive] public bool FlagImmLow { get; set; }
		[Reactive] public bool FlagImmHigh { get; set; }
		[Reactive] public bool FlagPrefix { get; set; }

		[Reactive] public bool FlagPlotTransparent { get; set; }
		[Reactive] public bool FlagPlotDither { get; set; }
		[Reactive] public bool FlagColorHighNibble { get; set; }
		[Reactive] public bool FlagColorFreezeHigh { get; set; }
		[Reactive] public bool FlagObjMode { get; set; }

		public GsuStatusViewModel()
		{
			this.WhenAnyValue(x => x.FlagZero, x => x.FlagCarry, x => x.FlagSign, x => x.FlagOverflow).Subscribe(x => UpdateSfrValue());
			this.WhenAnyValue(x => x.FlagAlt1, x => x.FlagAlt2, x => x.FlagIrq, x => x.FlagRomReadPending).Subscribe(x => UpdateSfrValue());
			this.WhenAnyValue(x => x.FlagRunning, x => x.FlagImmLow, x => x.FlagImmHigh, x => x.FlagPrefix).Subscribe(x => UpdateSfrValue());

			this.WhenAnyValue(x => x.FlagPlotTransparent, x => x.FlagPlotDither, x => x.FlagColorHighNibble).Subscribe(x => UpdatePorValue());
			this.WhenAnyValue(x => x.FlagColorFreezeHigh, x => x.FlagObjMode).Subscribe(x => UpdatePorValue());
		}

		private void UpdateSfrValue()
		{
			RegSfr = (UInt16)(
				(FlagZero ? 0x02 : 0) |
				(FlagCarry ? 0x04 : 0) |
				(FlagSign ? 0x08 : 0) |
				(FlagOverflow ? 0x10 : 0) |
				(FlagRunning ? 0x20 : 0) |
				(FlagRomReadPending ? 0x40 : 0) |
				(FlagAlt1 ? 0x100 : 0) |
				(FlagAlt2 ? 0x200 : 0) |
				(FlagImmLow ? 0x400 : 0) |
				(FlagImmHigh ? 0x800 : 0) |
				(FlagPrefix ? 0x1000 : 0) |
				(FlagIrq ? 0x8000 : 0)
			);
		}

		private void UpdatePorValue()
		{
			RegPor = (byte)(
				(FlagPlotTransparent ? 0x01 : 0) |
				(FlagPlotDither ? 0x02 : 0) |
				(FlagColorHighNibble ? 0x04 : 0) |
				(FlagColorFreezeHigh ? 0x08 : 0) |
				(FlagObjMode ? 0x10 : 0)
			);
		}

		protected override void InternalUpdateUiState()
		{
			GsuState cpu = DebugApi.GetCpuState<GsuState>(CpuType.Gsu);

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

			RegSrc = cpu.SrcReg;
			RegDst = cpu.DestReg;
			RegColor = cpu.ColorReg;
			RegPbr = cpu.ProgramBank;
			RomBank = cpu.RomBank;
			RamBank = cpu.RamBank;
			RamAddrCache = cpu.RamAddress;

			FlagCarry = cpu.SFR.Carry;
			FlagZero = cpu.SFR.Zero;
			FlagSign = cpu.SFR.Sign;
			FlagOverflow = cpu.SFR.Overflow;
			FlagRunning = cpu.SFR.Running;
			FlagRomReadPending = cpu.SFR.RomReadPending;
			FlagAlt1 = cpu.SFR.Alt1;
			FlagAlt2 = cpu.SFR.Alt2;
			FlagImmLow = cpu.SFR.ImmLow;
			FlagImmHigh = cpu.SFR.ImmHigh;
			FlagPrefix = cpu.SFR.Prefix;
			FlagIrq = cpu.SFR.Irq;

			FlagPlotTransparent = cpu.PlotTransparent;
			FlagPlotDither = cpu.PlotDither;
			FlagColorHighNibble = cpu.ColorHighNibble;
			FlagColorFreezeHigh = cpu.ColorFreezeHigh;
			FlagObjMode = cpu.ObjMode;
		}

		protected override void InternalUpdateConsoleState()
		{
			GsuState cpu = DebugApi.GetCpuState<GsuState>(CpuType.Gsu);

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

			cpu.SrcReg = RegSrc;
			cpu.DestReg = RegDst;
			cpu.ColorReg = RegColor;
			cpu.ProgramBank = RegPbr;
			cpu.RomBank = RomBank;
			cpu.RamBank = RamBank;
			cpu.RamAddress = RamAddrCache;

			cpu.SFR.Carry = FlagCarry;
			cpu.SFR.Zero = FlagZero;
			cpu.SFR.Sign = FlagSign;
			cpu.SFR.Overflow = FlagOverflow;
			cpu.SFR.Running = FlagRunning;
			cpu.SFR.RomReadPending = FlagRomReadPending;
			cpu.SFR.Alt1 = FlagAlt1;
			cpu.SFR.Alt2 = FlagAlt2;
			cpu.SFR.ImmLow = FlagImmLow;
			cpu.SFR.ImmHigh = FlagImmHigh;
			cpu.SFR.Prefix = FlagPrefix;
			cpu.SFR.Irq = FlagIrq;

			cpu.PlotTransparent = FlagPlotTransparent;
			cpu.PlotDither = FlagPlotDither;
			cpu.ColorHighNibble = FlagColorHighNibble;
			cpu.ColorFreezeHigh = FlagColorFreezeHigh;
			cpu.ObjMode = FlagObjMode;

			DebugApi.SetCpuState(cpu, CpuType.Gsu);
		}
	}
}
