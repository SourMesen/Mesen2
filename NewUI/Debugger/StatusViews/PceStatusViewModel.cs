using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Text;

namespace Mesen.Debugger.StatusViews
{
	public class PceStatusViewModel : BaseConsoleStatusViewModel
	{
		[Reactive] public byte RegA { get; set; }
		[Reactive] public byte RegX { get; set; }
		[Reactive] public byte RegY { get; set; }
		[Reactive] public byte RegSP { get; set; }
		[Reactive] public UInt16 RegPC { get; set; }
		[Reactive] public byte RegPS { get; set; }
		
		[Reactive] public bool FlagN { get; set; }
		[Reactive] public bool FlagV { get; set; }
		[Reactive] public bool FlagD { get; set; }
		[Reactive] public bool FlagI { get; set; }
		[Reactive] public bool FlagZ { get; set; }
		[Reactive] public bool FlagC { get; set; }
		[Reactive] public bool FlagT { get; set; }

		[Reactive] public UInt16 Cycle { get; private set; }
		[Reactive] public UInt16 Scanline { get; private set; }
		[Reactive] public UInt32 FrameCount { get; private set; }

		[Reactive] public string StackPreview { get; private set; } = "";

		public PceStatusViewModel()
		{
			this.WhenAnyValue(x => x.FlagC, x => x.FlagD, x => x.FlagI, x => x.FlagN, x => x.FlagV, x => x.FlagZ, x => x.FlagT).Subscribe(x => {
				RegPS = (byte)(
					(FlagN ? (byte)PceCpuFlags.Negative : 0) |
					(FlagV ? (byte)PceCpuFlags.Overflow : 0) |
					(FlagT ? (byte)PceCpuFlags.Memory : 0) |
					(FlagD ? (byte)PceCpuFlags.Decimal : 0) |
					(FlagI ? (byte)PceCpuFlags.IrqDisable : 0) |
					(FlagZ ? (byte)PceCpuFlags.Zero : 0) |
					(FlagC ? (byte)PceCpuFlags.Carry : 0)
				);
			});

			this.WhenAnyValue(x => x.RegPS).Subscribe(x => {
				using var delayNotifs = DelayChangeNotifications(); //don't reupdate PS while updating the flags
				FlagN = (x & (byte)PceCpuFlags.Negative) != 0;
				FlagV = (x & (byte)PceCpuFlags.Overflow) != 0;
				FlagT = (x & (byte)PceCpuFlags.Memory) != 0;
				FlagD = (x & (byte)PceCpuFlags.Decimal) != 0;
				FlagI = (x & (byte)PceCpuFlags.IrqDisable) != 0;
				FlagZ = (x & (byte)PceCpuFlags.Zero) != 0;
				FlagC = (x & (byte)PceCpuFlags.Carry) != 0;
			});
		}

		protected override void InternalUpdateUiState()
		{
			PceCpuState cpu = DebugApi.GetCpuState<PceCpuState>(CpuType.Pce);
			PceVideoState video = DebugApi.GetPpuState<PceVideoState>(CpuType.Pce);

			UpdateCycleCount(cpu.CycleCount);

			RegA = cpu.A;
			RegX = cpu.X;
			RegY = cpu.Y;
			RegSP = cpu.SP;
			RegPC = cpu.PC;
			RegPS = cpu.PS;

			StringBuilder sb = new StringBuilder();
			for(UInt32 i = (UInt32)0x2100 + cpu.SP + 1; i < 0x2200; i++) {
				sb.Append($"${DebugApi.GetMemoryValue(MemoryType.PceMemory, i):X2} ");
			}
			StackPreview = sb.ToString();

			Cycle = video.Vdc.HClock;
			Scanline = video.Vdc.Scanline;
			FrameCount = video.Vdc.FrameCount;
		}

		protected override void InternalUpdateConsoleState()
		{
			PceCpuState cpu = DebugApi.GetCpuState<PceCpuState>(CpuType.Pce);

			cpu.A = RegA;
			cpu.X = RegX;
			cpu.Y = RegY;
			cpu.SP = RegSP;
			cpu.PC = RegPC;
			cpu.PS = RegPS;

			DebugApi.SetCpuState(cpu, CpuType.Pce);
		}
	}
}
