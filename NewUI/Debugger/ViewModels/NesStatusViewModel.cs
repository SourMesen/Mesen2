using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System;
using System.Text;

namespace Mesen.Debugger.ViewModels
{
	public class NesStatusViewModel : BaseConsoleStatusViewModel
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

		[Reactive] public bool FlagNmi { get; set; }
		
		[Reactive] public bool FlagIrqExternal { get; set; }
		[Reactive] public bool FlagIrqFrameCount { get; set; }
		[Reactive] public bool FlagIrqDmc { get; set; }
		[Reactive] public bool FlagIrqFdsDisk { get; set; }

		[Reactive] public uint Cycle { get; private set; }
		[Reactive] public int Scanline { get; private set; }
		[Reactive] public UInt16 VramAddr { get; set; }
		[Reactive] public UInt16 TmpVramAddr { get; set; }
		[Reactive] public UInt16 BusAddr { get; set; }
		[Reactive] public byte ScrollX { get; set; }
		[Reactive] public bool Sprite0Hit { get; set; }
		[Reactive] public bool SpriteOverflow { get; set; }
		[Reactive] public bool VerticalBlank { get; set; }
		[Reactive] public bool WriteToggle { get; set; }
		
		[Reactive] public bool BgEnabled { get; set; }
		[Reactive] public bool SpritesEnabled { get; set; }
		[Reactive] public bool LargeSprites { get; set; }
		[Reactive] public bool NmiOnVBlank { get; set; }
		[Reactive] public bool VerticalWrite { get; set; }
		
		[Reactive] public UInt16 BgAddr { get; private set; }
		[Reactive] public UInt16 SpriteAddr { get; private set; }

		[Reactive] public UInt64 CycleCount { get; private set; }
		[Reactive] public string StackPreview { get; private set; } = "";

		public NesStatusViewModel()
		{
		}

		public override void UpdateUiState()
		{
			NesCpuState cpu = DebugApi.GetCpuState<NesCpuState>(CpuType.Nes);
			NesPpuState ppu = DebugApi.GetPpuState<NesPpuState>(CpuType.Nes);

			RegA = cpu.A;
			RegX = cpu.X;
			RegY = cpu.Y;
			RegSP = cpu.SP;
			RegPC = cpu.PC;
			RegPS = cpu.PS;

			CycleCount = cpu.CycleCount;

			FlagNmi = cpu.NMIFlag;
			FlagIrqExternal = (cpu.IRQFlag & (byte)NesIrqSources.External) != 0;
			FlagIrqFrameCount = (cpu.IRQFlag & (byte)NesIrqSources.FrameCounter) != 0;
			FlagIrqDmc = (cpu.IRQFlag & (byte)NesIrqSources.DMC) != 0;
			FlagIrqFdsDisk = (cpu.IRQFlag & (byte)NesIrqSources.FdsDisk) != 0;

			FlagN = (RegPS & (byte)NesCpuFlags.Negative) != 0;
			FlagV = (RegPS & (byte)NesCpuFlags.Overflow) != 0;
			FlagD = (RegPS & (byte)NesCpuFlags.Decimal) != 0;
			FlagI = (RegPS & (byte)NesCpuFlags.IrqDisable) != 0;
			FlagZ = (RegPS & (byte)NesCpuFlags.Zero) != 0;
			FlagC = (RegPS & (byte)NesCpuFlags.Carry) != 0;

			StringBuilder sb = new StringBuilder();
			for(UInt32 i = (UInt32)0x100 + cpu.SP + 1; i < 0x200; i++) {
				sb.Append("$");
				sb.Append(DebugApi.GetMemoryValue(SnesMemoryType.NesMemory, i).ToString("X2"));
				sb.Append(" ");
			}
			StackPreview = sb.ToString().TrimEnd();

			Cycle = ppu.Cycle;
			Scanline = ppu.Scanline;
			VramAddr = ppu.VideoRamAddr;
			TmpVramAddr = ppu.TmpVideoRamAddr;
			ScrollX = ppu.ScrollX;
			BusAddr = ppu.BusAddress;

			Sprite0Hit = ppu.StatusFlags.Sprite0Hit;
			SpriteOverflow = ppu.StatusFlags.SpriteOverflow;
			VerticalBlank = ppu.StatusFlags.VerticalBlank;
			WriteToggle = ppu.WriteToggle;

			LargeSprites = (ppu.ControlReg & 0x20) != 0;
			NmiOnVBlank = (ppu.ControlReg & 0x80) != 0;
			VerticalWrite = (ppu.ControlReg & 0x04) != 0;
			BgAddr = (UInt16)((ppu.ControlReg & 0x10) != 0 ? 0x1000 : 0);
			SpriteAddr = (UInt16)((ppu.ControlReg & 0x08) != 0 ? 0x1000 : 0);
			
			BgEnabled = (ppu.MaskReg & 0x08) != 0;
			SpritesEnabled = (ppu.MaskReg & 0x10) != 0;
		}

		public override void UpdateEmulationState()
		{
			
		}
	}
}
