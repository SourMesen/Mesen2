using Mesen.Config;
using Mesen.Interop;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.Labels
{
	public class DefaultLabelHelper
	{
		public static void SetDefaultLabels()
		{
			if(ConfigManager.Config.Debug.Debugger.DisableDefaultLabels) {
				return;
			}

			HashSet<CpuType> cpuTypes = EmuApi.GetRomInfo().CpuTypes;
			if(cpuTypes.Contains(CpuType.Gameboy)) {
				SetGameboyDefaultLabels();
			}

			if(cpuTypes.Contains(CpuType.Snes)) {
				SetSnesDefaultLabels();
			} else if(cpuTypes.Contains(CpuType.Nes)) {
				SetDefaultNesLabels();
			} else if(cpuTypes.Contains(CpuType.Pce)) {
				SetPceDefaultLabels();
			} else if(cpuTypes.Contains(CpuType.Sms)) {
				SetSmsDefaultLabels();
			} else if(cpuTypes.Contains(CpuType.Gba)) {
				SetGbaDefaultLabels();
			} else if(cpuTypes.Contains(CpuType.Ws)) {
				SetWsDefaultLabels();
			}
		}

		private static void SetSnesDefaultLabels()
		{
			//B-Bus registers
			LabelManager.SetLabel(0x2100, MemoryType.SnesRegister, "INIDISP", "Screen Display Register");
			LabelManager.SetLabel(0x2101, MemoryType.SnesRegister, "OBSEL", "Object Size and Character Size Register");
			LabelManager.SetLabel(0x2102, MemoryType.SnesRegister, "OAMADDL", "OAM Address Registers (Low)");
			LabelManager.SetLabel(0x2103, MemoryType.SnesRegister, "OAMADDH", "OAM Address Registers (High)");
			LabelManager.SetLabel(0x2104, MemoryType.SnesRegister, "OAMDATA", "OAM Data Write Register");
			LabelManager.SetLabel(0x2105, MemoryType.SnesRegister, "BGMODE", "BG Mode and Character Size Register");
			LabelManager.SetLabel(0x2106, MemoryType.SnesRegister, "MOSAIC", "Mosaic Register");
			LabelManager.SetLabel(0x2107, MemoryType.SnesRegister, "BG1SC", "BG Tilemap Address Registers (BG1)");
			LabelManager.SetLabel(0x2108, MemoryType.SnesRegister, "BG2SC", "BG Tilemap Address Registers (BG2)");
			LabelManager.SetLabel(0x2109, MemoryType.SnesRegister, "BG3SC", "BG Tilemap Address Registers (BG3)");
			LabelManager.SetLabel(0x210A, MemoryType.SnesRegister, "BG4SC", "BG Tilemap Address Registers (BG4)");
			LabelManager.SetLabel(0x210B, MemoryType.SnesRegister, "BG12NBA", "BG Character Address Registers (BG1&2)");
			LabelManager.SetLabel(0x210C, MemoryType.SnesRegister, "BG34NBA", "BG Character Address Registers (BG3&4)");
			LabelManager.SetLabel(0x210D, MemoryType.SnesRegister, "BG1HOFS", "BG Scroll Registers (BG1)");
			LabelManager.SetLabel(0x210E, MemoryType.SnesRegister, "BG1VOFS", "BG Scroll Registers (BG1)");
			LabelManager.SetLabel(0x210F, MemoryType.SnesRegister, "BG2HOFS", "BG Scroll Registers (BG2)");
			LabelManager.SetLabel(0x2110, MemoryType.SnesRegister, "BG2VOFS", "BG Scroll Registers (BG2)");
			LabelManager.SetLabel(0x2111, MemoryType.SnesRegister, "BG3HOFS", "BG Scroll Registers (BG3)");
			LabelManager.SetLabel(0x2112, MemoryType.SnesRegister, "BG3VOFS", "BG Scroll Registers (BG3)");
			LabelManager.SetLabel(0x2113, MemoryType.SnesRegister, "BG4HOFS", "BG Scroll Registers (BG4)");
			LabelManager.SetLabel(0x2114, MemoryType.SnesRegister, "BG4VOFS", "BG Scroll Registers (BG4)");
			LabelManager.SetLabel(0x2115, MemoryType.SnesRegister, "VMAIN", "Video Port Control Register");
			LabelManager.SetLabel(0x2116, MemoryType.SnesRegister, "VMADDL", "VRAM Address Registers (Low)");
			LabelManager.SetLabel(0x2117, MemoryType.SnesRegister, "VMADDH", "VRAM Address Registers (High)");
			LabelManager.SetLabel(0x2118, MemoryType.SnesRegister, "VMDATAL", "VRAM Data Write Registers (Low)");
			LabelManager.SetLabel(0x2119, MemoryType.SnesRegister, "VMDATAH", "VRAM Data Write Registers (High)");
			LabelManager.SetLabel(0x211A, MemoryType.SnesRegister, "M7SEL", "Mode 7 Settings Register");
			LabelManager.SetLabel(0x211B, MemoryType.SnesRegister, "M7A", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x211C, MemoryType.SnesRegister, "M7B", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x211D, MemoryType.SnesRegister, "M7C", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x211E, MemoryType.SnesRegister, "M7D", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x211F, MemoryType.SnesRegister, "M7X", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x2120, MemoryType.SnesRegister, "M7Y", "Mode 7 Matrix Registers");
			LabelManager.SetLabel(0x2121, MemoryType.SnesRegister, "CGADD", "CGRAM Address Register");
			LabelManager.SetLabel(0x2122, MemoryType.SnesRegister, "CGDATA", "CGRAM Data Write Register");
			LabelManager.SetLabel(0x2123, MemoryType.SnesRegister, "W12SEL", "Window Mask Settings Registers");
			LabelManager.SetLabel(0x2124, MemoryType.SnesRegister, "W34SEL", "Window Mask Settings Registers");
			LabelManager.SetLabel(0x2125, MemoryType.SnesRegister, "WOBJSEL", "Window Mask Settings Registers");
			LabelManager.SetLabel(0x2126, MemoryType.SnesRegister, "WH0", "Window Position Registers (WH0)");
			LabelManager.SetLabel(0x2127, MemoryType.SnesRegister, "WH1", "Window Position Registers (WH1)");
			LabelManager.SetLabel(0x2128, MemoryType.SnesRegister, "WH2", "Window Position Registers (WH2)");
			LabelManager.SetLabel(0x2129, MemoryType.SnesRegister, "WH3", "Window Position Registers (WH3)");
			LabelManager.SetLabel(0x212A, MemoryType.SnesRegister, "WBGLOG", "Window Mask Logic registers (BG)");
			LabelManager.SetLabel(0x212B, MemoryType.SnesRegister, "WOBJLOG", "Window Mask Logic registers (OBJ)");
			LabelManager.SetLabel(0x212C, MemoryType.SnesRegister, "TM", "Screen Destination Registers");
			LabelManager.SetLabel(0x212D, MemoryType.SnesRegister, "TS", "Screen Destination Registers");
			LabelManager.SetLabel(0x212E, MemoryType.SnesRegister, "TMW", "Window Mask Destination Registers");
			LabelManager.SetLabel(0x212F, MemoryType.SnesRegister, "TSW", "Window Mask Destination Registers");
			LabelManager.SetLabel(0x2130, MemoryType.SnesRegister, "CGWSEL", "Color Math Registers");
			LabelManager.SetLabel(0x2131, MemoryType.SnesRegister, "CGADSUB", "Color Math Registers");
			LabelManager.SetLabel(0x2132, MemoryType.SnesRegister, "COLDATA", "Color Math Registers");
			LabelManager.SetLabel(0x2133, MemoryType.SnesRegister, "SETINI", "Screen Mode Select Register");
			LabelManager.SetLabel(0x2134, MemoryType.SnesRegister, "MPYL", "Multiplication Result Registers");
			LabelManager.SetLabel(0x2135, MemoryType.SnesRegister, "MPYM", "Multiplication Result Registers");
			LabelManager.SetLabel(0x2136, MemoryType.SnesRegister, "MPYH", "Multiplication Result Registers");
			LabelManager.SetLabel(0x2137, MemoryType.SnesRegister, "SLHV", "Software Latch Register");
			LabelManager.SetLabel(0x2138, MemoryType.SnesRegister, "OAMDATAREAD", "OAM Data Read Register");
			LabelManager.SetLabel(0x2139, MemoryType.SnesRegister, "VMDATALREAD", "VRAM Data Read Register (Low)");
			LabelManager.SetLabel(0x213A, MemoryType.SnesRegister, "VMDATAHREAD", "VRAM Data Read Register (High)");
			LabelManager.SetLabel(0x213B, MemoryType.SnesRegister, "CGDATAREAD", "CGRAM Data Read Register");
			LabelManager.SetLabel(0x213C, MemoryType.SnesRegister, "OPHCT", "Scanline Location Registers (Horizontal)");
			LabelManager.SetLabel(0x213D, MemoryType.SnesRegister, "OPVCT", "Scanline Location Registers (Vertical)");
			LabelManager.SetLabel(0x213E, MemoryType.SnesRegister, "STAT77", "PPU Status Register");
			LabelManager.SetLabel(0x213F, MemoryType.SnesRegister, "STAT78", "PPU Status Register");
			LabelManager.SetLabel(0x2140, MemoryType.SnesRegister, "APUIO0", "APU IO Registers");
			LabelManager.SetLabel(0x2141, MemoryType.SnesRegister, "APUIO1", "APU IO Registers");
			LabelManager.SetLabel(0x2142, MemoryType.SnesRegister, "APUIO2", "APU IO Registers");
			LabelManager.SetLabel(0x2143, MemoryType.SnesRegister, "APUIO3", "APU IO Registers");
			LabelManager.SetLabel(0x2180, MemoryType.SnesRegister, "WMDATA", "WRAM Data Register");
			LabelManager.SetLabel(0x2181, MemoryType.SnesRegister, "WMADDL", "WRAM Address Registers");
			LabelManager.SetLabel(0x2182, MemoryType.SnesRegister, "WMADDM", "WRAM Address Registers");
			LabelManager.SetLabel(0x2183, MemoryType.SnesRegister, "WMADDH", "WRAM Address Registers");

			//A-Bus registers (CPU registers)
			LabelManager.SetLabel(0x4016, MemoryType.SnesRegister, "JOYSER0", "Old Style Joypad Registers");
			LabelManager.SetLabel(0x4017, MemoryType.SnesRegister, "JOYSER1", "Old Style Joypad Registers");

			LabelManager.SetLabel(0x4200, MemoryType.SnesRegister, "NMITIMEN", "Interrupt Enable Register");
			LabelManager.SetLabel(0x4201, MemoryType.SnesRegister, "WRIO", "IO Port Write Register");
			LabelManager.SetLabel(0x4202, MemoryType.SnesRegister, "WRMPYA", "Multiplicand Registers");
			LabelManager.SetLabel(0x4203, MemoryType.SnesRegister, "WRMPYB", "Multiplicand Registers");
			LabelManager.SetLabel(0x4204, MemoryType.SnesRegister, "WRDIVL", "Divisor & Dividend Registers");
			LabelManager.SetLabel(0x4205, MemoryType.SnesRegister, "WRDIVH", "Divisor & Dividend Registers");
			LabelManager.SetLabel(0x4206, MemoryType.SnesRegister, "WRDIVB", "Divisor & Dividend Registers");
			LabelManager.SetLabel(0x4207, MemoryType.SnesRegister, "HTIMEL", "IRQ Timer Registers (Horizontal - Low)");
			LabelManager.SetLabel(0x4208, MemoryType.SnesRegister, "HTIMEH", "IRQ Timer Registers (Horizontal - High)");
			LabelManager.SetLabel(0x4209, MemoryType.SnesRegister, "VTIMEL", "IRQ Timer Registers (Vertical - Low)");
			LabelManager.SetLabel(0x420A, MemoryType.SnesRegister, "VTIMEH", "IRQ Timer Registers (Vertical - High)");
			LabelManager.SetLabel(0x420B, MemoryType.SnesRegister, "MDMAEN", "DMA Enable Register");
			LabelManager.SetLabel(0x420C, MemoryType.SnesRegister, "HDMAEN", "HDMA Enable Register");
			LabelManager.SetLabel(0x420D, MemoryType.SnesRegister, "MEMSEL", "ROM Speed Register");
			LabelManager.SetLabel(0x4210, MemoryType.SnesRegister, "RDNMI", "Interrupt Flag Registers");
			LabelManager.SetLabel(0x4211, MemoryType.SnesRegister, "TIMEUP", "Interrupt Flag Registers");
			LabelManager.SetLabel(0x4212, MemoryType.SnesRegister, "HVBJOY", "PPU Status Register");
			LabelManager.SetLabel(0x4213, MemoryType.SnesRegister, "RDIO", "IO Port Read Register");
			LabelManager.SetLabel(0x4214, MemoryType.SnesRegister, "RDDIVL", "Multiplication Or Divide Result Registers (Low)");
			LabelManager.SetLabel(0x4215, MemoryType.SnesRegister, "RDDIVH", "Multiplication Or Divide Result Registers (High)");
			LabelManager.SetLabel(0x4216, MemoryType.SnesRegister, "RDMPYL", "Multiplication Or Divide Result Registers (Low)");
			LabelManager.SetLabel(0x4217, MemoryType.SnesRegister, "RDMPYH", "Multiplication Or Divide Result Registers (High)");
			LabelManager.SetLabel(0x4218, MemoryType.SnesRegister, "JOY1L", "Controller Port Data Registers (Pad 1 - Low)");
			LabelManager.SetLabel(0x4219, MemoryType.SnesRegister, "JOY1H", "Controller Port Data Registers (Pad 1 - High)");
			LabelManager.SetLabel(0x421A, MemoryType.SnesRegister, "JOY2L", "Controller Port Data Registers (Pad 2 - Low)");
			LabelManager.SetLabel(0x421B, MemoryType.SnesRegister, "JOY2H", "Controller Port Data Registers (Pad 2 - High)");
			LabelManager.SetLabel(0x421C, MemoryType.SnesRegister, "JOY3L", "Controller Port Data Registers (Pad 3 - Low)");
			LabelManager.SetLabel(0x421D, MemoryType.SnesRegister, "JOY3H", "Controller Port Data Registers (Pad 3 - High)");
			LabelManager.SetLabel(0x421E, MemoryType.SnesRegister, "JOY4L", "Controller Port Data Registers (Pad 4 - Low)");
			LabelManager.SetLabel(0x421F, MemoryType.SnesRegister, "JOY4H", "Controller Port Data Registers (Pad 4 - High)");

			//DMA registers
			for(uint i = 0; i < 8; i++) {
				LabelManager.SetLabel(0x4300 + i * 0x10, MemoryType.SnesRegister, "DMAP" + i.ToString(), "(H)DMA Control");
				LabelManager.SetLabel(0x4301 + i * 0x10, MemoryType.SnesRegister, "BBAD" + i.ToString(), "(H)DMA B-Bus Address");
				LabelManager.SetLabel(0x4302 + i * 0x10, MemoryType.SnesRegister, "A1T" + i.ToString() + "L", "DMA A-Bus Address / HDMA Table Address (Low)");
				LabelManager.SetLabel(0x4303 + i * 0x10, MemoryType.SnesRegister, "A1T" + i.ToString() + "H", "DMA A-Bus Address / HDMA Table Address (High)");
				LabelManager.SetLabel(0x4304 + i * 0x10, MemoryType.SnesRegister, "A1B" + i.ToString(), "DMA A-Bus Address / HDMA Table Address (Bank)");
				LabelManager.SetLabel(0x4305 + i * 0x10, MemoryType.SnesRegister, "DAS" + i.ToString() + "L", "DMA Size / HDMA Indirect Address (Low)");
				LabelManager.SetLabel(0x4306 + i * 0x10, MemoryType.SnesRegister, "DAS" + i.ToString() + "H", "DMA Size / HDMA Indirect Address (High)");
				LabelManager.SetLabel(0x4307 + i * 0x10, MemoryType.SnesRegister, "DAS" + i.ToString() + "B", "HDMA Indirect Address (Bank)");
				LabelManager.SetLabel(0x4308 + i * 0x10, MemoryType.SnesRegister, "A2A" + i.ToString() + "L", "HDMA Mid Frame Table Address (Low)");
				LabelManager.SetLabel(0x4309 + i * 0x10, MemoryType.SnesRegister, "A2A" + i.ToString() + "H", "HDMA Mid Frame Table Address (High)");
				LabelManager.SetLabel(0x430A + i * 0x10, MemoryType.SnesRegister, "NTLR" + i.ToString(), "HDMA Line Counter");
			}

			//SPC registers
			LabelManager.SetLabel(0xF0, MemoryType.SpcRam, "TEST", "Testing functions");
			LabelManager.SetLabel(0xF1, MemoryType.SpcRam, "CONTROL", "I/O and Timer Control");
			LabelManager.SetLabel(0xF2, MemoryType.SpcRam, "DSPADDR", "DSP Address");
			LabelManager.SetLabel(0xF3, MemoryType.SpcRam, "DSPDATA", "DSP Data");
			LabelManager.SetLabel(0xF4, MemoryType.SpcRam, "CPUIO0", "CPU I/O 0");
			LabelManager.SetLabel(0xF5, MemoryType.SpcRam, "CPUIO1", "CPU I/O 1");
			LabelManager.SetLabel(0xF6, MemoryType.SpcRam, "CPUIO2", "CPU I/O 2");
			LabelManager.SetLabel(0xF7, MemoryType.SpcRam, "CPUIO3", "CPU I/O 3");
			LabelManager.SetLabel(0xF8, MemoryType.SpcRam, "RAMREG1", "Memory Register 1");
			LabelManager.SetLabel(0xF9, MemoryType.SpcRam, "RAMREG2", "Memory Register 2");
			LabelManager.SetLabel(0xFA, MemoryType.SpcRam, "T0TARGET", "Timer 0 scaling target");
			LabelManager.SetLabel(0xFB, MemoryType.SpcRam, "T1TARGET", "Timer 1 scaling target");
			LabelManager.SetLabel(0xFC, MemoryType.SpcRam, "T2TARGET", "Timer 2 scaling target");
			LabelManager.SetLabel(0xFD, MemoryType.SpcRam, "T0OUT", "Timer 0 output");
			LabelManager.SetLabel(0xFE, MemoryType.SpcRam, "T1OUT", "Timer 1 output");
			LabelManager.SetLabel(0xFF, MemoryType.SpcRam, "T2OUT", "Timer 2 output");
		}

		private static void SetGameboyDefaultLabels()
		{
			//LCD
			LabelManager.SetLabel(0xFF40, MemoryType.GameboyMemory, "LCDC_FF40", "LCD Control");
			LabelManager.SetLabel(0xFF41, MemoryType.GameboyMemory, "STAT_FF41", "LCD Status");
			LabelManager.SetLabel(0xFF42, MemoryType.GameboyMemory, "SCY_FF42", "Scroll Y");
			LabelManager.SetLabel(0xFF43, MemoryType.GameboyMemory, "SCX_FF43", "Scroll X");
			LabelManager.SetLabel(0xFF44, MemoryType.GameboyMemory, "LY_FF44", "LCD Y-Coordinate");
			LabelManager.SetLabel(0xFF45, MemoryType.GameboyMemory, "LYC_FF45", "LY Compare");

			LabelManager.SetLabel(0xFF47, MemoryType.GameboyMemory, "BGP_FF47", "BG Palette Data");
			LabelManager.SetLabel(0xFF48, MemoryType.GameboyMemory, "OBP0_FF48", "Object Palette 0 Data");
			LabelManager.SetLabel(0xFF49, MemoryType.GameboyMemory, "OBP1_FF49", "Object Palette 1 Data");

			LabelManager.SetLabel(0xFF4A, MemoryType.GameboyMemory, "WY_FF4A", "Window Y Position");
			LabelManager.SetLabel(0xFF4B, MemoryType.GameboyMemory, "WX_FF4B", "Window X Position");

			//APU
			LabelManager.SetLabel(0xFF10, MemoryType.GameboyMemory, "NR10_FF10", "Channel 1 Sweep");
			LabelManager.SetLabel(0xFF11, MemoryType.GameboyMemory, "NR11_FF11", "Channel 1 Length/Wave Pattern Duty");
			LabelManager.SetLabel(0xFF12, MemoryType.GameboyMemory, "NR12_FF12", "Channel 1 Volume Envelope");
			LabelManager.SetLabel(0xFF13, MemoryType.GameboyMemory, "NR13_FF13", "Channel 1 Frequency Low");
			LabelManager.SetLabel(0xFF14, MemoryType.GameboyMemory, "NR14_FF14", "Channel 1 Frequency High");

			LabelManager.SetLabel(0xFF16, MemoryType.GameboyMemory, "NR21_FF16", "Channel 2 Length/Wave Pattern Duty");
			LabelManager.SetLabel(0xFF17, MemoryType.GameboyMemory, "NR22_FF17", "Channel 2 Volume Envelope");
			LabelManager.SetLabel(0xFF18, MemoryType.GameboyMemory, "NR23_FF18", "Channel 2 Frequency Low");
			LabelManager.SetLabel(0xFF19, MemoryType.GameboyMemory, "NR24_FF19", "Channel 2 Frequency High");

			LabelManager.SetLabel(0xFF1A, MemoryType.GameboyMemory, "NR30_FF1A", "Channel 3 On/Off ");
			LabelManager.SetLabel(0xFF1B, MemoryType.GameboyMemory, "NR31_FF1B", "Channel 3 Length");
			LabelManager.SetLabel(0xFF1C, MemoryType.GameboyMemory, "NR32_FF1C", "Channel 3 Output Level");
			LabelManager.SetLabel(0xFF1D, MemoryType.GameboyMemory, "NR33_FF1D", "Channel 3 Frequency Low");
			LabelManager.SetLabel(0xFF1E, MemoryType.GameboyMemory, "NR34_FF1E", "Channel 3 Frequency High");

			LabelManager.SetLabel(0xFF20, MemoryType.GameboyMemory, "NR41_FF20", "Channel 4 Length");
			LabelManager.SetLabel(0xFF21, MemoryType.GameboyMemory, "NR42_FF21", "Channel 4 Volume Envelope");
			LabelManager.SetLabel(0xFF22, MemoryType.GameboyMemory, "NR43_FF22", "Channel 4 Polynomial Counter");
			LabelManager.SetLabel(0xFF23, MemoryType.GameboyMemory, "NR44_FF23", "Channel 4 Loop");

			LabelManager.SetLabel(0xFF24, MemoryType.GameboyMemory, "NR50_FF24", "Channel Volume");
			LabelManager.SetLabel(0xFF25, MemoryType.GameboyMemory, "NR51_FF25", "Channel Left/Right");
			LabelManager.SetLabel(0xFF26, MemoryType.GameboyMemory, "NR52_FF26", "Channel On/Off");

			//Others
			LabelManager.SetLabel(0xFF00, MemoryType.GameboyMemory, "JOYP_FF00", "Joypad");
			LabelManager.SetLabel(0xFF01, MemoryType.GameboyMemory, "SB_FF01", "Serial Data");
			LabelManager.SetLabel(0xFF02, MemoryType.GameboyMemory, "SC_FF02", "Serial Control");

			LabelManager.SetLabel(0xFF04, MemoryType.GameboyMemory, "DIV_FF04", "Divider");
			LabelManager.SetLabel(0xFF05, MemoryType.GameboyMemory, "TIMA_FF05", "Timer Counter");
			LabelManager.SetLabel(0xFF06, MemoryType.GameboyMemory, "TMA_FF06", "Timer Modulo");
			LabelManager.SetLabel(0xFF07, MemoryType.GameboyMemory, "TAC_FF07", "Timer Control");

			LabelManager.SetLabel(0xFF0F, MemoryType.GameboyMemory, "IF_FF0F", "Interrupt Flag");
			LabelManager.SetLabel(0xFFFF, MemoryType.GameboyMemory, "IE_FFFF", "Interrupt Enable");

			LabelManager.SetLabel(0xFF46, MemoryType.GameboyMemory, "DMA_FF46", "OAM DMA Start");
		}

		private static void SetDefaultNesLabels()
		{
			LabelManager.SetLabel(0x2000, MemoryType.NesMemory, "PpuControl_2000", $"7  bit  0{Environment.NewLine}---- ----{Environment.NewLine}VPHB SINN{Environment.NewLine}|||| ||||{Environment.NewLine}|||| ||++- Base nametable address{Environment.NewLine}|||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00){Environment.NewLine}|||| |+--- VRAM address increment per CPU read/write of PPUDATA{Environment.NewLine}|||| |     (0: add 1, going across; 1: add 32, going down){Environment.NewLine}|||| +---- Sprite pattern table address for 8x8 sprites{Environment.NewLine}||||       (0: $0000; 1: $1000; ignored in 8x16 mode){Environment.NewLine}|||+------ Background pattern table address (0: $0000; 1: $1000){Environment.NewLine}||+------- Sprite size (0: 8x8; 1: 8x16){Environment.NewLine}|+-------- PPU master/slave select{Environment.NewLine}|          (0: read backdrop from EXT pins; 1: output color on EXT pins){Environment.NewLine}+--------- Generate an NMI at the start of the{Environment.NewLine}           vertical blanking interval (0: off; 1: on)");
			LabelManager.SetLabel(0x2001, MemoryType.NesMemory, "PpuMask_2001", $"7  bit  0{Environment.NewLine}---- ----{Environment.NewLine}BGRs bMmG{Environment.NewLine}|||| ||||{Environment.NewLine}|||| |||+- Display type: (0: color, 1: grayscale){Environment.NewLine}|||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0: Hide{Environment.NewLine}|||| |+--- 1: Show sprites in leftmost 8 pixels of screen, 0: Hide{Environment.NewLine}|||| +---- 1: Show background{Environment.NewLine}|||+------ 1: Show sprites{Environment.NewLine}||+------- Emphasize red{Environment.NewLine}|+-------- Emphasize green{Environment.NewLine}+--------- Emphasize blue");
			LabelManager.SetLabel(0x2002, MemoryType.NesMemory, "PpuStatus_2002", $"7  bit  0{Environment.NewLine}---- ----{Environment.NewLine}VSO. ....{Environment.NewLine}|||| ||||{Environment.NewLine}|||+-++++- Least significant bits previously written into a PPU register{Environment.NewLine}|||        (due to register not being updated for this address){Environment.NewLine}||+------- Sprite overflow. The intent was for this flag to be set{Environment.NewLine}||         whenever more than eight sprites appear on a scanline, but a{Environment.NewLine}||         hardware bug causes the actual behavior to be more complicated{Environment.NewLine}||         and generate false positives as well as false negatives; see{Environment.NewLine}||         PPU sprite evaluation. This flag is set during sprite{Environment.NewLine}||         evaluation and cleared at dot 1 (the second dot) of the{Environment.NewLine}||         pre-render line.{Environment.NewLine}|+-------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps{Environment.NewLine}|          a nonzero background pixel; cleared at dot 1 of the pre-render{Environment.NewLine}|          line.  Used for raster timing.{Environment.NewLine}+--------- Vertical blank has started (0: not in vblank; 1: in vblank).{Environment.NewLine}           Set at dot 1 of line 241 (the line *after* the post-render{Environment.NewLine}           line); cleared after reading $2002 and at dot 1 of the{Environment.NewLine}           pre-render line.");
			LabelManager.SetLabel(0x2003, MemoryType.NesMemory, "OamAddr_2003", "Set OAM address - Write only");
			LabelManager.SetLabel(0x2004, MemoryType.NesMemory, "OamData_2004", "Read/Write OAM data");
			LabelManager.SetLabel(0x2005, MemoryType.NesMemory, "PpuScroll_2005", "Set PPU scroll, write twice - Write only");
			LabelManager.SetLabel(0x2006, MemoryType.NesMemory, "PpuAddr_2006", "Set PPU address, write twice - Write only");
			LabelManager.SetLabel(0x2007, MemoryType.NesMemory, "PpuData_2007", "Read/Write VRAM");

			LabelManager.SetLabel(0x4000, MemoryType.NesMemory, "Sq0Duty_4000", $"DDLC VVVV{Environment.NewLine}Duty (D), envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)");
			LabelManager.SetLabel(0x4001, MemoryType.NesMemory, "Sq0Sweep_4001", $"EPPP NSSS{Environment.NewLine}Sweep unit: enabled (E), period (P), negate (N), shift (S)");
			LabelManager.SetLabel(0x4002, MemoryType.NesMemory, "Sq0Timer_4002", $"TTTT TTTT{Environment.NewLine}Timer low (T)");
			LabelManager.SetLabel(0x4003, MemoryType.NesMemory, "Sq0Length_4003", $"LLLL LTTT{Environment.NewLine}Length counter load (L), timer high (T)");

			LabelManager.SetLabel(0x4004, MemoryType.NesMemory, "Sq1Duty_4004", $"DDLC VVVV{Environment.NewLine}Duty (D), envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)");
			LabelManager.SetLabel(0x4005, MemoryType.NesMemory, "Sq1Sweep_4005", $"EPPP NSSS{Environment.NewLine}Sweep unit: enabled (E), period (P), negate (N), shift (S)");
			LabelManager.SetLabel(0x4006, MemoryType.NesMemory, "Sq1Timer_4006", $"TTTT TTTT{Environment.NewLine}Timer low (T)");
			LabelManager.SetLabel(0x4007, MemoryType.NesMemory, "Sq1Length_4007", $"LLLL LTTT{Environment.NewLine}Length counter load (L), timer high (T)");

			LabelManager.SetLabel(0x4008, MemoryType.NesMemory, "TrgLinear_4008", $"CRRR RRRR{Environment.NewLine}Length counter halt / linear counter control (C), linear counter load (R)");
			LabelManager.SetLabel(0x400A, MemoryType.NesMemory, "TrgTimer_400A", $"TTTT TTTT{Environment.NewLine}Timer low (T)");
			LabelManager.SetLabel(0x400B, MemoryType.NesMemory, "TrgLength_400B", $"LLLL LTTT{Environment.NewLine}Length counter load (L), timer high (T)");

			LabelManager.SetLabel(0x400C, MemoryType.NesMemory, "NoiseVolume_400C", $"--LC VVVV{Environment.NewLine}Envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)");
			LabelManager.SetLabel(0x400E, MemoryType.NesMemory, "NoisePeriod_400E", $"L--- PPPP{Environment.NewLine}Loop noise (L), noise period (P)");
			LabelManager.SetLabel(0x400F, MemoryType.NesMemory, "NoiseLength_400F", $"LLLL L---{Environment.NewLine}Length counter load (L)");

			LabelManager.SetLabel(0x4010, MemoryType.NesMemory, "DmcFreq_4010", $"IL-- RRRR{Environment.NewLine}IRQ enable (I), loop (L), frequency (R)");
			LabelManager.SetLabel(0x4011, MemoryType.NesMemory, "DmcCounter_4011", $"-DDD DDDD{Environment.NewLine}Load counter (D)");
			LabelManager.SetLabel(0x4012, MemoryType.NesMemory, "DmcAddress_4012", $"AAAA AAAA{Environment.NewLine}Sample address (A)");
			LabelManager.SetLabel(0x4013, MemoryType.NesMemory, "DmcLength_4013", $"LLLL LLLL{Environment.NewLine}Sample length (L)");

			LabelManager.SetLabel(0x4014, MemoryType.NesMemory, "SpriteDma_4014", "Writing $XX will upload 256 bytes of data from CPU page $XX00-$XXFF to the internal PPU OAM.");

			LabelManager.SetLabel(0x4015, MemoryType.NesMemory, "ApuStatus_4015", $"Read:{Environment.NewLine}IF-D NT21{Environment.NewLine}DMC interrupt (I), frame interrupt (F), DMC active (D), length counter > 0 (N/T/2/1){Environment.NewLine + Environment.NewLine}Write:{Environment.NewLine}---D NT21{Environment.NewLine}Enable DMC (D), noise (N), triangle (T), and pulse channels (2/1)");

			LabelManager.SetLabel(0x4016, MemoryType.NesMemory, "Ctrl1_4016", $"Read (NES - input):{Environment.NewLine}---4 3210{Environment.NewLine}Read data from controller port #1.{Environment.NewLine}{Environment.NewLine}Write:{Environment.NewLine}---- ---A{Environment.NewLine}Output data (strobe) to both controllers.");
			LabelManager.SetLabel(0x4017, MemoryType.NesMemory, "Ctrl2_FrameCtr_4017", $"Read (NES - input):{Environment.NewLine}---4 3210{Environment.NewLine}Read data from controller port #2.{Environment.NewLine}{Environment.NewLine}Write (Frame counter): MI-- ----{Environment.NewLine}Mode (M, 0 = 4-step, 1 = 5-step), IRQ inhibit flag (I)");

			if(EmuApi.GetRomInfo().Format == RomFormat.Fds) {
				LabelManager.SetLabel(0x01F8, MemoryType.NesPrgRom, "LoadFiles", "Input: Pointer to Disk ID, Pointer to File List" + Environment.NewLine + "Output: A = error #, Y = # of files loaded" + Environment.NewLine + "Desc: Loads files specified by DiskID into memory from disk. Load addresses are decided by the file's header.");
				LabelManager.SetLabel(0x0237, MemoryType.NesPrgRom, "AppendFile", "Input: Pointer to Disk ID, Pointer to File Header" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Appends the file data given by DiskID to the disk. This means that the file is tacked onto the end of the disk, and the disk file count is incremented. The file is then read back to verify the write. If an error occurs during verification, the disk's file count is decremented (logically hiding the written file).");
				LabelManager.SetLabel(0x0239, MemoryType.NesPrgRom, "WriteFile", "Input: Pointer to Disk ID, Pointer to File Header, A = file #" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Same as \"Append File\", but instead of writing the file to the end of the disk, A specifies the sequential position on the disk to write the file (0 is the first). This also has the effect of setting the disk's file count to the A value, therefore logically hiding any other files that may reside after the written one.");
				LabelManager.SetLabel(0x02B7, MemoryType.NesPrgRom, "CheckFileCount", "Input: Pointer to Disk ID, A = # to set file count to" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Reads in disk's file count, compares it to A, then sets the disk's file count to A.");
				LabelManager.SetLabel(0x02BB, MemoryType.NesPrgRom, "AdjustFileCount", "Input: Pointer to Disk ID, A = number to reduce current file count by" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Reads in disk's file count, decrements it by A, then writes the new value back.");
				LabelManager.SetLabel(0x0301, MemoryType.NesPrgRom, "SetFileCount1", "Input: Pointer to Disk ID, A = file count minus one = # of the last file" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Set the file count to A + 1");
				LabelManager.SetLabel(0x0305, MemoryType.NesPrgRom, "SetFileCount", "Input: Pointer to Disk ID, A = file count" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Set the file count to A");
				LabelManager.SetLabel(0x032A, MemoryType.NesPrgRom, "GetDiskInfo", "Input: Pointer to Disk Info" + Environment.NewLine + "Output: A = error #" + Environment.NewLine + "Desc: Fills DiskInfo up with data read off the current disk.");

				LabelManager.SetLabel(0x0445, MemoryType.NesPrgRom, "CheckDiskHeader", "Input: Pointer to 10 byte string at $00 " + Environment.NewLine + "Output:  " + Environment.NewLine + "Desc: Compares the first 10 bytes on the disk coming after the FDS string, to 10 bytes pointed to by Ptr($00). To bypass the checking of any byte, a -1 can be placed in the equivelant place in the compare string.  Otherwise, if the comparison fails, an appropriate error will be generated.");
				LabelManager.SetLabel(0x0484, MemoryType.NesPrgRom, "GetNumFiles", "Input:  " + Environment.NewLine + "Output:  " + Environment.NewLine + "Desc: Reads number of files stored on disk, stores the result in $06");
				LabelManager.SetLabel(0x0492, MemoryType.NesPrgRom, "SetNumFiles", "Input:  " + Environment.NewLine + "Output: A = number of files " + Environment.NewLine + "Desc: Writes new number of files to disk header.");
				LabelManager.SetLabel(0x04A0, MemoryType.NesPrgRom, "FileMatchTest", "Input: Pointer to FileID list at $02 " + Environment.NewLine + "Output:   " + Environment.NewLine + "Desc: Uses a byte string pointed at by Ptr($02) to tell the disk system which files to load.  The file ID's number is searched for in the string.  If an exact match is found, [$09] is 0'd, and [$0E] is incremented.  If no matches are found after 20 bytes, or a -1 entry is encountered, [$09] is set to -1.  If the first byte in the string is -1, the BootID number is used for matching files (any FileID that is not greater than the BootID qualifies as a match).");
				LabelManager.SetLabel(0x04DA, MemoryType.NesPrgRom, "SkipFiles", "Input: Number of files to skip in $06 " + Environment.NewLine + "Output:  " + Environment.NewLine + "Desc: Skips over specified number of files.");

				LabelManager.SetLabel(0x0149, MemoryType.NesPrgRom, "Delay132", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: " + Environment.NewLine + "Desc: 132 clock cycle delay");
				LabelManager.SetLabel(0x0153, MemoryType.NesPrgRom, "Delayms", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: X, Y " + Environment.NewLine + "Desc: Delay routine, Y = delay in ms (approximate)");
				LabelManager.SetLabel(0x0161, MemoryType.NesPrgRom, "DisPFObj", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Disable sprites and background");
				LabelManager.SetLabel(0x016B, MemoryType.NesPrgRom, "EnPFObj", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Enable sprites and background");
				LabelManager.SetLabel(0x0171, MemoryType.NesPrgRom, "DisObj", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Disable sprites");
				LabelManager.SetLabel(0x0178, MemoryType.NesPrgRom, "EnObj", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Enable sprites");
				LabelManager.SetLabel(0x017E, MemoryType.NesPrgRom, "DisPF", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Disable background");
				LabelManager.SetLabel(0x0185, MemoryType.NesPrgRom, "EnPF", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, $fe " + Environment.NewLine + "Desc: Enable background");
				LabelManager.SetLabel(0x01B2, MemoryType.NesPrgRom, "VINTWait", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: $ff " + Environment.NewLine + "Desc: Wait until next VBlank NMI fires, and return (for programs that does it the \"everything in main\" way). NMI vector selection at $100 is preserved, but further VBlanks are disabled.");
				LabelManager.SetLabel(0x07BB, MemoryType.NesPrgRom, "VRAMStructWrite", "Input: Pointer to VRAM buffer to be written " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00, $01, $ff " + Environment.NewLine + "Desc: Set VRAM increment to 1 (clear [[PPUCTRL]]/$ff bit 2), and write a VRAM buffer to VRAM. Read below for information on the structure.");
				LabelManager.SetLabel(0x0844, MemoryType.NesPrgRom, "FetchDirectPtr", "Input: " + Environment.NewLine + "Output: $00, $01 = pointer fetched " + Environment.NewLine + "Affects: A, X, Y, $05, $06 " + Environment.NewLine + "Desc: Fetch a direct pointer from the stack (the pointer should be placed after the return address of the routine that calls this one (see \"important notes\" above)), save the pointer at ($00) and fix the return address.");
				LabelManager.SetLabel(0x086A, MemoryType.NesPrgRom, "WriteVRAMBuffer", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $301, $302 " + Environment.NewLine + "Desc: Write the VRAM Buffer at $302 to VRAM. Read below for information on the structure.");
				LabelManager.SetLabel(0x08B3, MemoryType.NesPrgRom, "ReadVRAMBuffer", "Input: X = start address of read buffer, Y = # of bytes to read " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y " + Environment.NewLine + "Desc: Read individual bytes from VRAM to the VRAMBuffer. (see notes below)");
				LabelManager.SetLabel(0x08D2, MemoryType.NesPrgRom, "PrepareVRAMString", "Input: A = High VRAM address, X = Low VRAM address, Y = string length, Direct Pointer = data to be written to VRAM " + Environment.NewLine + "Output: A = $ff : no error, A = $01 : string didn't fit in buffer " + Environment.NewLine + "Affects: A, X, Y, $00, $01, $02, $03, $04, $05, $06 " + Environment.NewLine + "Desc: This routine copies pointed data into the VRAM buffer.");
				LabelManager.SetLabel(0x08E1, MemoryType.NesPrgRom, "PrepareVRAMStrings", "Input: A = High VRAM address, X = Low VRAM address, Direct pointer = data to be written to VRAM " + Environment.NewLine + "Output: A = $ff : no error, A = $01 : data didn't fit in buffer " + Environment.NewLine + "Affects: A, X, Y, $00, $01, $02, $03, $04, $05, $06 " + Environment.NewLine + "Desc: This routine copies a 2D string into the VRAM buffer. The first byte of the data determines the width and height of the following string (in tiles): Upper nybble = height, lower nybble = width.");
				LabelManager.SetLabel(0x094F, MemoryType.NesPrgRom, "GetVRAMBufferByte", "Input: X = starting index of read buffer, Y = # of address to compare (starting at 1), $00, $01 = address to read from " + Environment.NewLine + "Output: carry clear : a previously read byte was returned, carry set : no byte was read, should wait next call to ReadVRAMBuffer " + Environment.NewLine + "Affects: A, X, Y " + Environment.NewLine + "Desc: This routine was likely planned to be used in order to avoid useless latency on a VRAM reads (see notes below). It compares the VRAM address in ($00) with the Yth (starting at 1) address of the read buffer. If both addresses match, the corresponding data byte is returned exit with c clear. If the addresses are different, the buffer address is overwritten by the address in ($00) and the routine exit with c set.");
				LabelManager.SetLabel(0x097D, MemoryType.NesPrgRom, "Pixel2NamConv", "Input: $02 = Pixel X cord, $03 = Pixel Y cord " + Environment.NewLine + "Output: $00 = High nametable address, $01 = Low nametable address " + Environment.NewLine + "Affects: A " + Environment.NewLine + "Desc: This routine convert pixel screen coordinates to corresponding nametable address (assumes no scrolling, and points to first nametable at $2000-$23ff).");
				LabelManager.SetLabel(0x0997, MemoryType.NesPrgRom, "Nam2PixelConv", "Input: $00 = High nametable address, $01 = low nametable address " + Environment.NewLine + "Output: $02 = Pixel X cord, $03 = Pixel Y cord " + Environment.NewLine + "Affects: A " + Environment.NewLine + "Desc: This routine convert a nametable address to corresponding pixel coordinates (assume no scrolling).");
				LabelManager.SetLabel(0x09B1, MemoryType.NesPrgRom, "Random", "Input: X = Zero Page address where the random bytes are placed, Y = # of shift register bytes (normally $02) " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00 " + Environment.NewLine + "Desc: This is a shift-register based random number generator, normally takes 2 bytes (using more won't affect random sequence). On reset you are supposed to write some non-zero values here (BIOS uses writes $d0, $d0), and call this routine several times before the data is actually random. Each call of this routine will shift the bytes ''right''.");
				LabelManager.SetLabel(0x09C8, MemoryType.NesPrgRom, "SpriteDMA", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A " + Environment.NewLine + "Desc: This routine does sprite DMA from RAM $200-$2ff");
				LabelManager.SetLabel(0x09D3, MemoryType.NesPrgRom, "CounterLogic", "Input: A, Y = end Zeropage address of counters, X = start zeropage address of counters " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, $00 " + Environment.NewLine + "Desc: This decrements several counters in Zeropage. The first counter is a decimal counter 9 -> 8 -> 7 -> ... -> 1 -> 0 -> 9 -> ... Counters 1...A are simply decremented and stays at 0. Counters A+1...Y are decremented when the first counter does a 0 -> 9 transition, and stays at 0.");
				LabelManager.SetLabel(0x09EB, MemoryType.NesPrgRom, "ReadPads", "Input: " + Environment.NewLine + "Output: $f5 = Joypad #1 data, $f6 = Joypad #2 data " + Environment.NewLine + "Affects: A, X, $00, $01, " + Environment.NewLine + "Desc: This read hardwired famicom joypads.");
				LabelManager.SetLabel(0x0A1A, MemoryType.NesPrgRom, "ReadDownPads", "Input: " + Environment.NewLine + "Output: $f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data " + Environment.NewLine + "Affects: A, X, $00, $01 " + Environment.NewLine + "Desc: This reads hardwired famicom joypads, and detect up->down button transitions");
				LabelManager.SetLabel(0x0A1F, MemoryType.NesPrgRom, "ReadOrDownPads", "Input: " + Environment.NewLine + "Output: $f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data " + Environment.NewLine + "Affects: A, X, $00, $01 " + Environment.NewLine + "Desc: This read both hardwired famicom and expansion port joypads and detect up->down button transitions.");
				LabelManager.SetLabel(0x0A36, MemoryType.NesPrgRom, "ReadDownVerifyPads", "Input: " + Environment.NewLine + "Output: $f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data " + Environment.NewLine + "Affects: A, X, $00, $01 " + Environment.NewLine + "Desc: This reads hardwired Famicom joypads, and detect up->down button transitions. Data is read until two consecutive read matches to work around the DMC reading glitches.");
				LabelManager.SetLabel(0x0A4C, MemoryType.NesPrgRom, "ReadOrDownVerifyPads", "Input: " + Environment.NewLine + "Output: $f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data " + Environment.NewLine + "Affects: A, X, $00, $01 " + Environment.NewLine + "Desc: This read both hardwired famicom and expansion port joypads and detect up->down button transitions. Data is read until two consecutive read matches to work around the DMC reading glitches.");
				LabelManager.SetLabel(0x0A68, MemoryType.NesPrgRom, "ReadDownExpPads", "Input: $f1-$f4 = up->down transitions, $f5-$f8 = Joypad data in the order : Pad1, Pad2, Expansion1, Expansion2 " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, $00, $01 " + Environment.NewLine + "Desc: This read both hardwired famicom and expansion port joypad, but stores their data separately instead of ORing them together like the other routines does. This routine is NOT DMC fortified.");
				LabelManager.SetLabel(0x0A84, MemoryType.NesPrgRom, "VRAMFill", "Input: A = High VRAM Address (aka tile row #), X = Fill value, Y = # of tile rows OR attribute fill data " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00, $01, $02 " + Environment.NewLine + "Desc: This routine does 2 things : If A < $20, it fills pattern table data with the value in X for 16 * Y tiles. If A >= $20, it fills the corresponding nametable with the value in X and attribute table with the value in Y.");
				LabelManager.SetLabel(0x0Ad2, MemoryType.NesPrgRom, "MemFill", "Input: A = fill value, X = first page #, Y = last page # " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00, $01 " + Environment.NewLine + "Desc: This routines fills RAM pages with specified value.");
				LabelManager.SetLabel(0x0AEA, MemoryType.NesPrgRom, "SetScroll", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A " + Environment.NewLine + "Desc: This routine set scroll registers according to values in $fc, $fd and $ff. Should typically be called in VBlank after VRAM updates");
				LabelManager.SetLabel(0x0AFD, MemoryType.NesPrgRom, "JumpEngine", "Input: A = Jump table entry " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00, $01 " + Environment.NewLine + "Desc: The instruction calling this is supposed to be followed by a jump table (16-bit pointers little endian, up to 128 pointers). A is the entry # to jump to, return address on stack is used to get jump table entries.");
				LabelManager.SetLabel(0x0B13, MemoryType.NesPrgRom, "ReadKeyboard", "Input: " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: " + Environment.NewLine + "Desc: Read Family Basic Keyboard expansion (detail is under analysis)");
				LabelManager.SetLabel(0x0B66, MemoryType.NesPrgRom, "LoadTileset", "Input: A = Low VRAM Address & Flags, Y = Hi VRAM Address, X = # of tiles to transfer to/from VRAM " + Environment.NewLine + "Output: " + Environment.NewLine + "Affects: A, X, Y, $00, $01, $02, $03, $04 " + Environment.NewLine + "Desc: This routine can read and write 2BP and 1BP tilesets to/from VRAM. See appendix below about the flags.");
				LabelManager.SetLabel(0x0C22, MemoryType.NesPrgRom, "unk_EC22", "Some kind of logic that some games use. (detail is under analysis)");
			}
		}

		private static void SetPceDefaultLabels()
		{
			LabelManager.SetLabel(0x000, MemoryType.PceMemory, "VDC_AR_0000", "Address Register (W) / Status Register (R)");
			LabelManager.SetLabel(0x002, MemoryType.PceMemory, "VDC_DATA_LO_0002", "Data (low byte)");
			LabelManager.SetLabel(0x003, MemoryType.PceMemory, "VDC_DATA_HI_0003", "Data (high byte) + Latch");

			LabelManager.SetLabel(0x400, MemoryType.PceMemory, "VCE_CONTROL_0400", "VCE Control Register");
			LabelManager.SetLabel(0x402, MemoryType.PceMemory, "VCE_ADDR_LO_0402", "Color Table Address Register (LSB)");
			LabelManager.SetLabel(0x403, MemoryType.PceMemory, "VCE_ADDR_HI_0403", "Color Table Address Register (MSB)");
			LabelManager.SetLabel(0x404, MemoryType.PceMemory, "VCE_DATA_LO_0404", "Color Table Data Register (LSB)");
			LabelManager.SetLabel(0x405, MemoryType.PceMemory, "VCE_DATA_HI_0405", "Color Table Data Register (MSB)");

			LabelManager.SetLabel(0x800, MemoryType.PceMemory, "PSG_CHANSELECT_0800", "PSG channel select ($00-$05)");
			LabelManager.SetLabel(0x801, MemoryType.PceMemory, "PSG_GLOBALVOL_0801", "Global Sound Volume (top 4 bits: Left; bottom 4 bits: Right)");
			LabelManager.SetLabel(0x802, MemoryType.PceMemory, "PSG_FREQLO_0802", "Frequency (LSB)");
			LabelManager.SetLabel(0x803, MemoryType.PceMemory, "PSG_FREQHI_0803", "Frequency (MSB - 4 bits)");
			LabelManager.SetLabel(0x804, MemoryType.PceMemory, "PSG_CHANCTRL_0804", "Channel on/off, DDA on/off, Channel volume");
			LabelManager.SetLabel(0x805, MemoryType.PceMemory, "PSG_CHANPAN_0805", "Channel sound balance/panning (Left/Right)");
			LabelManager.SetLabel(0x806, MemoryType.PceMemory, "PSG_CHANDATA_0806", "Channel sound data (5 bits)");
			LabelManager.SetLabel(0x807, MemoryType.PceMemory, "PSG_NOISE_0807", "Noise enable, Noise frequency");
			LabelManager.SetLabel(0x808, MemoryType.PceMemory, "PSG_LFOFREQ_0808", "LFO frequency");
			LabelManager.SetLabel(0x809, MemoryType.PceMemory, "PSG_LFOCONTROL_0809", "LFO trigger, LFO control");

			LabelManager.SetLabel(0xC00, MemoryType.PceMemory, "TIMER_COUNTER_0C00", "Timer Counter (read)/latch (write)");
			LabelManager.SetLabel(0xC01, MemoryType.PceMemory, "TIMER_CONTROL_0C01", "Timer Control");

			LabelManager.SetLabel(0x1000, MemoryType.PceMemory, "JOYPAD_1000", "Joypad I/O");

			LabelManager.SetLabel(0x1402, MemoryType.PceMemory, "IRQ_DISABLE_1402", "IRQ Disable toggle");
			LabelManager.SetLabel(0x1403, MemoryType.PceMemory, "IRQ_STATUS_1403", "IRQ Status (Read); Acknowledge Timer interrupt (write)");
		}

		private static void SetSmsDefaultLabels()
		{
			LabelManager.SetLabel(0x3E, MemoryType.SmsPort, "MEMORY_ENABLE_3E", "");
			LabelManager.SetLabel(0x3F, MemoryType.SmsPort, "IO_3F", "");
			LabelManager.SetLabel(0x7E, MemoryType.SmsPort, "VDP_V_COUNTER_7E", "");
			LabelManager.SetLabel(0x7F, MemoryType.SmsPort, "PSG_7F", "");
			LabelManager.SetLabel(0xBE, MemoryType.SmsPort, "VDP_DATA_BE", "");
			LabelManager.SetLabel(0xBF, MemoryType.SmsPort, "VDP_CMD_STATUS_BF", "");
			LabelManager.SetLabel(0xDC, MemoryType.SmsPort, "JOY1_DC", "");
			LabelManager.SetLabel(0xDD, MemoryType.SmsPort, "JOY2_DD", "");
		}

		private static void SetGbaDefaultLabels()
		{
			Action<uint, uint, string, string> addLabel = (addr, length, label, desc) => {
				LabelManager.SetLabel(new CodeLabel() {
					Address = 0x4000000 | addr,
					Length = length,
					MemoryType = MemoryType.GbaMemory,
					Label = label,
					Comment = desc
				}, false);
			};

			addLabel(0x000, 2, "DISPCNT", "Display Control");
			addLabel(0x002, 1, "GREENSWAP", "");
			addLabel(0x004, 1, "DISPSTAT", "Display Status");
			addLabel(0x005, 1, "LYC", "");
			addLabel(0x006, 1, "VCOUNT", "");

			addLabel(0x008, 2, "BG0CNT", "BG0 Control");
			addLabel(0x00A, 2, "BG1CNT", "BG1 Control");
			addLabel(0x00C, 2, "BG2CNT", "BG2 Control");
			addLabel(0x00E, 2, "BG3CNT", "BG3 Control");

			addLabel(0x010, 2, "BG0HOFS", "BG0 X Scroll Offset");
			addLabel(0x012, 2, "BG0VOFS", "BG0 Y Scroll Offset");
			addLabel(0x014, 2, "BG1HOFS", "BG1 X Scroll Offset");
			addLabel(0x016, 2, "BG1VOFS", "BG1 Y Scroll Offset");
			addLabel(0x018, 2, "BG2HOFS", "BG2 X Scroll Offset");
			addLabel(0x01A, 2, "BG2VOFS", "BG2 Y Scroll Offset");
			addLabel(0x01C, 2, "BG3HOFS", "BG3 X Scroll Offset");
			addLabel(0x01E, 2, "BG3VOFS", "BG3 Y Scroll Offset");
			
			addLabel(0x020, 2, "BG2PA", "BG2 Transform Param A");
			addLabel(0x022, 2, "BG2PB", "BG2 Transform Param B");
			addLabel(0x024, 2, "BG2PC", "BG2 Transform Param C");
			addLabel(0x026, 2, "BG2PD", "BG2 Transform Param D");
			addLabel(0x028, 4, "BG2X", "BG2 Origin X");
			addLabel(0x02C, 4, "BG2Y", "BG2 Origin Y");

			addLabel(0x030, 2, "BG3PA", "BG3 Transform Param A");
			addLabel(0x032, 2, "BG3PB", "BG3 Transform Param B");
			addLabel(0x034, 2, "BG3PC", "BG3 Transform Param C");
			addLabel(0x036, 2, "BG3PD", "BG3 Transform Param D");
			addLabel(0x038, 4, "BG3X", "BG3 Origin X");
			addLabel(0x03C, 4, "BG3Y", "BG3 Origin Y");

			addLabel(0x040, 2, "WIN0H", "Window 0 Start/End X");
			addLabel(0x042, 2, "WIN1H", "Window 1 Start/End X");

			addLabel(0x044, 2, "WIN0V", "Window 0 Start/End Y");
			addLabel(0x046, 2, "WIN1V", "Window 1 Start/End Y");
			
			addLabel(0x048, 2, "WININ", "Window 0/1 Config");
			addLabel(0x04A, 2, "WINOUT", "OBJ Window/Outside Window Config");
			
			addLabel(0x04C, 2, "MOSAIC", "Mosaic Size");
			addLabel(0x050, 2, "BLDCNT", "Blend Control");
			addLabel(0x052, 2, "BLDALPHA", "Blend Coefficients");
			addLabel(0x054, 1, "BLDY", "Brightness Coefficient");

			addLabel(0x060, 1, "NR10", "Channel 1 Sweep");
			addLabel(0x062, 1, "NR11", "Channel 1 Length/Wave Pattern Duty");
			addLabel(0x063, 1, "NR12", "Channel 1 Volume Envelope");
			addLabel(0x064, 1, "NR13", "Channel 1 Frequency Low");
			addLabel(0x065, 1, "NR14", "Channel 1 Frequency High");

			addLabel(0x068, 1, "NR21", "Channel 2 Length/Wave Pattern Duty");
			addLabel(0x069, 1, "NR22", "Channel 2 Volume Envelope");
			addLabel(0x06C, 1, "NR23", "Channel 2 Frequency Low");
			addLabel(0x06D, 1, "NR24", "Channel 2 Frequency High");

			addLabel(0x070, 1, "NR30", "Channel 3 On/Off ");
			addLabel(0x072, 1, "NR31", "Channel 3 Length");
			addLabel(0x073, 1, "NR32", "Channel 3 Output Level");
			addLabel(0x074, 1, "NR33", "Channel 3 Frequency Low");
			addLabel(0x075, 1, "NR34", "Channel 3 Frequency High");

			addLabel(0x078, 1, "NR41", "Channel 4 Length");
			addLabel(0x079, 1, "NR42", "Channel 4 Volume Envelope");
			addLabel(0x07C, 1, "NR43", "Channel 4 Polynomial Counter");
			addLabel(0x07D, 1, "NR44", "Channel 4 Loop");

			addLabel(0x080, 1, "NR50", "Channel Volume");
			addLabel(0x081, 1, "NR51", "Channel Left/Right");
			addLabel(0x082, 2, "SOUNDCNT_H", "Mixing Control");
			addLabel(0x084, 1, "NR52", "Channel On/Off");
			addLabel(0x088, 2, "SOUNDBIAS", "");

			addLabel(0x090, 0x10, "WAVERAM", "Channel 3 Wave RAM");

			addLabel(0x0A0, 4, "FIFO_A", "Channel A FIFO");
			addLabel(0x0A4, 4, "FIFO_B", "Channel B FIFO");

			addLabel(0x0B0, 4, "DMA0SAD", "DMA 0 Source Address");
			addLabel(0x0B4, 4, "DMA0DAD", "DMA 0 Destination Address");
			addLabel(0x0B8, 2, "DMA0CNT_L", "DMA 0 Length");
			addLabel(0x0BA, 2, "DMA0CNT_H", "DMA 0 Control");
			addLabel(0x0BC, 4, "DMA1SAD", "DMA 1 Source Address");
			addLabel(0x0C0, 4, "DMA1DAD", "DMA 1 Destination Address");
			addLabel(0x0C4, 2, "DMA1CNT_L", "DMA 1 Length");
			addLabel(0x0C6, 2, "DMA1CNT_H", "DMA 1 Control");
			addLabel(0x0C8, 4, "DMA2SAD", "DMA 2 Source Address");
			addLabel(0x0CC, 4, "DMA2DAD", "DMA 2 Destination Address");
			addLabel(0x0D0, 2, "DMA2CNT_L", "DMA 2 Length");
			addLabel(0x0D2, 2, "DMA2CNT_H", "DMA 2 Control");
			addLabel(0x0D4, 4, "DMA3SAD", "DMA 3 Source Address");
			addLabel(0x0D8, 4, "DMA3DAD", "DMA 3 Destination Address");
			addLabel(0x0DC, 2, "DMA3CNT_L", "DMA 3 Length");
			addLabel(0x0DE, 2, "DMA3CNT_H", "DMA 3 Control");

			addLabel(0x100, 2, "TM0CNT_L", "Timer 0 Counter/Reload");
			addLabel(0x102, 1, "TM0CNT_H", "Timer 0 Control");
			addLabel(0x104, 2, "TM1CNT_L", "Timer 1 Counter/Reload");
			addLabel(0x106, 1, "TM1CNT_H", "Timer 1 Control");
			addLabel(0x108, 2, "TM2CNT_L", "Timer 2 Counter/Reload");
			addLabel(0x10A, 1, "TM2CNT_H", "Timer 2 Control");
			addLabel(0x10C, 2, "TM3CNT_L", "Timer 3 Counter/Reload");
			addLabel(0x10E, 1, "TM3CNT_H", "Timer 3 Control");

			addLabel(0x120, 4, "SIODATA32", "Serial I/O Data (32-bit)");
			addLabel(0x124, 2, "SIOMULTI2", "Serial I/O Multiplayer Data 2");
			addLabel(0x126, 2, "SIOMULTI3", "Serial I/O Multiplayer Data 3");
			addLabel(0x128, 2, "SIOCNT", "Serial I/O Control");
			addLabel(0x12A, 2, "SIODATA8", "Serial I/O Data (8-bit)");
			
			addLabel(0x130, 2, "KEYINPUT", "Key Status");
			addLabel(0x132, 2, "KEYCNT", "Key IRQ Control");
			
			addLabel(0x134, 2, "RNT", "Serial I/O Mode Select");
			addLabel(0x140, 2, "JOYCNT", "Serial I/O JOY Bus Control");
			addLabel(0x150, 4, "JOYRECV", "Serial I/O JOY Bus Receive Data");
			addLabel(0x154, 4, "JOYSEND", "Serial I/O JOY Bus Send Data");
			addLabel(0x158, 2, "JOYSTAT", "Serial I/O JOY Bus Status");
			
			addLabel(0x200, 2, "IE", "IRQ Enable");
			addLabel(0x202, 2, "IF", "IRQ Flags");
			addLabel(0x204, 2, "WAITCNT", "Waitstate Control");
			addLabel(0x208, 2, "IME", "IRQ Master Enable");
			addLabel(0x300, 1, "POSTFLG", "Post Boot Flag");
			addLabel(0x301, 1, "HALTCNT", "Halt Control");
		}

		private static void SetWsDefaultLabels()
		{
			Action<uint, uint, string> addLabel = (addr, length, label) => {
				LabelManager.SetLabel(new CodeLabel() {
					Address = addr,
					Length = length,
					MemoryType = MemoryType.WsPort,
					Label = label
				}, false);
			};

			addLabel(0x00, 1, "IO_DISPLAY_CTRL");
			addLabel(0x01, 1, "IO_DISPLAY_BACK");
			addLabel(0x02, 1, "IO_LCD_LINE");
			addLabel(0x03, 1, "IO_LCD_INTERRUPT");
			addLabel(0x04, 1, "IO_SPR_BASE");
			addLabel(0x05, 1, "IO_SPR_FIRST");
			addLabel(0x06, 1, "IO_SPR_COUNT");
			addLabel(0x07, 1, "IO_SCR_BASE");
			addLabel(0x08, 1, "IO_SCR2_WIN_X1");
			addLabel(0x09, 1, "IO_SCR2_WIN_Y1");
			addLabel(0x0A, 1, "IO_SCR2_WIN_X2");
			addLabel(0x0B, 1, "IO_SCR2_WIN_Y2");
			addLabel(0x0C, 1, "IO_SPR_WIN_X1");
			addLabel(0x0D, 1, "IO_SPR_WIN_Y1");
			addLabel(0x0E, 1, "IO_SPR_WIN_X2");
			addLabel(0x0F, 1, "IO_SPR_WIN_Y2");
			addLabel(0x10, 1, "IO_SCR1_SCRL_X");
			addLabel(0x11, 1, "IO_SCR1_SCRL_Y");
			addLabel(0x12, 1, "IO_SCR2_SCRL_X");
			addLabel(0x13, 1, "IO_SCR2_SCRL_Y");
			addLabel(0x14, 1, "IO_LCD_CTRL");
			addLabel(0x15, 1, "IO_LCD_SEG");
			addLabel(0x16, 1, "IO_LCD_VTOTAL");
			addLabel(0x17, 1, "IO_LCD_VSYNC");
			addLabel(0x1A, 1, "IO_LCD_STATUS");
			addLabel(0x1C, 1, "IO_LCD_SHADE_01");
			addLabel(0x1D, 1, "IO_LCD_SHADE_23");
			addLabel(0x1E, 1, "IO_LCD_SHADE_45");
			addLabel(0x1F, 1, "IO_LCD_SHADE_67");
			addLabel(0x20, 2, "IO_PAL_0");
			addLabel(0x22, 2, "IO_PAL_1");
			addLabel(0x24, 2, "IO_PAL_2");
			addLabel(0x26, 2, "IO_PAL_3");
			addLabel(0x28, 2, "IO_PAL_4");
			addLabel(0x2A, 2, "IO_PAL_5");
			addLabel(0x2C, 2, "IO_PAL_6");
			addLabel(0x2E, 2, "IO_PAL_7");
			addLabel(0x30, 2, "IO_PAL_8");
			addLabel(0x32, 2, "IO_PAL_9");
			addLabel(0x34, 2, "IO_PAL_10");
			addLabel(0x36, 2, "IO_PAL_11");
			addLabel(0x38, 2, "IO_PAL_12");
			addLabel(0x3A, 2, "IO_PAL_13");
			addLabel(0x3C, 2, "IO_PAL_14");
			addLabel(0x3E, 2, "IO_PAL_15");
			addLabel(0x40, 2, "IO_DMA_SOURCE_L");
			addLabel(0x42, 1, "IO_DMA_SOURCE_H");
			addLabel(0x44, 2, "IO_DMA_DEST");
			addLabel(0x46, 2, "IO_DMA_LENGTH");
			addLabel(0x48, 1, "IO_DMA_CTRL");
			addLabel(0x4A, 2, "IO_SDMA_SOURCE_L");
			addLabel(0x4C, 1, "IO_SDMA_SOURCE_H");
			addLabel(0x4E, 2, "IO_SDMA_LENGTH_L");
			addLabel(0x50, 1, "IO_SDMA_LENGTH_H");
			addLabel(0x52, 1, "IO_SDMA_CTRL");
			addLabel(0x60, 1, "IO_SYSTEM_CTRL2");
			addLabel(0x62, 1, "IO_SYSTEM_CTRL3");
			addLabel(0x64, 2, "IO_HYPERV_OUT_L");
			addLabel(0x66, 2, "IO_HYPERV_OUT_R");
			addLabel(0x68, 1, "IO_HYPERV_IN_L");
			addLabel(0x69, 1, "IO_HYPERV_IN_R");
			addLabel(0x6A, 2, "IO_HYPERV_CTRL");
			addLabel(0x80, 2, "IO_SND_FREQ_CH1");
			addLabel(0x82, 2, "IO_SND_FREQ_CH2");
			addLabel(0x84, 2, "IO_SND_FREQ_CH3");
			addLabel(0x86, 2, "IO_SND_FREQ_CH4");
			addLabel(0x88, 1, "IO_SND_VOL_CH1");
			addLabel(0x89, 1, "IO_SND_VOL_CH2");
			addLabel(0x8A, 1, "IO_SND_VOL_CH3");
			addLabel(0x8B, 1, "IO_SND_VOL_CH4");
			addLabel(0x8C, 1, "IO_SND_SWEEP");
			addLabel(0x8D, 1, "IO_SND_SWEEP_TIME");
			addLabel(0x8E, 1, "IO_SND_NOISE_CTRL");
			addLabel(0x8F, 1, "IO_SND_WAVE_BASE");
			addLabel(0x90, 1, "IO_SND_CH_CTRL");
			addLabel(0x91, 1, "IO_SND_OUT_CTRL");
			addLabel(0x92, 2, "IO_SND_RANDOM");
			addLabel(0x94, 1, "IO_SND_VOL_CH2_VOICE");
			addLabel(0x95, 1, "IO_SND_TEST");
			addLabel(0x96, 2, "IO_SND_CH_OUT_R");
			addLabel(0x98, 2, "IO_SND_CH_OUT_L");
			addLabel(0x9A, 2, "IO_SND_CH_OUT_LR");
			addLabel(0x9E, 1, "IO_SND_HW_VOL");
			addLabel(0xA0, 1, "IO_SYSTEM_CTRL1");
			addLabel(0xA2, 1, "IO_TIMER_CTRL");
			addLabel(0xA4, 2, "IO_HBLANK_TIMER");
			addLabel(0xA6, 2, "IO_VBLANK_TIMER");
			addLabel(0xA8, 2, "IO_HBLANK_COUNTER");
			addLabel(0xAA, 2, "IO_VBLANK_COUNTER");
			addLabel(0xB0, 1, "IO_HWINT_VECTOR");
			addLabel(0xB2, 1, "IO_HWINT_ENABLE");
			addLabel(0xB4, 1, "IO_HWINT_STATUS");
			addLabel(0xB6, 1, "IO_HWINT_ACK");
			addLabel(0xB1, 1, "IO_SERIAL_DATA");
			addLabel(0xB3, 1, "IO_SERIAL_STATUS");
			addLabel(0xB5, 1, "IO_KEY_SCAN");
			addLabel(0xB7, 1, "IO_INT_NMI_CTRL");
			addLabel(0xBA, 2, "IO_IEEP_DATA");
			addLabel(0xBC, 2, "IO_IEEP_CMD");
			addLabel(0xBE, 1, "IO_IEEP_CTRL");
			addLabel(0xC1, 1, "IO_BANK_RAM");
			addLabel(0xC2, 1, "IO_BANK_ROM0");
			addLabel(0xC3, 1, "IO_BANK_ROM1");
			/*addLabel(0xC0, 1, "IO_BANK_ROM_LINEAR");
			addLabel(0xC4, 1, "IO_CART_EEP_DATA");
			addLabel(0xC6, 1, "IO_CART_EEP_CMD");
			addLabel(0xC8, 1, "IO_CART_EEP_CTRL");
			addLabel(0xCA, 1, "IO_CART_RTC_CTRL");
			addLabel(0xCB, 1, "IO_CART_RTC_DATA");
			addLabel(0xCC, 1, "IO_CART_GPO_CTRL");
			addLabel(0xCD, 1, "IO_CART_GPO_DATA");
			addLabel(0xCE, 1, "IO_CART_FLASH");
			addLabel(0xD0, 1, "IO_BANK_2003_RAM");
			addLabel(0xD2, 1, "IO_BANK_2003_ROM0");
			addLabel(0xD4, 1, "IO_BANK_2003_ROM1");
			addLabel(0xD6, 1, "IO_CART_KARNAK_TIMER");
			addLabel(0xD8, 1, "IO_CART_KARNAK_ADPCM_INPUT");
			addLabel(0xD9, 1, "IO_CART_KARNAK_ADPCM_OUTPUT");*/
		}
	}
}