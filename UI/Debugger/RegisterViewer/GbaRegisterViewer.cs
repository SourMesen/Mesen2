using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System.Collections.Generic;
using static Mesen.Debugger.ViewModels.RegEntry;

namespace Mesen.Debugger.RegisterViewer;

public class GbaRegisterViewer
{
	public static List<RegisterViewerTab> GetTabs(ref GbaState gbaState)
	{
		List<RegisterViewerTab> tabs = new List<RegisterViewerTab>() {
			GetGbaPpuTab(ref gbaState),
			GetGbaApuTab(ref gbaState),
			GetGbaDmaTab(ref gbaState),
			GetGbaTimerTab(ref gbaState),
			GetGbaMiscTab(ref gbaState),
		};
		return tabs;
	}

	private static RegisterViewerTab GetGbaMiscTab(ref GbaState gbaState)
	{
		List<RegEntry> entries = new List<RegEntry>();

		GbaMemoryManagerState memManager = gbaState.MemoryManager;
		entries.AddRange(new List<RegEntry>() {
			/*new RegEntry("", "Prefetch"),
			new RegEntry("", "Read Address", gbaState.Prefetch.ReadAddr, Format.X32),
			new RegEntry("", "Prefetch Address", gbaState.Prefetch.PrefetchAddr, Format.X32),
			new RegEntry("", "Length", (gbaState.Prefetch.PrefetchAddr - gbaState.Prefetch.ReadAddr) / 2, Format.X8),
			new RegEntry("", "Clock Counter", gbaState.Prefetch.ClockCounter),
			new RegEntry("", "Filled", (gbaState.Prefetch.PrefetchAddr - gbaState.Prefetch.ReadAddr) >= 16),
			new RegEntry("", "Was Filled", gbaState.Prefetch.WasFilled),*/

			new RegEntry("", "Input IRQ Control"),
			new RegEntry("$4000132-3", "Register Value", gbaState.ControlManager.KeyControl, Format.X16),
			new RegEntry("$4000132.0", "A", (gbaState.ControlManager.KeyControl & 0x01) != 0),
			new RegEntry("$4000132.1", "B", (gbaState.ControlManager.KeyControl & 0x02) != 0),
			new RegEntry("$4000132.2", "Select", (gbaState.ControlManager.KeyControl & 0x04) != 0),
			new RegEntry("$4000132.3", "Start", (gbaState.ControlManager.KeyControl & 0x08) != 0),
			new RegEntry("$4000132.4", "Right", (gbaState.ControlManager.KeyControl & 0x10) != 0),
			new RegEntry("$4000132.5", "Left", (gbaState.ControlManager.KeyControl & 0x20) != 0),
			new RegEntry("$4000132.6", "Up", (gbaState.ControlManager.KeyControl & 0x40) != 0),
			new RegEntry("$4000132.7", "Down", (gbaState.ControlManager.KeyControl & 0x80) != 0),
			new RegEntry("$4000133.0", "R", (gbaState.ControlManager.KeyControl & 0x100) != 0),
			new RegEntry("$4000133.1", "L", (gbaState.ControlManager.KeyControl & 0x200) != 0),
			new RegEntry("$4000133.6", "IRQ Enabled", (gbaState.ControlManager.KeyControl & 0x4000) != 0),
			new RegEntry("$4000133.7", "IRQ Condition", (gbaState.ControlManager.KeyControl & 0x8000) == 0 ? "OR" : "AND", gbaState.ControlManager.KeyControl & 0x8000),

			new RegEntry("", "IRQ"),
			new RegEntry("", "IE - IRQ Enabled"),
			new RegEntry("$4000200-1", "IE - Register Value", memManager.IE, Format.X16),
			new RegEntry("$4000200.0", "Vertical Blank IRQ Enabled", ((memManager.IE >> 0) & 0x01) != 0),
			new RegEntry("$4000200.1", "Horizontal Blank IRQ Enabled", ((memManager.IE >> 1) & 0x01) != 0),
			new RegEntry("$4000200.2", "LYC Match IRQ Enabled", ((memManager.IE >> 2) & 0x01) != 0),
			new RegEntry("$4000200.3", "Timer 0 IRQ Enabled", ((memManager.IE >> 3) & 0x01) != 0),
			new RegEntry("$4000200.4", "Timer 1 IRQ Enabled", ((memManager.IE >> 4) & 0x01) != 0),
			new RegEntry("$4000200.5", "Timer 2 IRQ Enabled", ((memManager.IE >> 5) & 0x01) != 0),
			new RegEntry("$4000200.6", "Timer 3 IRQ Enabled", ((memManager.IE >> 6) & 0x01) != 0),
			new RegEntry("$4000200.7", "Serial IRQ Enabled", ((memManager.IE >> 7) & 0x01) != 0),
			new RegEntry("$4000201.0", "DMA Channel 0 IRQ Enabled", ((memManager.IE >> 8) & 0x01) != 0),
			new RegEntry("$4000201.1", "DMA Channel 1 IRQ Enabled", ((memManager.IE >> 9) & 0x01) != 0),
			new RegEntry("$4000201.2", "DMA Channel 2 IRQ Enabled", ((memManager.IE >> 10) & 0x01) != 0),
			new RegEntry("$4000201.3", "DMA Channel 3 IRQ Enabled", ((memManager.IE >> 11) & 0x01) != 0),
			new RegEntry("$4000201.4", "Input IRQ Enabled", ((memManager.IE >> 12) & 0x01) != 0),
			new RegEntry("$4000201.5", "Cartridge IRQ Enabled", ((memManager.IE >> 13) & 0x01) != 0),

			new RegEntry("", "IE - IRQ Flags"),
			new RegEntry("$4000202-3", "IF - Register Value", memManager.IF, Format.X16),
			new RegEntry("$4000202.0", "Vertical Blank IRQ Active", ((memManager.IF >> 0) & 0x01) != 0),
			new RegEntry("$4000202.1", "Horizontal Blank IRQ Active", ((memManager.IF >> 1) & 0x01) != 0),
			new RegEntry("$4000202.2", "LYC Match IRQ Active", ((memManager.IF >> 2) & 0x01) != 0),
			new RegEntry("$4000202.3", "Timer 0 IRQ Active", ((memManager.IF >> 3) & 0x01) != 0),
			new RegEntry("$4000202.4", "Timer 1 IRQ Active", ((memManager.IF >> 4) & 0x01) != 0),
			new RegEntry("$4000202.5", "Timer 2 IRQ Active", ((memManager.IF >> 5) & 0x01) != 0),
			new RegEntry("$4000202.6", "Timer 3 IRQ Active", ((memManager.IF >> 6) & 0x01) != 0),
			new RegEntry("$4000202.7", "Serial IRQ Active", ((memManager.IF >> 7) & 0x01) != 0),
			new RegEntry("$4000203.0", "DMA Channel 0 IRQ Active", ((memManager.IF >> 8) & 0x01) != 0),
			new RegEntry("$4000203.1", "DMA Channel 1 IRQ Active", ((memManager.IF >> 9) & 0x01) != 0),
			new RegEntry("$4000203.2", "DMA Channel 2 IRQ Active", ((memManager.IF >> 10) & 0x01) != 0),
			new RegEntry("$4000203.3", "DMA Channel 3 IRQ Active", ((memManager.IF >> 11) & 0x01) != 0),
			new RegEntry("$4000203.4", "Input IRQ Active", ((memManager.IF >> 12) & 0x01) != 0),
			new RegEntry("$4000203.5", "Cartridge IRQ Active", ((memManager.IF >> 13) & 0x01) != 0),

			new RegEntry("$4000208.0", "IME", (memManager.IME & 0x01) != 0),

			new RegEntry("", "Misc"),
			new RegEntry("$4000204-5", "Wait Control", memManager.WaitControl),
			new RegEntry("$4000204.0-1", "SRAM (Bank $E)", memManager.SramWaitStates + " clocks", null),
			new RegEntry("$4000204.2-3", "Bank $8/9", memManager.PrgWaitStates0[0] + " clocks", null),
			new RegEntry("$4000204.4", "Bank $8/9 - Sequential", memManager.PrgWaitStates0[1] + " clocks", null),
			new RegEntry("$4000204.5-6", "Bank $A/B", memManager.PrgWaitStates1[0] + " clocks", null),
			new RegEntry("$4000204.7", "Bank $A/B - Sequential", memManager.PrgWaitStates1[1] + " clocks", null),
			new RegEntry("$4000205.0-1", "Bank $C/D", memManager.PrgWaitStates2[0] + " clocks", null),
			new RegEntry("$4000205.2", "Bank $C/D - Sequential", memManager.PrgWaitStates2[1] + " clocks", null),
			new RegEntry("$4000205.6", "Prefetch Enabled", memManager.PrefetchEnabled),
		});

		return new RegisterViewerTab("Misc", entries, CpuType.Gba, MemoryType.GbaMemory);

	}

	private static RegisterViewerTab GetGbaPpuTab(ref GbaState gbaState)
	{
		List<RegEntry> entries = new List<RegEntry>();

		GbaPpuState ppu = gbaState.Ppu;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry($"$4000000-1", "Control", ppu.Control | (ppu.Control2 << 8), Format.X16),
			new RegEntry($"$4000000.0-2", "BG Mode", ppu.BgMode),
			new RegEntry($"$4000000.4", "Show 2nd Frame", ppu.DisplayFrameSelect),
			new RegEntry($"$4000000.5", "HBlank OAM Access", ppu.AllowHblankOamAccess),
			new RegEntry($"$4000000.6", "Sequential OBJ Mapping", ppu.ObjVramMappingOneDimension),
			new RegEntry($"$4000000.7", "Forced Blank", ppu.ForcedBlank),

			new RegEntry($"$4000001.0", "BG0 Enabled", ppu.BgLayers[0].Enabled),
			new RegEntry($"$4000001.1", "BG1 Enabled", ppu.BgLayers[1].Enabled),
			new RegEntry($"$4000001.2", "BG2 Enabled", ppu.BgLayers[2].Enabled),
			new RegEntry($"$4000001.3", "BG3 Enabled", ppu.BgLayers[3].Enabled),
			new RegEntry($"$4000001.4", "OBJ Enabled", ppu.ObjLayerEnabled),
			new RegEntry($"$4000001.5", "Window 0 Enabled", ppu.Window0Enabled),
			new RegEntry($"$4000001.6", "Window 1 Enabled", ppu.Window1Enabled),
			new RegEntry($"$4000001.7", "OBJ Window Enabled", ppu.ObjWindowEnabled),

			new RegEntry($"$4000004", "Status", ppu.DispStat, Format.X8),

			//TODOGBA fix this to always match real value
			new RegEntry($"$4000004.0", "Vertical Blank", ppu.Scanline >= 160 && ppu.Scanline != 227),
			new RegEntry($"$4000004.1", "Horizontal Blank", ppu.Cycle > 1007),
			new RegEntry($"$4000004.2", "LYC Match", ppu.Scanline == ppu.Lyc),

			new RegEntry($"$4000004.3", "VBlank IRQ Enabled", ppu.VblankIrqEnabled),
			new RegEntry($"$4000004.4", "HBlank IRQ Enabled", ppu.HblankIrqEnabled),
			new RegEntry($"$4000004.5", "LYC IRQ Enabled", ppu.ScanlineIrqEnabled),
			new RegEntry($"$4000005", "LYC", ppu.Lyc, Format.X8),
			new RegEntry($"$4000006", "Scanline", ppu.Scanline, Format.X8),
			new RegEntry($"", "Cycle", ppu.Cycle),
			new RegEntry($"", "Frame Number", ppu.FrameCount),
		});

		for(int i = 0; i < 4; i++) {
			GbaBgConfig layer = ppu.BgLayers[i];
			int baseAddr = 0x4000008 + i * 2;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "BG Layer " + i),
				new RegEntry($"$4000001." + i, "BG" + i + " Enabled", ppu.BgLayers[i].Enabled),

				new RegEntry($"${baseAddr:X}.0-1", "Priority", layer.Priority),
				new RegEntry($"${baseAddr:X}.2-3", "Tileset Address", layer.TilesetAddr),
				new RegEntry($"${baseAddr:X}.6", "Mosaic Enabled", layer.Mosaic),
				new RegEntry($"${baseAddr:X}.7", "BPP Select", layer.Bpp8Mode ? "8 BPP" : "4 BPP", layer.Bpp8Mode),

				new RegEntry($"${baseAddr+1:X}.0-4", "Tilemap Address", layer.TilemapAddr),
			});

			if(i >= 2) {
				entries.Add(new RegEntry($"${baseAddr + 1:X}.5", "Wraparound", layer.WrapAround));
			}

			if(ppu.BgMode == 0 || i < 2) {
				entries.Add(new RegEntry($"${baseAddr + 1:X}.6-7", "Size",
					(layer.DoubleWidth ? "512" : "256") + "x" +
					(layer.DoubleHeight ? "512" : "256")
				, layer.ScreenSize));
			} else {
				int size = 128 << layer.ScreenSize;
				entries.Add(new RegEntry($"${baseAddr + 1:X}.6-7", "Size", size + "x" + size, layer.ScreenSize));
			}

			baseAddr = 0x4000010 + i * 4;
			entries.Add(new RegEntry($"${baseAddr:X}.0-15", "Scroll X", layer.ScrollX));
			entries.Add(new RegEntry($"${baseAddr + 2:X}.0-15", "Scroll Y", layer.ScrollY));

			if(i >= 2) {
				GbaTransformConfig cfg = ppu.Transform[i - 2];
				entries.Add(new RegEntry($"${0x4000020 + (i - 2) * 0x10:X}-1", "Param A", cfg.Matrix[0]));
				entries.Add(new RegEntry($"${0x4000022 + (i - 2) * 0x10:X}-3", "Param B", cfg.Matrix[1]));
				entries.Add(new RegEntry($"${0x4000024 + (i - 2) * 0x10:X}-5", "Param C", cfg.Matrix[2]));
				entries.Add(new RegEntry($"${0x4000026 + (i - 2) * 0x10:X}-7", "Param D", cfg.Matrix[3]));
				entries.Add(new RegEntry($"${0x4000028 + (i - 2) * 0x10:X}-B.0-27", "Origin X", ((int)cfg.OriginX << 4) >> 4, Format.X28));
				entries.Add(new RegEntry($"${0x400002C + (i - 2) * 0x10:X}-F.0-27", "Origin Y", ((int)cfg.OriginY << 4) >> 4, Format.X28));
			}
		}

		entries.Add(new RegEntry($"", "Windows"));
		entries.Add(new RegEntry($"", "Window 0"));
		entries.Add(new RegEntry($"$4000040", "Right X", ppu.Window[0].RightX));
		entries.Add(new RegEntry($"$4000041", "Left X", ppu.Window[0].LeftX));
		entries.Add(new RegEntry($"$4000044", "Bottom Y", ppu.Window[0].BottomY));
		entries.Add(new RegEntry($"$4000045", "Top Y", ppu.Window[0].TopY));

		entries.Add(new RegEntry($"$4000048.0", "BG0 Enabled", ppu.WindowActiveLayers[0] != 0));
		entries.Add(new RegEntry($"$4000048.1", "BG1 Enabled", ppu.WindowActiveLayers[1] != 0));
		entries.Add(new RegEntry($"$4000048.2", "BG2 Enabled", ppu.WindowActiveLayers[2] != 0));
		entries.Add(new RegEntry($"$4000048.3", "BG3 Enabled", ppu.WindowActiveLayers[3] != 0));
		entries.Add(new RegEntry($"$4000048.4", "OBJ Enabled", ppu.WindowActiveLayers[4] != 0));
		entries.Add(new RegEntry($"$4000048.5", "Color Effects Enabled", ppu.WindowActiveLayers[5] != 0));

		entries.Add(new RegEntry($"", "Window 1"));
		entries.Add(new RegEntry($"$4000042", "Right X", ppu.Window[1].RightX));
		entries.Add(new RegEntry($"$4000043", "Left X", ppu.Window[1].LeftX));
		entries.Add(new RegEntry($"$4000046", "Bottom Y", ppu.Window[1].BottomY));
		entries.Add(new RegEntry($"$4000047", "Top Y", ppu.Window[1].TopY));

		entries.Add(new RegEntry($"$4000049.0", "BG0 Enabled", ppu.WindowActiveLayers[6] != 0));
		entries.Add(new RegEntry($"$4000049.1", "BG1 Enabled", ppu.WindowActiveLayers[7] != 0));
		entries.Add(new RegEntry($"$4000049.2", "BG2 Enabled", ppu.WindowActiveLayers[8] != 0));
		entries.Add(new RegEntry($"$4000049.3", "BG3 Enabled", ppu.WindowActiveLayers[9] != 0));
		entries.Add(new RegEntry($"$4000049.4", "OBJ Enabled", ppu.WindowActiveLayers[10] != 0));
		entries.Add(new RegEntry($"$4000049.5", "Color Effects Enabled", ppu.WindowActiveLayers[11] != 0));

		entries.Add(new RegEntry($"", "Outside Window"));
		entries.Add(new RegEntry($"$400004A.0", "BG0 Enabled", ppu.WindowActiveLayers[18] != 0));
		entries.Add(new RegEntry($"$400004A.1", "BG1 Enabled", ppu.WindowActiveLayers[19] != 0));
		entries.Add(new RegEntry($"$400004A.2", "BG2 Enabled", ppu.WindowActiveLayers[20] != 0));
		entries.Add(new RegEntry($"$400004A.3", "BG3 Enabled", ppu.WindowActiveLayers[21] != 0));
		entries.Add(new RegEntry($"$400004A.4", "OBJ Enabled", ppu.WindowActiveLayers[22] != 0));
		entries.Add(new RegEntry($"$400004A.5", "Color Effects Enabled", ppu.WindowActiveLayers[23] != 0));

		entries.Add(new RegEntry($"", "Object Window"));
		entries.Add(new RegEntry($"$400004B.0", "BG0 Enabled", ppu.WindowActiveLayers[12] != 0));
		entries.Add(new RegEntry($"$400004B.1", "BG1 Enabled", ppu.WindowActiveLayers[13] != 0));
		entries.Add(new RegEntry($"$400004B.2", "BG2 Enabled", ppu.WindowActiveLayers[14] != 0));
		entries.Add(new RegEntry($"$400004B.3", "BG3 Enabled", ppu.WindowActiveLayers[15] != 0));
		entries.Add(new RegEntry($"$400004B.4", "OBJ Enabled", ppu.WindowActiveLayers[16] != 0));
		entries.Add(new RegEntry($"$400004B.5", "Color Effects Enabled", ppu.WindowActiveLayers[17] != 0));

		entries.Add(new RegEntry($"", "Mosaic"));
		entries.Add(new RegEntry($"$400004C.0-3", "BG Mosaic X Size", ppu.BgMosaicSizeX));
		entries.Add(new RegEntry($"$400004C.4-7", "BG Mosaic Y Size", ppu.BgMosaicSizeY));
		entries.Add(new RegEntry($"$400004D.0-3", "OBJ Mosaic X Size", ppu.ObjMosaicSizeX));
		entries.Add(new RegEntry($"$400004D.4-7", "OBJ Mosaic Y Size", ppu.ObjMosaicSizeY));

		entries.Add(new RegEntry($"", "Color Effects"));
		entries.Add(new RegEntry($"$4000050.0", "BG0 Main Target", ppu.BlendMain[0] != 0));
		entries.Add(new RegEntry($"$4000050.1", "BG1 Main Target", ppu.BlendMain[1] != 0));
		entries.Add(new RegEntry($"$4000050.2", "BG2 Main Target", ppu.BlendMain[2] != 0));
		entries.Add(new RegEntry($"$4000050.3", "BG3 Main Target", ppu.BlendMain[3] != 0));
		entries.Add(new RegEntry($"$4000050.4", "OBJ Main Target", ppu.BlendMain[4] != 0));
		entries.Add(new RegEntry($"$4000050.5", "Backdrop Main Target", ppu.BlendMain[5] != 0));
		entries.Add(new RegEntry($"$4000050.6-7", "Effect Type", ppu.BlendEffect));
		entries.Add(new RegEntry($"$4000051.0", "BG0 Sub Target", ppu.BlendSub[0] != 0));
		entries.Add(new RegEntry($"$4000051.1", "BG1 Sub Target", ppu.BlendSub[1] != 0));
		entries.Add(new RegEntry($"$4000051.2", "BG2 Sub Target", ppu.BlendSub[2] != 0));
		entries.Add(new RegEntry($"$4000051.3", "BG3 Sub Target", ppu.BlendSub[3] != 0));
		entries.Add(new RegEntry($"$4000051.4", "OBJ Sub Target", ppu.BlendSub[4] != 0));
		entries.Add(new RegEntry($"$4000051.5", "Backdrop Sub Target", ppu.BlendSub[5] != 0));
		entries.Add(new RegEntry($"$4000052.0-4", "Blend Main Coefficient", ppu.BlendMainCoefficient));
		entries.Add(new RegEntry($"$4000053.0-4", "Blend Sub Coefficient", ppu.BlendSubCoefficient));
		entries.Add(new RegEntry($"$4000054.0-4", "Brightness Coefficient", ppu.Brightness));

		return new RegisterViewerTab("PPU", entries, CpuType.Gba, MemoryType.GbaMemory);
	}

	private static RegisterViewerTab GetGbaApuTab(ref GbaState gbaState)
	{
		List<RegEntry> entries = new List<RegEntry>();

		GbaApuState apu = gbaState.Apu.Common;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry($"", "Volume"),
			new RegEntry("$4000080.0-2", "Volume Right (GB)", apu.RightVolume),
			new RegEntry("$4000080.4-6", "Volume Left (GB)", apu.LeftVolume),
			new RegEntry("$4000081.0", "Right Square 1 Enabled", apu.EnableRightSq1 != 0),
			new RegEntry("$4000081.1", "Right Square 2 Enabled", apu.EnableRightSq2 != 0),
			new RegEntry("$4000081.2", "Right Wave Enabled", apu.EnableRightWave != 0),
			new RegEntry("$4000081.3", "Right Noise Enabled", apu.EnableRightNoise != 0),
			new RegEntry("$4000081.4", "Left Square 1 Enabled", apu.EnableLeftSq1 != 0),
			new RegEntry("$4000081.5", "Left Square 2 Enabled", apu.EnableLeftSq2 != 0),
			new RegEntry("$4000081.6", "Left Wave Enabled", apu.EnableLeftWave != 0),
			new RegEntry("$4000081.7", "Left Noise Enabled", apu.EnableLeftNoise != 0),

			new RegEntry($"$4000082.0-1", "Game Boy Channels Volume", apu.GbVolume switch {
				0 => "25%",
				1 => "50%",
				2 => "100%",
				3 or _ => "Invalid"
			}, apu.GbVolume),
			new RegEntry($"$4000082.2", "Channel A Volume", apu.VolumeA == 0 ? "50%" : "100%", apu.VolumeA),
			new RegEntry($"$4000082.3", "Channel B Volume", apu.VolumeB == 0 ? "50%" : "100%", apu.VolumeB),

			new RegEntry($"", "Channel A"),
			new RegEntry($"$4000083.0", "Channel A Left Enabled", apu.EnableLeftA),
			new RegEntry($"$4000083.1", "Channel A Right Enabled", apu.EnableRightA),
			new RegEntry($"$4000083.2", "Channel A Timer", apu.TimerA),
			new RegEntry($"", "Current Output", apu.DmaSampleA),

			new RegEntry($"", "Channel B"),
			new RegEntry($"$4000083.4", "Channel B Left Enabled", apu.EnableLeftB),
			new RegEntry($"$4000083.5", "Channel B Right Enabled", apu.EnableRightB),
			new RegEntry($"$4000083.6", "Channel B Timer", apu.TimerB),
			new RegEntry($"", "Current Output", apu.DmaSampleB),

			new RegEntry($"", "Misc"),
			new RegEntry($"$4000084.7", "APU Enabled", apu.ApuEnabled),
			new RegEntry($"$4000088-9.1-9", "Sound Bias", apu.Bias),
			new RegEntry($"$4000088-9.14-15", "Sampling Rate", apu.SamplingRate switch {
				0 => "9 bit, 32,768 Hz",
				1 => "8 bit, 65,536 Hz",
				2 => "7 bit, 131,072 Hz",
				3 or _ => "6 bit, 262,144 Hz",
			}, apu.SamplingRate),

			new RegEntry("", "Frame Sequencer", apu.FrameSequenceStep),
		});

		GbaSquareState sq1 = gbaState.Apu.Square1;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$4000060-65", "Square 1"),
			new RegEntry("$4000060.0-2", "Sweep Shift", sq1.SweepShift),
			new RegEntry("$4000060.3", "Sweep Negate", sq1.SweepNegate),
			new RegEntry("$4000060.4-7", "Sweep Period", sq1.SweepPeriod),

			new RegEntry("$4000062.0-5", "Length", sq1.Length),
			new RegEntry("$4000062.6-7", "Duty", sq1.Duty),

			new RegEntry("$4000063.0-2", "Envelope Period", sq1.EnvPeriod),
			new RegEntry("$4000063.3", "Envelope Increase Volume", sq1.EnvRaiseVolume),
			new RegEntry("$4000063.4-7", "Envelope Volume", sq1.EnvVolume),

			new RegEntry("$4000064-65.0-10", "Frequency", sq1.Frequency),
			new RegEntry("$4000065.6", "Length Counter Enabled", sq1.LengthEnabled),
			new RegEntry("$4000065.7", "Channel Enabled", sq1.Enabled),

			new RegEntry("--", "Timer", sq1.Timer),
			new RegEntry("--", "Duty Position", sq1.DutyPos),
			new RegEntry("--", "Sweep Enabled", sq1.SweepEnabled),
			new RegEntry("--", "Sweep Frequency", sq1.SweepFreq),
			new RegEntry("--", "Sweep Timer", sq1.SweepTimer),
			new RegEntry("--", "Envelope Timer", sq1.EnvTimer),
			new RegEntry("--", "Output", sq1.Output)
		});

		GbaSquareState sq2 = gbaState.Apu.Square2;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$4000068-6D", "Square 2"),
			new RegEntry("$4000068.0-5", "Length", sq2.Length),
			new RegEntry("$4000068.6-7", "Duty", sq2.Duty),

			new RegEntry("$4000069.0-2", "Envelope Period", sq2.EnvPeriod),
			new RegEntry("$4000069.3", "Envelope Increase Volume", sq2.EnvRaiseVolume),
			new RegEntry("$4000069.4-7", "Envelope Volume", sq2.EnvVolume),

			new RegEntry("$400006C-6D.0-10", "Frequency", sq2.Frequency),
			new RegEntry("$400006D.6", "Length Counter Enabled", sq2.LengthEnabled),
			new RegEntry("$400006D.7", "Channel Enabled", sq2.Enabled),

			new RegEntry("--", "Timer", sq2.Timer),
			new RegEntry("--", "Duty Position", sq2.DutyPos),
			new RegEntry("--", "Envelope Timer", sq2.EnvTimer),
			new RegEntry("--", "Output", sq2.Output)
		});

		GbaWaveState wave = gbaState.Apu.Wave;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$4000070-75", "Wave"),
			new RegEntry("$4000070.7", "Sound Enabled", wave.DacEnabled),
			new RegEntry("$4000070.6", "Selected Bank", wave.SelectedBank),
			new RegEntry("$4000070.5", "Double Size", wave.DoubleLength),

			new RegEntry("$4000072", "Length", wave.Length),

			new RegEntry("$4000073.5-6", "Volume", wave.Volume),
			new RegEntry("$4000073.7", "Force 75% volume", wave.OverrideVolume),

			new RegEntry("$4000074-75.0-10", "Frequency", wave.Frequency),

			new RegEntry("$4000075.6", "Length Counter Enabled", wave.LengthEnabled),
			new RegEntry("$4000075.7", "Channel Enabled", wave.Enabled),

			new RegEntry("--", "Timer", wave.Timer),
			new RegEntry("--", "Sample Buffer", wave.SampleBuffer),
			new RegEntry("--", "Position", wave.Position),
			new RegEntry("--", "Output", wave.Output),
		});

		GbaNoiseState noise = gbaState.Apu.Noise;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$4000078-$400007D", "Noise"),
			new RegEntry("$4000078.0-5", "Length", noise.Length),

			new RegEntry("$4000079.0-2", "Envelope Period", noise.EnvPeriod),
			new RegEntry("$4000079.3", "Envelope Increase Volume", noise.EnvRaiseVolume),
			new RegEntry("$4000079.4-7", "Envelope Volume", noise.EnvVolume),

			new RegEntry("$400007C.0-2", "Divisor", noise.Divisor),
			new RegEntry("$400007C.3", "Short Mode", noise.ShortWidthMode),
			new RegEntry("$400007C.4-7", "Period Shift", noise.PeriodShift),

			new RegEntry("$400007D.6", "Length Counter Enabled", noise.LengthEnabled),
			new RegEntry("$400007D.7", "Channel Enabled", noise.Enabled),

			new RegEntry("--", "Timer", noise.Timer),
			new RegEntry("--", "Envelope Timer", noise.EnvTimer),
			new RegEntry("--", "Shift Register", noise.ShiftRegister, Format.X16),
			new RegEntry("--", "Output", noise.Output)
		});

		return new RegisterViewerTab("APU", entries, CpuType.Gba, MemoryType.GbaMemory);
	}

	private static RegisterViewerTab GetGbaTimerTab(ref GbaState gbaState)
	{
		List<RegEntry> entries = new List<RegEntry>();

		GbaTimersState timers = gbaState.Timer;
		for(int i = 0; i < 4; i++) {
			GbaTimerState timer = timers.Timer[i];
			int baseAddr = 0x4000100 + i * 4;

			byte prescaler = timer.PrescaleMask switch {
				0 => 0,
				0x3F => 1,
				0xFF => 2,
				_ or 0x3FF => 3,
			};

			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "Timer " + i),
				new RegEntry($"${baseAddr:X}-{(baseAddr+1)&0xF:X}", "Reload Value (W)", timer.ReloadValue),
				new RegEntry($"${baseAddr:X}-{(baseAddr+1)&0xF:X}", "Timer Value (R)", timer.Timer),
				new RegEntry($"${baseAddr+2:X}.0-1", "Prescale", prescaler + " (" + (timer.PrescaleMask + 1) + ")", prescaler),
				new RegEntry($"${baseAddr+2:X}.2", "Count up mode", timer.Mode),
				new RegEntry($"${baseAddr+2:X}.6", "IRQ Enabled", timer.IrqEnabled),
				new RegEntry($"${baseAddr+2:X}.7", "Enabled", timer.Enabled),
			});
		}

		return new RegisterViewerTab("Timers", entries, CpuType.Gba, MemoryType.GbaMemory);
	}

	private static RegisterViewerTab GetGbaDmaTab(ref GbaState gbaState)
	{
		List<RegEntry> entries = new List<RegEntry>();

		GbaDmaControllerState state = gbaState.Dma;
		for(int i = 0; i < 4; i++) {
			GbaDmaChannel ch = state.Ch[i];
			int baseAddr = 0x40000B0 + i * 12;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "DMA Channel " + i),
				new RegEntry($"${baseAddr:X}-{(baseAddr+3)&0xF:X}", "Source", ch.Source),
				new RegEntry($"${baseAddr+4:X}-{(baseAddr+7)&0xF:X}", "Destination", ch.Destination),
				new RegEntry($"${baseAddr+8:X}-{(baseAddr+9)&0xF:X}", "Length", ch.Length),
				new RegEntry($"${baseAddr+10:X}-{(baseAddr+11)&0xF:X}.5-6", "Dst Mode", ch.DestMode),
				new RegEntry($"${baseAddr+10:X}-{(baseAddr+11)&0xF:X}.7-8", "Src Mode", ch.SrcMode),
				new RegEntry($"${baseAddr+10:X}-{(baseAddr+11)&0xF:X}.9", "Repeat", ch.Repeat),
				new RegEntry($"${baseAddr+10:X}-{(baseAddr+11)&0xF:X}.10", "32-bit transfer", ch.WordTransfer),
				new RegEntry($"${baseAddr+10:X}-{(baseAddr+11)&0xF:X}.11", "DRQ", ch.DrqMode),
				new RegEntry($"${baseAddr+10:X}-{(baseAddr+11)&0xF:X}.12-13", "Trigger", ch.Trigger),
				new RegEntry($"${baseAddr+10:X}-{(baseAddr+11)&0xF:X}.14", "IRQ Enabled", ch.IrqEnabled),
				new RegEntry($"${baseAddr+10:X}-{(baseAddr+11)&0xF:X}.15", "Enabled", ch.Enabled),
				new RegEntry("", "Active", ch.Active),

				new RegEntry($"", "Current Src", ch.SrcLatch),
				new RegEntry($"", "Current Dest", ch.DestLatch),
			});
		}

		return new RegisterViewerTab("DMA", entries, CpuType.Gba, MemoryType.GbaMemory);
	}
}
