using Mesen.Config;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using static Mesen.Debugger.ViewModels.RegEntry;

namespace Mesen.Debugger.RegisterViewer;

public class WsRegisterViewer
{
	public static List<RegisterViewerTab> GetTabs(ref WsState wsState)
	{
		List<RegisterViewerTab> tabs = new() {
			GetPpuTab(ref wsState),
			GetApuTab(ref wsState),
			GetCartTab(ref wsState),
			GetDmaTab(ref wsState),
			GetIrqTab(ref wsState),
			GetTimerTab(ref wsState),
			GetMiscTab(ref wsState),
		};

		return tabs;
	}

	private static RegisterViewerTab GetPpuTab(ref WsState ws)
	{
		List<RegEntry> entries = new List<RegEntry>();
		WsPpuState ppu = ws.Ppu;

		byte volumeLevel = 0;
		if(ws.Model == WsModel.Monochrome) {
			switch(ws.Apu.InternalMasterVolume) {
				default: case 0: volumeLevel = 0; break;
				case 1: volumeLevel = 2; break;
				case 2: volumeLevel = 3; break;
			}
		} else {
			switch(ws.Apu.InternalMasterVolume) {
				default: case 0: volumeLevel = 0; break;
				case 1: volumeLevel = 2; break;
				case 2: volumeLevel = 1; break;
				case 3: volumeLevel = 3; break;
			}
		}

		bool headphoneIconVisible = false;
		bool volumeIconVisible = false;
		if(ppu.ShowVolumeIconFrame <= ppu.FrameCount && ppu.FrameCount - ppu.ShowVolumeIconFrame < 128) {
			//Show speaker/headphone icons if sound button was pressed within the last 128 frames
			if(ConfigManager.Config.Ws.AudioMode == WsAudioMode.Headphones) {
				headphoneIconVisible = true;
			} else {
				volumeIconVisible = true;
			}
		}

		entries.AddRange(new List<RegEntry>() {
			new RegEntry("", "State"),
			new RegEntry("", "Cycle (H)", ppu.Cycle),
			new RegEntry("", "Scanline (V)", ppu.Scanline),
			new RegEntry("", "Frame Number", ppu.FrameCount),

			new RegEntry("", "Ports"),
			new RegEntry("$00.0", "Screen 1 Enabled", ppu.BgLayers[0].Enabled),
			new RegEntry("$00.1", "Screen 2 Enabled", ppu.BgLayers[1].Enabled),
			new RegEntry("$00.2", "Sprites Enabled", ppu.SpritesEnabled),
			new RegEntry("$00.3", "Sprite Window Enabled", ppu.SpriteWindow.Enabled),
			new RegEntry("$00.4", "Screen 2 Window Draw Outside", ppu.DrawOutsideBgWindow),
			new RegEntry("$00.5", "Screen 2 Window Enabled", ppu.BgWindow.Enabled),

			new RegEntry("$01", "Background Color (Mono)", ppu.BgColor & 0x07),
			new RegEntry("$01", "Background Color (WSC)", ppu.BgColor & 0x0F),
			new RegEntry("$01", "Background Color Palette (WSC)", (ppu.BgColor >> 4) & 0x0F),
			new RegEntry("$02", "Scanline", ppu.Scanline, Format.X8),
			new RegEntry("$03", "IRQ Scanline", ppu.IrqScanline, Format.X8),
			new RegEntry("$04", "Sprite Table Address", "$" + ppu.SpriteTableAddress.ToString("X4"), ppu.SpriteTableAddress >> 9),
			new RegEntry("$05.0-6", "First Sprite Index", ppu.FirstSpriteIndex, Format.X8),
			new RegEntry("$06", "Sprite Count", ppu.SpriteCount, Format.X8),
			new RegEntry("$07.0-3", "Screen 1 Address", "$" + ppu.BgLayers[0].MapAddress.ToString("X4"), ppu.BgLayers[0].MapAddress >> 11),
			new RegEntry("$07.4-7", "Screen 1 Address", "$" + ppu.BgLayers[1].MapAddress.ToString("X4"), ppu.BgLayers[1].MapAddress >> 11),
			new RegEntry("$08", "Screen 2 Window Left", ppu.BgWindow.Left, Format.X8),
			new RegEntry("$09", "Screen 2 Window Top", ppu.BgWindow.Top, Format.X8),
			new RegEntry("$0A", "Screen 2 Window Right", ppu.BgWindow.Right, Format.X8),
			new RegEntry("$0B", "Screen 2 Window Bottom", ppu.BgWindow.Bottom, Format.X8),
			new RegEntry("$0C", "Sprite Window Left", ppu.SpriteWindow.Left, Format.X8),
			new RegEntry("$0D", "Sprite Window Top", ppu.SpriteWindow.Top, Format.X8),
			new RegEntry("$0E", "Sprite Window Right", ppu.SpriteWindow.Right, Format.X8),
			new RegEntry("$0F", "Sprite Window Bottom", ppu.SpriteWindow.Bottom, Format.X8),

			new RegEntry("$10", "Screen 1 Scroll X", ppu.BgLayers[0].ScrollX, Format.X8),
			new RegEntry("$11", "Screen 1 Scroll Y", ppu.BgLayers[0].ScrollY, Format.X8),
			new RegEntry("$12", "Screen 2 Scroll X", ppu.BgLayers[1].ScrollX, Format.X8),
			new RegEntry("$13", "Screen 2 Scroll Y", ppu.BgLayers[1].ScrollY, Format.X8),

			new RegEntry("$14.0", "LCD Enabled", ppu.LcdEnabled),
			new RegEntry("$14.1", "LCD High Contrast", ppu.HighContrast),

			new RegEntry("$16", "Last Scanline", ppu.LastScanline, Format.X8),
			new RegEntry("$17", "Back Porch Scanline", ppu.BackPorchScanline, Format.X8),

			new RegEntry("$1A.0", "LCD Sleep Mode", ppu.SleepEnabled),
			new RegEntry("$1A.1", "Headphone Icon Visible", headphoneIconVisible),
			new RegEntry("$1A.2-3", "Volume Level", volumeLevel),
			new RegEntry("$1A.4", "Volume Icon Visible", volumeIconVisible),

			new RegEntry("$15", "Icons"),
			new RegEntry("$15.0", "Sleep", ppu.Icons.Sleep),
			new RegEntry("$15.1", "Vertical", ppu.Icons.Vertical),
			new RegEntry("$15.2", "Horizontal", ppu.Icons.Horizontal),
			new RegEntry("$15.3", "Aux 1", ppu.Icons.Aux1),
			new RegEntry("$15.4", "Aux 2", ppu.Icons.Aux2),
			new RegEntry("$15.5", "Aux 3", ppu.Icons.Aux3),

			new RegEntry("$70-77", "TFT LCD Config"),
			new RegEntry("$70", "", ppu.LcdTftConfig[0], Format.X8),
			new RegEntry("$71", "", ppu.LcdTftConfig[1], Format.X8),
			new RegEntry("$72", "", ppu.LcdTftConfig[2], Format.X8),
			new RegEntry("$73", "", ppu.LcdTftConfig[3], Format.X8),
			new RegEntry("$74", "", ppu.LcdTftConfig[4], Format.X8),
			new RegEntry("$75", "", ppu.LcdTftConfig[5], Format.X8),
			new RegEntry("$76", "", ppu.LcdTftConfig[6], Format.X8),
			new RegEntry("$77", "", ppu.LcdTftConfig[7], Format.X8),
		});

		return new RegisterViewerTab("PPU", entries, CpuType.Ws, MemoryType.WsPort);
	}

	private static RegisterViewerTab GetApuTab(ref WsState ws)
	{
		List<RegEntry> entries = new List<RegEntry>();

		WsApuState apu = ws.Apu;

		int rightOutput = (
			apu.Ch1.RightOutput +
			apu.Ch2.RightOutput +
			apu.Ch3.RightOutput +
			apu.Ch4.RightOutput
		);

		int leftOutput = (
			apu.Ch1.LeftOutput +
			apu.Ch2.LeftOutput +
			apu.Ch3.LeftOutput +
			apu.Ch4.LeftOutput
		);

		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$91", "Sound Output Control"),
			new RegEntry("$91.0", "Speaker Enabled", apu.SpeakerEnabled),
			new RegEntry("$91.1-2", "Speaker Volume", apu.SpeakerVolume switch {
				0 => "100%", 1 => "50%", 2 => "25%", 3 or _ => "12.5%"
			}, apu.SpeakerVolume),
			new RegEntry("$91.3", "Headphones Enabled", apu.HeadphoneEnabled),
			new RegEntry("$91.7", "Headphones Connected", ConfigManager.Config.Ws.AudioMode == WsAudioMode.Headphones),
			new RegEntry("$9E.0-1", "Master Volume", apu.MasterVolume),

			new RegEntry("", "Channel 1"),
			new RegEntry("$90.0", "Enabled", apu.Ch1.Enabled),
			new RegEntry("$80/1", "Frequency Divisor", apu.Ch1.Frequency, Format.X16),
			new RegEntry("$88.0-3", "Right Volume", apu.Ch1.RightVolume, Format.X8),
			new RegEntry("$88.4-7", "Left Volume", apu.Ch1.LeftVolume, Format.X8),
			new RegEntry("", "Left Output", apu.Ch1.LeftOutput, Format.X8),
			new RegEntry("", "Right Output", apu.Ch1.RightOutput, Format.X8),
			new RegEntry("", "Timer", apu.Ch1.Timer, Format.X16),
			new RegEntry("", "Sample Position", apu.Ch1.SamplePosition, Format.X8),

			new RegEntry("", "Channel 2 (Voice/PCM)"),
			new RegEntry("$90.1", "Enabled", apu.Ch2.Enabled),
			new RegEntry("$82/3", "Frequency Divisor", apu.Ch2.Frequency, Format.X16),
			new RegEntry("$89.0-3", "Right Volume", apu.Ch2.RightVolume, Format.X8),
			new RegEntry("$89.4-7", "Left Volume", apu.Ch2.LeftVolume, Format.X8),
			new RegEntry("$90.5", "PCM Enabled", apu.Ch2.PcmEnabled),
			new RegEntry("$89", "PCM Value", (apu.Ch2.LeftVolume << 4) | apu.Ch2.RightVolume, Format.X8),
			new RegEntry("$94.0-1", "PCM Right Volume", apu.Ch2.MaxPcmVolumeRight ? "100%" : (apu.Ch2.HalfPcmVolumeRight ? "50%" : "0%"), (apu.Ch2.MaxPcmVolumeRight ? 1 : 0) | (apu.Ch2.HalfPcmVolumeRight ? 2 : 0)),
			new RegEntry("$94.2-3", "PCM Left Volume", apu.Ch2.MaxPcmVolumeLeft ? "100%" : (apu.Ch2.HalfPcmVolumeLeft ? "50%" : "0%"), (apu.Ch2.MaxPcmVolumeLeft ? 1 : 0) | (apu.Ch2.HalfPcmVolumeLeft ? 2 : 0)),
			new RegEntry("", "Left Output", apu.Ch2.LeftOutput, Format.X8),
			new RegEntry("", "Right Output", apu.Ch2.RightOutput, Format.X8),
			new RegEntry("", "Timer", apu.Ch2.Timer, Format.X16),
			new RegEntry("", "Sample Position", apu.Ch2.SamplePosition, Format.X8),

			new RegEntry("", "Channel 3 (Sweep)"),
			new RegEntry("$90.2", "Enabled", apu.Ch3.Enabled),
			new RegEntry("$84/5", "Frequency Divisor", apu.Ch3.Frequency, Format.X16),
			new RegEntry("$8A.0-3", "Right Volume", apu.Ch3.RightVolume, Format.X8),
			new RegEntry("$8A.4-7", "Left Volume", apu.Ch3.LeftVolume, Format.X8),
			new RegEntry("$90.6", "Sweep Enabled", apu.Ch3.SweepEnabled),
			new RegEntry("$8C", "Sweep Value", apu.Ch3.SweepValue, Format.X8),
			new RegEntry("$8D.0-4", "Sweep Period", apu.Ch3.SweepPeriod, Format.X8),
			new RegEntry("", "Sweep Timer", apu.Ch3.SweepTimer, Format.X8),
			new RegEntry("", "Left Output", apu.Ch3.LeftOutput, Format.X8),
			new RegEntry("", "Right Output", apu.Ch3.RightOutput, Format.X8),
			new RegEntry("", "Timer", apu.Ch3.Timer, Format.X16),
			new RegEntry("", "Sample Position", apu.Ch3.SamplePosition, Format.X8),

			new RegEntry("", "Channel 4 (Noise)"),
			new RegEntry("$90.3", "Enabled", apu.Ch4.Enabled),
			new RegEntry("$86/7", "Frequency Divisor", apu.Ch4.Frequency, Format.X16),
			new RegEntry("$8B.0-3", "Right Volume", apu.Ch4.RightVolume, Format.X8),
			new RegEntry("$8B.4-7", "Left Volume", apu.Ch4.LeftVolume, Format.X8),
			new RegEntry("$90.7", "Noise Enabled", apu.Ch4.NoiseEnabled),
			new RegEntry("$8E.0-2", "Noise Tap Mode", apu.Ch4.TapMode, Format.X8),
			new RegEntry("$8E.4", "LFSR Enabled", apu.Ch4.LfsrEnabled),
			new RegEntry("$92/93.0-14", "LFSR Value", apu.Ch4.Lfsr),
			new RegEntry("", "Left Output", apu.Ch4.LeftOutput, Format.X8),
			new RegEntry("", "Right Output", apu.Ch4.RightOutput, Format.X8),
			new RegEntry("", "Timer", apu.Ch4.Timer, Format.X16),
			new RegEntry("", "Sample Position", apu.Ch4.SamplePosition, Format.X8),

			new RegEntry("", "Hyper Voice"),
			new RegEntry("$64/65", "Left Output", apu.Voice.LeftOutput, Format.X16),
			new RegEntry("$66/67", "Right Output", apu.Voice.RightOutput, Format.X16),
			new RegEntry("$69", "Left Sample", apu.Voice.LeftSample, Format.X8),
			new RegEntry("$69", "Right Sample", apu.Voice.RightSample, Format.X8),
			new RegEntry("$6A.0-1", "Volume", apu.Voice.Shift switch {
				0 => "100%", 1 => "50%", 2 => "25%", 3 or _ => "12.5%"
			}, apu.Voice.Shift),
			new RegEntry("$6A.2-3", "Sample Scaling Mode", apu.Voice.ScalingMode),
			new RegEntry("$6A.4-6", "Update Sample Rate", ((apu.Voice.ControlLow >> 4) & 0x07) switch {
				0 => "24 kHz", 1 => "12 kHz", 2 => "8 kHz", 3 => "6 kHz", 4 => "4.8 kHz", 5 => "4 kHz", 6 => "3 kHz", 7 or _ => "2 kHz"
			}, (apu.Voice.ControlLow >> 4) & 0x07),
			new RegEntry("$6A.7", "Enabled", apu.Voice.Enabled),
			new RegEntry("$6B.13-14", "Channel Mode", apu.Voice.ChannelMode),

			new RegEntry("$95", "Sound Test"),
			new RegEntry("$95.0", "Hold channels 1-4", apu.HoldChannels),
			new RegEntry("$95.1", "Use CPU clock for sweep", apu.Ch3.UseSweepCpuClock),
			new RegEntry("$95.2-3", "Hold noise LFSR", apu.Ch4.HoldLfsr),
			new RegEntry("$95.5", "Force output to (ch2 voice * 5)", apu.ForceOutputCh2Voice),
			new RegEntry("$95.6", "Force channels 1-4 output to 2", apu.ForceOutput2),
			new RegEntry("$95.7", "Force channels 1-4 output to 4", apu.ForceOutput4),

			new RegEntry("", "Sound Output"),
			new RegEntry("$96/7", "Right Output Sum", rightOutput, Format.X16),
			new RegEntry("$98/9", "Left Output Sum", leftOutput, Format.X16),
			new RegEntry("$99/A", "Output Sum", leftOutput+rightOutput, Format.X16),
		});

		return new RegisterViewerTab("APU", entries, CpuType.Ws, MemoryType.WsPort);
	}

	private static RegisterViewerTab GetDmaTab(ref WsState ws)
	{
		List<RegEntry> entries = new List<RegEntry>();

		WsDmaControllerState dma = ws.DmaController;

		entries.AddRange(new List<RegEntry>() {
			new RegEntry("", "General DMA"),
			new RegEntry("$40/41/42", "Source", dma.GdmaSrc),
			new RegEntry("$44/45", "Destination", dma.GdmaDest),
			new RegEntry("$46/47", "Length", dma.GdmaLength),

			new RegEntry("$48.6", "Decrement", (dma.GdmaControl & 0x40) != 0),
			new RegEntry("$48.7", "Enabled", (dma.GdmaControl & 0x80) != 0),

			new RegEntry("", "Sound DMA"),
			new RegEntry("$4A/4B/4C", "Source", dma.SdmaSrc),
			new RegEntry("$4E/4F/50", "Length", dma.SdmaLength),
			new RegEntry("$52.0-1", "Frequency", (dma.SdmaControl & 0x03) switch {
				0 => "4 kHz", 1 => "6 kHz", 2 => "12 kHz", 3 or _ => "24 kHz"
			}, dma.SdmaControl & 0x03),
			new RegEntry("$52.2", "Hold", dma.SdmaHold),
			new RegEntry("$52.3", "Auto-repeat", dma.SdmaHyperVoice),
			new RegEntry("$52.4", "Target", dma.SdmaHyperVoice ? "Hyper Voice" : "Ch 2 PCM", dma.SdmaHyperVoice ? 1 : 0),
			new RegEntry("$52.6", "Decrement", dma.SdmaDecrement),
			new RegEntry("$52.7", "Enabled", dma.SdmaEnabled),
			new RegEntry("", "Source Reload Value", dma.SdmaSrcReloadValue),
			new RegEntry("", "Length Reload Value", dma.SdmaLengthReloadValue)
		});

		return new RegisterViewerTab("DMA", entries, CpuType.Ws, MemoryType.WsPort);
	}

	private static RegisterViewerTab GetTimerTab(ref WsState ws)
	{
		List<RegEntry> entries = new List<RegEntry>();

		WsTimerState timer = ws.Timer;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$A2.0", "Horizontal Timer Enabled", timer.HBlankEnabled),
			new RegEntry("$A2.1", "Horizontal Timer Auto-Reload", timer.HBlankAutoReload),
			new RegEntry("$A2.2", "Vertical Timer Enabled", timer.VBlankEnabled),
			new RegEntry("$A2.3", "Vertital Timer Auto-Reload", timer.VBlankAutoReload),
			new RegEntry("$A4/5", "Horizontal Reload Value", timer.HReloadValue, Format.X16),
			new RegEntry("$A6/7", "Vertical Reload Value", timer.VReloadValue, Format.X16),
			new RegEntry("$A8/9", "Horizontal Timer", timer.HTimer, Format.X16),
			new RegEntry("$AA/B", "Vertical Timer", timer.VTimer, Format.X16),
		});

		return new RegisterViewerTab("Timer", entries, CpuType.Ws, MemoryType.WsPort);
	}

	private static RegisterViewerTab GetIrqTab(ref WsState ws)
	{
		List<RegEntry> entries = new List<RegEntry>();

		WsMemoryManagerState mm = ws.MemoryManager;
		WsSerialState serial = ws.Serial;

		byte irqVector = mm.IrqVectorOffset;

		//TODOWS cleanup
		byte activeIrqs = mm.ActiveIrqs;
		if(serial.Enabled && (mm.EnabledIrqs & (int)WsIrqSource.UartSendReady) != 0) {
			bool hasSendData = serial.HasSendData;
			if(hasSendData) {
				int cyclesPerByte = serial.HighSpeed ? 800 : 3200;
				int cyclesElapsed = (int)(ws.Cpu.CycleCount - serial.SendClock);
				if(cyclesElapsed > cyclesPerByte) {
					hasSendData = false;
				}
			}

			if(!hasSendData) {
				activeIrqs |= (int)WsIrqSource.UartSendReady;
			}
		}

		for(int i = 7; i >= 0; i--) {
			if((activeIrqs & mm.EnabledIrqs & (1 << i)) != 0) {
				irqVector += (byte)i;
				break;
			}
		}

		entries.AddRange(new List<RegEntry>() {
			new RegEntry("", "Interrupts"),
			new RegEntry("$B0.3-7", "IRQ Vector Offset (W)", mm.IrqVectorOffset),
			new RegEntry("$B0", "Active IRQ Vector (R)", irqVector, Format.X8),
			new RegEntry("$B2", "Enabled IRQs"),
			new RegEntry("$B2.0", "UART Send Ready", (mm.EnabledIrqs & (int)WsIrqSource.UartSendReady) != 0),
			new RegEntry("$B2.1", "Key Pressed", (mm.EnabledIrqs & (int)WsIrqSource.KeyPressed) != 0),
			new RegEntry("$B2.2", "Cartridge", (mm.EnabledIrqs & (int)WsIrqSource.Cart) != 0),
			new RegEntry("$B2.3", "UART Receive Ready", (mm.EnabledIrqs & (int)WsIrqSource.UartRecvReady) != 0),
			new RegEntry("$B2.4", "Scanline IRQ", (mm.EnabledIrqs & (int)WsIrqSource.Scanline) != 0),
			new RegEntry("$B2.5", "Vertical Timer IRQ", (mm.EnabledIrqs & (int)WsIrqSource.VerticalBlankTimer) != 0),
			new RegEntry("$B2.6", "Vertical Blank IRQ", (mm.EnabledIrqs & (int)WsIrqSource.VerticalBlank) != 0),
			new RegEntry("$B2.7", "Horizontal Timer IRQ", (mm.EnabledIrqs & (int)WsIrqSource.HorizontalBlankTimer) != 0),

			new RegEntry("$B4", "Active IRQs"),
			new RegEntry("$B4.0", "UART Send Ready", (activeIrqs & (int)WsIrqSource.UartSendReady) != 0),
			new RegEntry("$B4.1", "Key Pressed", (activeIrqs & (int)WsIrqSource.KeyPressed) != 0),
			new RegEntry("$B4.2", "Cartridge", (activeIrqs & (int)WsIrqSource.Cart) != 0),
			new RegEntry("$B4.3", "UART Receive Ready", (activeIrqs & (int)WsIrqSource.UartRecvReady) != 0),
			new RegEntry("$B4.4", "Scanline IRQ", (activeIrqs & (int)WsIrqSource.Scanline) != 0),
			new RegEntry("$B4.5", "Vertical Timer IRQ", (activeIrqs & (int)WsIrqSource.VerticalBlankTimer) != 0),
			new RegEntry("$B4.6", "Vertical Blank IRQ", (activeIrqs & (int)WsIrqSource.VerticalBlank) != 0),
			new RegEntry("$B4.7", "Horizontal Timer IRQ", (activeIrqs & (int)WsIrqSource.HorizontalBlankTimer) != 0),

			new RegEntry("", "Misc"),
			new RegEntry("$B7.4", "NMI on low battery", mm.EnableLowBatteryNmi),
		});

		return new RegisterViewerTab("IRQ", entries, CpuType.Ws, MemoryType.WsPort);
	}

	private static RegisterViewerTab GetCartTab(ref WsState ws)
	{
		List<RegEntry> entries = new List<RegEntry>();

		WsCartState cart = ws.Cart;

		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$C0", "ROM Linear Bank", cart.SelectedBanks[0], Format.X8),
			new RegEntry("$C1", "RAM Bank", cart.SelectedBanks[1], Format.X8),
			new RegEntry("$C2", "ROM0 Bank", cart.SelectedBanks[2], Format.X8),
			new RegEntry("$C3", "ROM1 Bank", cart.SelectedBanks[3], Format.X8),
		});

		if(ws.CartEeprom.Size != WsEepromSize.Size0) {
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "Cart EEPROM"),
				new RegEntry("$C4/C5", "Write Data", ws.CartEeprom.WriteBuffer, Format.X16),
				new RegEntry("$C4/C5", "Read Data", ws.CartEeprom.ReadBuffer, Format.X16),
				new RegEntry("$C6/C7", "Command", ws.CartEeprom.Command, Format.X16),
				new RegEntry("$C8.0", "Read Done", ws.CartEeprom.ReadDone),
				new RegEntry("$C8.1", "Idle", ws.CartEeprom.Idle)
			});
		}

		return new RegisterViewerTab("Cart", entries, CpuType.Ws, MemoryType.WsPort);
	}

	private static RegisterViewerTab GetMiscTab(ref WsState ws)
	{
		List<RegEntry> entries = new List<RegEntry>();

		WsMemoryManagerState mm = ws.MemoryManager;
		WsSerialState serial = ws.Serial;

		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$60", "System Control 2"),
			new RegEntry("$60.1", "SRAM Wait State", mm.SlowSram),
			new RegEntry("$60.3", "Cart I/O Wait State", mm.SlowPort),
			new RegEntry("$60.5", "4 BPP Packed Format", mm.Enable4bppPacked),
			new RegEntry("$60.6", "4 BPP Enabled", mm.Enable4bpp),
			new RegEntry("$60.7", "Color Enabled", mm.ColorEnabled),

			new RegEntry("$62", "System Control 3"),
			new RegEntry("$62.7", "SwanCrystal System", ws.Model == WsModel.SwanCrystal),

			new RegEntry("$A0", "System Control"),
			new RegEntry("$A0.0", "Boot ROM Disabled", mm.BootRomDisabled),
			new RegEntry("$A0.1", "Color System", ws.Model != WsModel.Monochrome),
			new RegEntry("$A0.2", "16-bit ROM Bus", mm.CartWordBus),
			new RegEntry("$A0.3", "ROM Wait State", mm.SlowRom),

			new RegEntry("", "Serial"),
			new RegEntry("$B1", "Receive Buffer (R)", serial.ReceiveBuffer, Format.X8),
			new RegEntry("$B1", "Send Buffer (W)", serial.SendBuffer, Format.X8),
			new RegEntry("$B3.0", "Receive Buffer Filled", serial.HasReceiveData),
			new RegEntry("$B3.1", "Receive Buffer Overflow", serial.ReceiveOverflow),
			new RegEntry("$B3.2", "Send Buffer Empty", !serial.HasSendData),
			new RegEntry("$B3.6", "High Speed", serial.HighSpeed),
			new RegEntry("$B3.7", "Enabled", serial.Enabled),

			new RegEntry("$B5", "Keypad"),
			new RegEntry("$B5.0-3", "Output Column", ws.ControlManager.InputSelect & 0x0F, Format.X8),
			new RegEntry("$B5.4-6", "Input Row", (ws.ControlManager.InputSelect & 0x70) >> 4, Format.X8),

			new RegEntry("", "Internal EEPROM"),
			new RegEntry("$BA/BB", "Write Data", ws.InternalEeprom.WriteBuffer, Format.X16),
			new RegEntry("$BA/BB", "Read Data", ws.InternalEeprom.ReadBuffer, Format.X16),
			new RegEntry("$BC/BD", "Command", ws.InternalEeprom.Command, Format.X16),
			new RegEntry("$BE.0", "Read Done", ws.InternalEeprom.ReadDone),
			new RegEntry("$BE.1", "Idle", ws.InternalEeprom.Idle),
			new RegEntry("$BE.7", "Write Protection", ws.InternalEeprom.InternalEepromWriteProtected),
		});

		return new RegisterViewerTab("Misc", entries, CpuType.Ws, MemoryType.WsPort);
	}
}
