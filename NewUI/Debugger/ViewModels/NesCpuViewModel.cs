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
	public class NesCpuViewModel : ViewModelBase
	{
		[Reactive] public byte RegA { get; set; }
		[Reactive] public byte RegX { get; set; }
		[Reactive] public byte RegY { get; set; }
		[Reactive] public byte RegSP { get; set; }
		[Reactive] public UInt16 RegPC { get; set; }
		[Reactive] public byte RegPS { get; set; }
		
		[Reactive] public UInt64 Cycle { get; set; }

		[Reactive] public bool FlagN { get; set; }
		[Reactive] public bool FlagV { get; set; }
		[Reactive] public bool FlagD { get; set; }
		[Reactive] public bool FlagI { get; set; }
		[Reactive] public bool FlagZ { get; set; }
		[Reactive] public bool FlagC { get; set; }

		[Reactive] public bool FlagNmi { get; set; }
		
		[Reactive] public bool FlagIrqExternal { get; set; }
		[Reactive] public bool FlagIrqFrameCount { get; set; }
		[Reactive] public bool FlagIrqDmc { get; set; }
		[Reactive] public bool FlagIrqFdsDisk { get; set; }
		
		[Reactive] public string StackPreview { get; set; }

		public NesCpuViewModel()
		{
		}

		public void UpdateState(NesCpuState state)
		{
			RegA = state.A;
			RegX = state.X;
			RegY = state.Y;
			RegSP = state.SP;
			RegPC = state.PC;
			RegPS = state.PS;

			Cycle = state.CycleCount;

			FlagNmi = state.NMIFlag;
			FlagIrqExternal = (state.IRQFlag & (byte)NesIrqSources.External) != 0;
			FlagIrqFrameCount = (state.IRQFlag & (byte)NesIrqSources.FrameCounter) != 0;
			FlagIrqDmc = (state.IRQFlag & (byte)NesIrqSources.DMC) != 0;
			FlagIrqFdsDisk = (state.IRQFlag & (byte)NesIrqSources.FdsDisk) != 0;

			FlagN = (RegPS & (byte)NesCpuFlags.Negative) != 0;
			FlagV = (RegPS & (byte)NesCpuFlags.Overflow) != 0;
			FlagD = (RegPS & (byte)NesCpuFlags.Decimal) != 0;
			FlagI = (RegPS & (byte)NesCpuFlags.IrqDisable) != 0;
			FlagZ = (RegPS & (byte)NesCpuFlags.Zero) != 0;
			FlagC = (RegPS & (byte)NesCpuFlags.Carry) != 0;

			StringBuilder sb = new StringBuilder();
			for(UInt32 i = (UInt32)0x100 + state.SP + 1; i < 0x200; i++) {
				sb.Append("$");
				sb.Append(DebugApi.GetMemoryValue(SnesMemoryType.NesMemory, i).ToString("X2"));
				sb.Append(" ");
			}
			StackPreview = sb.ToString().TrimEnd();
		}
	}
}
