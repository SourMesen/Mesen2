using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System.Collections.Generic;
using static Mesen.Debugger.ViewModels.RegEntry;

namespace Mesen.Debugger.RegisterViewer;

public class SmsRegisterViewer
{
	public static List<RegisterViewerTab> GetTabs(ref SmsState smsState, RomFormat romFormat)
	{
		List<RegisterViewerTab> tabs = new() {
			GetSmsVdpTab(ref smsState),
			GetSmsPsgTab(ref smsState, romFormat == RomFormat.GameGear),
			GetSmsMiscTab(ref smsState),
		};
		return tabs;
	}

	private static RegisterViewerTab GetSmsVdpTab(ref SmsState sms)
	{
		List<RegEntry> entries = new List<RegEntry>();

		SmsVdpState vdp = sms.Vdp;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("", "State"),
			new RegEntry("", "Cycle (H)", vdp.Cycle),
			new RegEntry("", "Scanline (V)", vdp.Scanline),
			new RegEntry("", "Frame Number", vdp.FrameCount),

			new RegEntry("", "Ports"),
			new RegEntry("$7E", "Vertical counter", vdp.Scanline),
			new RegEntry("$7F", "Horizontal counter latch", vdp.HCounterLatch),
			new RegEntry("$BF.7", "Vertical blank IRQ pending", vdp.VerticalBlankIrqPending),
			new RegEntry("$BF.6", "Sprite overflow", vdp.SpriteOverflow),
			new RegEntry("$BF.5", "Sprite collision", vdp.SpriteCollision),
			new RegEntry("--", "Data port read buffer", vdp.ReadBuffer),
			new RegEntry("--", "Address register", vdp.AddressReg, Format.X16),
			new RegEntry("--", "Code register", vdp.CodeReg),
			new RegEntry("--", "Control port MSB toggle", vdp.ControlPortMsbToggle),

			new RegEntry("", "Registers"),
			new RegEntry("$00.0", "Sync disabled", vdp.SyncDisabled),
			new RegEntry("$00.1", "M2 - Allow 224/240-line mode", vdp.M2_AllowHeightChange),
			new RegEntry("$00.2", "M4 - Use mode 4", vdp.UseMode4),
			new RegEntry("$00.3", "Shift sprites left", vdp.ShiftSpritesLeft),
			new RegEntry("$00.4", "Scanline IRQ enabled", vdp.EnableScanlineIrq),
			new RegEntry("$00.5", "Mask first column", vdp.MaskFirstColumn),
			new RegEntry("$00.6", "Horizontal scroll lock", vdp.HorizontalScrollLock),
			new RegEntry("$00.7", "Vertical scroll lock", vdp.VerticalScrollLock),

			new RegEntry("$01.0", "Zoom sprites (2x size)", vdp.EnableDoubleSpriteSize),
			new RegEntry("$01.1", "Large sprites (8x16 or 16x16)", vdp.UseLargeSprites),
			new RegEntry("$01.3", "M3 - 240-line output", vdp.M3_Use240LineMode),
			new RegEntry("$01.4", "M1 - 224-line output", vdp.M1_Use224LineMode),
			new RegEntry("$01.5", "Vertical blank IRQ enabled", vdp.EnableVerticalBlankIrq),
			new RegEntry("$01.6", "Rendering enabled", vdp.RenderingEnabled),
			new RegEntry("$01.7", "SG-1000 - 16K VRAM Mode", vdp.Sg16KVramMode),

			new RegEntry("$02.0-3", "Nametable address", vdp.NametableAddress),
			new RegEntry("$03", "Color table address", vdp.ColorTableAddress),
			new RegEntry("$04.0-2", "Pattern table address", vdp.BgPatternTableAddress),
			new RegEntry("$05.0-6", "Sprite table address", vdp.SpriteTableAddress),
			new RegEntry("$06.0-2", "Sprite tileset address", vdp.SpritePatternSelector),
			new RegEntry("$07.0-3", "Background color index", vdp.BackgroundColorIndex),
			new RegEntry("$07.4-7", "Text color index", vdp.TextColorIndex),
			new RegEntry("$08", "Horizontal scroll", vdp.HorizontalScroll),
			new RegEntry("$09", "Vertical scroll", vdp.VerticalScroll),
			new RegEntry("$0A", "Scanline IRQ reload value", vdp.ScanlineCounter),
			new RegEntry("--", "Scanline IRQ counter", vdp.ScanlineCounterLatch),
		});

		return new RegisterViewerTab("VDP", entries, CpuType.Sms);
	}

	private static RegisterViewerTab GetSmsPsgTab(ref SmsState sms, bool isGameGear)
	{
		List<RegEntry> entries = new List<RegEntry>();

		SmsPsgState psg = sms.Psg;

		entries.Add(new RegEntry("", "Latched register", psg.SelectedReg));

		for(int i = 0; i < 3; i++) {
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "Tone " + (i + 1)),
				new RegEntry("", "Volume", psg.Tone[i].Volume, Format.X8),
				new RegEntry("", "Reload value", psg.Tone[i].ReloadValue, Format.X16),
				new RegEntry("", "Timer", psg.Tone[i].Timer, Format.X16),
				new RegEntry("", "Output", psg.Tone[i].Output, Format.X8),
			});
		}

		entries.AddRange(new List<RegEntry>() {
			new RegEntry("", "Noise"),
			new RegEntry("", "Volume", psg.Noise.Volume, Format.X8),
			new RegEntry("", "White noise mode", (psg.Noise.Control & 0x04) != 0),
			new RegEntry("", "Divider", psg.Noise.Control & 0x03),
			new RegEntry("", "Timer", psg.Noise.Timer, Format.X16),
			new RegEntry("", "Output", psg.Noise.Output),
		});

		if(isGameGear) {
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "Panning"),
				new RegEntry("$06.0", "Right tone 1 enabled", (psg.GameGearPanningReg & 0x01) != 0),
				new RegEntry("$06.1", "Right tone 2 enabled", (psg.GameGearPanningReg & 0x02) != 0),
				new RegEntry("$06.2", "Right tone 3 enabled", (psg.GameGearPanningReg & 0x04) != 0),
				new RegEntry("$06.3", "Right noise enabled", (psg.GameGearPanningReg & 0x08) != 0),
				new RegEntry("$06.4", "Left tone 1 enabled", (psg.GameGearPanningReg & 0x10) != 0),
				new RegEntry("$06.5", "Left tone 2 enabled", (psg.GameGearPanningReg & 0x20) != 0),
				new RegEntry("$06.6", "Left tone 3 enabled", (psg.GameGearPanningReg & 0x40) != 0),
				new RegEntry("$06.7", "Left noise enabled", (psg.GameGearPanningReg & 0x80) != 0),
			});
		}

		return new RegisterViewerTab("PSG", entries, CpuType.Sms);
	}

	private static RegisterViewerTab GetSmsMiscTab(ref SmsState sms)
	{
		List<RegEntry> entries = new List<RegEntry>();

		SmsControlManagerState ctrl = sms.ControlManager;
		SmsMemoryManagerState mem = sms.MemoryManager;

		entries.AddRange(new List<RegEntry>() {
			new RegEntry("", "Port $3E"),
			new RegEntry("$3E.2", "I/O disabled", !mem.IoEnabled),
			new RegEntry("$3E.3", "BIOS disabled", !mem.BiosEnabled),
			new RegEntry("$3E.4", "System RAM disabled", !mem.WorkRamEnabled),
			new RegEntry("$3E.5", "Card slot disabled", !mem.CardEnabled),
			new RegEntry("$3E.6", "Cartridge disabled", !mem.CartridgeEnabled),
			new RegEntry("$3E.7", "Expansion slot disabled", !mem.ExpEnabled),
			new RegEntry("", "Port $3F"),
			new RegEntry("$3F.0", "Port A TR pin direction", (ctrl.ControlPort & 0x01) != 0 ? "Input" : "Output", (ctrl.ControlPort & 0x01) != 0),
			new RegEntry("$3F.1", "Port A TH pin direction", (ctrl.ControlPort & 0x02) != 0 ? "Input" : "Output", (ctrl.ControlPort & 0x02) != 0),
			new RegEntry("$3F.2", "Port B TR pin direction", (ctrl.ControlPort & 0x04) != 0 ? "Input" : "Output", (ctrl.ControlPort & 0x04) != 0),
			new RegEntry("$3F.3", "Port B TH pin direction", (ctrl.ControlPort & 0x08) != 0 ? "Input" : "Output", (ctrl.ControlPort & 0x08) != 0),
			new RegEntry("$3F.4", "Port A TR output level", (ctrl.ControlPort & 0x10) != 0 ? "High" : "Low", (ctrl.ControlPort & 0x10) != 0),
			new RegEntry("$3F.5", "Port A TH output level", (ctrl.ControlPort & 0x20) != 0 ? "High" : "Low", (ctrl.ControlPort & 0x20) != 0),
			new RegEntry("$3F.6", "Port B TR output level", (ctrl.ControlPort & 0x40) != 0 ? "High" : "Low", (ctrl.ControlPort & 0x40) != 0),
			new RegEntry("$3F.7", "Port B TH output level", (ctrl.ControlPort & 0x80) != 0 ? "High" : "Low", (ctrl.ControlPort & 0x80) != 0)
		});

		return new RegisterViewerTab("Ports", entries, CpuType.Sms);
	}
}
