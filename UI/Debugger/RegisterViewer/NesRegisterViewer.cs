using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using static Mesen.Debugger.ViewModels.RegEntry;

namespace Mesen.Debugger.RegisterViewer;

public class NesRegisterViewer
{
	public static List<RegisterViewerTab> GetTabs(ref NesState nesState)
	{
		List<RegisterViewerTab> tabs = new() {
			GetNesPpuTab(ref nesState),
			GetNesApuTab(ref nesState)
		};

		RegisterViewerTab cartTab = GetNesCartTab(ref nesState);
		if(cartTab.Data.Count > 0) {
			tabs.Add(cartTab);
		}

		return tabs;
	}

	private static RegisterViewerTab GetNesPpuTab(ref NesState state)
	{
		NesPpuState ppu = state.Ppu;

		List<RegEntry> entries = new List<RegEntry>() {
			new RegEntry("", "State"),
			new RegEntry("", "Cycle (H)", ppu.Cycle),
			new RegEntry("", "Scanline (V)", ppu.Scanline),
			new RegEntry("", "Frame Number", ppu.FrameCount),
			new RegEntry("", "PPU Bus Address", ppu.BusAddress, Format.X16),
			new RegEntry("", "PPU Register Buffer", ppu.MemoryReadBuffer, Format.X8),

			new RegEntry("$2000", "Control"),
			new RegEntry("$2000.2", "Increment Mode", ppu.Control.VerticalWrite ? "32 bytes" : "1 byte", ppu.Control.VerticalWrite),
			new RegEntry("$2000.3", "Sprite Table Address", ppu.Control.SpritePatternAddr == 0 ? "$0000" : "$1000", ppu.Control.SpritePatternAddr),
			new RegEntry("$2000.4", "BG Table Address", ppu.Control.BackgroundPatternAddr == 0 ? "$0000" : "$1000", ppu.Control.BackgroundPatternAddr),
			new RegEntry("$2000.5", "Sprite Size", ppu.Control.LargeSprites ? "8x16" : "8x8", ppu.Control.LargeSprites),
			new RegEntry("$2000.6", "Main/secondary PPU select", ppu.Control.SecondaryPpu ? "Secondary" : "Main", ppu.Control.SecondaryPpu),
			new RegEntry("$2000.7", "NMI enabled", ppu.Control.NmiOnVerticalBlank),

			new RegEntry("$2001", "Mask"),
			new RegEntry("$2001.0", "Grayscale", ppu.Mask.Grayscale),
			new RegEntry("$2001.1", "BG - Show leftmost 8 pixels", ppu.Mask.BackgroundMask),
			new RegEntry("$2001.2", "Sprites - Show leftmost 8 pixels", ppu.Mask.SpriteMask),
			new RegEntry("$2001.3", "Background enabled", ppu.Mask.BackgroundEnabled),
			new RegEntry("$2001.4", "Sprites enabled", ppu.Mask.SpritesEnabled),
			new RegEntry("$2001.5", "Red emphasis", ppu.Mask.IntensifyRed),
			new RegEntry("$2001.6", "Green emphasis", ppu.Mask.IntensifyGreen),
			new RegEntry("$2001.7", "Blue emphasis", ppu.Mask.IntensifyBlue),

			new RegEntry("$2002", "Status"),
			new RegEntry("$2002.5", "Sprite overflow", ppu.StatusFlags.SpriteOverflow),
			new RegEntry("$2002.6", "Sprite 0 hit", ppu.StatusFlags.Sprite0Hit),
			new RegEntry("$2002.7", "Vertical blank", ppu.StatusFlags.VerticalBlank),

			new RegEntry("$2003", "OAM address", ppu.SpriteRamAddr, Format.X8),

			new RegEntry("$2005-2006", "VRAM Address / Scrolling"),
			new RegEntry("", "VRAM Address", ppu.VideoRamAddr, Format.X16),
			new RegEntry("", "T", ppu.TmpVideoRamAddr, Format.X16),
			new RegEntry("", "X Scroll", ppu.ScrollX),
			new RegEntry("", "Write Toggle", ppu.WriteToggle)
		};

		return new RegisterViewerTab("PPU", entries, CpuType.Nes, MemoryType.NesMemory);
	}

	private static RegisterViewerTab GetNesApuTab(ref NesState state)
	{
		List<RegEntry> entries = new List<RegEntry>();
		NesApuState apu = state.Apu;

		NesApuSquareState sq1 = apu.Square1;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$4000-$4003", "Square 1"),
			new RegEntry("$4000.0-3", "Envelope Volume", sq1.Envelope.Volume, Format.X8),
			new RegEntry("$4000.4", "Envelope - Constant Volume", sq1.Envelope.ConstantVolume),
			new RegEntry("$4000.5", "Length Counter - Halted", sq1.LengthCounter.Halt),
			new RegEntry("$4000.6-7", "Duty", sq1.Duty),

			new RegEntry("$4001.0-2", "Sweep - Shift", sq1.SweepShift),
			new RegEntry("$4001.3", "Sweep - Negate", sq1.SweepNegate),
			new RegEntry("$4001.4-6", "Sweep - Period", sq1.SweepPeriod),
			new RegEntry("$4001.7", "Sweep - Enabled", sq1.SweepEnabled),

			new RegEntry("$4002/$4003.0-2", "Period", sq1.Period, Format.X16),
			new RegEntry("$4003.3-7", "Length Counter - Reload Value", sq1.LengthCounter.ReloadValue, Format.X16),

			new RegEntry("--", "Enabled", sq1.Enabled),
			new RegEntry("--", "Timer", sq1.Timer, Format.X16),
			new RegEntry("--", "Frequency", Math.Round(sq1.Frequency).ToString("0.") + " Hz", null),
			new RegEntry("--", "Duty Position", sq1.DutyPosition),

			new RegEntry("--", "Length Counter - Counter", sq1.LengthCounter.Counter, Format.X8),

			new RegEntry("--", "Envelope - Counter", sq1.Envelope.Counter, Format.X8),
			new RegEntry("--", "Envelope - Divider", sq1.Envelope.Divider, Format.X8),

			new RegEntry("--", "Output", sq1.OutputVolume, Format.X8),
		});

		NesApuSquareState sq2 = apu.Square2;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$4004-$4007", "Square 2"),
			new RegEntry("$4004.0-3", "Envelope Volume", sq2.Envelope.Volume, Format.X8),
			new RegEntry("$4004.4", "Envelope - Constant Volume", sq2.Envelope.ConstantVolume),
			new RegEntry("$4004.5", "Length Counter - Halted", sq2.LengthCounter.Halt),
			new RegEntry("$4004.6-7", "Duty", sq2.Duty),

			new RegEntry("$4005.0-2", "Sweep - Shift", sq2.SweepShift),
			new RegEntry("$4005.3", "Sweep - Negate", sq2.SweepNegate),
			new RegEntry("$4005.4-6", "Sweep - Period", sq2.SweepPeriod),
			new RegEntry("$4005.7", "Sweep - Enabled", sq2.SweepEnabled),

			new RegEntry("$4006/$4007.0-2", "Period", sq2.Period, Format.X16),
			new RegEntry("$4007.3-7", "Length Counter - Reload Value", sq2.LengthCounter.ReloadValue, Format.X16),

			new RegEntry("--", "Enabled", sq2.Enabled),
			new RegEntry("--", "Timer", sq2.Timer, Format.X16),
			new RegEntry("--", "Frequency", Math.Round(sq2.Frequency).ToString("0.") + " Hz", null),
			new RegEntry("--", "Duty Position", sq2.DutyPosition),

			new RegEntry("--", "Length Counter - Counter", sq2.LengthCounter.Counter, Format.X8),

			new RegEntry("--", "Envelope - Counter", sq2.Envelope.Counter, Format.X8),
			new RegEntry("--", "Envelope - Divider", sq2.Envelope.Divider, Format.X8),

			new RegEntry("--", "Output", sq2.OutputVolume, Format.X8),
		});

		NesApuTriangleState trg = apu.Triangle;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$4008-$400B", "Triangle"),
			new RegEntry("$4008.0-6", "Linear Counter - Reload", trg.LinearCounterReload, Format.X8),
			new RegEntry("$4008.7", "Length Counter - Halted", trg.LengthCounter.Halt),

			new RegEntry("$400A/$400B.0-2", "Period", trg.Period, Format.X16),
			new RegEntry("$400B.3-7", "Length Counter - Reload Value", trg.LengthCounter.ReloadValue, Format.X16),

			new RegEntry("--", "Enabled", trg.Enabled),
			new RegEntry("--", "Timer", trg.Timer),
			new RegEntry("--", "Frequency", Math.Round(trg.Frequency).ToString("0.") + " Hz", null),
			new RegEntry("--", "Sequence Position", trg.SequencePosition),

			new RegEntry("--", "Length Counter - Counter", trg.LengthCounter.Counter),

			new RegEntry("--", "Linear Counter - Counter", trg.LinearCounter),
			new RegEntry("--", "Linear Counter - Reload Flag", trg.LinearReloadFlag),

			new RegEntry("--", "Output", trg.OutputVolume),
		});

		NesApuNoiseState noise = apu.Noise;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$400C-$400F", "Noise"),
			new RegEntry("$400C.0-3", "Envelope Volume", noise.Envelope.Volume, Format.X8),
			new RegEntry("$400C.4", "Envelope - Constant Volume", noise.Envelope.ConstantVolume),
			new RegEntry("$400C.5", "Length Counter - Halted", noise.LengthCounter.Halt),

			new RegEntry("$400E.0-3", "Period", noise.Period, Format.X16),
			new RegEntry("$400E.7", "Mode Flag", noise.ModeFlag),

			new RegEntry("$400F.3-7", "Length Counter - Reload Value", noise.LengthCounter.ReloadValue, Format.X8),

			new RegEntry("--", "Enabled", noise.Enabled),
			new RegEntry("--", "Timer", noise.Timer),
			new RegEntry("--", "Frequency", Math.Round(noise.Frequency).ToString("0.") + " Hz", null),
			new RegEntry("--", "Shift Register", noise.ShiftRegister),

			new RegEntry("--", "Envelope - Counter", noise.Envelope.Counter, Format.X8),
			new RegEntry("--", "Envelope - Divider", noise.Envelope.Divider, Format.X8),

			new RegEntry("--", "Length Counter - Counter", noise.LengthCounter.Counter),

			new RegEntry("--", "Output", noise.OutputVolume),
		});

		NesApuDmcState dmc = apu.Dmc;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$4010-4013", "DMC"),
			new RegEntry("$4010.0-3", "Period", dmc.Period, Format.X16),
			new RegEntry("$4010.6", "Loop Flag", dmc.Loop),
			new RegEntry("$4010.7", "IRQ Enabled", dmc.IrqEnabled),

			new RegEntry("$4011", "Output Level", dmc.OutputVolume),

			new RegEntry("$4012", "Sample Address", dmc.SampleAddr, Format.X16),
			new RegEntry("$4013", "Sample Length", dmc.SampleLength, Format.X16),

			new RegEntry("--", "Timer", dmc.Timer),
			new RegEntry("--", "Frequency", Math.Round(dmc.SampleRate).ToString("0."), null),
			new RegEntry("--", "Bytes Remaining", dmc.BytesRemaining),
			new RegEntry("--", "Next sample address", dmc.NextSampleAddr, Format.X16),
		});

		NesApuFrameCounterState frameCounter = apu.FrameCounter;
		entries.AddRange(new List<RegEntry>() {
			new RegEntry("$4017", "Frame Counter"),
			new RegEntry("$4017.6", "IRQ Enabled", frameCounter.IrqEnabled),
			new RegEntry("$4017.7", "5-step Mode", frameCounter.FiveStepMode),
			new RegEntry("--", "Sequence Position", frameCounter.SequencePosition),
		});

		return new RegisterViewerTab("APU", entries, CpuType.Nes, MemoryType.NesMemory);
	}

	private static RegisterViewerTab GetNesCartTab(ref NesState state)
	{
		NesCartridgeState cart = state.Cartridge;

		List<RegEntry> entries = new List<RegEntry>();
		for(int i = 0; i < cart.CustomEntryCount; i++) {
			ref MapperStateEntry entry = ref cart.CustomEntries[i];
			Format format = entry.Type switch {
				MapperStateValueType.Number8 => Format.X8,
				MapperStateValueType.Number16 => Format.X16,
				MapperStateValueType.Number32 => Format.X32,
				_ => Format.None
			};

			object? value = entry.GetValue();
			string addr = entry.GetAddress();
			string name = entry.GetName();

			if(value is ISpanFormattable) {
				entries.Add(new RegEntry(addr, name, (ISpanFormattable)value, format));
			} else if(value is bool) {
				entries.Add(new RegEntry(addr, name, (bool)value));
			} else if(value is string) {
				entries.Add(new RegEntry(addr, name, (string)value, entry.RawValue != Int64.MinValue ? entry.RawValue : null));
			} else {
				entries.Add(new RegEntry(addr, name));
			}
		}

		return new RegisterViewerTab("Cart", entries, CpuType.Nes, MemoryType.NesMemory);
	}
}
