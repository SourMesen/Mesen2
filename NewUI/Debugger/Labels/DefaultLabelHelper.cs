using Mesen.Interop;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.Labels
{
	public class DefaultLabelHelper
	{
		public static void SetDefaultLabels()
		{
			HashSet<CpuType> cpuTypes = EmuApi.GetRomInfo().CpuTypes;
			if(cpuTypes.Contains(CpuType.Gameboy)) {
				SetGameboyDefaultLabels();
			} else if(cpuTypes.Contains(CpuType.Nes)) {
				SetDefaultNesLabels();
			} else if(cpuTypes.Contains(CpuType.Snes)) {
				SetSnesDefaultLabels();
			}
		}

		private static void SetSnesDefaultLabels()
		{
			//B-Bus registers
			LabelManager.SetLabel(0x2100, SnesMemoryType.Register, "INIDISP", "Screen Display Register");
			LabelManager.SetLabel(0x2101, SnesMemoryType.Register, "OBSEL", "Object Size and Character Size Register");
			LabelManager.SetLabel(0x2102, SnesMemoryType.Register, "OAMADDL", "OAM Address Registers (Low)");
			LabelManager.SetLabel(0x2103, SnesMemoryType.Register, "OAMADDH", "OAM Address Registers (High)");
			LabelManager.SetLabel(0x2104, SnesMemoryType.Register, "OAMDATA", "OAM Data Write Register");
			LabelManager.SetLabel(0x2105, SnesMemoryType.Register, "BGMODE", "BG Mode and Character Size Register");
			LabelManager.SetLabel(0x2106, SnesMemoryType.Register, "MOSAIC", "Mosaic Register");
			LabelManager.SetLabel(0x2107, SnesMemoryType.Register, "BG1SC", "BG Tilemap Address Registers (BG1)");
			LabelManager.SetLabel(0x2108, SnesMemoryType.Register, "BG2SC", "BG Tilemap Address Registers (BG2)");
			LabelManager.SetLabel(0x2109, SnesMemoryType.Register, "BG3SC", "BG Tilemap Address Registers (BG3)");
			LabelManager.SetLabel(0x210A, SnesMemoryType.Register, "BG4SC", "BG Tilemap Address Registers (BG4)");
			LabelManager.SetLabel(0x210B, SnesMemoryType.Register, "BG12NBA", "BG Character Address Registers (BG1&2)");
			LabelManager.SetLabel(0x210C, SnesMemoryType.Register, "BG34NBA", "BG Character Address Registers (BG3&4)");
			LabelManager.SetLabel(0x210D, SnesMemoryType.Register, "BG1HOFS", "BG Scroll Registers (BG1)");
			LabelManager.SetLabel(0x210E, SnesMemoryType.Register, "BG1VOFS", "BG Scroll Registers (BG1)");
			LabelManager.SetLabel(0x210F, SnesMemoryType.Register, "BG2HOFS", "BG Scroll Registers (BG2)");
			LabelManager.SetLabel(0x2110, SnesMemoryType.Register, "BG2VOFS", "BG Scroll Registers (BG2)");
			LabelManager.SetLabel(0x2111, SnesMemoryType.Register, "BG3HOFS", "BG Scroll Registers (BG3)");
			LabelManager.SetLabel(0x2112, SnesMemoryType.Register, "BG3VOFS", "BG Scroll Registers (BG3)");
			LabelManager.SetLabel(0x2113, SnesMemoryType.Register, "BG4HOFS", "BG Scroll Registers (BG4)");
			LabelManager.SetLabel(0x2114, SnesMemoryType.Register, "BG4VOFS", "BG Scroll Registers (BG4)");
			LabelManager.SetLabel(0x2115, SnesMemoryType.Register, "VMAIN", "Video Port Control Register");
			LabelManager.SetLabel(0x2116, SnesMemoryType.Register, "VMADDL", "VRAM Address Registers (Low)");
			LabelManager.SetLabel(0x2117, SnesMemoryType.Register, "VMADDH", "VRAM Address Registers (High)");
			LabelManager.SetLabel(0x2118, SnesMemoryType.Register, "VMDATAL", "VRAM Data Write Registers (Low)");
			LabelManager.SetLabel(0x2119, SnesMemoryType.Register, "VMDATAH", "VRAM Data Write Registers (High)");
			LabelManager.SetLabel(0x211A, SnesMemoryType.Register, "M7SEL", "Mode 7 Settings Register");
			LabelManager.SetLabel(0x211B, SnesMemoryType.Register, "M7A", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x211C, SnesMemoryType.Register, "M7B", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x211D, SnesMemoryType.Register, "M7C", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x211E, SnesMemoryType.Register, "M7D", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x211F, SnesMemoryType.Register, "M7X", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x2120, SnesMemoryType.Register, "M7Y", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x2121, SnesMemoryType.Register, "CGADD", "CGRAM Address Register");
			LabelManager.SetLabel(0x2122, SnesMemoryType.Register, "CGDATA", "CGRAM Data Write Register");
			LabelManager.SetLabel(0x2123, SnesMemoryType.Register, "W12SEL", "Window Mask Settings Registers");
			LabelManager.SetLabel(0x2124, SnesMemoryType.Register, "W34SEL", "Window Mask Settings Registers");
			LabelManager.SetLabel(0x2125, SnesMemoryType.Register, "WOBJSEL", "Window Mask Settings Registers");
			LabelManager.SetLabel(0x2126, SnesMemoryType.Register, "WH0", "Window Position Registers (WH0)");
			LabelManager.SetLabel(0x2127, SnesMemoryType.Register, "WH1", "Window Position Registers (WH1)");
			LabelManager.SetLabel(0x2128, SnesMemoryType.Register, "WH2", "Window Position Registers (WH2)");
			LabelManager.SetLabel(0x2129, SnesMemoryType.Register, "WH3", "Window Position Registers (WH3)");
			LabelManager.SetLabel(0x212A, SnesMemoryType.Register, "WBGLOG", "Window Mask Logic registers (BG)");
			LabelManager.SetLabel(0x212B, SnesMemoryType.Register, "WOBJLOG", "Window Mask Logic registers (OBJ)");
			LabelManager.SetLabel(0x212C, SnesMemoryType.Register, "TM", "Screen Destination Registers");
			LabelManager.SetLabel(0x212D, SnesMemoryType.Register, "TS", "Screen Destination Registers");
			LabelManager.SetLabel(0x212E, SnesMemoryType.Register, "TMW", "Window Mask Destination Registers");
			LabelManager.SetLabel(0x212F, SnesMemoryType.Register, "TSW", "Window Mask Destination Registers");
			LabelManager.SetLabel(0x2130, SnesMemoryType.Register, "CGWSEL", "Color Math Registers");
			LabelManager.SetLabel(0x2131, SnesMemoryType.Register, "CGADSUB", "Color Math Registers");
			LabelManager.SetLabel(0x2132, SnesMemoryType.Register, "COLDATA", "Color Math Registers");
			LabelManager.SetLabel(0x2133, SnesMemoryType.Register, "SETINI", "Screen Mode Select Register");
			LabelManager.SetLabel(0x2134, SnesMemoryType.Register, "MPYL", "Multiplication Result Registers");
			LabelManager.SetLabel(0x2135, SnesMemoryType.Register, "MPYM", "Multiplication Result Registers");
			LabelManager.SetLabel(0x2136, SnesMemoryType.Register, "MPYH", "Multiplication Result Registers");
			LabelManager.SetLabel(0x2137, SnesMemoryType.Register, "SLHV", "Software Latch Register");
			LabelManager.SetLabel(0x2138, SnesMemoryType.Register, "OAMDATAREAD", "OAM Data Read Register");
			LabelManager.SetLabel(0x2139, SnesMemoryType.Register, "VMDATALREAD", "VRAM Data Read Register (Low)");
			LabelManager.SetLabel(0x213A, SnesMemoryType.Register, "VMDATAHREAD", "VRAM Data Read Register (High)");
			LabelManager.SetLabel(0x213B, SnesMemoryType.Register, "CGDATAREAD", "CGRAM Data Read Register");
			LabelManager.SetLabel(0x213C, SnesMemoryType.Register, "OPHCT", "Scanline Location Registers (Horizontal)");
			LabelManager.SetLabel(0x213D, SnesMemoryType.Register, "OPVCT", "Scanline Location Registers (Vertical)");
			LabelManager.SetLabel(0x213E, SnesMemoryType.Register, "STAT77", "PPU Status Register");
			LabelManager.SetLabel(0x213F, SnesMemoryType.Register, "STAT78", "PPU Status Register");
			LabelManager.SetLabel(0x2140, SnesMemoryType.Register, "APUIO0", "APU IO Registers");
			LabelManager.SetLabel(0x2141, SnesMemoryType.Register, "APUIO1", "APU IO Registers");
			LabelManager.SetLabel(0x2142, SnesMemoryType.Register, "APUIO2", "APU IO Registers");
			LabelManager.SetLabel(0x2143, SnesMemoryType.Register, "APUIO3", "APU IO Registers");
			LabelManager.SetLabel(0x2180, SnesMemoryType.Register, "WMDATA", "WRAM Data Register");
			LabelManager.SetLabel(0x2181, SnesMemoryType.Register, "WMADDL", "WRAM Address Registers");
			LabelManager.SetLabel(0x2182, SnesMemoryType.Register, "WMADDM", "WRAM Address Registers");
			LabelManager.SetLabel(0x2183, SnesMemoryType.Register, "WMADDH", "WRAM Address Registers");

			//A-Bus registers (CPU registers)
			LabelManager.SetLabel(0x4016, SnesMemoryType.Register, "JOYSER0", "Old Style Joypad Registers");
			LabelManager.SetLabel(0x4017, SnesMemoryType.Register, "JOYSER1", "Old Style Joypad Registers");

			LabelManager.SetLabel(0x4200, SnesMemoryType.Register, "NMITIMEN", "Interrupt Enable Register");
			LabelManager.SetLabel(0x4201, SnesMemoryType.Register, "WRIO", "IO Port Write Register");
			LabelManager.SetLabel(0x4202, SnesMemoryType.Register, "WRMPYA", "Multiplicand Registers");
			LabelManager.SetLabel(0x4203, SnesMemoryType.Register, "WRMPYB", "Multiplicand Registers");
			LabelManager.SetLabel(0x4204, SnesMemoryType.Register, "WRDIVL", "Divisor & Dividend Registers");
			LabelManager.SetLabel(0x4205, SnesMemoryType.Register, "WRDIVH", "Divisor & Dividend Registers");
			LabelManager.SetLabel(0x4206, SnesMemoryType.Register, "WRDIVB", "Divisor & Dividend Registers");
			LabelManager.SetLabel(0x4207, SnesMemoryType.Register, "HTIMEL", "IRQ Timer Registers (Horizontal - Low)");
			LabelManager.SetLabel(0x4208, SnesMemoryType.Register, "HTIMEH", "IRQ Timer Registers (Horizontal - High)");
			LabelManager.SetLabel(0x4209, SnesMemoryType.Register, "VTIMEL", "IRQ Timer Registers (Vertical - Low)");
			LabelManager.SetLabel(0x420A, SnesMemoryType.Register, "VTIMEH", "IRQ Timer Registers (Vertical - High)");
			LabelManager.SetLabel(0x420B, SnesMemoryType.Register, "MDMAEN", "DMA Enable Register");
			LabelManager.SetLabel(0x420C, SnesMemoryType.Register, "HDMAEN", "HDMA Enable Register");
			LabelManager.SetLabel(0x420D, SnesMemoryType.Register, "MEMSEL", "ROM Speed Register");
			LabelManager.SetLabel(0x4210, SnesMemoryType.Register, "RDNMI", "Interrupt Flag Registers");
			LabelManager.SetLabel(0x4211, SnesMemoryType.Register, "TIMEUP", "Interrupt Flag Registers");
			LabelManager.SetLabel(0x4212, SnesMemoryType.Register, "HVBJOY", "PPU Status Register");
			LabelManager.SetLabel(0x4213, SnesMemoryType.Register, "RDIO", "IO Port Read Register");
			LabelManager.SetLabel(0x4214, SnesMemoryType.Register, "RDDIVL", "Multiplication Or Divide Result Registers (Low)");
			LabelManager.SetLabel(0x4215, SnesMemoryType.Register, "RDDIVH", "Multiplication Or Divide Result Registers (High)");
			LabelManager.SetLabel(0x4216, SnesMemoryType.Register, "RDMPYL", "Multiplication Or Divide Result Registers (Low)");
			LabelManager.SetLabel(0x4217, SnesMemoryType.Register, "RDMPYH", "Multiplication Or Divide Result Registers (High)");
			LabelManager.SetLabel(0x4218, SnesMemoryType.Register, "JOY1L", "Controller Port Data Registers (Pad 1 - Low)");
			LabelManager.SetLabel(0x4219, SnesMemoryType.Register, "JOY1H", "Controller Port Data Registers (Pad 1 - High)");
			LabelManager.SetLabel(0x421A, SnesMemoryType.Register, "JOY2L", "Controller Port Data Registers (Pad 2 - Low)");
			LabelManager.SetLabel(0x421B, SnesMemoryType.Register, "JOY2H", "Controller Port Data Registers (Pad 2 - High)");
			LabelManager.SetLabel(0x421C, SnesMemoryType.Register, "JOY3L", "Controller Port Data Registers (Pad 3 - Low)");
			LabelManager.SetLabel(0x421D, SnesMemoryType.Register, "JOY3H", "Controller Port Data Registers (Pad 3 - High)");
			LabelManager.SetLabel(0x421E, SnesMemoryType.Register, "JOY4L", "Controller Port Data Registers (Pad 4 - Low)");
			LabelManager.SetLabel(0x421F, SnesMemoryType.Register, "JOY4H", "Controller Port Data Registers (Pad 4 - High)");

			//DMA registers
			for(uint i = 0; i < 8; i++) {
				LabelManager.SetLabel(0x4300 + i * 0x10, SnesMemoryType.Register, "DMAP" + i.ToString(), "(H)DMA Control");
				LabelManager.SetLabel(0x4301 + i * 0x10, SnesMemoryType.Register, "BBAD" + i.ToString(), "(H)DMA B-Bus Address");
				LabelManager.SetLabel(0x4302 + i * 0x10, SnesMemoryType.Register, "A1T" + i.ToString() + "L", "DMA A-Bus Address / HDMA Table Address (Low)");
				LabelManager.SetLabel(0x4303 + i * 0x10, SnesMemoryType.Register, "A1T" + i.ToString() + "H", "DMA A-Bus Address / HDMA Table Address (High)");
				LabelManager.SetLabel(0x4304 + i * 0x10, SnesMemoryType.Register, "A1B" + i.ToString(), "DMA A-Bus Address / HDMA Table Address (Bank)");
				LabelManager.SetLabel(0x4305 + i * 0x10, SnesMemoryType.Register, "DAS" + i.ToString() + "L", "DMA Size / HDMA Indirect Address (Low)");
				LabelManager.SetLabel(0x4306 + i * 0x10, SnesMemoryType.Register, "DAS" + i.ToString() + "H", "DMA Size / HDMA Indirect Address (High)");
				LabelManager.SetLabel(0x4307 + i * 0x10, SnesMemoryType.Register, "DAS" + i.ToString() + "B", "HDMA Indirect Address (Bank)");
				LabelManager.SetLabel(0x4308 + i * 0x10, SnesMemoryType.Register, "A2A" + i.ToString() + "L", "HDMA Mid Frame Table Address (Low)");
				LabelManager.SetLabel(0x4309 + i * 0x10, SnesMemoryType.Register, "A2A" + i.ToString() + "H", "HDMA Mid Frame Table Address (High)");
				LabelManager.SetLabel(0x430A + i * 0x10, SnesMemoryType.Register, "NTLR" + i.ToString(), "HDMA Line Counter");
			}

			//SPC registers
			LabelManager.SetLabel(0xF0, SnesMemoryType.SpcRam, "TEST", "Testing functions");
			LabelManager.SetLabel(0xF1, SnesMemoryType.SpcRam, "CONTROL", "I/O and Timer Control");
			LabelManager.SetLabel(0xF2, SnesMemoryType.SpcRam, "DSPADDR", "DSP Address");
			LabelManager.SetLabel(0xF3, SnesMemoryType.SpcRam, "DSPDATA", "DSP Data");
			LabelManager.SetLabel(0xF4, SnesMemoryType.SpcRam, "CPUIO1", "CPU I/O 1");
			LabelManager.SetLabel(0xF5, SnesMemoryType.SpcRam, "CPUIO2", "CPU I/O 1");
			LabelManager.SetLabel(0xF6, SnesMemoryType.SpcRam, "CPUIO3", "CPU I/O 1");
			LabelManager.SetLabel(0xF7, SnesMemoryType.SpcRam, "CPUIO4", "CPU I/O 1");
			LabelManager.SetLabel(0xF8, SnesMemoryType.SpcRam, "RAMREG1", "Memory Register 1");
			LabelManager.SetLabel(0xF9, SnesMemoryType.SpcRam, "RAMREG2", "Memory Register 2");
			LabelManager.SetLabel(0xFA, SnesMemoryType.SpcRam, "T0TARGET", "Timer 0 scaling target");
			LabelManager.SetLabel(0xFB, SnesMemoryType.SpcRam, "T1TARGET", "Timer 1 scaling target");
			LabelManager.SetLabel(0xFC, SnesMemoryType.SpcRam, "T2TARGET", "Timer 2 scaling target");
			LabelManager.SetLabel(0xFD, SnesMemoryType.SpcRam, "T0OUT", "Timer 0 output");
			LabelManager.SetLabel(0xFE, SnesMemoryType.SpcRam, "T1OUT", "Timer 1 output");
			LabelManager.SetLabel(0xFF, SnesMemoryType.SpcRam, "T2OUT", "Timer 2 output");
		}

		private static void SetGameboyDefaultLabels()
		{
			//LCD
			LabelManager.SetLabel(0xFF40, SnesMemoryType.GameboyMemory, "LCDC_FF40", "LCD Control");
			LabelManager.SetLabel(0xFF41, SnesMemoryType.GameboyMemory, "STAT_FF41", "LCD Status");
			LabelManager.SetLabel(0xFF42, SnesMemoryType.GameboyMemory, "SCY_FF42", "Scroll Y");
			LabelManager.SetLabel(0xFF43, SnesMemoryType.GameboyMemory, "SCX_FF43", "Scroll X");
			LabelManager.SetLabel(0xFF44, SnesMemoryType.GameboyMemory, "LY_FF44", "LCD Y-Coordinate");
			LabelManager.SetLabel(0xFF45, SnesMemoryType.GameboyMemory, "LYC_FF45", "LY Compare");

			LabelManager.SetLabel(0xFF47, SnesMemoryType.GameboyMemory, "BGP_FF47", "BG Palette Data");
			LabelManager.SetLabel(0xFF48, SnesMemoryType.GameboyMemory, "OBP0_FF48", "Object Palette 0 Data");
			LabelManager.SetLabel(0xFF49, SnesMemoryType.GameboyMemory, "OBP1_FF49", "Object Palette 1 Data");

			LabelManager.SetLabel(0xFF4A, SnesMemoryType.GameboyMemory, "WY_FF4A", "Window Y Position");
			LabelManager.SetLabel(0xFF4B, SnesMemoryType.GameboyMemory, "WX_FF4B", "Window X Position");

			//APU
			LabelManager.SetLabel(0xFF10, SnesMemoryType.GameboyMemory, "NR10_FF10", "Channel 1 Sweep");
			LabelManager.SetLabel(0xFF11, SnesMemoryType.GameboyMemory, "NR11_FF11", "Channel 1 Length/Wave Pattern Duty");
			LabelManager.SetLabel(0xFF12, SnesMemoryType.GameboyMemory, "NR12_FF12", "Channel 1 Volume Envelope");
			LabelManager.SetLabel(0xFF13, SnesMemoryType.GameboyMemory, "NR13_FF13", "Channel 1 Frequency Low");
			LabelManager.SetLabel(0xFF14, SnesMemoryType.GameboyMemory, "NR14_FF14", "Channel 1 Frequency High");

			LabelManager.SetLabel(0xFF16, SnesMemoryType.GameboyMemory, "NR21_FF16", "Channel 2 Length/Wave Pattern Duty");
			LabelManager.SetLabel(0xFF17, SnesMemoryType.GameboyMemory, "NR22_FF17", "Channel 2 Volume Envelope");
			LabelManager.SetLabel(0xFF18, SnesMemoryType.GameboyMemory, "NR23_FF18", "Channel 2 Frequency Low");
			LabelManager.SetLabel(0xFF19, SnesMemoryType.GameboyMemory, "NR24_FF19", "Channel 2 Frequency High");

			LabelManager.SetLabel(0xFF1A, SnesMemoryType.GameboyMemory, "NR30_FF1A", "Channel 3 On/Off ");
			LabelManager.SetLabel(0xFF1B, SnesMemoryType.GameboyMemory, "NR31_FF1B", "Channel 3 Length");
			LabelManager.SetLabel(0xFF1C, SnesMemoryType.GameboyMemory, "NR32_FF1C", "Channel 3 Output Level");
			LabelManager.SetLabel(0xFF1D, SnesMemoryType.GameboyMemory, "NR33_FF1D", "Channel 3 Frequency Low");
			LabelManager.SetLabel(0xFF1E, SnesMemoryType.GameboyMemory, "NR34_FF1E", "Channel 3 Frequency High");

			LabelManager.SetLabel(0xFF20, SnesMemoryType.GameboyMemory, "NR41_FF20", "Channel 4 Length");
			LabelManager.SetLabel(0xFF21, SnesMemoryType.GameboyMemory, "NR42_FF21", "Channel 4 Volume Envelope");
			LabelManager.SetLabel(0xFF22, SnesMemoryType.GameboyMemory, "NR43_FF22", "Channel 4 Polynomial Counter");
			LabelManager.SetLabel(0xFF23, SnesMemoryType.GameboyMemory, "NR44_FF23", "Channel 4 Loop");

			LabelManager.SetLabel(0xFF24, SnesMemoryType.GameboyMemory, "NR50_FF24", "Channel Volume");
			LabelManager.SetLabel(0xFF25, SnesMemoryType.GameboyMemory, "NR51_FF25", "Channel Left/Right");
			LabelManager.SetLabel(0xFF26, SnesMemoryType.GameboyMemory, "NR52_FF26", "Channel On/Off");

			//Others
			LabelManager.SetLabel(0xFF00, SnesMemoryType.GameboyMemory, "JOYP_FF00", "Joypad");
			LabelManager.SetLabel(0xFF01, SnesMemoryType.GameboyMemory, "SB_FF01", "Serial Data");
			LabelManager.SetLabel(0xFF02, SnesMemoryType.GameboyMemory, "SC_FF02", "Serial Control");

			LabelManager.SetLabel(0xFF04, SnesMemoryType.GameboyMemory, "DIV_FF04", "Divider");
			LabelManager.SetLabel(0xFF05, SnesMemoryType.GameboyMemory, "TIMA_FF05", "Timer Counter");
			LabelManager.SetLabel(0xFF06, SnesMemoryType.GameboyMemory, "TMA_FF06", "Timer Modulo");
			LabelManager.SetLabel(0xFF07, SnesMemoryType.GameboyMemory, "TAC_FF07", "Timer Control");

			LabelManager.SetLabel(0xFF0F, SnesMemoryType.GameboyMemory, "IF_FF0F", "Interrupt Flag");
			LabelManager.SetLabel(0xFFFF, SnesMemoryType.GameboyMemory, "IE_FFFF", "Interrupt Enable");

			LabelManager.SetLabel(0xFF46, SnesMemoryType.GameboyMemory, "DMA_FF46", "OAM DMA Start");
		}

		private static void SetDefaultNesLabels()
		{
			LabelManager.SetLabel(0x2000, SnesMemoryType.NesMemory, "PpuControl_2000", $"7  bit  0{Environment.NewLine}---- ----{Environment.NewLine}VPHB SINN{Environment.NewLine}|||| ||||{Environment.NewLine}|||| ||++- Base nametable address{Environment.NewLine}|||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00){Environment.NewLine}|||| |+--- VRAM address increment per CPU read/write of PPUDATA{Environment.NewLine}|||| |     (0: add 1, going across; 1: add 32, going down){Environment.NewLine}|||| +---- Sprite pattern table address for 8x8 sprites{Environment.NewLine}||||       (0: $0000; 1: $1000; ignored in 8x16 mode){Environment.NewLine}|||+------ Background pattern table address (0: $0000; 1: $1000){Environment.NewLine}||+------- Sprite size (0: 8x8; 1: 8x16){Environment.NewLine}|+-------- PPU master/slave select{Environment.NewLine}|          (0: read backdrop from EXT pins; 1: output color on EXT pins){Environment.NewLine}+--------- Generate an NMI at the start of the{Environment.NewLine}           vertical blanking interval (0: off; 1: on)");
			LabelManager.SetLabel(0x2001, SnesMemoryType.NesMemory, "PpuMask_2001", $"7  bit  0{Environment.NewLine}---- ----{Environment.NewLine}BGRs bMmG{Environment.NewLine}|||| ||||{Environment.NewLine}|||| |||+- Display type: (0: color, 1: grayscale){Environment.NewLine}|||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0: Hide{Environment.NewLine}|||| |+--- 1: Show sprites in leftmost 8 pixels of screen, 0: Hide{Environment.NewLine}|||| +---- 1: Show background{Environment.NewLine}|||+------ 1: Show sprites{Environment.NewLine}||+------- Emphasize red{Environment.NewLine}|+-------- Emphasize green{Environment.NewLine}+--------- Emphasize blue");
			LabelManager.SetLabel(0x2002, SnesMemoryType.NesMemory, "PpuStatus_2002", $"7  bit  0{Environment.NewLine}---- ----{Environment.NewLine}VSO. ....{Environment.NewLine}|||| ||||{Environment.NewLine}|||+-++++- Least significant bits previously written into a PPU register{Environment.NewLine}|||        (due to register not being updated for this address){Environment.NewLine}||+------- Sprite overflow. The intent was for this flag to be set{Environment.NewLine}||         whenever more than eight sprites appear on a scanline, but a{Environment.NewLine}||         hardware bug causes the actual behavior to be more complicated{Environment.NewLine}||         and generate false positives as well as false negatives; see{Environment.NewLine}||         PPU sprite evaluation. This flag is set during sprite{Environment.NewLine}||         evaluation and cleared at dot 1 (the second dot) of the{Environment.NewLine}||         pre-render line.{Environment.NewLine}|+-------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps{Environment.NewLine}|          a nonzero background pixel; cleared at dot 1 of the pre-render{Environment.NewLine}|          line.  Used for raster timing.{Environment.NewLine}+--------- Vertical blank has started (0: not in vblank; 1: in vblank).{Environment.NewLine}           Set at dot 1 of line 241 (the line *after* the post-render{Environment.NewLine}           line); cleared after reading $2002 and at dot 1 of the{Environment.NewLine}           pre-render line.");
			LabelManager.SetLabel(0x2003, SnesMemoryType.NesMemory, "OamAddr_2003", "Set OAM address - Write only");
			LabelManager.SetLabel(0x2004, SnesMemoryType.NesMemory, "OamData_2004", "Read/Write OAM data");
			LabelManager.SetLabel(0x2005, SnesMemoryType.NesMemory, "PpuScroll_2005", "Set PPU scroll, write twice - Write only");
			LabelManager.SetLabel(0x2006, SnesMemoryType.NesMemory, "PpuAddr_2006", "Set PPU address, write twice - Write only");
			LabelManager.SetLabel(0x2007, SnesMemoryType.NesMemory, "PpuData_2007", "Read/Write VRAM");

			LabelManager.SetLabel(0x4000, SnesMemoryType.NesMemory, "Sq0Duty_4000", $"DDLC VVVV{Environment.NewLine}Duty (D), envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)");
			LabelManager.SetLabel(0x4001, SnesMemoryType.NesMemory, "Sq0Sweep_4001", $"EPPP NSSS{Environment.NewLine}Sweep unit: enabled (E), period (P), negate (N), shift (S)");
			LabelManager.SetLabel(0x4002, SnesMemoryType.NesMemory, "Sq0Timer_4002", $"TTTT TTTT{Environment.NewLine}Timer low (T)");
			LabelManager.SetLabel(0x4003, SnesMemoryType.NesMemory, "Sq0Length_4003", $"LLLL LTTT{Environment.NewLine}Length counter load (L), timer high (T)");

			LabelManager.SetLabel(0x4004, SnesMemoryType.NesMemory, "Sq1Duty_4004", $"DDLC VVVV{Environment.NewLine}Duty (D), envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)");
			LabelManager.SetLabel(0x4005, SnesMemoryType.NesMemory, "Sq1Sweep_4005", $"EPPP NSSS{Environment.NewLine}Sweep unit: enabled (E), period (P), negate (N), shift (S)");
			LabelManager.SetLabel(0x4006, SnesMemoryType.NesMemory, "Sq1Timer_4006", $"TTTT TTTT{Environment.NewLine}Timer low (T)");
			LabelManager.SetLabel(0x4007, SnesMemoryType.NesMemory, "Sq1Length_4007", $"LLLL LTTT{Environment.NewLine}Length counter load (L), timer high (T)");

			LabelManager.SetLabel(0x4008, SnesMemoryType.NesMemory, "TrgLinear_4008", $"CRRR RRRR{Environment.NewLine}Length counter halt / linear counter control (C), linear counter load (R)");
			LabelManager.SetLabel(0x400A, SnesMemoryType.NesMemory, "TrgTimer_400A", $"TTTT TTTT{Environment.NewLine}Timer low (T)");
			LabelManager.SetLabel(0x400B, SnesMemoryType.NesMemory, "TrgLength_400B", $"LLLL LTTT{Environment.NewLine}Length counter load (L), timer high (T)");

			LabelManager.SetLabel(0x400C, SnesMemoryType.NesMemory, "NoiseVolume_400C", $"--LC VVVV{Environment.NewLine}Envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)");
			LabelManager.SetLabel(0x400E, SnesMemoryType.NesMemory, "NoisePeriod_400E", $"L--- PPPP{Environment.NewLine}Loop noise (L), noise period (P)");
			LabelManager.SetLabel(0x400F, SnesMemoryType.NesMemory, "NoiseLength_400F", $"LLLL L---{Environment.NewLine}Length counter load (L)");

			LabelManager.SetLabel(0x4010, SnesMemoryType.NesMemory, "DmcFreq_4010", $"IL-- RRRR{Environment.NewLine}IRQ enable (I), loop (L), frequency (R)");
			LabelManager.SetLabel(0x4011, SnesMemoryType.NesMemory, "DmcCounter_4011", $"-DDD DDDD{Environment.NewLine}Load counter (D)");
			LabelManager.SetLabel(0x4012, SnesMemoryType.NesMemory, "DmcAddress_4012", $"AAAA AAAA{Environment.NewLine}Sample address (A)");
			LabelManager.SetLabel(0x4013, SnesMemoryType.NesMemory, "DmcLength_4013", $"LLLL LLLL{Environment.NewLine}Sample length (L)");

			LabelManager.SetLabel(0x4014, SnesMemoryType.NesMemory, "SpriteDma_4014", "Writing $XX will upload 256 bytes of data from CPU page $XX00-$XXFF to the internal PPU OAM.");

			LabelManager.SetLabel(0x4015, SnesMemoryType.NesMemory, "ApuStatus_4015", $"Read:{Environment.NewLine}IF-D NT21{Environment.NewLine}DMC interrupt (I), frame interrupt (F), DMC active (D), length counter > 0 (N/T/2/1){Environment.NewLine + Environment.NewLine}Write:{Environment.NewLine}---D NT21{Environment.NewLine}Enable DMC (D), noise (N), triangle (T), and pulse channels (2/1)");

			LabelManager.SetLabel(0x4016, SnesMemoryType.NesMemory, "Ctrl1_4016", $"Read (NES - input):{Environment.NewLine}---4 3210{Environment.NewLine}Read data from controller port #1.{Environment.NewLine}{Environment.NewLine}Write:{Environment.NewLine}---- ---A{Environment.NewLine}Output data (strobe) to both controllers.");
			LabelManager.SetLabel(0x4017, SnesMemoryType.NesMemory, "Ctrl2_FrameCtr_4017", $"Read (NES - input):{Environment.NewLine}---4 3210{Environment.NewLine}Read data from controller port #2.{Environment.NewLine}{Environment.NewLine}Write (Frame counter): MI-- ----{Environment.NewLine}Mode (M, 0 = 4-step, 1 = 5-step), IRQ inhibit flag (I)");

			if(EmuApi.GetRomInfo().Format == RomFormat.Fds) {
				LabelManager.SetLabel(0x01F8, SnesMemoryType.NesPrgRom, "LoadFiles", "Input: Pointer to Disk ID, Pointer to File List" + Environment.NewLine + "Output: A = error #, Y = # of files loaded" + Environment.NewLine + "Desc: Loads files specified by DiskID into memory from disk. Load addresses are decided by the file's header.");
				LabelManager.SetLabel(0x0237, SnesMemoryType.NesPrgRom, "AppendFile", "Input: Pointer to Disk ID, Pointer to File Header" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Appends the file data given by DiskID to the disk. This means that the file is tacked onto the end of the disk, and the disk file count is incremented. The file is then read back to verify the write. If an error occurs during verification, the disk's file count is decremented (logically hiding the written file).");
				LabelManager.SetLabel(0x0239, SnesMemoryType.NesPrgRom, "WriteFile", "Input: Pointer to Disk ID, Pointer to File Header, A = file #" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Same as \"Append File\", but instead of writing the file to the end of the disk, A specifies the sequential position on the disk to write the file (0 is the first). This also has the effect of setting the disk's file count to the A value, therefore logically hiding any other files that may reside after the written one.");
				LabelManager.SetLabel(0x02B7, SnesMemoryType.NesPrgRom, "CheckFileCount", "Input: Pointer to Disk ID, A = # to set file count to" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Reads in disk's file count, compares it to A, then sets the disk's file count to A.");
				LabelManager.SetLabel(0x02BB, SnesMemoryType.NesPrgRom, "AdjustFileCount", "Input: Pointer to Disk ID, A = number to reduce current file count by" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Reads in disk's file count, decrements it by A, then writes the new value back.");
				LabelManager.SetLabel(0x0301, SnesMemoryType.NesPrgRom, "SetFileCount1", "Input: Pointer to Disk ID, A = file count minus one = # of the last file" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Set the file count to A + 1");
				LabelManager.SetLabel(0x0305, SnesMemoryType.NesPrgRom, "SetFileCount", "Input: Pointer to Disk ID, A = file count" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Set the file count to A");
				LabelManager.SetLabel(0x032A, SnesMemoryType.NesPrgRom, "GetDiskInfo", "Input: Pointer to Disk Info" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Fills DiskInfo up with data read off the current disk.");

				LabelManager.SetLabel(0x0445, SnesMemoryType.NesPrgRom, "CheckDiskHeader", "Input: Pointer to 10 byte string at $00 " + Environment.NewLine + "Output:  " + Environment.NewLine + "Desc: Compares the first 10 bytes on the disk coming after the FDS string, to 10 bytes pointed to by Ptr($00). To bypass the checking of any byte, a -1 can be placed in the equivelant place in the compare string.  Otherwise, if the comparison fails, an appropriate error will be generated.");
				LabelManager.SetLabel(0x0484, SnesMemoryType.NesPrgRom, "GetNumFiles", "Input:  " + Environment.NewLine + "Output:  " + Environment.NewLine + "Desc: Reads number of files stored on disk, stores the result in $06");
				LabelManager.SetLabel(0x0492, SnesMemoryType.NesPrgRom, "SetNumFiles", "Input:  " + Environment.NewLine + "Output: A = number of files " + Environment.NewLine + "Desc: Writes new number of files to disk header.");
				LabelManager.SetLabel(0x04A0, SnesMemoryType.NesPrgRom, "FileMatchTest", "Input: Pointer to FileID list at $02 " + Environment.NewLine + "Output:   " + Environment.NewLine + "Desc: Uses a byte string pointed at by Ptr($02) to tell the disk system which files to load.  The file ID's number is searched for in the string.  If an exact match is found, [$09] is 0'd, and [$0E] is incremented.  If no matches are found after 20 bytes, or a -1 entry is encountered, [$09] is set to -1.  If the first byte in the string is -1, the BootID number is used for matching files (any FileID that is not greater than the BootID qualifies as a match).");
				LabelManager.SetLabel(0x04DA, SnesMemoryType.NesPrgRom, "SkipFiles", "Input: Number of files to skip in $06 " + Environment.NewLine + "Output:  " + Environment.NewLine + "Desc: Skips over specified number of files.");

				LabelManager.SetLabel(0x0149, SnesMemoryType.NesPrgRom, "Delay132", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: " + Environment.NewLine + "Desc: 132 clock cycle delay");
				LabelManager.SetLabel(0x0153, SnesMemoryType.NesPrgRom, "Delayms", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: X, Y " + Environment.NewLine + "Desc: Delay routine, Y = delay in ms (approximate)");
				LabelManager.SetLabel(0x0161, SnesMemoryType.NesPrgRom, "DisPFObj", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Disable sprites and background");
				LabelManager.SetLabel(0x016B, SnesMemoryType.NesPrgRom, "EnPFObj", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Enable sprites and background");
				LabelManager.SetLabel(0x0171, SnesMemoryType.NesPrgRom, "DisObj", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Disable sprites");
				LabelManager.SetLabel(0x0178, SnesMemoryType.NesPrgRom, "EnObj", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Enable sprites");
				LabelManager.SetLabel(0x017E, SnesMemoryType.NesPrgRom, "DisPF", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Disable background");
				LabelManager.SetLabel(0x0185, SnesMemoryType.NesPrgRom, "EnPF", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Enable background");
				LabelManager.SetLabel(0x01B2, SnesMemoryType.NesPrgRom, "VINTWait", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: $ff " + Environment.NewLine + "Desc: Wait until next VBlank NMI fires, and return (for programs that does it the \"everything in main\" way). NMI vector selection at $100 is preserved, but further VBlanks are disabled.");
				LabelManager.SetLabel(0x07BB, SnesMemoryType.NesPrgRom, "VRAMStructWrite", "Input: Pointer to VRAM buffer to be written " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00, $01, $ff " + Environment.NewLine + "Desc: Set VRAM increment to 1 (clear [[PPUCTRL]]/$ff bit 2), and write a VRAM buffer to VRAM. Read below for information on the structure.");
				LabelManager.SetLabel(0x0844, SnesMemoryType.NesPrgRom, "FetchDirectPtr", "Input: " + Environment.NewLine + "Output: $00, $01 = pointer fetched " + Environment.NewLine + "Affects: A, X, Y, $05, $06 " + Environment.NewLine + "Desc: Fetch a direct pointer from the stack (the pointer should be placed after the return address of the routine that calls this one (see \"important notes\" above)), save the pointer at ($00) and fix the return address.");
				LabelManager.SetLabel(0x086A, SnesMemoryType.NesPrgRom, "WriteVRAMBuffer", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $301, $302 " + Environment.NewLine + "Desc: Write the VRAM Buffer at $302 to VRAM. Read below for information on the structure.");
				LabelManager.SetLabel(0x08B3, SnesMemoryType.NesPrgRom, "ReadVRAMBuffer", "Input: X = start address of read buffer, Y = # of bytes to read " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y " + Environment.NewLine + "Desc: Read individual bytes from VRAM to the VRAMBuffer. (see notes below)");
				LabelManager.SetLabel(0x08D2, SnesMemoryType.NesPrgRom, "PrepareVRAMString", "Input: A = High VRAM address, X = Low VRAM address, Y = string length, Direct Pointer = data to be written to VRAM " + Environment.NewLine + "Output: A = $ff : no error, A = $01 : string didn't fit in buffer " + Environment.NewLine + "Affects: A, X, Y, $00, $01, $02, $03, $04, $05, $06 " + Environment.NewLine + "Desc: This routine copies pointed data into the VRAM buffer.");
				LabelManager.SetLabel(0x08E1, SnesMemoryType.NesPrgRom, "PrepareVRAMStrings", "Input: A = High VRAM address, X = Low VRAM address, Direct pointer = data to be written to VRAM " + Environment.NewLine + "Output: A = $ff : no error, A = $01 : data didn't fit in buffer " + Environment.NewLine + "Affects: A, X, Y, $00, $01, $02, $03, $04, $05, $06 " + Environment.NewLine + "Desc: This routine copies a 2D string into the VRAM buffer. The first byte of the data determines the width and height of the following string (in tiles): Upper nybble = height, lower nybble = width.");
				LabelManager.SetLabel(0x094F, SnesMemoryType.NesPrgRom, "GetVRAMBufferByte", "Input: X = starting index of read buffer, Y = # of address to compare (starting at 1), $00, $01 = address to read from " + Environment.NewLine + "Output: carry clear : a previously read byte was returned, carry set : no byte was read, should wait next call to ReadVRAMBuffer " + Environment.NewLine + "Affects: A, X, Y " + Environment.NewLine + "Desc: This routine was likely planned to be used in order to avoid useless latency on a VRAM reads (see notes below). It compares the VRAM address in ($00) with the Yth (starting at 1) address of the read buffer. If both addresses match, the corresponding data byte is returned exit with c clear. If the addresses are different, the buffer address is overwritten by the address in ($00) and the routine exit with c set.");
				LabelManager.SetLabel(0x097D, SnesMemoryType.NesPrgRom, "Pixel2NamConv", "Input: $02 = Pixel X cord, $03 = Pixel Y cord " + Environment.NewLine + "Output: $00 = High nametable address, $01 = Low nametable address " + Environment.NewLine + "Affects: A " + Environment.NewLine + "Desc: This routine convert pixel screen coordinates to corresponding nametable address (assumes no scrolling, and points to first nametable at $2000-$23ff).");
				LabelManager.SetLabel(0x0997, SnesMemoryType.NesPrgRom, "Nam2PixelConv", "Input: $00 = High nametable address, $01 = low nametable address " + Environment.NewLine + "Output: $02 = Pixel X cord, $03 = Pixel Y cord " + Environment.NewLine + "Affects: A " + Environment.NewLine + "Desc: This routine convert a nametable address to corresponding pixel coordinates (assume no scrolling).");
				LabelManager.SetLabel(0x09B1, SnesMemoryType.NesPrgRom, "Random", "Input: X = Zero Page address where the random bytes are placed, Y = # of shift register bytes (normally $02) " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00 " + Environment.NewLine + "Desc: This is a shift-register based random number generator, normally takes 2 bytes (using more won't affect random sequence). On reset you are supposed to write some non-zero values here (BIOS uses writes $d0, $d0), and call this routine several times before the data is actually random. Each call of this routine will shift the bytes ''right''.");
				LabelManager.SetLabel(0x09C8, SnesMemoryType.NesPrgRom, "SpriteDMA", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A " + Environment.NewLine + "Desc: This routine does sprite DMA from RAM $200-$2ff");
				LabelManager.SetLabel(0x09D3, SnesMemoryType.NesPrgRom, "CounterLogic", "Input: A, Y = end Zeropage address of counters, X = start zeropage address of counters " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, $00 " + Environment.NewLine + "Desc: This decrements several counters in Zeropage. The first counter is a decimal counter 9 -> 8 -> 7 -> ... -> 1 -> 0 -> 9 -> ... Counters 1...A are simply decremented and stays at 0. Counters A+1...Y are decremented when the first counter does a 0 -> 9 transition, and stays at 0.");
				LabelManager.SetLabel(0x09EB, SnesMemoryType.NesPrgRom, "ReadPads", "Input: " + Environment.NewLine + "Output: $f5 = Joypad #1 data, $f6 = Joypad #2 data " + Environment.NewLine + "Affects: A, X, $00, $01, " + Environment.NewLine + "Desc: This read hardwired famicom joypads.");
				LabelManager.SetLabel(0x0A1A, SnesMemoryType.NesPrgRom, "ReadDownPads", "Input: " + Environment.NewLine + "Output: $f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data " + Environment.NewLine + "Affects: A, X, $00, $01 " + Environment.NewLine + "Desc: This reads hardwired famicom joypads, and detect up->down button transitions");
				LabelManager.SetLabel(0x0A1F, SnesMemoryType.NesPrgRom, "ReadOrDownPads", "Input: " + Environment.NewLine + "Output: $f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data " + Environment.NewLine + "Affects: A, X, $00, $01 " + Environment.NewLine + "Desc: This read both hardwired famicom and expansion port joypads and detect up->down button transitions.");
				LabelManager.SetLabel(0x0A36, SnesMemoryType.NesPrgRom, "ReadDownVerifyPads", "Input: " + Environment.NewLine + "Output: $f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data " + Environment.NewLine + "Affects: A, X, $00, $01 " + Environment.NewLine + "Desc: This reads hardwired Famicom joypads, and detect up->down button transitions. Data is read until two consecutive read matches to work around the DMC reading glitches.");
				LabelManager.SetLabel(0x0A4C, SnesMemoryType.NesPrgRom, "ReadOrDownVerifyPads", "Input: " + Environment.NewLine + "Output: $f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data " + Environment.NewLine + "Affects: A, X, $00, $01 " + Environment.NewLine + "Desc: This read both hardwired famicom and expansion port joypads and detect up->down button transitions. Data is read until two consecutive read matches to work around the DMC reading glitches.");
				LabelManager.SetLabel(0x0A68, SnesMemoryType.NesPrgRom, "ReadDownExpPads", "Input: $f1-$f4 = up->down transitions, $f5-$f8 = Joypad data in the order : Pad1, Pad2, Expansion1, Expansion2 " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, $00, $01 " + Environment.NewLine + "Desc: This read both hardwired famicom and expansion port joypad, but stores their data separately instead of ORing them together like the other routines does. This routine is NOT DMC fortified.");
				LabelManager.SetLabel(0x0A84, SnesMemoryType.NesPrgRom, "VRAMFill", "Input: A = High VRAM Address (aka tile row #), X = Fill value, Y = # of tile rows OR attribute fill data " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00, $01, $02 " + Environment.NewLine + "Desc: This routine does 2 things : If A < $20, it fills pattern table data with the value in X for 16 * Y tiles. If A >= $20, it fills the corresponding nametable with the value in X and attribute table with the value in Y.");
				LabelManager.SetLabel(0x0Ad2, SnesMemoryType.NesPrgRom, "MemFill", "Input: A = fill value, X = first page #, Y = last page # " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00, $01 " + Environment.NewLine + "Desc: This routines fills RAM pages with specified value.");
				LabelManager.SetLabel(0x0AEA, SnesMemoryType.NesPrgRom, "SetScroll", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A " + Environment.NewLine + "Desc: This routine set scroll registers according to values in $fc, $fd and $ff. Should typically be called in VBlank after VRAM updates");
				LabelManager.SetLabel(0x0AFD, SnesMemoryType.NesPrgRom, "JumpEngine", "Input: A = Jump table entry " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00, $01 " + Environment.NewLine + "Desc: The instruction calling this is supposed to be followed by a jump table (16-bit pointers little endian, up to 128 pointers). A is the entry # to jump to, return address on stack is used to get jump table entries.");
				LabelManager.SetLabel(0x0B13, SnesMemoryType.NesPrgRom, "ReadKeyboard", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: " + Environment.NewLine + "Desc: Read Family Basic Keyboard expansion (detail is under analysis)");
				LabelManager.SetLabel(0x0B66, SnesMemoryType.NesPrgRom, "LoadTileset", "Input: A = Low VRAM Address & Flags, Y = Hi VRAM Address, X = # of tiles to transfer to/from VRAM " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00, $01, $02, $03, $04 " + Environment.NewLine + "Desc: This routine can read and write 2BP and 1BP tilesets to/from VRAM. See appendix below about the flags.");
				LabelManager.SetLabel(0x0C22, SnesMemoryType.NesPrgRom, "unk_EC22", "Some kind of logic that some games use. (detail is under analysis)");

			}
		}
	}
}