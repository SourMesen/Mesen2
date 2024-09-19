using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System.Collections.Generic;
using static Mesen.Debugger.ViewModels.RegEntry;

namespace Mesen.Debugger.RegisterViewer;

public class GbRegisterViewer
{
	public static List<RegisterViewerTab> GetTabs(ref GbState gbState)
	{
		List<RegisterViewerTab> tabs = new() {
			GetGbLcdTab(ref gbState),
			GetGbApuTab(ref gbState),
			GetGbMiscTab(ref gbState),
		};

		if(gbState.Type == GbType.Cgb) {
			tabs.Add(GetGbCgbTab(ref gbState));
		}
		return tabs;
	}

	public static RegisterViewerTab GetGbLcdTab(ref GbState gb, string tabPrefix = "")
	{
		List<RegEntry> entries = new List<RegEntry>();

		GbPpuState ppu = gb.Ppu;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("", "State"),
			new RegEntry("", "Cycle (H)", ppu.Cycle),
			new RegEntry("", "Scanline (V)", ppu.Scanline),
			new RegEntry("", "Frame Number", ppu.FrameCount),

			new RegEntry("$FF40", "LCD Control (LCDC)"),
			new RegEntry("$FF40.0", "Background Enabled", ppu.BgEnabled),
			new RegEntry("$FF40.1", "Sprites Enabled", ppu.SpritesEnabled),
			new RegEntry("$FF40.2", "Sprite size", ppu.LargeSprites ? "8x16" : "8x8", ppu.LargeSprites),
			new RegEntry("$FF40.3", "BG Tilemap Select", ppu.BgTilemapSelect ? 0x9C00 : 0x9800, Format.X16),
			new RegEntry("$FF40.4", "BG Tile Select", ppu.BgTileSelect ? "$8000-$8FFF" : "$8800-$97FF", ppu.BgTileSelect),
			new RegEntry("$FF40.5", "Window Enabled", ppu.WindowEnabled),
			new RegEntry("$FF40.6", "Window Tilemap Select", ppu.WindowTilemapSelect ? 0x9C00 : 0x9800, Format.X16),
			new RegEntry("$FF40.7", "LCD Enabled", ppu.LcdEnabled),

			new RegEntry("$FF41", "LCD Status (STAT)"),
			new RegEntry("$FF41.0-1", "Mode", (int)ppu.Mode),
			new RegEntry("$FF41.2", "Coincidence Flag", ppu.LyCoincidenceFlag),
			new RegEntry("$FF41.3", "Mode 0 H-Blank IRQ", (ppu.Status & 0x08) != 0),
			new RegEntry("$FF41.4", "Mode 1 V-Blank IRQ", (ppu.Status & 0x10) != 0),
			new RegEntry("$FF41.5", "Mode 2 OAM IRQ", (ppu.Status & 0x20) != 0),
			new RegEntry("$FF41.6", "LYC=LY Coincidence IRQ", (ppu.Status & 0x40) != 0),

			new RegEntry("", "LCD Registers"),
			new RegEntry("$FF42", "Scroll Y (SCY)", ppu.ScrollY, Format.X8),
			new RegEntry("$FF43", "Scroll X (SCX)", ppu.ScrollX, Format.X8),
			new RegEntry("$FF44", "Y-Coordinate (LY)", ppu.Ly, Format.X8),
			new RegEntry("$FF45", "LY Compare (LYC)", ppu.LyCompare, Format.X8),
			new RegEntry("$FF47", "BG Palette (BGP)", ppu.BgPalette, Format.X8),
			new RegEntry("$FF48", "OBJ Palette 0 (OBP0)", ppu.ObjPalette0, Format.X8),
			new RegEntry("$FF49", "OBJ Palette 1 (OBP1)", ppu.ObjPalette1, Format.X8),
			new RegEntry("$FF4A", "Window Y (WY)", ppu.WindowY, Format.X8),
			new RegEntry("$FF4B", "Window X (WX)", ppu.WindowX, Format.X8),
		});

		return new RegisterViewerTab(tabPrefix + "LCD", entries, CpuType.Gameboy, MemoryType.GameboyMemory);
	}

	private static RegisterViewerTab GetGbCgbTab(ref GbState gb)
	{
		List<RegEntry> entries = new List<RegEntry>();

		GbPpuState ppu = gb.Ppu;
		GbDmaControllerState dma = gb.Dma;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$FF4D.0", "CPU Switch Speed Request", gb.MemoryManager.CgbSwitchSpeedRequest),
			new RegEntry("$FF4D.7", "CPU Speed", gb.MemoryManager.CgbHighSpeed ? "8.39 MHz" : "4.19 MHz", gb.MemoryManager.CgbHighSpeed),

			new RegEntry("$FF4F.0", "Video RAM Bank", ppu.CgbVramBank),

			new RegEntry("", "DMA registers"),
			new RegEntry("$FF51-52", "DMA Source", dma.CgbDmaSource, Format.X16),
			new RegEntry("$FF53-54", "DMA Destination", dma.CgbDmaDest, Format.X16),
			new RegEntry("$FF55.0-6", "DMA Length", dma.CgbDmaLength, Format.X8),
			new RegEntry("$FF55.7", "HDMA Inactive", !dma.CgbHdmaRunning),

			new RegEntry("", "Palette registers"),
			new RegEntry("$FF68", "BGPI - Background Palette Index"),
			new RegEntry("$FF68.0-5", "BG Palette Address", ppu.CgbBgPalPosition, Format.X8),
			new RegEntry("$FF68.7", "BG Palette Auto-increment", ppu.CgbBgPalAutoInc),
			new RegEntry("$FF6A", "OBPI - OBJ Palette Index"),
			new RegEntry("$FF6A.0-5", "OBJ Palette Address", ppu.CgbObjPalPosition, Format.X8),
			new RegEntry("$FF6A.7", "OBJ Palette Auto-increment", ppu.CgbObjPalAutoInc),

			new RegEntry("", "Misc. registers"),
			new RegEntry("$FF70.0-2", "Work RAM Bank", gb.MemoryManager.CgbWorkRamBank, Format.X8),
			new RegEntry("$FF72", "Undocumented", gb.MemoryManager.CgbRegFF72, Format.X8),
			new RegEntry("$FF73", "Undocumented", gb.MemoryManager.CgbRegFF73, Format.X8),
			new RegEntry("$FF74", "Undocumented", gb.MemoryManager.CgbRegFF74, Format.X8),
			new RegEntry("$FF75", "Undocumented", gb.MemoryManager.CgbRegFF75, Format.X8),
		});

		return new RegisterViewerTab("CGB", entries, CpuType.Gameboy, MemoryType.GameboyMemory);
	}

	public static RegisterViewerTab GetGbMiscTab(ref GbState gb, string tabPrefix = "")
	{
		List<RegEntry> entries = new List<RegEntry>();

		GbTimerState timer = gb.Timer;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$FF04-7", "Timer"),
			new RegEntry("$FF04", "DIV - Divider", timer.Divider, Format.X16),
			new RegEntry("$FF05", "TIMA - Counter", timer.Counter, Format.X8),
			new RegEntry("$FF06", "TMA - Modulo", timer.Modulo, Format.X8),
			new RegEntry("$FF07", "TAC - Control", timer.Control, Format.X8)
		});

		GbDmaControllerState dma = gb.Dma;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("", "DMA"),
			new RegEntry("$FF46", "OAM DMA - Source", dma.OamDmaSource << 8, Format.X16),
			new RegEntry("", "OAM DMA - Running", dma.OamDmaRunning)
		});

		GbMemoryManagerState memManager = gb.MemoryManager;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("", "IRQ"),
			new RegEntry("$FF0F", "IF - IRQ Flags", memManager.IrqRequests, Format.X8),
			new RegEntry("$FF0F.0", "IF - Vertical Blank IRQ", (memManager.IrqRequests & 0x01) != 0),
			new RegEntry("$FF0F.1", "IF - STAT IRQ", (memManager.IrqRequests & 0x02) != 0),
			new RegEntry("$FF0F.2", "IF - Timer IRQ", (memManager.IrqRequests & 0x04) != 0),
			new RegEntry("$FF0F.3", "IF - Serial IRQ", (memManager.IrqRequests & 0x08) != 0),
			new RegEntry("$FF0F.4", "IF - Joypad IRQ", (memManager.IrqRequests & 0x10) != 0),

			new RegEntry("$FFFF", "IE - IRQ Enabled", memManager.IrqEnabled, Format.X8),
			new RegEntry("$FFFF.0", "IE - Vertical Blank IRQ Enabled", (memManager.IrqEnabled & 0x01) != 0),
			new RegEntry("$FFFF.1", "IE - STAT IRQ Enabled", (memManager.IrqEnabled & 0x02) != 0),
			new RegEntry("$FFFF.2", "IE - Timer IRQ Enabled", (memManager.IrqEnabled & 0x04) != 0),
			new RegEntry("$FFFF.3", "IE - Serial IRQ Enabled", (memManager.IrqEnabled & 0x08) != 0),
			new RegEntry("$FFFF.4", "IE - Joypad IRQ Enabled", (memManager.IrqEnabled & 0x10) != 0),

			new RegEntry("", "Misc"),
			new RegEntry("$FF00", "Input Select", gb.ControlManager.InputSelect, Format.X8),
			new RegEntry("$FF01", "Serial Data", memManager.SerialData, Format.X8),
			new RegEntry("$FF02", "Serial Control", memManager.SerialControl, Format.X8),
			new RegEntry("", "Serial Bit Count", memManager.SerialBitCount),
		});

		return new RegisterViewerTab(tabPrefix + "Timer/DMA/IRQ", entries, CpuType.Gameboy, MemoryType.GameboyMemory);
	}

	public static RegisterViewerTab GetGbApuTab(ref GbState gb, string tabPrefix = "")
	{
		List<RegEntry> entries = new List<RegEntry>();

		GbApuState apu = gb.Apu.Common;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("", "APU"),
			new RegEntry("$FF24.0-2", "Volume Right", apu.RightVolume),
			new RegEntry("$FF24.3", "External Audio Right Enabled", apu.ExtAudioRightEnabled),
			new RegEntry("$FF24.4-6", "Volume Left", apu.LeftVolume),
			new RegEntry("$FF24.7", "External Audio Left Enabled", apu.ExtAudioRightEnabled),
			new RegEntry("$FF25.0", "Right Square 1 Enabled", apu.EnableRightSq1 != 0),
			new RegEntry("$FF25.1", "Right Square 2 Enabled", apu.EnableRightSq2 != 0),
			new RegEntry("$FF25.2", "Right Wave Enabled", apu.EnableRightWave != 0),
			new RegEntry("$FF25.3", "Right Noise Enabled", apu.EnableRightNoise != 0),
			new RegEntry("$FF25.4", "Left Square 1 Enabled", apu.EnableLeftSq1 != 0),
			new RegEntry("$FF25.5", "Left Square 2 Enabled", apu.EnableLeftSq2 != 0),
			new RegEntry("$FF25.6", "Left Wave Enabled", apu.EnableLeftWave != 0),
			new RegEntry("$FF25.7", "Left Noise Enabled", apu.EnableLeftNoise != 0),
			new RegEntry("$FF26.7", "APU Enabled", apu.ApuEnabled),
			new RegEntry("", "Frame Sequencer", apu.FrameSequenceStep),
		});

		GbSquareState sq1 = gb.Apu.Square1;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$FF10-$FF14", "Square 1"),
			new RegEntry("$FF10.0-2", "Sweep Shift", sq1.SweepShift),
			new RegEntry("$FF10.3", "Sweep Negate", sq1.SweepNegate),
			new RegEntry("$FF10.4-7", "Sweep Period", sq1.SweepPeriod),

			new RegEntry("$FF11.0-5", "Length", sq1.Length),
			new RegEntry("$FF11.6-7", "Duty", sq1.Duty),

			new RegEntry("$FF12.0-2", "Envelope Period", sq1.EnvPeriod),
			new RegEntry("$FF12.3", "Envelope Increase Volume", sq1.EnvRaiseVolume),
			new RegEntry("$FF12.4-7", "Envelope Volume", sq1.EnvVolume),

			new RegEntry("$FF13-$FF14.0-2", "Frequency", sq1.Frequency),
			new RegEntry("$FF14.6", "Length Counter Enabled", sq1.LengthEnabled),
			new RegEntry("$FF14.7", "Channel Enabled", sq1.Enabled),

			new RegEntry("--", "Timer", sq1.Timer),
			new RegEntry("--", "Duty Position", sq1.DutyPos),
			new RegEntry("--", "Sweep Enabled", sq1.SweepEnabled),
			new RegEntry("--", "Sweep Frequency", sq1.SweepFreq),
			new RegEntry("--", "Sweep Timer", sq1.SweepTimer),
			new RegEntry("--", "Envelope Timer", sq1.EnvTimer),
			new RegEntry("--", "Output", sq1.Output)
		});

		GbSquareState sq2 = gb.Apu.Square2;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$FF16-$FF19", "Square 2"),
			new RegEntry("$FF16.0-5", "Length", sq2.Length),
			new RegEntry("$FF16.6-7", "Duty", sq2.Duty),

			new RegEntry("$FF17.0-2", "Envelope Period", sq2.EnvPeriod),
			new RegEntry("$FF17.3", "Envelope Increase Volume", sq2.EnvRaiseVolume),
			new RegEntry("$FF17.4-7", "Envelope Volume", sq2.EnvVolume),

			new RegEntry("$FF18-$FF19.0-2", "Frequency", sq2.Frequency),
			new RegEntry("$FF19.6", "Length Counter Enabled", sq2.LengthEnabled),
			new RegEntry("$FF19.7", "Channel Enabled", sq2.Enabled),

			new RegEntry("--", "Timer", sq2.Timer),
			new RegEntry("--", "Duty Position", sq2.DutyPos),
			new RegEntry("--", "Envelope Timer", sq2.EnvTimer),
			new RegEntry("--", "Output", sq2.Output)
		});

		GbWaveState wave = gb.Apu.Wave;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$FF1A-$FF1E", "Wave"),
			new RegEntry("$FF1A.7", "Sound Enabled", wave.DacEnabled),

			new RegEntry("$FF1B", "Length", wave.Length),

			new RegEntry("$FF1C.5-6", "Volume", wave.Volume),

			new RegEntry("$FF1D-$FF1E.0-2", "Frequency", wave.Frequency),

			new RegEntry("$FF1E.6", "Length Counter Enabled", wave.LengthEnabled),
			new RegEntry("$FF1E.7", "Channel Enabled", wave.Enabled),

			new RegEntry("--", "Timer", wave.Timer),
			new RegEntry("--", "Sample Buffer", wave.SampleBuffer),
			new RegEntry("--", "Position", wave.Position),
			new RegEntry("--", "Output", wave.Output),
		});

		GbNoiseState noise = gb.Apu.Noise;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$FF20-$FF23", "Noise"),
			new RegEntry("$FF20.0-5", "Length", noise.Length),

			new RegEntry("$FF21.0-2", "Envelope Period", noise.EnvPeriod),
			new RegEntry("$FF21.3", "Envelope Increase Volume", noise.EnvRaiseVolume),
			new RegEntry("$FF21.4-7", "Envelope Volume", noise.EnvVolume),

			new RegEntry("$FF23.0-2", "Divisor", noise.Divisor),
			new RegEntry("$FF23.3", "Short Mode", noise.ShortWidthMode),
			new RegEntry("$FF23.4-7", "Period Shift", noise.PeriodShift),

			new RegEntry("$FF24.6", "Length Counter Enabled", noise.LengthEnabled),
			new RegEntry("$FF24.7", "Channel Enabled", noise.Enabled),

			new RegEntry("--", "Timer", noise.Timer),
			new RegEntry("--", "Envelope Timer", noise.EnvTimer),
			new RegEntry("--", "Shift Register", noise.ShiftRegister, Format.X16),
			new RegEntry("--", "Output", noise.Output)
		});

		return new RegisterViewerTab(tabPrefix + "APU", entries, CpuType.Gameboy, MemoryType.GameboyMemory);
	}
}
