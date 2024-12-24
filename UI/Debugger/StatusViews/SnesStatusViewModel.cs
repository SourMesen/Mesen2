using Avalonia.Controls;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive.Linq;
using System.Text;

namespace Mesen.Debugger.StatusViews
{
	public class SnesStatusViewModel : BaseConsoleStatusViewModel
	{
		[Reactive] public UInt16 RegA { get; set; }
		[Reactive] public UInt16 RegX { get; set; }
		[Reactive] public UInt16 RegY { get; set; }
		[Reactive] public UInt16 RegSP { get; set; }
		[Reactive] public UInt16 RegD { get; set; }
		[Reactive] public UInt32 RegPC { get; set; }
		[Reactive] public byte RegDBR { get; set; }
		[Reactive] public byte RegPS { get; set; }

		[Reactive] public bool FlagN { get; set; }
		[Reactive] public bool FlagV { get; set; }
		[Reactive] public bool FlagM { get; set; }
		[Reactive] public bool FlagX { get; set; }
		[Reactive] public bool FlagD { get; set; }
		[Reactive] public bool FlagI { get; set; }
		[Reactive] public bool FlagZ { get; set; }
		[Reactive] public bool FlagC { get; set; }

		[Reactive] public bool FlagE { get; set; }
		
		[Reactive] public bool FlagNmi { get; set; }
		[Reactive] public bool FlagIrqHvCounters { get; set; }
		[Reactive] public bool FlagIrqCoprocessor { get; set; }

		[Reactive] public int Cycle { get; private set; }
		[Reactive] public int Scanline { get; private set; }
		[Reactive] public int HClock { get; private set; }
		
		[Reactive] public int VramAddress { get; private set; }
		[Reactive] public int OamAddress { get; private set; }
		[Reactive] public int CgRamAddress { get; private set; }

		[Reactive] public string StackPreview { get; set; } = "";

		private CpuType _cpuType;

		[Obsolete("For designer only")]
		public SnesStatusViewModel() : this(CpuType.Snes) { }

		public SnesStatusViewModel(CpuType cpuType)
		{
			_cpuType = cpuType;

			this.WhenAnyValue(x => x.FlagC, x => x.FlagD, x => x.FlagI, x => x.FlagN).Subscribe(x => UpdatePsValue());
			this.WhenAnyValue(x => x.FlagV, x => x.FlagZ, x => x.FlagM, x => x.FlagX).Subscribe(x => UpdatePsValue());

			this.WhenAnyValue(x => x.RegPS).Subscribe(x => {
				using var delayNotifs = DelayChangeNotifications(); //don't reupdate PS while updating the flags
				FlagN = (x & (byte)SnesCpuFlags.Negative) != 0;
				FlagV = (x & (byte)SnesCpuFlags.Overflow) != 0;
				FlagD = (x & (byte)SnesCpuFlags.Decimal) != 0;
				FlagI = (x & (byte)SnesCpuFlags.IrqDisable) != 0;
				FlagZ = (x & (byte)SnesCpuFlags.Zero) != 0;
				FlagC = (x & (byte)SnesCpuFlags.Carry) != 0;
				FlagX = (x & (byte)SnesCpuFlags.IndexMode8) != 0;
				FlagM = (x & (byte)SnesCpuFlags.MemoryMode8) != 0;
			});
		}

		private void UpdatePsValue()
		{
			RegPS = (byte)(
				(FlagN ? (byte)SnesCpuFlags.Negative : 0) |
				(FlagV ? (byte)SnesCpuFlags.Overflow : 0) |
				(FlagD ? (byte)SnesCpuFlags.Decimal : 0) |
				(FlagI ? (byte)SnesCpuFlags.IrqDisable : 0) |
				(FlagZ ? (byte)SnesCpuFlags.Zero : 0) |
				(FlagC ? (byte)SnesCpuFlags.Carry : 0) |
				(FlagX ? (byte)SnesCpuFlags.IndexMode8 : 0) |
				(FlagM ? (byte)SnesCpuFlags.MemoryMode8 : 0)
			);
		}

		protected override void InternalUpdateUiState()
		{
			SnesCpuState cpu = DebugApi.GetCpuState<SnesCpuState>(_cpuType);
			SnesPpuState ppu = DebugApi.GetPpuState<SnesPpuState>(CpuType.Snes);

			UpdateCycleCount(cpu.CycleCount);

			RegA = cpu.A;
			RegX = cpu.X;
			RegY = cpu.Y;
			RegSP = cpu.SP;
			RegPC = (UInt32)((cpu.K << 16) | cpu.PC);

			RegD = cpu.D;
			RegDBR = cpu.DBR;

			RegPS = (byte)cpu.PS;

			FlagE = cpu.EmulationMode;
			FlagNmi = cpu.NmiFlagCounter > 0 || cpu.NeedNmi;
			FlagIrqHvCounters = (cpu.IrqSource & (byte)SnesIrqSource.Ppu) != 0;
			FlagIrqCoprocessor = (cpu.IrqSource & (byte)SnesIrqSource.Coprocessor) != 0;

			StringBuilder sb = new StringBuilder();
			MemoryType memType = _cpuType.ToMemoryType();
			for(UInt32 i = (uint)cpu.SP + 1; (i & 0xFF) != 0; i++) {
				sb.Append($"${DebugApi.GetMemoryValue(memType, i):X2} ");
			}
			StackPreview = sb.ToString();

			Cycle = ppu.Cycle;
			HClock = ppu.HClock;
			Scanline = ppu.Scanline;
			
			VramAddress = ppu.VramAddress;
			OamAddress = ppu.InternalOamRamAddress;
			CgRamAddress = ppu.CgramAddress;
		}

		protected override void InternalUpdateConsoleState()
		{
			SnesCpuState cpu = DebugApi.GetCpuState<SnesCpuState>(_cpuType);

			cpu.A = RegA;
			cpu.X = RegX;
			cpu.Y = RegY;
			cpu.SP = RegSP;
			cpu.K = (byte)((RegPC >> 16) & 0xFF);
			cpu.PC = (UInt16)(RegPC & 0xFFFF);

			cpu.D = RegD;
			cpu.DBR = RegDBR;

			cpu.PS = (SnesCpuFlags)RegPS;

			cpu.EmulationMode = FlagE;
			if(FlagNmi && cpu.NmiFlagCounter == 0 && !cpu.NeedNmi) {
				cpu.NmiFlagCounter = (byte)(FlagNmi ? 1 : 0);
			}
			cpu.IrqSource = (byte)(
				(FlagIrqHvCounters ? (byte)SnesIrqSource.Ppu : 0) | 
				(FlagIrqCoprocessor ? (byte)SnesIrqSource.Coprocessor : 0)
			);

			DebugApi.SetCpuState(cpu, _cpuType);
		}
	}
}
