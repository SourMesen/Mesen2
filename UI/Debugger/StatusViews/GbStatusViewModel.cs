using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Text;

namespace Mesen.Debugger.StatusViews
{
	public class GbStatusViewModel : BaseConsoleStatusViewModel
	{
		[Reactive] public byte RegA { get; set; }
		[Reactive] public byte RegB { get; set; }
		[Reactive] public byte RegC { get; set; }
		[Reactive] public byte RegD { get; set; }
		[Reactive] public byte RegE { get; set; }
		[Reactive] public byte RegFlags { get; set; }

		[Reactive] public byte RegH { get; set; }
		[Reactive] public byte RegL { get; set; }
		
		[Reactive] public UInt16 RegSP { get; set; }
		[Reactive] public UInt16 RegPC { get; set; }

		[Reactive] public UInt16 Scanline { get; set; }
		[Reactive] public UInt16 Cycle { get; set; }

		[Reactive] public bool FlagCarry { get; set; }
		[Reactive] public bool FlagHalf { get; set; }
		[Reactive] public bool FlagAddSub { get; set; }
		[Reactive] public bool FlagZero { get; set; }

		[Reactive] public bool FlagHalted { get; set; }
		[Reactive] public bool FlagEiPending { get; set; }
		[Reactive] public bool FlagIme { get; set; }

		[Reactive] public string StackPreview { get; private set; } = "";

		public GbStatusViewModel()
		{
			this.WhenAnyValue(x => x.FlagCarry, x => x.FlagHalf, x => x.FlagAddSub, x => x.FlagZero).Subscribe(x => UpdateFlagsValue());

			this.WhenAnyValue(x => x.RegFlags).Subscribe(x => {
				using var delayNotifs = DelayChangeNotifications(); //don't reupdate RegFlags while updating the flags
				FlagCarry = (x & (byte)GameboyFlags.Carry) != 0;
				FlagHalf = (x & (byte)GameboyFlags.HalfCarry) != 0;
				FlagAddSub = (x & (byte)GameboyFlags.AddSub) != 0;
				FlagZero = (x & (byte)GameboyFlags.Zero) != 0;
			});
		}

		private void UpdateFlagsValue()
		{
			RegFlags = (byte)(
				(FlagCarry ? (byte)GameboyFlags.Carry : 0) |
				(FlagHalf ? (byte)GameboyFlags.HalfCarry : 0) |
				(FlagAddSub ? (byte)GameboyFlags.AddSub : 0) |
				(FlagZero ? (byte)GameboyFlags.Zero : 0)
			);
		}

		protected override void InternalUpdateUiState()
		{
			GbState state = DebugApi.GetConsoleState<GbState>(ConsoleType.Gameboy);

			GbCpuState cpu = state.Cpu;
			GbPpuState ppu = DebugApi.GetPpuState<GbPpuState>(CpuType.Gameboy);

			UpdateCycleCount(state.Cpu.CycleCount);

			RegA = cpu.A;
			RegB = cpu.B;
			RegC = cpu.C;
			RegD = cpu.D;
			RegE = cpu.E;
			RegFlags = cpu.Flags;

			RegH = cpu.H;
			RegL = cpu.L;

			RegPC = cpu.PC;
			RegSP = cpu.SP;

			FlagEiPending = cpu.EiPending;
			FlagHalted = cpu.HaltCounter > 0;
			FlagIme = cpu.IME;

			Scanline = ppu.Scanline;
			Cycle = ppu.Cycle;

			StringBuilder sb = new StringBuilder();
			for(UInt32 i = (UInt32)cpu.SP; (i & 0xFF) != 0; i++) {
				sb.Append($"${DebugApi.GetMemoryValue(MemoryType.GameboyMemory, i):X2} ");
			}
			StackPreview = sb.ToString();
		}

		protected override void InternalUpdateConsoleState()
		{
			GbCpuState cpu = DebugApi.GetCpuState<GbCpuState>(CpuType.Gameboy);

			cpu.A = RegA;
			cpu.B = RegB;
			cpu.C = RegC;
			cpu.D = RegD;
			cpu.E = RegE;
			cpu.Flags = RegFlags;

			cpu.H = RegH;
			cpu.L = RegL;

			cpu.PC = RegPC;
			cpu.SP = RegSP;

			cpu.EiPending = FlagEiPending;
			if(cpu.HaltCounter == 0 && FlagHalted) {
				cpu.HaltCounter = 1;
			} else if(!FlagHalted) {
				cpu.HaltCounter = 0;
			}
			cpu.IME = FlagIme;

			DebugApi.SetCpuState(cpu, CpuType.Gameboy);
		}
	}
}
