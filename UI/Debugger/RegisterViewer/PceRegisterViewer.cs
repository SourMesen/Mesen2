using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using static Mesen.Debugger.ViewModels.RegEntry;

namespace Mesen.Debugger.RegisterViewer;

public class PceRegisterViewer
{
	public static List<RegisterViewerTab> GetTabs(ref PceState pceState)
	{
		List<RegisterViewerTab> tabs = new List<RegisterViewerTab>() {
			GetPceCpuTab(ref pceState)
		};

		if(pceState.IsSuperGrafx) {
			tabs.Add(GetPceVdcTab(ref pceState.Video.Vdc, "1"));
			tabs.Add(GetPceVdcTab(ref pceState.Video.Vdc2, "2"));
			tabs.Add(GetPceVpcTab(ref pceState));
		} else {
			tabs.Add(GetPceVdcTab(ref pceState.Video.Vdc));
		}
		tabs.Add(GetPceVceTab(ref pceState));
		tabs.Add(GetPcePsgTab(ref pceState));

		if(pceState.HasCdRom) {
			tabs.Add(GetPceCdRomTab(ref pceState));
		}

		if(pceState.HasArcadeCard) {
			tabs.Add(GetPceArcadeCardTab(ref pceState));
		}

		return tabs;
	}

	private static RegisterViewerTab GetPceVceTab(ref PceState state)
	{
		ref PceVceState vce = ref state.Video.Vce;

		List<RegEntry> entries = new List<RegEntry>() {
			new RegEntry("$00.0-1", "CR - Clock Speed", vce.ClockDivider == 4 ? "5.37 MHz" : vce.ClockDivider == 3 ? "7.16 MHz" : "10.74 MHz", vce.ClockDivider),
			new RegEntry("$00.2", "CR - Number of Scanlines", vce.ScanlineCount),
			new RegEntry("$00.7", "CR - Grayscale", vce.Grayscale),
			new RegEntry("$01.0-8", "CTA - Color Table Address", vce.PalAddr, Format.X16),
		};

		return new RegisterViewerTab("VCE", entries, CpuType.Pce, MemoryType.PceMemory);
	}

	private static RegisterViewerTab GetPceVpcTab(ref PceState state)
	{
		ref PceVpcState vpc = ref state.Video.Vpc;

		List<RegEntry> entries = new List<RegEntry>() {
			new RegEntry("$08.0-3", "Priority Config (Both windows)"),
			new RegEntry("$08.0", "VDC1 Enabled", vpc.WindowCfg[3].Vdc1Enabled),
			new RegEntry("$08.1", "VDC2 Enabled", vpc.WindowCfg[3].Vdc2Enabled),
			new RegEntry("$08.2-3", "Priority Mode", (vpc.Priority1 >> 2) & 0x03),

			new RegEntry("$08.4-7", "Priority Config (Window 2 only)"),
			new RegEntry("$08.4", "VDC1 Enabled", vpc.WindowCfg[2].Vdc1Enabled),
			new RegEntry("$08.5", "VDC2 Enabled", vpc.WindowCfg[2].Vdc2Enabled),
			new RegEntry("$08.6-7", "Priority Mode", (vpc.Priority1 >> 6) & 0x03),

			new RegEntry("$09.0-3", "Priority Config (Window 1 only)"),
			new RegEntry("$09.0", "VDC1 Enabled", vpc.WindowCfg[1].Vdc1Enabled),
			new RegEntry("$09.1", "VDC2 Enabled", vpc.WindowCfg[1].Vdc2Enabled),
			new RegEntry("$09.2-3", "Priority Mode", (vpc.Priority2 >> 2) & 0x03),

			new RegEntry("$09.4-7", "Priority Config (No window)"),
			new RegEntry("$09.4", "VDC1 Enabled", vpc.WindowCfg[0].Vdc1Enabled),
			new RegEntry("$09.5", "VDC2 Enabled", vpc.WindowCfg[0].Vdc2Enabled),
			new RegEntry("$09.6-7", "Priority Mode", (vpc.Priority2 >> 6) & 0x03),

			new RegEntry("$0A-0D", "Windows"),
			new RegEntry("$0A-0B", "Window 1", vpc.Window1, Format.X16),
			new RegEntry("$0C-0D", "Window 2", vpc.Window2, Format.X16),

			new RegEntry("$0E", ""),
			new RegEntry("$0E", "STn writes to VDC2", vpc.StToVdc2Mode),
		};

		return new RegisterViewerTab("VPC", entries, CpuType.Pce, MemoryType.PceMemory);
	}

	private static RegisterViewerTab GetPceVdcTab(ref PceVdcState vdc, string suffix = "")
	{
		List<RegEntry> entries = new List<RegEntry>() {
			new RegEntry("", "State"),
			new RegEntry("", "HClock (H)", vdc.HClock, Format.X16),
			new RegEntry("", "Scanline (V)", vdc.Scanline, Format.X16),
			new RegEntry("", "Frame Number", vdc.FrameCount),

			new RegEntry("", "Selected Register", vdc.CurrentReg, Format.X8),

			new RegEntry("", "VDC Registers"),

			new RegEntry("", "Status"),
			new RegEntry("$00.0", "Sprite 0 Hit", vdc.Sprite0Hit),
			new RegEntry("$00.1", "Sprite Overflow", vdc.SpriteOverflow),
			new RegEntry("$00.2", "RCR Scanline Detected", vdc.ScanlineDetected),
			new RegEntry("$00.3", "SATB Transfer Completed", vdc.SatbTransferDone),
			new RegEntry("$00.4", "VRAM Transfer Completed", vdc.VramTransferDone),
			new RegEntry("$00.5", "Vertical Blank", vdc.VerticalBlank),
			//TODOv2
			//new RegEntry("$00.6", "Busy", ...),

			new RegEntry("", "VRAM Address/Data"),
			new RegEntry("$00", "MAWR - Memory Write Address", vdc.MemAddrWrite, Format.X16),
			new RegEntry("$01", "MARR - Memory Read Address", vdc.MemAddrRead, Format.X16),
			new RegEntry("$02", "VWR - VRAM Write Data", vdc.VramData, Format.X16),

			new RegEntry("$05", "CR - Control"),
			new RegEntry("$05.0", "Sprite 0 Hit IRQ Enabled", vdc.EnableCollisionIrq),
			new RegEntry("$05.1", "Overflow IRQ Enabled", vdc.EnableOverflowIrq),
			new RegEntry("$05.2", "Scanline Detect (RCR) IRQ Enabled", vdc.EnableScanlineIrq),
			new RegEntry("$05.3", "Vertical Blank IRQ Enabled", vdc.EnableVerticalBlankIrq),
			new RegEntry("$05.4-5", "External Sync",
				(vdc.OutputHorizontalSync ? "HSYNC Out" : "HSYNC In") + ", " +
				(vdc.OutputVerticalSync ? "VSYNC Out" : "VSYNC In")
			, null),
			new RegEntry("$05.6", "Sprites Enabled", vdc.NextSpritesEnabled),
			new RegEntry("$05.7", "Background Enabled", vdc.NextBackgroundEnabled),
			new RegEntry("$05.11-12", "VRAM Address Increment", vdc.VramAddrIncrement),

			new RegEntry("$06", "RCR - Raster Compare Register", vdc.RasterCompareRegister, Format.X16),
			new RegEntry("$07", "BXR - BG Scroll X", vdc.HvReg.BgScrollX, Format.X16),
			new RegEntry("$08", "BYR - BG Scroll Y", vdc.HvReg.BgScrollY, Format.X16),

			new RegEntry("$09", "MWR - Memory Width"),
			new RegEntry("$09.0-1", "VRAM Access Mode", vdc.HvReg.VramAccessMode),
			new RegEntry("$09.2-3", "Sprite Access Mode", vdc.HvReg.SpriteAccessMode),
			new RegEntry("$09.4-5", "Column Count", vdc.HvReg.ColumnCount),
			new RegEntry("$09.6", "Row Count", vdc.HvReg.RowCount),
			new RegEntry("$09.7", "CG Mode", vdc.HvReg.CgMode),

			new RegEntry("$0A", "HSR - Horizontal Sync"),
			new RegEntry("$0A.0-4", "HSW - Horizontal Sync Width", vdc.HvReg.HorizSyncWidth, Format.X8),
			new RegEntry("$0A.8-14", "HDS - Horizontal Display Start Position", vdc.HvReg.HorizDisplayStart, Format.X8),

			new RegEntry("$0B", "HDR - Horizontal Display"),
			new RegEntry("$0B.0-6", "HDW - Horizontal Display Width", vdc.HvReg.HorizDisplayWidth, Format.X8),
			new RegEntry("$0B.8-14", "HDE - Horizontal Display End Position", vdc.HvReg.HorizDisplayEnd, Format.X8),

			new RegEntry("$0C", "VPR - Vertical Sync"),
			new RegEntry("$0C.0-4", "VSW - Vertical Sync Width", vdc.HvReg.VertSyncWidth, Format.X8),
			new RegEntry("$0C.8-15", "VDS - Vertical Display Start Position", vdc.HvReg.VertDisplayStart, Format.X8),

			new RegEntry("$0D", "VDR - Vertical Display"),
			new RegEntry("$0D.0-8", "VDW - Vertical Display Width", vdc.HvReg.VertDisplayWidth, Format.X16),

			new RegEntry("$0E", "VCR - Vertical Display End"),
			new RegEntry("$0E.0-7", "VCR - Vertical Display End Position", vdc.HvReg.VertEndPosVcr, Format.X8),

			new RegEntry("$0F", "DCR - Block Transfer Control"),
			new RegEntry("$0F.0", "VRAM-SATB Transfer Complete IRQ Enabled", vdc.VramSatbIrqEnabled),
			new RegEntry("$0F.1", "VRAM-VRAM Transfer Complete IRQ Enabled", vdc.VramVramIrqEnabled),
			new RegEntry("$0F.2", "Decrement Source Address", vdc.DecrementSrc),
			new RegEntry("$0F.3", "Decrement Destination Address", vdc.DecrementDst),
			new RegEntry("$0F.4", "VRAM-SATB Transfer Auto-Repeat", vdc.RepeatSatbTransfer),

			new RegEntry("$10", "SOUR - Block Transfer Source Address", vdc.BlockSrc, Format.X16),
			new RegEntry("$11", "DESR - Block Transfer Source Address", vdc.BlockDst, Format.X16),
			new RegEntry("$12", "LENR - Block Transfer Source Address", vdc.BlockLen, Format.X16),
			new RegEntry("$13", "DVSSR - VRAM-SATB Transfer Source Address", vdc.SatbBlockSrc, Format.X16)
		};

		return new RegisterViewerTab("VDC" + suffix, entries);
	}

	private static RegisterViewerTab GetPceCpuTab(ref PceState state)
	{
		ref PceMemoryManagerState mem = ref state.MemoryManager;
		ref PceTimerState timer = ref state.Timer;

		List<RegEntry> entries = new List<RegEntry>() {
			new RegEntry("", "CPU Speed", mem.FastCpuSpeed ? "7.16 MHz" : "1.79 MHz", mem.FastCpuSpeed),

			new RegEntry("", "Timer"),
			new RegEntry("$C00.0-6", "Reload Value", timer.ReloadValue, Format.X8),
			new RegEntry("$C01.0", "Enabled", timer.Enabled),
			new RegEntry("", "Counter", timer.Counter, Format.X8),
			new RegEntry("", "Scaler", timer.Scaler, Format.X16),

			new RegEntry("", "IRQ"),
			new RegEntry("$1402", "Disabled IRQs"),
			new RegEntry("$1402.0", "IRQ2 (CDROM) Disabled", (mem.DisabledIrqs & 0x01) != 0),
			new RegEntry("$1402.1", "IRQ1 (VDC) Disabled", (mem.DisabledIrqs & 0x02) != 0),
			new RegEntry("$1402.2", "Timer IRQ Disabled", (mem.DisabledIrqs & 0x04) != 0),
			new RegEntry("$1403", "Active IRQs"),
			new RegEntry("$1403.0", "IRQ2 (CDROM)", (mem.ActiveIrqs & 0x01) != 0),
			new RegEntry("$1403.1", "IRQ1 (VDC)", (mem.ActiveIrqs & 0x02) != 0),
			new RegEntry("$1403.2", "Timer IRQ", (mem.ActiveIrqs & 0x04) != 0),

			new RegEntry("", "MPR"),
			new RegEntry("", "MPR #0", mem.Mpr[0], Format.X8),
			new RegEntry("", "MPR #1", mem.Mpr[1], Format.X8),
			new RegEntry("", "MPR #2", mem.Mpr[2], Format.X8),
			new RegEntry("", "MPR #3", mem.Mpr[3], Format.X8),
			new RegEntry("", "MPR #4", mem.Mpr[4], Format.X8),
			new RegEntry("", "MPR #5", mem.Mpr[5], Format.X8),
			new RegEntry("", "MPR #6", mem.Mpr[6], Format.X8),
			new RegEntry("", "MPR #7", mem.Mpr[7], Format.X8),
		};

		return new RegisterViewerTab("CPU", entries, CpuType.Pce, MemoryType.PceMemory);
	}

	private static RegisterViewerTab GetPcePsgTab(ref PceState pceState)
	{
		List<RegEntry> entries = new List<RegEntry>() {
			new RegEntry("$800.0-2", "Channel Select", pceState.Psg.ChannelSelect, Format.X8),
			new RegEntry("$801.0-3", "Right Amplitude", pceState.Psg.RightVolume, Format.X8),
			new RegEntry("$801.4-7", "Left Amplitude", pceState.Psg.LeftVolume, Format.X8),
			new RegEntry("$808.4-7", "LFO Frequency", pceState.Psg.LfoFrequency, Format.X8),
			new RegEntry("$809", "LFO Control", pceState.Psg.LfoControl, Format.X8),
		};

		for(int i = 0; i < 6; i++) {
			ref PcePsgChannelState ch = ref pceState.PsgChannels[i];

			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "Channel " + (i + 1)),
				new RegEntry("$802-$803", "Frequency", ch.Frequency, Format.X16),
				new RegEntry("$804.0-4", "Amplitude", ch.Amplitude, Format.X8),
				new RegEntry("$804.6", "DDA Enabled", ch.DdaEnabled),
				new RegEntry("$804.7", "Channel Enabled", ch.Enabled),
				new RegEntry("$805.0-3", "Right Amplitude", ch.RightVolume, Format.X8),
				new RegEntry("$805.4-7", "Left Amplitude", ch.LeftVolume, Format.X8),
				new RegEntry("$806.0-4", "DDA Output Value", ch.DdaOutputValue, Format.X8),
				new RegEntry("", "Timer", ch.Timer),
			});

			if(i >= 4) {
				entries.Add(new RegEntry("$807.7", "Noise Enabled", ch.NoiseEnabled));
				entries.Add(new RegEntry("$807.0-4", "Noise Frequency", ch.NoiseFrequency, Format.X8));
				entries.Add(new RegEntry("", "Noise Timer", ch.NoiseTimer));
				entries.Add(new RegEntry("", "Noise Output", ch.NoiseOutput == 0x0F ? 1 : 0));
				entries.Add(new RegEntry("", "Noise LSFR", ch.NoiseLfsr, Format.X24));
			}
		}

		return new RegisterViewerTab("PSG", entries, CpuType.Pce, MemoryType.PceMemory);
	}

	private static RegisterViewerTab GetPceCdRomTab(ref PceState pceState)
	{
		ref PceCdRomState cdrom = ref pceState.CdRom;
		ref PceCdAudioPlayerState player = ref pceState.CdPlayer;
		ref PceAudioFaderState fader = ref pceState.AudioFader;
		ref PceAdpcmState adpcm = ref pceState.Adpcm;
		ref PceScsiBusState scsi = ref pceState.ScsiDrive;

		List<RegEntry> entries = new List<RegEntry>() {
			new RegEntry("$1807.7", "BRAM Enabled", !cdrom.BramLocked),

			new RegEntry("", "ADPCM"),
			new RegEntry("$1808-$1809", "Address", adpcm.AddressPort, Format.X16),
			new RegEntry("$180A", "Write Buffer", adpcm.WriteBuffer, Format.X8),
			new RegEntry("$180A", "Read Buffer", adpcm.ReadBuffer, Format.X8),
			new RegEntry("$180B", "DMA Control", adpcm.DmaControl, Format.X8),
			new RegEntry("$180B.0", "DMA Requested", (adpcm.DmaControl & 0x01) != 0),
			new RegEntry("$180B.1", "DMA Enabled", (adpcm.DmaControl & 0x02) != 0),
			new RegEntry("$180C.0", "End Reached", adpcm.EndReached),
			new RegEntry("$180C.2", "Write Pending", adpcm.WriteClockCounter > 0),
			new RegEntry("$180C.3", "ADPCM Playing", adpcm.Playing),
			new RegEntry("$180C.7", "Read Pending", adpcm.ReadClockCounter > 0),
			new RegEntry("$180D", "Control", adpcm.Control),
			new RegEntry("$180D.4", "Latch Length", (adpcm.Control & 0x10) != 0),
			new RegEntry("$180D.5", "Play", (adpcm.Control & 0x20) != 0),
			new RegEntry("$180D.7", "Reset", (adpcm.Control & 0x80) != 0),
			new RegEntry("$180E", "Value", adpcm.PlaybackRate),
			new RegEntry("$180E.0-3", "Playback Rate", Math.Round(32000.0 / (16 - adpcm.PlaybackRate)) + " Hz", (adpcm.PlaybackRate & 0xF)),
			new RegEntry("", "Half Reached", adpcm.HalfReached),
			new RegEntry("", "ADPCM Length", adpcm.AdpcmLength, Format.X16),
			new RegEntry("", "Read Address", adpcm.ReadAddress, Format.X16),
			new RegEntry("", "Write Address", adpcm.WriteAddress, Format.X16),

			new RegEntry("$1802", "Enabled IRQs", cdrom.EnabledIrqs),
			new RegEntry("$1802.2", "ADPCM - Half Reached IRQ Enabled", (cdrom.EnabledIrqs & (int)PceCdRomIrqSource.Adpcm) != 0),
			new RegEntry("$1802.3", "ADPCM - End Reached IRQ Enabled", (cdrom.EnabledIrqs & (int)PceCdRomIrqSource.Stop) != 0),
			new RegEntry("$1802.5", "Transfer Done IRQ Enabled", (cdrom.EnabledIrqs & (int)PceCdRomIrqSource.DataTransferDone) != 0),
			new RegEntry("$1802.6", "Transfer Ready IRQ Enabled", (cdrom.EnabledIrqs & (int)PceCdRomIrqSource.DataTransferReady) != 0),

			new RegEntry("", "Active IRQs"),
			new RegEntry("", "ADPCM - Half Reached IRQ", (cdrom.ActiveIrqs & (int)PceCdRomIrqSource.Adpcm) != 0),
			new RegEntry("", "ADPCM - End Reached IRQ", (cdrom.ActiveIrqs & (int)PceCdRomIrqSource.Stop) != 0),
			new RegEntry("", "Transfer Done IRQ", (cdrom.ActiveIrqs & (int)PceCdRomIrqSource.DataTransferDone) != 0),
			new RegEntry("", "Transfer Ready IRQ", (cdrom.ActiveIrqs & (int)PceCdRomIrqSource.DataTransferReady) != 0),

			new RegEntry("", "SCSI Drive"),
			new RegEntry("", "Current Sector", scsi.Sector),
			new RegEntry("", "Read Until Sector", scsi.Sector + scsi.SectorsToRead),
			new RegEntry("$1801", "Data Port (Write)", scsi.DataPort),
			new RegEntry("$1801", "Data Port (Read)", scsi.ReadDataPort),
			new RegEntry("", "SCSI Phase", scsi.Phase),
			new RegEntry("", "SCSI Signals"),
			new RegEntry("$1800.3-7", "Status", (
				(scsi.Signals[4] != 0 ? 0x08 : 0) |
				(scsi.Signals[3] != 0 ? 0x10 : 0) |
				(scsi.Signals[5] != 0 ? 0x20 : 0) |
				(scsi.Signals[6] != 0 ? 0x40 : 0) |
				(scsi.Signals[2] != 0 ? 0x80 : 0)
			)),
			new RegEntry("", "ACK", scsi.Signals[0] != 0),
			//new RegEntry("", "ATN", scsi.Signals[1] != 0), //unused
			new RegEntry("$1800.7", "BSY", scsi.Signals[2] != 0),
			new RegEntry("$1800.4", "CD", scsi.Signals[3] != 0),
			new RegEntry("$1800.3", "IO", scsi.Signals[4] != 0),
			new RegEntry("$1800.5", "MSG", scsi.Signals[5] != 0),
			new RegEntry("$1800.6", "REQ", scsi.Signals[6] != 0),
			new RegEntry("$1804.1", "RST", scsi.Signals[7] != 0),
			new RegEntry("", "SEL", scsi.Signals[8] != 0),

			new RegEntry("", "CD Audio Player"),
			new RegEntry("", "CD Audio Status", player.Status),
			new RegEntry("", "Current Sector", player.CurrentSector, Format.X16),
			new RegEntry("", "Current Sample", player.CurrentSample, Format.X16),
			new RegEntry("", "Start Sector", player.StartSector, Format.X16),
			new RegEntry("", "End Sector", player.EndSector, Format.X16),
			new RegEntry("", "End Behavior", player.EndBehavior),

			new RegEntry("$180F", "Audio Fader", fader.RegValue),
			new RegEntry("$180F.1", "Target", fader.Target),
			new RegEntry("$180F.2", "Fade speed", fader.FastFade ? "2.5 secs" : "6 secs", fader.FastFade),
			new RegEntry("$180F.3", "Enabled", fader.Enabled),
			new RegEntry("", "Effective Volume", fader.Enabled ? (int)Math.Max(0.0, 100 - ((pceState.MemoryManager.CycleCount - fader.StartClock) / ((fader.FastFade ? 0.025 : 0.06) * 21477270))) : 100)
		};

		return new RegisterViewerTab("CD-ROM", entries, CpuType.Pce, MemoryType.PceMemory);
	}

	private static RegisterViewerTab GetPceArcadeCardTab(ref PceState pceState)
	{
		ref PceArcadeCardState state = ref pceState.ArcadeCard;

		List<RegEntry> entries = new List<RegEntry>() {
			new RegEntry("$1AE0-3", "Shift Register", state.ValueReg, Format.X32),
			new RegEntry("$1AE4", "Shift Value", state.ShiftReg, Format.X8),
			new RegEntry("$1AE5", "Rotate Value", state.RotateReg, Format.X8),
		};

		for(int i = 0; i < 4; i++) {
			ref PceArcadeCardPortConfig port = ref state.Port[i];

			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "Port " + (i + 1)),
				new RegEntry("$1A" + i + "2-4", "Base Address", port.BaseAddress, Format.X24),
				new RegEntry("$1A" + i + "5-6", "Offset", port.Offset, Format.X16),
				new RegEntry("$1A" + i + "7-8", "Increment Value", port.IncValue, Format.X16),
				new RegEntry("$1A" + i + "9", "Control", port.Control, Format.X8),
				new RegEntry("$1A" + i + "9.0", "Auto-increment", port.AutoIncrement),
				new RegEntry("$1A" + i + "9.1", "Add offset", port.AddOffset),
				new RegEntry("$1A" + i + "9.3", "Negative offset", port.SignedOffset),
				new RegEntry("$1A" + i + "9.4", "Add increment to base", port.AddIncrementToBase),
				new RegEntry("$1A" + i + "9.5-6", "Add offset trigger", port.AddOffsetTrigger)
			});
		}

		return new RegisterViewerTab("Arcade Card", entries, CpuType.Pce, MemoryType.PceMemory);
	}
}
