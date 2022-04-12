using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using static Mesen.Debugger.ViewModels.RegEntry;

namespace Mesen.Debugger.ViewModels
{
	public class RegisterViewerWindowViewModel : DisposableViewModel, ICpuTypeModel
	{
		[Reactive] public List<RegisterViewerTab> Tabs { get; set; } = new List<RegisterViewerTab>();
		
		public RegisterViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }

		[Reactive] public List<object> FileMenuActions { get; private set; } = new();
		[Reactive] public List<object> ViewMenuActions { get; private set; } = new();

		private BaseState? _state = null;
		
		public CpuType CpuType
		{
			get => _romInfo.ConsoleType.GetMainCpuType();
			set { }
		}

		private RomInfo _romInfo = new RomInfo();

		public RegisterViewerWindowViewModel()
		{
			Config = ConfigManager.Config.Debug.RegisterViewer;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming);

			if(Design.IsDesignMode) {
				return;
			}

			UpdateRomInfo();
			RefreshData();
		}

		public void InitMenu(Window wnd)
		{
			FileMenuActions = AddDisposables(new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd?.Close()
				}
			});

			ViewMenuActions = AddDisposables(new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.Refresh,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Refresh),
					OnClick = () => RefreshData()
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.EnableAutoRefresh,
					IsSelected = () => Config.RefreshTiming.AutoRefresh,
					OnClick = () => Config.RefreshTiming.AutoRefresh = !Config.RefreshTiming.AutoRefresh
				},
				new ContextMenuAction() {
					ActionType = ActionType.RefreshOnBreakPause,
					IsSelected = () => Config.RefreshTiming.RefreshOnBreakPause,
					OnClick = () => Config.RefreshTiming.RefreshOnBreakPause = !Config.RefreshTiming.RefreshOnBreakPause
				}
			});

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);
		}

		public void UpdateRomInfo()
		{
			_romInfo = EmuApi.GetRomInfo();
		}

		public void RefreshData()
		{
			if(_romInfo.ConsoleType == ConsoleType.Snes) {
				_state = DebugApi.GetConsoleState<SnesState>(ConsoleType.Snes);
			} else if(_romInfo.ConsoleType == ConsoleType.Nes) {
				_state = DebugApi.GetConsoleState<NesState>(ConsoleType.Nes);
			} else if(_romInfo.ConsoleType == ConsoleType.Gameboy || _romInfo.ConsoleType == ConsoleType.GameboyColor) {
				_state = DebugApi.GetConsoleState<GbState>(ConsoleType.Gameboy);
			} else if(_romInfo.ConsoleType == ConsoleType.PcEngine) {
				_state = DebugApi.GetConsoleState<PceState>(ConsoleType.PcEngine);
			}

			Dispatcher.UIThread.Post(() => {
				RefreshTabs();
			});
		}

		public void RefreshTabs()
		{
			if(_state == null) {
				return;
			}

			List<RegisterViewerTab> tabs = new List<RegisterViewerTab>();
			BaseState lastState = _state;

			if(lastState is SnesState snesState) {
				HashSet<CpuType> cpuTypes = _romInfo.CpuTypes;

				tabs = new List<RegisterViewerTab>() {
					GetSnesCpuTab(ref snesState),
					GetSnesPpuTab(ref snesState),
					GetSnesDmaTab(ref snesState),
					GetSnesSpcTab(ref snesState),
					GetSnesDspTab(ref snesState)
				};

				if(cpuTypes.Contains(CpuType.Sa1)) {
					tabs.Add(GetSnesSa1Tab(ref snesState));
				}
				if(cpuTypes.Contains(CpuType.Gameboy)) {
					GbState gbState = DebugApi.GetConsoleState<GbState>(ConsoleType.Gameboy);
					string tabPrefix = "GB - ";
					tabs.Add(GetGbLcdTab(ref gbState, tabPrefix));
					tabs.Add(GetGbApuTab(ref gbState, tabPrefix));
					tabs.Add(GetGbMiscTab(ref gbState, tabPrefix));
				}
			} else if(lastState is NesState nesState) {
				tabs = new List<RegisterViewerTab>() {
					GetNesPpuTab(ref nesState),
					GetNesApuTab(ref nesState),
					GetNesCartTab(ref nesState)
				};
			} else if(lastState is GbState gbState) {
				tabs = new List<RegisterViewerTab>() {
					GetGbLcdTab(ref gbState),
					GetGbApuTab(ref gbState),
					GetGbMiscTab(ref gbState)
				};
			} else if(lastState is PceState pceState) {
				tabs = new List<RegisterViewerTab>() {
					GetPceCpuTab(ref pceState),
					GetPcePpuTab(ref pceState),
					GetPcePsgTab(ref pceState)
				};
			}

			if(Tabs.Count != tabs.Count) {
				Tabs = tabs;
			} else {
				for(int i = 0; i < Tabs.Count; i++) {
					Tabs[i].SetData(tabs[i].Data);
					Tabs[i].TabName = tabs[i].TabName;
				}
			}
		}

		private RegisterViewerTab GetGbLcdTab(ref GbState gb, string tabPrefix = "")
		{
			List<RegEntry> entries = new List<RegEntry>();

			GbPpuState ppu = gb.Ppu;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "State", null),
				new RegEntry("", "Cycle (H)", ppu.Cycle),
				new RegEntry("", "Scanline (V)", ppu.Scanline),
				new RegEntry("", "Frame Number", ppu.FrameCount),

				new RegEntry("$FF40", "LCD Control (LCDC)", null),
				new RegEntry("$FF40.0", "Background Enabled", ppu.BgEnabled),
				new RegEntry("$FF40.1", "Sprites Enabled", ppu.BgEnabled),
				new RegEntry("$FF40.2", "Sprite size", ppu.LargeSprites ? "8x16" : "8x8"),
				new RegEntry("$FF40.3", "BG Tilemap Select", ppu.BgTilemapSelect ? 0x9C00 : 0x9800, Format.X16),
				new RegEntry("$FF40.4", "BG Tile Select", ppu.BgTileSelect ? "$8000-$8FFF" : "$8800-$97FF"),
				new RegEntry("$FF40.5", "Window Enabled", ppu.WindowEnabled),
				new RegEntry("$FF40.6", "Window Tilemap Select", ppu.WindowTilemapSelect ? 0x9C00 : 0x9800, Format.X16),
				new RegEntry("$FF40.7", "LCD Enabled", ppu.LcdEnabled),

				new RegEntry("$FF41", "LCD Status (STAT)", null),
				new RegEntry("$FF41.0-1", "Mode", (int)ppu.Mode),
				new RegEntry("$FF41.2", "Coincidence Flag", ppu.LyCoincidenceFlag),
				new RegEntry("$FF41.3", "Mode 0 H-Blank IRQ", (ppu.Status & 0x08) != 0),
				new RegEntry("$FF41.4", "Mode 1 V-Blank IRQ", (ppu.Status & 0x10) != 0),
				new RegEntry("$FF41.5", "Mode 2 OAM IRQ", (ppu.Status & 0x20) != 0),
				new RegEntry("$FF41.6", "LYC=LY Coincidence IRQ", (ppu.Status & 0x40) != 0),

				new RegEntry("", "LCD Registers", null),
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

			return new RegisterViewerTab() {
				TabName = tabPrefix + "LCD",
				Data = entries
			};
		}

		private RegisterViewerTab GetGbMiscTab(ref GbState gb, string tabPrefix = "")
		{
			List<RegEntry> entries = new List<RegEntry>();

			GbTimerState timer = gb.Timer;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("$FF04-7", "Timer", null),
				new RegEntry("$FF04", "DIV - Divider", timer.Divider, Format.X16),
				new RegEntry("$FF05", "TIMA - Counter", timer.Counter, Format.X8),
				new RegEntry("$FF06", "TMA - Modulo", timer.Modulo, Format.X8),
				new RegEntry("$FF07", "TAC - Control", timer.Control, Format.X8)
			});

			GbDmaControllerState dma = gb.Dma;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "DMA", null),
				new RegEntry("$FF46", "OAM DMA - Source", (dma.OamDmaSource << 8), Format.X16),
				new RegEntry("$FF51-2", "CGB - Source", dma.CgbDmaSource, Format.X16),
				new RegEntry("$FF53-4", "CGB - Destination", dma.CgbDmaDest, Format.X16),
				new RegEntry("$FF55.0-6", "CGB - Length", dma.CgbDmaLength, Format.X8),
				new RegEntry("$FF55.7", "CGB - HDMA Done", dma.CgbHdmaDone),
				new RegEntry("", "CGB - HDMA Running", dma.CgbHdmaRunning),
			});

			GbMemoryManagerState memManager = gb.MemoryManager;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "IRQ", null),
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

				new RegEntry("", "Misc", null),
				new RegEntry("$FF00", "Input Select", gb.ControlManager.InputSelect, Format.X8),
				new RegEntry("$FF01", "Serial Data", memManager.SerialData, Format.X8),
				new RegEntry("$FF02", "Serial Control", memManager.SerialControl, Format.X8),
				new RegEntry("", "Serial Bit Count", memManager.SerialBitCount),
			});

			return new RegisterViewerTab() {
				TabName = tabPrefix + "Timer/DMA/IRQ",
				Data = entries
			};
		}

		private RegisterViewerTab GetGbApuTab(ref GbState gb, string tabPrefix = "")
		{
			List<RegEntry> entries = new List<RegEntry>();

			GbApuState apu = gb.Apu.Common;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "APU", null),
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
				new RegEntry("$FF10-$FF14", "Square 1", null),
				new RegEntry("$FF10.0-2", "Sweep Shift", sq1.SweepShift),
				new RegEntry("$FF10.3", "Sweep Negate", sq1.SweepNegate),
				new RegEntry("$FF10.4-7", "Sweep Period", sq1.SweepPeriod),

				new RegEntry("$FF11.0-5", "Length", sq1.Length),
				new RegEntry("$FF11.6-7", "Duty", sq1.Duty),

				new RegEntry("$FF12.0-2", "Envelope Period", sq1.EnvPeriod),
				new RegEntry("$FF12.3", "Envelope Increase Volume", sq1.EnvRaiseVolume),
				new RegEntry("$FF12.4-7", "Envelope Volume", sq1.EnvVolume),

				new RegEntry("$FF13+$FF14.0-2", "Frequency", sq1.Frequency),
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
				new RegEntry("$FF16-$FF19", "Square 2", null),
				new RegEntry("$FF16.0-5", "Length", sq2.Length),
				new RegEntry("$FF16.6-7", "Duty", sq2.Duty),

				new RegEntry("$FF17.0-2", "Envelope Period", sq2.EnvPeriod),
				new RegEntry("$FF17.3", "Envelope Increase Volume", sq2.EnvRaiseVolume),
				new RegEntry("$FF17.4-7", "Envelope Volume", sq2.EnvVolume),

				new RegEntry("$FF18+$FF19.0-2", "Frequency", sq2.Frequency),
				new RegEntry("$FF19.6", "Length Counter Enabled", sq2.LengthEnabled),
				new RegEntry("$FF19.7", "Channel Enabled", sq2.Enabled),

				new RegEntry("--", "Timer", sq2.Timer),
				new RegEntry("--", "Duty Position", sq2.DutyPos),
				new RegEntry("--", "Envelope Timer", sq2.EnvTimer),
				new RegEntry("--", "Output", sq2.Output)
			});

			GbNoiseState noise = gb.Apu.Noise;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("$FF20-$FF23", "Noise", null),
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


			GbWaveState wave = gb.Apu.Wave;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("$FF1A-$FF1E", "Wave", null),
				new RegEntry("$FF1A.7", "Sound Enabled", wave.DacEnabled),

				new RegEntry("$FF1B", "Length", wave.Length),

				new RegEntry("$FF1C.5-6", "Volume", wave.Volume),

				new RegEntry("$FF1D+$FF1E.0-2", "Frequency", wave.Frequency),

				new RegEntry("$FF1E.6", "Length Counter Enabled", wave.LengthEnabled),
				new RegEntry("$FF1E.7", "Channel Enabled", wave.Enabled),

				new RegEntry("--", "Timer", wave.Timer),
				new RegEntry("--", "Sample Buffer", wave.SampleBuffer),
				new RegEntry("--", "Position", wave.Position),
				new RegEntry("--", "Output", wave.Output),
			});

			return new RegisterViewerTab() {
				TabName = tabPrefix + "APU",
				Data = entries
			};
		}

		private RegisterViewerTab GetSnesSa1Tab(ref SnesState state)
		{
			Sa1State sa1 = state.Sa1.Sa1;

			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("$2200", "SA-1 CPU Control", null),
				new RegEntry("$2200.0-3", "Message", sa1.Sa1MessageReceived, Format.X8),
				new RegEntry("$2200.4", "NMI Requested", sa1.Sa1NmiRequested),
				new RegEntry("$2200.5", "Reset", sa1.Sa1Reset),
				new RegEntry("$2200.6", "Wait", sa1.Sa1Wait),
				new RegEntry("$2200.7", "IRQ Requested", sa1.Sa1IrqRequested),

				new RegEntry("$2201", "S-CPU Interrupt Enable", null),
				new RegEntry("$2201.5", "Character Conversion IRQ Enable", sa1.CharConvIrqEnabled),
				new RegEntry("$2201.7", "IRQ Enabled", sa1.CpuIrqEnabled),

				new RegEntry("$2202", "S-CPU Interrupt Clear", null),
				new RegEntry("$2202.5", "Character IRQ Flag", sa1.CharConvIrqFlag),
				new RegEntry("$2202.7", "IRQ Flag", sa1.CpuIrqRequested),

				new RegEntry("$2203/4", "SA-1 Reset Vector", sa1.Sa1ResetVector, Format.X16),
				new RegEntry("$2205/6", "SA-1 NMI Vector", sa1.Sa1ResetVector, Format.X16),
				new RegEntry("$2207/8", "SA-1 IRQ Vector", sa1.Sa1ResetVector, Format.X16),

				new RegEntry("$2209", "S-CPU Control", null),
				new RegEntry("$2209.0-3", "Message", sa1.CpuMessageReceived, Format.X8),
				new RegEntry("$2209.4", "Use NMI Vector", sa1.UseCpuNmiVector),
				new RegEntry("$2209.6", "Use IRQ Vector", sa1.UseCpuIrqVector),
				new RegEntry("$2209.7", "IRQ Requested", sa1.CpuIrqRequested),

				new RegEntry("$220A", "SA-1 CPU Interrupt Enable", null),
				new RegEntry("$220A.4", "SA-1 NMI Enabled", sa1.Sa1NmiEnabled),
				new RegEntry("$220A.5", "DMA IRQ Enabled", sa1.DmaIrqEnabled),
				new RegEntry("$220A.6", "Timer IRQ Enabled", sa1.TimerIrqEnabled),
				new RegEntry("$220A.7", "SA-1 IRQ Enabled", sa1.Sa1IrqEnabled),

				new RegEntry("$220B", "S-CPU Interrupt Clear", null),
				new RegEntry("$220B.4", "SA-1 NMI Requested", sa1.Sa1NmiRequested),
				new RegEntry("$220B.5", "DMA IRQ Flag", sa1.DmaIrqFlag),
				new RegEntry("$220B.7", "SA-1 IRQ Requested", sa1.Sa1IrqRequested),

				new RegEntry("$220C/D", "S-CPU NMI Vector", sa1.CpuNmiVector, Format.X16),
				new RegEntry("$220E/F", "S-CPU IRQ Vector", sa1.CpuIrqVector, Format.X16),

				new RegEntry("$2210", "H/V Timer Control", null),
				new RegEntry("$2210.0", "Horizontal Timer Enabled", sa1.HorizontalTimerEnabled),
				new RegEntry("$2210.1", "Vertical Timer Enabled", sa1.VerticalTimerEnabled),
				new RegEntry("$2210.7", "Linear Timer", sa1.UseLinearTimer),

				new RegEntry("$2212/3", "H-Timer", sa1.HTimer, Format.X16),
				new RegEntry("$2214/5", "V-Timer", sa1.VTimer, Format.X16),

				new RegEntry("", "ROM/BWRAM/IRAM Mappings", null),
				new RegEntry("$2220", "MMC Bank C", sa1.Banks[0], Format.X8),
				new RegEntry("$2221", "MMC Bank D", sa1.Banks[1], Format.X8),
				new RegEntry("$2222", "MMC Bank E", sa1.Banks[2], Format.X8),
				new RegEntry("$2223", "MMC Bank F", sa1.Banks[3], Format.X8),

				new RegEntry("$2224", "S-CPU BW-RAM Bank", sa1.CpuBwBank, Format.X8),
				new RegEntry("$2225.0-6", "SA-1 CPU BW-RAM Bank", sa1.Sa1BwBank, Format.X8),
				new RegEntry("$2225.7", "SA-1 CPU BW-RAM Mode", sa1.Sa1BwMode, Format.X8),
				new RegEntry("$2226.7", "S-CPU BW-RAM Write Enabled", sa1.CpuBwWriteEnabled),
				new RegEntry("$2227.7", "SA-1 BW-RAM Write Enabled", sa1.Sa1BwWriteEnabled),
				new RegEntry("$2228.0-3", "S-CPU BW-RAM Write Protected Area", sa1.BwWriteProtectedArea, Format.X8),
				new RegEntry("$2229", "S-CPU I-RAM Write Protection", sa1.CpuIRamWriteProtect, Format.X8),
				new RegEntry("$222A", "SA-1 CPU I-RAM Write Protection", sa1.Sa1IRamWriteProtect, Format.X8),

				new RegEntry("$2230", "DMA Control", null),
				new RegEntry("$2230.0-1", "DMA Source Device", sa1.DmaSrcDevice.ToString()),
				new RegEntry("$2230.2-3", "DMA Destination Device", sa1.DmaDestDevice.ToString()),
				new RegEntry("$2230.4", "Automatic DMA Character Conversion", sa1.DmaCharConvAuto),
				new RegEntry("$2230.5", "DMA Character Conversion", sa1.DmaCharConv),
				new RegEntry("$2230.6", "DMA Priority", sa1.DmaPriority),
				new RegEntry("$2230.7", "DMA Enabled", sa1.DmaEnabled),

				new RegEntry("$2231.0-1", "Character Format (BPP)", sa1.CharConvBpp, Format.D),
				new RegEntry("$2231.2-5", "Character Conversion Width", sa1.CharConvWidth, Format.X8),
				new RegEntry("$2231.7", "Character DMA Active", sa1.CharConvDmaActive),

				new RegEntry("$2232/3/4", "DMA Source Address", sa1.DmaSrcAddr, Format.X24),
				new RegEntry("$2235/6/7", "DMA Destination Address", sa1.DmaDestAddr, Format.X24),

				new RegEntry("$2238/9", "DMA Size", sa1.DmaSize, Format.X16),
				new RegEntry("$223F.7", "BW-RAM 2 bpp mode", sa1.BwRam2BppMode)
			};

			entries.Add(new RegEntry("", "Bitmap Register File", null));
			for(int i = 0; i < 8; i++) {
				entries.Add(new RegEntry("$224" + i, "BRF #" + i, sa1.BitmapRegister1[i]));
			}
			for(int i = 0; i < 8; i++) {
				entries.Add(new RegEntry("$224" + (8 + i).ToString("X"), "BRF #" + (i + 8), sa1.BitmapRegister2[i]));
			}

			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "Math Registers", null),
				new RegEntry("$2250.0-1", "Math Operation", sa1.MathOp.ToString()),
				new RegEntry("$2251/2", "Multiplicand/Dividend", sa1.MultiplicandDividend, Format.X16),
				new RegEntry("$2253/4", "Multiplier/Divisor", sa1.MultiplierDivisor, Format.X16),

				new RegEntry("", "Variable Length Registers", null),
				new RegEntry("$2258", "Variable Length Bit Processing", null),
				new RegEntry("$2258.0-3", "Variable Length Bit Count", sa1.VarLenBitCount, Format.X8),
				new RegEntry("$2258.7", "Variable Length Auto-Increment", sa1.VarLenAutoInc),
				new RegEntry("$2259/A/B", "Variable Length Address", sa1.VarLenAddress, Format.X24),

				new RegEntry("$2300", "S-CPU Status Flags", null),
				new RegEntry("$2300.0-3", "Message Received", sa1.CpuMessageReceived, Format.X8),
				new RegEntry("$2300.4", "Use NMI Vector", sa1.UseCpuNmiVector),
				new RegEntry("$2300.5", "Character Conversion IRQ Flag", sa1.CharConvIrqFlag),
				new RegEntry("$2300.6", "Use IRQ Vector", sa1.UseCpuIrqVector),
				new RegEntry("$2300.7", "IRQ Requested", sa1.CpuIrqRequested),

				new RegEntry("$2301", "SA-1 Status Flags", null),
				new RegEntry("$2301.0-3", "Message Received", sa1.Sa1MessageReceived, Format.X8),
				new RegEntry("$2301.4", "NMI Requested", sa1.Sa1NmiRequested),
				new RegEntry("$2301.5", "DMA IRQ Flag", sa1.DmaIrqFlag),
				new RegEntry("$2301.7", "IRQ Requested", sa1.Sa1IrqRequested),

				new RegEntry("$2302/3", "SA-1 H-Counter", 0, Format.X16),
				new RegEntry("$2304/5", "SA-1 V-Counter", 0, Format.X16),

				new RegEntry("$2306/7/8/9/A", "Math Result", sa1.MathOpResult),
				new RegEntry("$230B.7", "Math Overflow", sa1.MathOverflow)
			});

			return new RegisterViewerTab() {
				TabName = "SA-1",
				Data = entries
			};
		}

		private RegisterViewerTab GetSnesPpuTab(ref SnesState state)
		{
			SnesPpuState ppu = state.Ppu;

			string GetLayerSize(LayerConfig layer)
			{
				return (layer.DoubleWidth ? "64" : "32") + "x" + (layer.DoubleHeight ? "64" : "32");
			}

			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("", "State", null),
				new RegEntry("", "Cycle (H)", ppu.Cycle),
				new RegEntry("", "HClock", ppu.HClock),
				new RegEntry("", "Scanline (V)", ppu.Scanline),
				new RegEntry("", "Frame Number", ppu.FrameCount),

				new RegEntry("$2100", "Brightness", null),
				new RegEntry("$2100.0-3", "Brightness", ppu.ScreenBrightness),
				new RegEntry("$2100.7", "Forced Blank", ppu.ForcedVblank),
				new RegEntry("$2101", "OAM Settings", null),
				new RegEntry("$2100.0-2", "OAM Table Address", ppu.OamBaseAddress, Format.X16),
				new RegEntry("$2100.3-4", "OAM Second Table Address", (ppu.OamBaseAddress + ppu.OamAddressOffset) & 0x7FFF, Format.X16),
				new RegEntry("$2101.5-7", "OAM Size Mode", ppu.OamMode),
				new RegEntry("$2102-2103", "OAM Base Address", ppu.OamRamAddress),
				new RegEntry("$2103.7", "OAM Priority", ppu.EnableOamPriority),

				new RegEntry("$2105", "BG Mode/Size", null),
				new RegEntry("$2105.0-2", "BG Mode", ppu.BgMode),
				new RegEntry("$2105.3", "Mode 1 BG3 Priority", ppu.Mode1Bg3Priority),
				new RegEntry("$2105.4", "BG1 16x16 Tiles", ppu.Layers[0].LargeTiles),
				new RegEntry("$2105.5", "BG2 16x16 Tiles", ppu.Layers[1].LargeTiles),
				new RegEntry("$2105.6", "BG3 16x16 Tiles", ppu.Layers[2].LargeTiles),
				new RegEntry("$2105.7", "BG4 16x16 Tiles", ppu.Layers[3].LargeTiles),

				new RegEntry("$2106", "Mosaic", null),
				new RegEntry("$2106.0", "BG1 Mosaic Enabled", (ppu.MosaicEnabled & 0x01) != 0),
				new RegEntry("$2106.1", "BG2 Mosaic Enabled", (ppu.MosaicEnabled & 0x02) != 0),
				new RegEntry("$2106.2", "BG3 Mosaic Enabled", (ppu.MosaicEnabled & 0x04) != 0),
				new RegEntry("$2106.3", "BG4 Mosaic Enabled", (ppu.MosaicEnabled & 0x08) != 0),
				new RegEntry("$2106.4-7", "Mosaic Size", (ppu.MosaicSize - 1).ToString() + " (" + ppu.MosaicSize.ToString() + "x" + ppu.MosaicSize.ToString() + ")"),

				new RegEntry("$2107 - $210A", "Tilemap Addresses/Sizes", null),
				new RegEntry("$2107.0-1", "BG1 Size", GetLayerSize(ppu.Layers[0])),
				new RegEntry("$2107.2-6", "BG1 Address", ppu.Layers[0].TilemapAddress, Format.X16),
				new RegEntry("$2108.0-1", "BG2 Size", GetLayerSize(ppu.Layers[1])),
				new RegEntry("$2108.2-6", "BG2 Address", ppu.Layers[1].TilemapAddress, Format.X16),
				new RegEntry("$2109.0-1", "BG3 Size", GetLayerSize(ppu.Layers[2])),
				new RegEntry("$2109.2-6", "BG3 Address", ppu.Layers[2].TilemapAddress, Format.X16),
				new RegEntry("$210A.0-1", "BG4 Size", GetLayerSize(ppu.Layers[3])),
				new RegEntry("$210A.2-6", "BG4 Address", ppu.Layers[3].TilemapAddress, Format.X16),

				new RegEntry("$210B - $210C", "Tile Addresses", null),
				new RegEntry("$210B.0-2", "BG1 Tile Address", ppu.Layers[0].ChrAddress, Format.X16),
				new RegEntry("$210B.4-6", "BG2 Tile Address", ppu.Layers[1].ChrAddress, Format.X16),
				new RegEntry("$210C.0-2", "BG3 Tile Address", ppu.Layers[2].ChrAddress, Format.X16),
				new RegEntry("$210C.4-6", "BG4 Tile Address", ppu.Layers[3].ChrAddress, Format.X16),

				new RegEntry("$210D - $2114", "H/V Scroll Offsets", null),
				new RegEntry("$210D", "BG1 H Offset", ppu.Layers[0].HScroll, Format.X16),
				new RegEntry("$210D", "Mode7 H Offset", ppu.Mode7.HScroll, Format.X16),
				new RegEntry("$210E", "BG1 V Offset", ppu.Layers[0].VScroll, Format.X16),
				new RegEntry("$210E", "Mode7 V Offset", ppu.Mode7.VScroll, Format.X16),

				new RegEntry("$210F", "BG2 H Offset", ppu.Layers[1].HScroll, Format.X16),
				new RegEntry("$2110", "BG2 V Offset", ppu.Layers[1].VScroll, Format.X16),
				new RegEntry("$2111", "BG3 H Offset", ppu.Layers[2].HScroll, Format.X16),
				new RegEntry("$2112", "BG3 V Offset", ppu.Layers[2].VScroll, Format.X16),
				new RegEntry("$2113", "BG4 H Offset", ppu.Layers[3].HScroll, Format.X16),
				new RegEntry("$2114", "BG4 V Offset", ppu.Layers[3].VScroll, Format.X16),

				new RegEntry("$2115 - $2117", "VRAM", null),
				new RegEntry("$2115.0-1", "Increment Value", ppu.VramIncrementValue),
				new RegEntry("$2115.2-3", "Address Mapping", ppu.VramAddressRemapping),
				new RegEntry("$2115.7", "Increment on $2119", ppu.VramAddrIncrementOnSecondReg),
				new RegEntry("$2116/7", "VRAM Address", ppu.VramAddress, Format.X16),

				new RegEntry("$211A - $2120", "Mode 7", null),
				new RegEntry("$211A.0", "Mode 7 - Hor. Mirroring", ppu.Mode7.HorizontalMirroring),
				new RegEntry("$211A.1", "Mode 7 - Vert. Mirroring", ppu.Mode7.VerticalMirroring),
				new RegEntry("$211A.6", "Mode 7 - Fill w/ Tile 0", ppu.Mode7.FillWithTile0),
				new RegEntry("$211A.7", "Mode 7 - Large Tilemap", ppu.Mode7.LargeMap),

				new RegEntry("$211B", "Mode 7 - Matrix A", ppu.Mode7.Matrix[0], Format.X16),
				new RegEntry("$211C", "Mode 7 - Matrix B", ppu.Mode7.Matrix[1], Format.X16),
				new RegEntry("$211D", "Mode 7 - Matrix C", ppu.Mode7.Matrix[2], Format.X16),
				new RegEntry("$211E", "Mode 7 - Matrix D", ppu.Mode7.Matrix[3], Format.X16),

				new RegEntry("$211F", "Mode 7 - Center X", ppu.Mode7.CenterX, Format.X16),
				new RegEntry("$2120", "Mode 7 - Center Y", ppu.Mode7.CenterY, Format.X16),

				new RegEntry("$2123 - $212B", "Windows", null),
				new RegEntry("", "BG1 Windows", null),
				new RegEntry("$2123.0", "BG1 Window 1 Inverted", ppu.Window[0].InvertedLayers[0] != 0),
				new RegEntry("$2123.1", "BG1 Window 1 Active", ppu.Window[0].ActiveLayers[0] != 0),
				new RegEntry("$2123.2", "BG1 Window 2 Inverted", ppu.Window[1].InvertedLayers[0] != 0),
				new RegEntry("$2123.3", "BG1 Window 2 Active", ppu.Window[1].ActiveLayers[0] != 0),

				new RegEntry("", "BG2 Windows", null),
				new RegEntry("$2123.4", "BG2 Window 1 Inverted", ppu.Window[0].InvertedLayers[1] != 0),
				new RegEntry("$2123.5", "BG2 Window 1 Active", ppu.Window[0].ActiveLayers[1] != 0),
				new RegEntry("$2123.6", "BG2 Window 2 Inverted", ppu.Window[1].InvertedLayers[1] != 0),
				new RegEntry("$2123.7", "BG2 Window 2 Active", ppu.Window[1].ActiveLayers[1] != 0),

				new RegEntry("", "BG3 Windows", null),
				new RegEntry("$2124.0", "BG3 Window 1 Inverted", ppu.Window[0].InvertedLayers[2] != 0),
				new RegEntry("$2124.1", "BG3 Window 1 Active", ppu.Window[0].ActiveLayers[2] != 0),
				new RegEntry("$2124.2", "BG3 Window 2 Inverted", ppu.Window[1].InvertedLayers[2] != 0),
				new RegEntry("$2124.3", "BG3 Window 2 Active", ppu.Window[1].ActiveLayers[2] != 0),

				new RegEntry("", "BG4 Windows", null),
				new RegEntry("$2124.4", "BG4 Window 1 Inverted", ppu.Window[0].InvertedLayers[3] != 0),
				new RegEntry("$2124.5", "BG4 Window 1 Active", ppu.Window[0].ActiveLayers[3] != 0),
				new RegEntry("$2124.6", "BG4 Window 2 Inverted", ppu.Window[1].InvertedLayers[3] != 0),
				new RegEntry("$2124.7", "BG4 Window 2 Active", ppu.Window[1].ActiveLayers[3] != 0),

				new RegEntry("", "OAM Windows", null),
				new RegEntry("$2125.0", "OAM Window 1 Inverted", ppu.Window[0].InvertedLayers[4] != 0),
				new RegEntry("$2125.1", "OAM Window 1 Active", ppu.Window[0].ActiveLayers[4] != 0),
				new RegEntry("$2125.2", "OAM Window 2 Inverted", ppu.Window[1].InvertedLayers[4] != 0),
				new RegEntry("$2125.3", "OAM Window 2 Active", ppu.Window[1].ActiveLayers[4] != 0),

				new RegEntry("", "Color Windows", null),
				new RegEntry("$2125.4", "Color Window 1 Inverted", ppu.Window[0].InvertedLayers[5] != 0),
				new RegEntry("$2125.5", "Color Window 1 Active", ppu.Window[0].ActiveLayers[5] != 0),
				new RegEntry("$2125.6", "Color Window 2 Inverted", ppu.Window[1].InvertedLayers[5] != 0),
				new RegEntry("$2125.7", "Color Window 2 Active", ppu.Window[1].ActiveLayers[5] != 0),

				new RegEntry("", "Window Position", null),
				new RegEntry("$2126", "Window 1 Left", ppu.Window[0].Left),
				new RegEntry("$2127", "Window 1 Right", ppu.Window[0].Right),
				new RegEntry("$2128", "Window 2 Left", ppu.Window[1].Left),
				new RegEntry("$2129", "Window 2 Right", ppu.Window[1].Right),

				new RegEntry("", "Window Masks", null),
				new RegEntry("$212A.0-1", "BG1 Window Mask", ppu.MaskLogic[0].ToString().ToUpper()),
				new RegEntry("$212A.2-3", "BG2 Window Mask", ppu.MaskLogic[1].ToString().ToUpper()),
				new RegEntry("$212A.4-5", "BG3 Window Mask", ppu.MaskLogic[2].ToString().ToUpper()),
				new RegEntry("$212A.6-7", "BG4 Window Mask", ppu.MaskLogic[3].ToString().ToUpper()),
				new RegEntry("$212B.6-7", "OAM Window Mask", ppu.MaskLogic[4].ToString().ToUpper()),
				new RegEntry("$212B.6-7", "Color Window Mask", ppu.MaskLogic[5].ToString().ToUpper()),

				new RegEntry("$212C", "Main Screen Layers", null),
				new RegEntry("$212C.0", "BG1 Enabled", (ppu.MainScreenLayers & 0x01) != 0),
				new RegEntry("$212C.1", "BG2 Enabled", (ppu.MainScreenLayers & 0x02) != 0),
				new RegEntry("$212C.2", "BG3 Enabled", (ppu.MainScreenLayers & 0x04) != 0),
				new RegEntry("$212C.3", "BG4 Enabled", (ppu.MainScreenLayers & 0x08) != 0),
				new RegEntry("$212C.4", "OAM Enabled", (ppu.MainScreenLayers & 0x10) != 0),

				new RegEntry("$212D", "Sub Screen Layers", null),
				new RegEntry("$212D.0", "BG1 Enabled", (ppu.SubScreenLayers & 0x01) != 0),
				new RegEntry("$212D.1", "BG2 Enabled", (ppu.SubScreenLayers & 0x02) != 0),
				new RegEntry("$212D.2", "BG3 Enabled", (ppu.SubScreenLayers & 0x04) != 0),
				new RegEntry("$212D.3", "BG4 Enabled", (ppu.SubScreenLayers & 0x08) != 0),
				new RegEntry("$212D.4", "OAM Enabled", (ppu.SubScreenLayers & 0x10) != 0),

				new RegEntry("$212E", "Main Screen Windows", null),
				new RegEntry("$212E.0", "BG1 Mainscreen Window Enabled", ppu.WindowMaskMain[0] != 0),
				new RegEntry("$212E.1", "BG2 Mainscreen Window Enabled", ppu.WindowMaskMain[1] != 0),
				new RegEntry("$212E.2", "BG3 Mainscreen Window Enabled", ppu.WindowMaskMain[2] != 0),
				new RegEntry("$212E.3", "BG4 Mainscreen Window Enabled", ppu.WindowMaskMain[3] != 0),
				new RegEntry("$212E.4", "OAM Mainscreen Window Enabled", ppu.WindowMaskMain[4] != 0),

				new RegEntry("$212F", "Sub Screen Windows", null),
				new RegEntry("$212F.0", "BG1 Subscreen Window Enabled", ppu.WindowMaskSub[0] != 0),
				new RegEntry("$212F.1", "BG2 Subscreen Window Enabled", ppu.WindowMaskSub[1] != 0),
				new RegEntry("$212F.2", "BG3 Subscreen Window Enabled", ppu.WindowMaskSub[2] != 0),
				new RegEntry("$212F.3", "BG4 Subscreen Window Enabled", ppu.WindowMaskSub[3] != 0),
				new RegEntry("$212F.4", "OAM Subscreen Window Enabled", ppu.WindowMaskSub[4] != 0),

				new RegEntry("$2130 - $2131", "Color Math", null),
				new RegEntry("$2130.0", "Direct Color Mode", ppu.DirectColorMode),
				new RegEntry("$2130.1", "CM - Add Subscreen", ppu.ColorMathAddSubscreen),
				new RegEntry("$2130.4-5", "CM - Prevent Mode", ppu.ColorMathPreventMode.ToString()),
				new RegEntry("$2130.6-7", "CM - Clip Mode", ppu.ColorMathClipMode.ToString()),

				new RegEntry("$2131.0", "CM - BG1 Enabled", (ppu.ColorMathEnabled & 0x01) != 0),
				new RegEntry("$2131.1", "CM - BG2 Enabled", (ppu.ColorMathEnabled & 0x02) != 0),
				new RegEntry("$2131.2", "CM - BG3 Enabled", (ppu.ColorMathEnabled & 0x04) != 0),
				new RegEntry("$2131.3", "CM - BG4 Enabled", (ppu.ColorMathEnabled & 0x08) != 0),
				new RegEntry("$2131.4", "CM - OAM Enabled", (ppu.ColorMathEnabled & 0x10) != 0),
				new RegEntry("$2131.5", "CM - Background Enabled", (ppu.ColorMathEnabled & 0x20) != 0),
				new RegEntry("$2131.6", "CM - Half Mode", ppu.ColorMathHalveResult),
				new RegEntry("$2131.7", "CM - Substract Mode", ppu.ColorMathSubstractMode),

				new RegEntry("$2132 - $2133", "Misc.", null),
				new RegEntry("$2132", "Fixed Color - BGR", ppu.FixedColor, Format.X16),

				new RegEntry("$2133.0", "Screen Interlace", ppu.ScreenInterlace),
				new RegEntry("$2133.1", "OAM Interlace", ppu.ObjInterlace),
				new RegEntry("$2133.2", "Overscan Mode", ppu.OverscanMode),
				new RegEntry("$2133.3", "High Resolution Mode", ppu.HiResMode),
				new RegEntry("$2133.4", "Ext. BG Enabled", ppu.ExtBgEnabled),
			};

			return new RegisterViewerTab() {
				TabName = "PPU",
				Data = entries
			};
		}

		private RegisterViewerTab GetSnesDspTab(ref SnesState state)
		{
			DspState dsp = state.Dsp;
			List<RegEntry> entries = new List<RegEntry>();

			void AddReg(int i, string name)
			{
				entries.Add(new RegEntry("$" + i.ToString("X2"), name, dsp.Regs[i], Format.X8));
			}

			AddReg(0x0C, "Main Volume (MVOL) - Left");
			AddReg(0x1C, "Main Volume (MVOL) - Right");
			AddReg(0x2C, "Echo Volume (EVOL) - Left");
			AddReg(0x3C, "Echo Volume (EVOL) - Right");

			AddReg(0x4C, "Key On (KON)");
			AddReg(0x5C, "Key Off (KOF)");

			AddReg(0x7C, "Source End Block (ENDX)");
			AddReg(0x0D, "Echo Feedback (EFB)");
			AddReg(0x2D, "Pitch Modulation (PMON)");
			AddReg(0x3D, "Noise Enable (NON)");
			AddReg(0x4D, "Echo Enable (EON)");
			AddReg(0x5D, "Source Directory (Offset) (DIR)");
			AddReg(0x6D, "Echo Buffer (Offset) (ESA)");
			AddReg(0x7D, "Echo Delay (EDL)");

			entries.Add(new RegEntry("$6C", "Flags (FLG)", null));
			entries.Add(new RegEntry("$6C.0-4", "Noise Clock", dsp.Regs[0x6C] & 0x1F, Format.X8));
			entries.Add(new RegEntry("$6C.5", "Echo Disabled", (dsp.Regs[0x6C] & 0x20) != 0));
			entries.Add(new RegEntry("$6C.6", "Mute", (dsp.Regs[0x6C] & 0x40) != 0));
			entries.Add(new RegEntry("$6C.7", "Reset", (dsp.Regs[0x6C] & 0x80) != 0));

			entries.Add(new RegEntry("$xF", "Coefficients", null));
			for(int i = 0; i < 8; i++) {
				AddReg((i << 4) | 0x0F, "Coefficient " + i);
			}

			for(int i = 0; i < 8; i++) {
				entries.Add(new RegEntry("Voice #" + i.ToString(), "", null));

				int voice = i << 4;
				AddReg(voice | 0x00, "Left Volume (VOL)");
				AddReg(voice | 0x01, "Right Volume (VOL)");
				entries.Add(new RegEntry("$" + i + "2 + $" + i + "3", "Pitch (P)", dsp.Regs[voice | 0x02] | (dsp.Regs[voice | 0x03] << 8), Format.X16));
				AddReg(voice | 0x04, "Source (SRCN)");
				AddReg(voice | 0x05, "ADSR1");
				AddReg(voice | 0x06, "ADSR2");
				AddReg(voice | 0x07, "GAIN");
				AddReg(voice | 0x08, "ENVX");
				AddReg(voice | 0x09, "OUTX");
			}

			return new RegisterViewerTab() {
				TabName = "DSP",
				Data = entries
			};
		}

		private RegisterViewerTab GetSnesSpcTab(ref SnesState state)
		{
			string GetTimerFrequency(double baseFreq, int divider)
			{
				return (divider == 0 ? (baseFreq / 256) : (baseFreq / divider)).ToString(".00") + " Hz";
			}

			SpcState spc = state.Spc;
			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("$F0", "Test", null),
				new RegEntry("$F0.0", "Timers Disabled", spc.TimersDisabled),
				new RegEntry("$F0.1", "RAM Write Enabled", spc.WriteEnabled),
				new RegEntry("$F0.3", "Timers Enabled", spc.TimersEnabled),
				new RegEntry("$F0.4-5", "External Speed", spc.ExternalSpeed, Format.D),
				new RegEntry("$F0.6-7", "Internal Speed", spc.InternalSpeed, Format.D),

				new RegEntry("$F1", "Control", null),
				new RegEntry("$F1.0", "Timer 0 Enabled", spc.Timer0.Enabled),
				new RegEntry("$F1.1", "Timer 1 Enabled", spc.Timer1.Enabled),
				new RegEntry("$F1.2", "Timer 2 Enabled", spc.Timer2.Enabled),
				new RegEntry("$F1.7", "IPL ROM Enabled", spc.RomEnabled),

				new RegEntry("$F2", "DSP", null),
				new RegEntry("$F2", "DSP Register", spc.DspReg, Format.X8),

				new RegEntry("$F4 - $F7", "CPU<->SPC Ports", null),
				new RegEntry("$F4", "Port 0 (CPU read)", spc.OutputReg[0], Format.X8),
				new RegEntry("$F4", "Port 0 (SPC read)", spc.CpuRegs[0], Format.X8),
				new RegEntry("$F5", "Port 1 (CPU read)", spc.OutputReg[1], Format.X8),
				new RegEntry("$F5", "Port 1 (SPC read)", spc.CpuRegs[1], Format.X8),
				new RegEntry("$F6", "Port 2 (CPU read)", spc.OutputReg[2], Format.X8),
				new RegEntry("$F6", "Port 2 (SPC read)", spc.CpuRegs[2], Format.X8),
				new RegEntry("$F7", "Port 3 (CPU read)", spc.OutputReg[3], Format.X8),
				new RegEntry("$F7", "Port 3 (SPC read)", spc.CpuRegs[3], Format.X8),

				new RegEntry("$F8 - $F9", "RAM Registers", null),
				new RegEntry("$F8", "RAM Reg 0", spc.RamReg[0], Format.X8),
				new RegEntry("$F9", "RAM Reg 1", spc.RamReg[1], Format.X8),

				new RegEntry("$FA - $FF", "Timers", null),
				new RegEntry("$FA", "Timer 0 Divider", spc.Timer0.Target, Format.X8),
				new RegEntry("$FA", "Timer 0 Frequency", GetTimerFrequency(8000, spc.Timer0.Target)),
				new RegEntry("$FB", "Timer 1 Divider", spc.Timer1.Target, Format.X8),
				new RegEntry("$FB", "Timer 1 Frequency", GetTimerFrequency(8000, spc.Timer1.Target)),
				new RegEntry("$FC", "Timer 2 Divider", spc.Timer2.Target, Format.X8),
				new RegEntry("$FC", "Timer 2 Frequency", GetTimerFrequency(64000, spc.Timer2.Target)),

				new RegEntry("$FD", "Timer 0 Output", spc.Timer0.Output, Format.X8),
				new RegEntry("$FE", "Timer 1 Output", spc.Timer1.Output, Format.X8),
				new RegEntry("$FF", "Timer 2 Output", spc.Timer2.Output, Format.X8),
			};

			return new RegisterViewerTab() {
				TabName = "SPC",
				Data = entries
			};
		}

		private RegisterViewerTab GetSnesDmaTab(ref SnesState state)
		{
			List<RegEntry> entries = new List<RegEntry>();

			//TODO
			/*for(int i = 0; i < 8; i++) {
				entries.Add(new RegEntry("$420C." + i.ToString(), "HDMA Channel " + i.ToString() + " Enabled", _state.DmaChannels[i].DmaActive));
			}*/

			for(int i = 0; i < 8; i++) {
				DmaChannelConfig ch = state.DmaChannels[i];
				entries.Add(new RegEntry("DMA Channel " + i.ToString(), "", null));
				entries.Add(new RegEntry("$420B." + i.ToString(), "Channel Enabled", ch.DmaActive));
				entries.Add(new RegEntry("$43" + i.ToString() + "0.0-2", "Transfer Mode", ch.TransferMode, Format.D));
				entries.Add(new RegEntry("$43" + i.ToString() + "0.3", "Fixed", ch.FixedTransfer));
				entries.Add(new RegEntry("$43" + i.ToString() + "0.4", "Decrement", ch.Decrement));
				entries.Add(new RegEntry("$43" + i.ToString() + "0.6", "Indirect HDMA", ch.HdmaIndirectAddressing));
				entries.Add(new RegEntry("$43" + i.ToString() + "0.7", "Direction", ch.InvertDirection ? "B -> A" : "A -> B"));

				entries.Add(new RegEntry("$43" + i.ToString() + "1", "B Bus Address", ch.DestAddress, Format.X8));
				entries.Add(new RegEntry("$43" + i.ToString() + "2/3/4", "A Bus Address", ((ch.SrcBank << 16) | ch.SrcAddress), Format.X24));
				entries.Add(new RegEntry("$43" + i.ToString() + "5/6", "Size", ch.TransferSize, Format.X16));
				entries.Add(new RegEntry("$43" + i.ToString() + "7", "HDMA Bank", ch.HdmaBank, Format.X8));
				entries.Add(new RegEntry("$43" + i.ToString() + "8/9", "HDMA Address", ch.HdmaTableAddress, Format.X16));
				entries.Add(new RegEntry("$43" + i.ToString() + "A", "HDMA Line Counter", ch.HdmaLineCounterAndRepeat, Format.X8));
				entries.Add(new RegEntry("$43" + i.ToString() + "B", "Unused register", ch.UnusedRegister, Format.X8));
			}

			return new RegisterViewerTab() {
				TabName = "DMA",
				Data = entries
			};
		}

		private RegisterViewerTab GetSnesCpuTab(ref SnesState state)
		{
			InternalRegisterState regs = state.InternalRegs;
			AluState alu = state.Alu;
			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("$4200 - $4201", "IRQ/NMI/Autopoll Enabled", null),
				new RegEntry("$4200.7", "NMI Enabled", regs.EnableNmi),
				new RegEntry("$4200.5", "V IRQ Enabled", regs.EnableVerticalIrq),
				new RegEntry("$4200.4", "H IRQ Enabled", regs.EnableHorizontalIrq),
				new RegEntry("$4200.0", "Auto Joypad Poll", regs.EnableAutoJoypadRead),

				new RegEntry("$4201", "IO Port", regs.IoPortOutput, Format.X8),

				new RegEntry("$4202 - $4206", "Mult/Div Registers (Input)", null),
				new RegEntry("$4202", "Multiplicand", alu.MultOperand1, Format.X8),
				new RegEntry("$4203", "Multiplier", alu.MultOperand2, Format.X8),
				new RegEntry("$4204/5", "Dividend", alu.Dividend, Format.X16),
				new RegEntry("$4206", "Divisor", alu.Divisor, Format.X8),

				new RegEntry("$4207 - $420A", "H/V IRQ Timers", null),
				new RegEntry("$4207/8", "H Timer", regs.HorizontalTimer, Format.X16),
				new RegEntry("$4209/A", "V Timer", regs.VerticalTimer, Format.X16),

				new RegEntry("$4207 - $420A", "Misc. Flags", null),

				new RegEntry("$420D", "FastROM Enabled", regs.EnableFastRom),
				//TODO
				/*new RegEntry("$4210", "NMI Flag", (_reg4210 & 0x80) != 0),
				new RegEntry("$4211", "IRQ Flag", (_reg4211 & 0x80) != 0),

				new RegEntry("$4212.x", "V-Blank Flag", (_reg4212 & 0x80) != 0),
				new RegEntry("$4212.x", "H-Blank Flag", (_reg4212 & 0x40) != 0),
				new RegEntry("$4212.x", "Auto Joypad Read", (_reg4212 & 0x01) != 0),*/

				new RegEntry("$4214 - $4217", "Mult/Div Registers (Result)", null),
				new RegEntry("$4214/5", "Quotient", alu.DivResult, Format.X16),
				new RegEntry("$4216/7", "Product / Remainder", alu.MultOrRemainderResult, Format.X16),

				new RegEntry("$4218 - $421F", "Input Data", null),
				new RegEntry("$4218/9", "P1 Data", regs.ControllerData[0], Format.X16),
				new RegEntry("$421A/B", "P2 Data", regs.ControllerData[1], Format.X16),
				new RegEntry("$421C/D", "P3 Data", regs.ControllerData[2], Format.X16),
				new RegEntry("$421E/F", "P4 Data", regs.ControllerData[3], Format.X16),
			};

			return new RegisterViewerTab() {
				TabName = "CPU",
				Data = entries
			};
		}

		private RegisterViewerTab GetNesPpuTab(ref NesState state)
		{
			NesPpuState ppu = state.Ppu;

			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("", "State", null),
				new RegEntry("", "Cycle (H)", ppu.Cycle),
				new RegEntry("", "Scanline (V)", ppu.Scanline),
				new RegEntry("", "Frame Number", ppu.FrameCount),
				new RegEntry("", "PPU Bus Address", ppu.BusAddress, Format.X16),
				new RegEntry("", "PPU Register Buffer", ppu.MemoryReadBuffer, Format.X8),

				new RegEntry("$2000", "Control", null),
				//new RegEntry("$2000.0-1", "Nametable Select", ppu.ControlReg & 0x03),
				new RegEntry("$2000.2", "Increment Mode", ppu.Control.VerticalWrite ? "32 bytes" : "1 byte"),
				new RegEntry("$2000.3", "Sprite Table Address", ppu.Control.SpritePatternAddr == 0 ? "$0000" : "$1000"),
				new RegEntry("$2000.4", "BG Table Address", ppu.Control.BackgroundPatternAddr == 0 ? "$0000" : "$1000"),
				new RegEntry("$2000.5", "Sprite Size", ppu.Control.LargeSprites ? "8x16" : "8x8"),
				new RegEntry("$2000.6", "Main/secondary PPU select", ppu.Control.SecondaryPpu ? "Secondary" : "Main"),
				new RegEntry("$2000.7", "NMI enabled", ppu.Control.NmiOnVerticalBlank),

				new RegEntry("$2001", "Mask", null),
				new RegEntry("$2001.0", "Grayscale", ppu.Mask.Grayscale),
				new RegEntry("$2001.1", "BG - Show leftmost 8 pixels", ppu.Mask.BackgroundMask),
				new RegEntry("$2001.2", "Sprites - Show leftmost 8 pixels", ppu.Mask.SpriteMask),
				new RegEntry("$2001.3", "Background enabled", ppu.Mask.BackgroundEnabled),
				new RegEntry("$2001.4", "Sprites enabled", ppu.Mask.SpritesEnabled),
				new RegEntry("$2001.5", "Red emphasis", ppu.Mask.IntensifyRed),
				new RegEntry("$2001.6", "Green emphasis", ppu.Mask.IntensifyGreen),
				new RegEntry("$2001.7", "Blue emphasis", ppu.Mask.IntensifyBlue),

				new RegEntry("$2002", "Status", null),
				new RegEntry("$2002.5", "Sprite overflow", ppu.StatusFlags.SpriteOverflow),
				new RegEntry("$2002.6", "Sprite 0 hit", ppu.StatusFlags.Sprite0Hit),
				new RegEntry("$2002.7", "Vertical blank", ppu.StatusFlags.VerticalBlank),
				
				new RegEntry("$2003", "OAM address", ppu.SpriteRamAddr, Format.X8),
				
				new RegEntry("$2005-2006", "VRAM Address / Scrolling", null),
				new RegEntry("", "VRAM Address", ppu.VideoRamAddr, Format.X16),
				new RegEntry("", "T", ppu.TmpVideoRamAddr, Format.X16),
				new RegEntry("", "X Scroll", ppu.ScrollX),
				new RegEntry("", "Write Toggle", ppu.WriteToggle)
			};

			return new RegisterViewerTab() {
				TabName = "PPU",
				Data = entries
			};
		}

		private RegisterViewerTab GetNesApuTab(ref NesState state)
		{
			List<RegEntry> entries = new List<RegEntry>();
			NesApuState apu = state.Apu;
			
			NesApuSquareState sq1 = apu.Square1;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("$4000-$4003", "Square 1", null),
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
				new RegEntry("--", "Frequency", Math.Round(sq1.Frequency).ToString("0.") + " Hz"),
				new RegEntry("--", "Duty Position", sq1.DutyPosition),

				new RegEntry("--", "Length Counter - Counter", sq1.LengthCounter.Counter, Format.X8),
				
				new RegEntry("--", "Envelope - Counter", sq1.Envelope.Counter, Format.X8),
				new RegEntry("--", "Envelope - Divider", sq1.Envelope.Divider, Format.X8),

				new RegEntry("--", "Output", sq1.OutputVolume, Format.X8),
			});

			NesApuSquareState sq2 = apu.Square2;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("$4004-$4007", "Square 2", null),
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
				new RegEntry("--", "Frequency", Math.Round(sq2.Frequency).ToString("0.") + " Hz"),
				new RegEntry("--", "Duty Position", sq2.DutyPosition),

				new RegEntry("--", "Length Counter - Counter", sq2.LengthCounter.Counter, Format.X8),

				new RegEntry("--", "Envelope - Counter", sq2.Envelope.Counter, Format.X8),
				new RegEntry("--", "Envelope - Divider", sq2.Envelope.Divider, Format.X8),

				new RegEntry("--", "Output", sq2.OutputVolume, Format.X8),
			});

			NesApuTriangleState trg = apu.Triangle;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("$4008-$400B", "Triangle", null),
				new RegEntry("$4008.0-6", "Linear Counter - Reload", trg.LinearCounterReload, Format.X8),
				new RegEntry("$4008.1", "Length Counter - Halted", trg.LengthCounter.Halt),

				new RegEntry("$400A/$400B.0-2", "Period", trg.Period, Format.X16),
				new RegEntry("$400B.3-7", "Length Counter - Reload Value", trg.LengthCounter.ReloadValue, Format.X16),

				new RegEntry("--", "Enabled", trg.Enabled),
				new RegEntry("--", "Timer", trg.Timer),
				new RegEntry("--", "Frequency", Math.Round(trg.Frequency).ToString("0.") + " Hz"),
				new RegEntry("--", "Sequence Position", trg.SequencePosition),

				new RegEntry("--", "Length Counter - Counter", trg.LengthCounter.Counter),
				
				new RegEntry("--", "Linear Counter - Counter", trg.LinearCounter),
				new RegEntry("--", "Linear Counter - Reload Flag", trg.LinearReloadFlag),

				new RegEntry("--", "Output", trg.OutputVolume),
			});

			NesApuNoiseState noise = apu.Noise;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("$400C-$400F", "Triangle", null),
				new RegEntry("$400C.0-3", "Envelope Volume", noise.Envelope.Volume, Format.X8),
				new RegEntry("$400C.4", "Envelope - Constant Volume", noise.Envelope.ConstantVolume),
				new RegEntry("$400C.5", "Length Counter - Halted", noise.LengthCounter.Halt),

				new RegEntry("$400E.0-3", "Period", noise.Period, Format.X16),
				new RegEntry("$400E.7", "Mode Flag", noise.ModeFlag),
				
				new RegEntry("$400F.3-7", "Length Counter - Reload Value", noise.LengthCounter.ReloadValue, Format.X8),

				new RegEntry("--", "Enabled", noise.Enabled),
				new RegEntry("--", "Timer", noise.Timer),
				new RegEntry("--", "Frequency", Math.Round(noise.Frequency).ToString("0.") + " Hz"),
				new RegEntry("--", "Shift Register", noise.ShiftRegister),

				new RegEntry("--", "Envelope - Counter", noise.Envelope.Counter, Format.X8),
				new RegEntry("--", "Envelope - Divider", noise.Envelope.Divider, Format.X8),

				new RegEntry("--", "Length Counter - Counter", noise.LengthCounter.Counter),

				new RegEntry("--", "Output", noise.OutputVolume),
			});

			NesApuDmcState dmc = apu.Dmc;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("$4010-4013", "DMC", null),
				new RegEntry("$4010.0-3", "Period", dmc.Period, Format.X16),
				new RegEntry("$4010.6", "Loop Flag", dmc.Loop),
				new RegEntry("$4010.7", "IRQ Enabled", dmc.IrqEnabled),

				new RegEntry("$4011", "Output Level", dmc.OutputVolume),
				
				new RegEntry("$4012", "Sample Address", dmc.SampleAddr, Format.X16),
				new RegEntry("$4013", "Sample Length", dmc.SampleLength, Format.X16),

				new RegEntry("--", "Timer", dmc.Timer),
				new RegEntry("--", "Frequency", Math.Round(dmc.SampleRate).ToString("0.")),
				new RegEntry("--", "Bytes Remaining", dmc.BytesRemaining),
			});

			NesApuFrameCounterState frameCounter = apu.FrameCounter;
			entries.AddRange(new List<RegEntry>() {
				new RegEntry("$4017", "Frame Counter", null),
				new RegEntry("$4017.6", "IRQ Enabled", frameCounter.IrqEnabled),
				new RegEntry("$4017.7", "5-step Mode", frameCounter.FiveStepMode),
				new RegEntry("--", "Sequence Position", frameCounter.SequencePosition),
			});

			return new RegisterViewerTab() {
				TabName = "APU",
				Data = entries
			};
		}

		private RegisterViewerTab GetNesCartTab(ref NesState state)
		{
			NesCartridgeState cart = state.Cartridge;

			List<RegEntry> entries = new List<RegEntry>();
			for(int i = 0; i < cart.CustomEntryCount; i++) {
				Format format = cart.CustomEntries[i].Type switch {
					MapperStateValueType.Number8 => Format.X8,
					MapperStateValueType.Number16 => Format.X16,
					MapperStateValueType.Number32 => Format.X32,
					_ => Format.None
				};

				entries.Add(new RegEntry(
					cart.CustomEntries[i].GetAddress(),
					cart.CustomEntries[i].GetName(),
					cart.CustomEntries[i].GetValue(),
					format
				));
			}

			return new RegisterViewerTab() {
				TabName = "Cart",
				Data = entries
			};
		}

		private RegisterViewerTab GetPcePpuTab(ref PceState state)
		{
			PcePpuState ppu = state.Ppu;

			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("", "State", null),
				new RegEntry("", "Cycle (H)", ppu.Cycle, Format.X16),
				new RegEntry("", "Scanline (V)", ppu.Scanline, Format.X16),
				new RegEntry("", "Frame Number", ppu.FrameCount),

				new RegEntry("$00", "MAWR - Memory Write Address", ppu.MemAddrWrite, Format.X16),
				new RegEntry("$01", "MARR - Memory Read Address", ppu.MemAddrRead, Format.X16),
				new RegEntry("$02", "VWR - VRAM Write Data", ppu.VramData, Format.X16),

				new RegEntry("$05", "CR - Control", null),
				new RegEntry("$05.0", "Sprite 0 Hit IRQ Enabled", ppu.EnableCollisionIrq),
				new RegEntry("$05.1", "Overflow IRQ Enabled", ppu.EnableOverflowIrq),
				new RegEntry("$05.2", "Scanline Detect (RCR) IRQ Enabled", ppu.EnableScanlineIrq),
				new RegEntry("$05.3", "Vertical Blank IRQ Enabled", ppu.EnableVerticalBlankIrq),
				new RegEntry("$05.6", "Sprites Enabled", ppu.NextSpritesEnabled),
				new RegEntry("$05.7", "Background Enabled", ppu.NextBackgroundEnabled),
				new RegEntry("$05.11-12", "VRAM Address Increment", ppu.VramAddrIncrement),

				new RegEntry("$06", "RCR - Raster Compare Register", ppu.RasterCompareRegister, Format.X16),
				new RegEntry("$07", "BXR - BG Scroll X", ppu.BgScrollX, Format.X16),
				new RegEntry("$08", "BYR - BG Scroll Y", ppu.BgScrollY, Format.X16),
				
				new RegEntry("$09", "MWR - Memory Width", null),
				new RegEntry("$09.0-1", "VRAM Access Mode", ppu.VramAccessMode),
				new RegEntry("$09.2-3", "Sprite Access Mode", ppu.SpriteAccessMode),
				new RegEntry("$09.4-5", "Column Count", ppu.ColumnCount),
				new RegEntry("$09.6", "Row Count", ppu.RowCount),
				new RegEntry("$09.7", "CG Mode", ppu.CgMode),

				new RegEntry("$0A", "HSR - Horizontal Sync", null),
				new RegEntry("$0A.0-4", "HSW - Horizontal Sync Width", ppu.HorizSyncWidth, Format.X8),
				new RegEntry("$0A.8-14", "HDS - Horizontal Display Start Position", ppu.HorizDisplayStart, Format.X8),
				
				new RegEntry("$0B", "HDR - Horizontal Display", null),
				new RegEntry("$0B.0-6", "HDW - Horizontal Display Width", ppu.HorizDisplayWidth, Format.X8),
				new RegEntry("$0B.8-14", "HDE - Horizontal Display End Position", ppu.HorizDisplayEnd, Format.X8),

				new RegEntry("$0C", "VPR - Vertical Sync", null),
				new RegEntry("$0C.0-4", "VSW - Vertical Sync Width", ppu.VertSyncWidth, Format.X8),
				new RegEntry("$0C.8-15", "VDS - Vertical Display Start Position", ppu.VertDisplayStart, Format.X8),

				new RegEntry("$0D", "VDR - Vertical Display", null),
				new RegEntry("$0D.0-8", "VDW - Vertical Display Width", ppu.VertDisplayWidth, Format.X16),

				new RegEntry("$0E", "VCR - Vertical Display End", null),
				new RegEntry("$0E.0-7", "HDE - Vertical Display End Position", ppu.HorizDisplayEnd, Format.X8),

				new RegEntry("$0F", "DCR - Block Transfer Control", null),
				new RegEntry("$0F.0", "VRAM-SATB Transfer Complete IRQ Enabled", ppu.VramSatbIrqEnabled),
				new RegEntry("$0F.1", "VRAM-VRAM Transfer Complete IRQ Enabled", ppu.VramVramIrqEnabled),
				new RegEntry("$0F.2", "Decrement Source Address", ppu.DecrementSrc),
				new RegEntry("$0F.3", "Decrement Destination Address", ppu.DecrementDst),
				new RegEntry("$0F.4", "VRAM-SATB Transfer Auto-Repeat", ppu.RepeatSatbTransfer),
				
				new RegEntry("$10", "SOUR - Block Transfer Source Address", ppu.BlockSrc, Format.X16),
				new RegEntry("$11", "DESR - Block Transfer Source Address", ppu.BlockDst, Format.X16),
				new RegEntry("$12", "LENR - Block Transfer Source Address", ppu.BlockLen, Format.X16),
				new RegEntry("$13", "DVSSR - VRAM-SATB Transfer Source Address", ppu.BgScrollX, Format.X16)
			};

			return new RegisterViewerTab() {
				TabName = "VDC",
				Data = entries
			};
		}

		private RegisterViewerTab GetPceCpuTab(ref PceState state)
		{
			PceMemoryManager mem = state.MemoryManager;

			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("", "IRQ", null),
				new RegEntry("$1402", "Disabled IRQs", null),
				new RegEntry("$1402.0", "IRQ2 Disabled", (mem.DisabledIrqs & 0x01) != 0),
				new RegEntry("$1402.1", "IRQ1 (VDC) Disabled", (mem.DisabledIrqs & 0x02) != 0),
				new RegEntry("$1402.2", "Timer IRQ Disabled", (mem.DisabledIrqs & 0x04) != 0),
				new RegEntry("$1403", "Active IRQs", null),
				new RegEntry("$1403.0", "IRQ2", (mem.ActiveIrqs & 0x01) != 0),
				new RegEntry("$1403.1", "IRQ1 (VDC)", (mem.ActiveIrqs & 0x02) != 0),
				new RegEntry("$1403.2", "Timer IRQ", (mem.ActiveIrqs & 0x04) != 0),

				new RegEntry("", "MPR", null),
				new RegEntry("", "MPR #0", mem.Mpr[0], Format.X8),
				new RegEntry("", "MPR #1", mem.Mpr[1], Format.X8),
				new RegEntry("", "MPR #2", mem.Mpr[2], Format.X8),
				new RegEntry("", "MPR #3", mem.Mpr[3], Format.X8),
				new RegEntry("", "MPR #4", mem.Mpr[4], Format.X8),
				new RegEntry("", "MPR #5", mem.Mpr[5], Format.X8),
				new RegEntry("", "MPR #6", mem.Mpr[6], Format.X8),
				new RegEntry("", "MPR #7", mem.Mpr[7], Format.X8),
			};

			return new RegisterViewerTab() {
				TabName = "CPU",
				Data = entries
			};
		}

		private RegisterViewerTab GetPcePsgTab(ref PceState pceState)
		{
			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("$800.0-2", "Channel Select", pceState.Psg.ChannelSelect, Format.X8),
				new RegEntry("$801.0-3", "Right Amplitude", pceState.Psg.RightVolume, Format.X8),
				new RegEntry("$801.4-7", "Left Amplitude", pceState.Psg.LeftVolume, Format.X8),
				new RegEntry("$807.4-7", "LFO Frequency", pceState.Psg.LfoFrequency, Format.X8),
				new RegEntry("$808", "LFO Control", pceState.Psg.LfoControl, Format.X8),
			};

			for(int i = 0; i < 6; i++) {
				ref PcePsgChannelState ch = ref pceState.PsgChannels[i];

				entries.AddRange(new List<RegEntry>() {
					new RegEntry("", "Channel " + (i + 1), null),
					new RegEntry("$802-$803", "Frequency", ch.Frequency, Format.X16),
					new RegEntry("$804.0-4", "Amplitude", ch.Amplitude, Format.X8),
					new RegEntry("$804.6", "DDA Enabled", ch.DdaEnabled),
					new RegEntry("$804.7", "Channel Enabled", ch.Enabled),
					new RegEntry("$805.0-3", "Right Amplitude", ch.RightVolume, Format.X8),
					new RegEntry("$805.4-7", "Left Amplitude", ch.LeftVolume, Format.X8),
				});

				if(i >= 4) {
					entries.Add(new RegEntry("$806.7", "Noise Enabled", ch.NoiseEnabled));
					entries.Add(new RegEntry("$806.0-4", "Noise Frequency", ch.NoiseFrequency, Format.X8));
				}
			}

			return new RegisterViewerTab() {
				TabName = "PSG",
				Data = entries
			};
		}
	}

	public class RegisterViewerTab : ReactiveObject
	{
		[Reactive] public string TabName { get; set; } = "";
		[Reactive] public List<RegEntry> Data { get; set; } = new List<RegEntry>();
		
		public RegisterViewerTab()
		{
		}

		public void SetData(List<RegEntry> rows)
		{
			if(Data.Count != rows.Count) {
				Data = rows;
			} else {
				for(int i = 0; i < rows.Count; i++) {
					Data[i].Value = rows[i].Value;
					Data[i].ValueHex = rows[i].ValueHex;
				}
			}
		}
	}

	public class RegEntry : ReactiveObject
	{
		private static ISolidColorBrush HeaderBgBrush = new SolidColorBrush(0xFFF0F0F0);

		public string Address { get; set; } = "";
		public string Name { get; set; } = "";
		public ISolidColorBrush Background { get; set; }

		[Reactive] public string Value { get; set; } = "";
		[Reactive] public string ValueHex { get; set; } = "";

		public RegEntry(string reg, string name, object? value, Format format = Format.None)
		{
			Address = reg;
			Name = name;

			Value = GetValue(value);
			ValueHex = GetHexValue(value, format);
			Background = value == null ? RegEntry.HeaderBgBrush: Brushes.Transparent;
		}

		public string GetValue(object? value)
		{
			if(value is string str) {
				return str;
			} else if(value is bool) {
				return (bool)value ? "☑ true" : "☐ false";
			} else if(value is IFormattable formattable) {
				return formattable.ToString() ?? "";
			} else if(value == null) {
				return "";
			}
			throw new Exception("Unsupported type");
		}

		public string GetHexValue(object? value, Format format)
		{
			if(value == null) {
				return "";
			}

			switch(format) {
				default: return "";
				case Format.X8: return "$" + ((IFormattable)value).ToString("X2", null);
				case Format.X16: return "$" + ((IFormattable)value).ToString("X4", null);
				case Format.X24: return "$" + ((IFormattable)value).ToString("X6", null);
				case Format.X32: return "$" + ((IFormattable)value).ToString("X8", null);
			}
		}

		public enum Format
		{
			None,
			D,
			X8,
			X16,
			X24,
			X32
		}
	}
}
