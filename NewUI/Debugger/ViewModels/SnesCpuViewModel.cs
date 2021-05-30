#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia.Controls;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive.Linq;
using System.Text;

namespace Mesen.Debugger.ViewModels
{
	public class SnesCpuViewModel : ViewModelBase
	{
		[Reactive] public UInt16 RegA { get; set; }
		[Reactive] public UInt16 RegX { get; set; }
		[Reactive] public UInt16 RegY { get; set; }
		[Reactive] public UInt16 RegSP { get; set; }
		[Reactive] public UInt16 RegD { get; set; }
		[Reactive] public UInt16 RegPC { get; set; }
		[Reactive] public byte RegK { get; set; }
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
		[Reactive] public bool FlagIrq { get; set; }

		[Reactive] public string StackPreview { get; set; }

		public SnesCpuViewModel()
		{
		}

		public void UpdateState(CpuState state)
		{
			RegA = state.A;
			RegX = state.X;
			RegY = state.Y;
			RegSP = state.SP;
			RegD = state.D;
			RegPC = state.PC;

			RegK = state.K;
			RegDBR = state.DBR;
			RegPS = (byte)state.PS;

			//TODO
			/*
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.Negative)).ToPropertyEx(this, x => x.FlagN);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.Overflow)).ToPropertyEx(this, x => x.FlagV);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.MemoryMode8)).ToPropertyEx(this, x => x.FlagM);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.IndexMode8)).ToPropertyEx(this, x => x.FlagX);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.Decimal)).ToPropertyEx(this, x => x.FlagD);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.IrqDisable)).ToPropertyEx(this, x => x.FlagI);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.Zero)).ToPropertyEx(this, x => x.FlagZ);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.Carry)).ToPropertyEx(this, x => x.FlagC);
			this.WhenAnyValue(x => x.State).Select(st => st.EmulationMode).ToPropertyEx(this, x => x.FlagE);
			this.WhenAnyValue(x => x.State).Select(st => st.NmiFlag).ToPropertyEx(this, x => x.FlagNmi);
			this.WhenAnyValue(x => x.State).Select(st => st.IrqSource != 0).ToPropertyEx(this, x => x.FlagIrq);
			*/

			StringBuilder sb = new StringBuilder();
			for(UInt32 i = (uint)state.SP + 1; (i & 0xFF) != 0; i++) {
				sb.Append("$");
				sb.Append(DebugApi.GetMemoryValue(SnesMemoryType.CpuMemory, i).ToString("X2"));
				sb.Append(", ");
			}
			string stack = sb.ToString();
			if(stack.Length > 2) {
				stack = stack.Substring(0, stack.Length - 2);
			}
			StackPreview = stack;
		}
	}
}
