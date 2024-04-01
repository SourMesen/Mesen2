using Avalonia.Controls;
using Avalonia.Controls.Selection;
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
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
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
		private byte _snesReg4210;
		private byte _snesReg4211;
		private byte _snesReg4212;

		public RegisterViewerWindowViewModel()
		{
			Config = ConfigManager.Config.Debug.RegisterViewer.Clone();
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming, CpuType);

			if(Design.IsDesignMode) {
				return;
			}

			UpdateRomInfo();
			RefreshTiming.UpdateMinMaxValues(CpuType);
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
				_snesReg4210 = DebugApi.GetMemoryValue(MemoryType.SnesMemory, 0x4210);
				_snesReg4211 = DebugApi.GetMemoryValue(MemoryType.SnesMemory, 0x4211);
				_snesReg4212 = DebugApi.GetMemoryValue(MemoryType.SnesMemory, 0x4212);
				_state = DebugApi.GetConsoleState<SnesState>(ConsoleType.Snes);
			} else if(_romInfo.ConsoleType == ConsoleType.Nes) {
				_state = DebugApi.GetConsoleState<NesState>(ConsoleType.Nes);
			} else if(_romInfo.ConsoleType == ConsoleType.Gameboy) {
				_state = DebugApi.GetConsoleState<GbState>(ConsoleType.Gameboy);
			} else if(_romInfo.ConsoleType == ConsoleType.PcEngine) {
				_state = DebugApi.GetConsoleState<PceState>(ConsoleType.PcEngine);
			} else if(_romInfo.ConsoleType == ConsoleType.Sms) {
				_state = DebugApi.GetConsoleState<SmsState>(ConsoleType.Sms);
			} else if(_romInfo.ConsoleType == ConsoleType.Gba) {
				_state = DebugApi.GetConsoleState<GbaState>(ConsoleType.Gba);
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
				} else if(cpuTypes.Contains(CpuType.Gameboy)) {
					GbState gbState = DebugApi.GetConsoleState<GbState>(ConsoleType.Gameboy);
					string tabPrefix = "GB - ";
					tabs.Add(GetGbLcdTab(ref gbState, tabPrefix));
					tabs.Add(GetGbApuTab(ref gbState, tabPrefix));
					tabs.Add(GetGbMiscTab(ref gbState, tabPrefix));
				} else if(cpuTypes.Contains(CpuType.Gsu)) {
					tabs.Add(GetSnesGsuTab(ref snesState.Gsu));
				}
			} else if(lastState is NesState nesState) {
				tabs = new List<RegisterViewerTab>() {
					GetNesPpuTab(ref nesState),
					GetNesApuTab(ref nesState)
				};

				RegisterViewerTab cartTab = GetNesCartTab(ref nesState);
				if(cartTab.Data.Count > 0) {
					tabs.Add(cartTab);
				}
			} else if(lastState is GbState gbState) {
				tabs = new List<RegisterViewerTab>() {
					GetGbLcdTab(ref gbState),
					GetGbApuTab(ref gbState),
					GetGbMiscTab(ref gbState),
				};
				if(gbState.Type == GbType.Cgb) {
					tabs.Add(GetGbCgbTab(ref gbState));
				}
			} else if(lastState is PceState pceState) {
				tabs = new List<RegisterViewerTab>() {
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
			} else if(lastState is SmsState smsState) {
				tabs = new List<RegisterViewerTab>() {
					GetSmsVdpTab(ref smsState),
					GetSmsPsgTab(ref smsState),
					GetSmsMiscTab(ref smsState),
				};
			} else if(lastState is GbaState gbaState) {
				tabs = new List<RegisterViewerTab>() {
					GetGbaPpuTab(ref gbaState),
					GetGbaApuTab(ref gbaState),
					GetGbaDmaTab(ref gbaState),
					GetGbaTimerTab(ref gbaState),
					GetGbaMiscTab(ref gbaState),
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

		private RegisterViewerTab GetGbaMiscTab(ref GbaState gbaState)
		{
			List<RegEntry> entries = new List<RegEntry>();

			GbaMemoryManagerState memManager = gbaState.MemoryManager;
			entries.AddRange(new List<RegEntry>() {
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
				new RegEntry("$4000204.2", "Bank $C/D - Sequential", memManager.PrgWaitStates2[1] + " clocks", null),
				new RegEntry("$4000204.3", "Prefetch Enabled", memManager.PrefetchEnabled),
			});

			return new RegisterViewerTab("Misc", entries, Config, CpuType.Gba, MemoryType.GbaMemory);

		}

		private RegisterViewerTab GetGbaPpuTab(ref GbaState gbaState)
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

			return new RegisterViewerTab("PPU", entries, Config, CpuType.Gba, MemoryType.GbaMemory);
		}

		private RegisterViewerTab GetGbaApuTab(ref GbaState gbaState)
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

			return new RegisterViewerTab("APU", entries, Config, CpuType.Gba, MemoryType.GbaMemory);
		}

		private RegisterViewerTab GetGbaTimerTab(ref GbaState gbaState)
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

			return new RegisterViewerTab("Timers", entries, Config, CpuType.Gba, MemoryType.GbaMemory);
		}

		private RegisterViewerTab GetGbaDmaTab(ref GbaState gbaState)
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

			return new RegisterViewerTab("DMA", entries, Config, CpuType.Gba, MemoryType.GbaMemory);
		}

		private RegisterViewerTab GetGbLcdTab(ref GbState gb, string tabPrefix = "")
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
				new RegEntry("$FF40.1", "Sprites Enabled", ppu.BgEnabled),
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

			return new RegisterViewerTab(tabPrefix + "LCD", entries, Config, CpuType.Gameboy, MemoryType.GameboyMemory);
		}

		private RegisterViewerTab GetGbCgbTab(ref GbState gb)
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

			return new RegisterViewerTab("CGB", entries, Config, CpuType.Gameboy, MemoryType.GameboyMemory);
		}

		private RegisterViewerTab GetGbMiscTab(ref GbState gb, string tabPrefix = "")
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

			return new RegisterViewerTab(tabPrefix + "Timer/DMA/IRQ", entries, Config, CpuType.Gameboy, MemoryType.GameboyMemory);
		}

		private RegisterViewerTab GetGbApuTab(ref GbState gb, string tabPrefix = "")
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

			return new RegisterViewerTab(tabPrefix + "APU", entries, Config, CpuType.Gameboy, MemoryType.GameboyMemory);
		}

		private RegisterViewerTab GetSnesGsuTab(ref GsuState gsu)
		{
			List<RegEntry> entries = new List<RegEntry>() {
				//new RegEntry("$3033.0", "Backup RAM Enabled", gsu.BackupRamEnabled),
				new RegEntry("", "Registers"),
				new RegEntry("$3037.5", "High Speed Mode", gsu.HighSpeedMode),
				new RegEntry("$3037.7", "IRQ Disabled", gsu.IrqDisabled),
				new RegEntry("$3038", "Screen Base Address", gsu.ScreenBase, Format.X8),
				new RegEntry("$3039.0", "Clock Select", gsu.ClockSelect),
				new RegEntry("$303A.0-1", "Color Gradient", gsu.PlotBpp + " BPP", gsu.ColorGradient),
				new RegEntry("$303A.2+5", "Screen Height", gsu.ScreenHeight switch {
					0 => "128 px",
					1 => "160 px",
					2 => "192 px",
					3 or _ => "OBJ mode",
				}, gsu.ScreenHeight),
				new RegEntry("$303A.3", "GSU RAM Access Enabled", gsu.GsuRamAccess),
				new RegEntry("$303A.4", "GSU ROM Access Enabled", gsu.GsuRomAccess),
				
				new RegEntry("", "Plot Option Register (CMODE)"),
				new RegEntry("", "Transparent", gsu.PlotTransparent),
				new RegEntry("", "Dither", gsu.PlotDither),
				new RegEntry("", "Color High Nibble", gsu.ColorHighNibble),
				new RegEntry("", "Color Freeze High", gsu.ColorFreezeHigh),
				new RegEntry("", "Object Mode", gsu.ObjMode),
				new RegEntry("", "Transparent", gsu.PlotTransparent),
			};

			return new RegisterViewerTab("GSU", entries, Config);
		}

		private RegisterViewerTab GetSnesSa1Tab(ref SnesState state)
		{
			Sa1State sa1 = state.Sa1.Sa1;

			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("$2200", "SA-1 CPU Control"),
				new RegEntry("$2200.0-3", "Message", sa1.Sa1MessageReceived, Format.X8),
				new RegEntry("$2200.4", "NMI Requested", sa1.Sa1NmiRequested),
				new RegEntry("$2200.5", "Reset", sa1.Sa1Reset),
				new RegEntry("$2200.6", "Wait", sa1.Sa1Wait),
				new RegEntry("$2200.7", "IRQ Requested", sa1.Sa1IrqRequested),

				new RegEntry("$2201", "S-CPU Interrupt Enable"),
				new RegEntry("$2201.5", "Character Conversion IRQ Enable", sa1.CharConvIrqEnabled),
				new RegEntry("$2201.7", "IRQ Enabled", sa1.CpuIrqEnabled),

				new RegEntry("$2202", "S-CPU Interrupt Clear"),
				new RegEntry("$2202.5", "Character IRQ Flag", sa1.CharConvIrqFlag),
				new RegEntry("$2202.7", "IRQ Flag", sa1.CpuIrqRequested),

				new RegEntry("$2203/4", "SA-1 Reset Vector", sa1.Sa1ResetVector, Format.X16),
				new RegEntry("$2205/6", "SA-1 NMI Vector", sa1.Sa1ResetVector, Format.X16),
				new RegEntry("$2207/8", "SA-1 IRQ Vector", sa1.Sa1ResetVector, Format.X16),

				new RegEntry("$2209", "S-CPU Control"),
				new RegEntry("$2209.0-3", "Message", sa1.CpuMessageReceived, Format.X8),
				new RegEntry("$2209.4", "Use NMI Vector", sa1.UseCpuNmiVector),
				new RegEntry("$2209.6", "Use IRQ Vector", sa1.UseCpuIrqVector),
				new RegEntry("$2209.7", "IRQ Requested", sa1.CpuIrqRequested),

				new RegEntry("$220A", "SA-1 CPU Interrupt Enable"),
				new RegEntry("$220A.4", "SA-1 NMI Enabled", sa1.Sa1NmiEnabled),
				new RegEntry("$220A.5", "DMA IRQ Enabled", sa1.DmaIrqEnabled),
				new RegEntry("$220A.6", "Timer IRQ Enabled", sa1.TimerIrqEnabled),
				new RegEntry("$220A.7", "SA-1 IRQ Enabled", sa1.Sa1IrqEnabled),

				new RegEntry("$220B", "S-CPU Interrupt Clear"),
				new RegEntry("$220B.4", "SA-1 NMI Requested", sa1.Sa1NmiRequested),
				new RegEntry("$220B.5", "DMA IRQ Flag", sa1.DmaIrqFlag),
				new RegEntry("$220B.7", "SA-1 IRQ Requested", sa1.Sa1IrqRequested),

				new RegEntry("$220C/D", "S-CPU NMI Vector", sa1.CpuNmiVector, Format.X16),
				new RegEntry("$220E/F", "S-CPU IRQ Vector", sa1.CpuIrqVector, Format.X16),

				new RegEntry("$2210", "H/V Timer Control"),
				new RegEntry("$2210.0", "Horizontal Timer Enabled", sa1.HorizontalTimerEnabled),
				new RegEntry("$2210.1", "Vertical Timer Enabled", sa1.VerticalTimerEnabled),
				new RegEntry("$2210.7", "Linear Timer", sa1.UseLinearTimer),

				new RegEntry("$2212/3", "H-Timer", sa1.HTimer, Format.X16),
				new RegEntry("$2214/5", "V-Timer", sa1.VTimer, Format.X16),

				new RegEntry("", "ROM/BWRAM/IRAM Mappings"),
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

				new RegEntry("$2230", "DMA Control"),
				new RegEntry("$2230.0-1", "DMA Source Device", sa1.DmaSrcDevice),
				new RegEntry("$2230.2-3", "DMA Destination Device", sa1.DmaDestDevice),
				new RegEntry("$2230.4", "Automatic DMA Character Conversion", sa1.DmaCharConvAuto),
				new RegEntry("$2230.5", "DMA Character Conversion", sa1.DmaCharConv),
				new RegEntry("$2230.6", "DMA Priority", sa1.DmaPriority),
				new RegEntry("$2230.7", "DMA Enabled", sa1.DmaEnabled),

				new RegEntry("$2231.0-1", "Character Format (BPP)", sa1.CharConvBpp),
				new RegEntry("$2231.2-5", "Character Conversion Width", sa1.CharConvWidth, Format.X8),
				new RegEntry("$2231.7", "Character DMA Active", sa1.CharConvDmaActive),

				new RegEntry("$2232/3/4", "DMA Source Address", sa1.DmaSrcAddr, Format.X24),
				new RegEntry("$2235/6/7", "DMA Destination Address", sa1.DmaDestAddr, Format.X24),

				new RegEntry("$2238/9", "DMA Size", sa1.DmaSize, Format.X16),
				new RegEntry("$223F.7", "BW-RAM 2 bpp mode", sa1.BwRam2BppMode)
			};

			entries.Add(new RegEntry("", "Bitmap Register File"));
			for(int i = 0; i < 8; i++) {
				entries.Add(new RegEntry("$224" + i, "BRF #" + i, sa1.BitmapRegister1[i]));
			}
			for(int i = 0; i < 8; i++) {
				entries.Add(new RegEntry("$224" + (8 + i).ToString("X"), "BRF #" + (i + 8), sa1.BitmapRegister2[i]));
			}

			entries.AddRange(new List<RegEntry>() {
				new RegEntry("", "Math Registers"),
				new RegEntry("$2250.0-1", "Math Operation", sa1.MathOp),
				new RegEntry("$2251/2", "Multiplicand/Dividend", sa1.MultiplicandDividend, Format.X16),
				new RegEntry("$2253/4", "Multiplier/Divisor", sa1.MultiplierDivisor, Format.X16),

				new RegEntry("", "Variable Length Registers"),
				new RegEntry("$2258", "Variable Length Bit Processing"),
				new RegEntry("$2258.0-3", "Variable Length Bit Count", sa1.VarLenBitCount, Format.X8),
				new RegEntry("$2258.7", "Variable Length Auto-Increment", sa1.VarLenAutoInc),
				new RegEntry("$2259/A/B", "Variable Length Address", sa1.VarLenAddress, Format.X24),

				new RegEntry("$2300", "S-CPU Status Flags"),
				new RegEntry("$2300.0-3", "Message Received", sa1.CpuMessageReceived, Format.X8),
				new RegEntry("$2300.4", "Use NMI Vector", sa1.UseCpuNmiVector),
				new RegEntry("$2300.5", "Character Conversion IRQ Flag", sa1.CharConvIrqFlag),
				new RegEntry("$2300.6", "Use IRQ Vector", sa1.UseCpuIrqVector),
				new RegEntry("$2300.7", "IRQ Requested", sa1.CpuIrqRequested),

				new RegEntry("$2301", "SA-1 Status Flags"),
				new RegEntry("$2301.0-3", "Message Received", sa1.Sa1MessageReceived, Format.X8),
				new RegEntry("$2301.4", "NMI Requested", sa1.Sa1NmiRequested),
				new RegEntry("$2301.5", "DMA IRQ Flag", sa1.DmaIrqFlag),
				new RegEntry("$2301.7", "IRQ Requested", sa1.Sa1IrqRequested),

				new RegEntry("$2302/3", "SA-1 H-Counter", 0, Format.X16),
				new RegEntry("$2304/5", "SA-1 V-Counter", 0, Format.X16),

				new RegEntry("$2306/7/8/9/A", "Math Result", sa1.MathOpResult),
				new RegEntry("$230B.7", "Math Overflow", sa1.MathOverflow)
			});

			return new RegisterViewerTab("SA-1", entries, Config);
		}

		private RegisterViewerTab GetSnesPpuTab(ref SnesState state)
		{
			SnesPpuState ppu = state.Ppu;

			string GetLayerSize(LayerConfig layer)
			{
				return (layer.DoubleWidth ? "64" : "32") + "x" + (layer.DoubleHeight ? "64" : "32");
			}

			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("", "State"),
				new RegEntry("", "Cycle (H)", ppu.Cycle),
				new RegEntry("", "HClock", ppu.HClock),
				new RegEntry("", "Scanline (V)", ppu.Scanline),
				new RegEntry("", "Frame Number", ppu.FrameCount),

				new RegEntry("$2100", "Brightness"),
				new RegEntry("$2100.0-3", "Brightness", ppu.ScreenBrightness),
				new RegEntry("$2100.7", "Forced Blank", ppu.ForcedBlank),
				new RegEntry("$2101", "OAM Settings"),
				new RegEntry("$2101.0-2", "OAM Table Address", ppu.OamBaseAddress, Format.X16),
				new RegEntry("$2101.3-4", "OAM Second Table Address", (ppu.OamBaseAddress + ppu.OamAddressOffset) & 0x7FFF, Format.X16),
				new RegEntry("$2101.5-7", "OAM Size Mode", ppu.OamMode),
				new RegEntry("$2102-2103", "OAM Base Address", ppu.OamRamAddress),
				new RegEntry("$2103.7", "OAM Priority", ppu.EnableOamPriority),
				new RegEntry("", "OAM Address", ppu.InternalOamRamAddress),

				new RegEntry("$2105", "BG Mode/Size"),
				new RegEntry("$2105.0-2", "BG Mode", ppu.BgMode),
				new RegEntry("$2105.3", "Mode 1 BG3 Priority", ppu.Mode1Bg3Priority),
				new RegEntry("$2105.4", "BG1 16x16 Tiles", ppu.Layers[0].LargeTiles),
				new RegEntry("$2105.5", "BG2 16x16 Tiles", ppu.Layers[1].LargeTiles),
				new RegEntry("$2105.6", "BG3 16x16 Tiles", ppu.Layers[2].LargeTiles),
				new RegEntry("$2105.7", "BG4 16x16 Tiles", ppu.Layers[3].LargeTiles),

				new RegEntry("$2106", "Mosaic"),
				new RegEntry("$2106.0", "BG1 Mosaic Enabled", (ppu.MosaicEnabled & 0x01) != 0),
				new RegEntry("$2106.1", "BG2 Mosaic Enabled", (ppu.MosaicEnabled & 0x02) != 0),
				new RegEntry("$2106.2", "BG3 Mosaic Enabled", (ppu.MosaicEnabled & 0x04) != 0),
				new RegEntry("$2106.3", "BG4 Mosaic Enabled", (ppu.MosaicEnabled & 0x08) != 0),
				new RegEntry("$2106.4-7", "Mosaic Size", (ppu.MosaicSize - 1).ToString() + " (" + ppu.MosaicSize.ToString() + "x" + ppu.MosaicSize.ToString() + ")", ppu.MosaicSize - 1),

				new RegEntry("$2107 - $210A", "Tilemap Addresses/Sizes"),
				new RegEntry("$2107.0-1", "BG1 Size", GetLayerSize(ppu.Layers[0]), (ppu.Layers[0].DoubleWidth ? 0x01 : 0) | (ppu.Layers[0].DoubleHeight ? 0x02 : 0)),
				new RegEntry("$2107.2-6", "BG1 Address", ppu.Layers[0].TilemapAddress, Format.X16),
				new RegEntry("$2108.0-1", "BG2 Size", GetLayerSize(ppu.Layers[1]), (ppu.Layers[1].DoubleWidth ? 0x01 : 0) | (ppu.Layers[1].DoubleHeight ? 0x02 : 0)),
				new RegEntry("$2108.2-6", "BG2 Address", ppu.Layers[1].TilemapAddress, Format.X16),
				new RegEntry("$2109.0-1", "BG3 Size", GetLayerSize(ppu.Layers[2]), (ppu.Layers[2].DoubleWidth ? 0x01 : 0) | (ppu.Layers[2].DoubleHeight ? 0x02 : 0)),
				new RegEntry("$2109.2-6", "BG3 Address", ppu.Layers[2].TilemapAddress, Format.X16),
				new RegEntry("$210A.0-1", "BG4 Size", GetLayerSize(ppu.Layers[3]), (ppu.Layers[3].DoubleWidth ? 0x01 : 0) | (ppu.Layers[3].DoubleHeight ? 0x02 : 0)),
				new RegEntry("$210A.2-6", "BG4 Address", ppu.Layers[3].TilemapAddress, Format.X16),

				new RegEntry("$210B - $210C", "Tile Addresses"),
				new RegEntry("$210B.0-2", "BG1 Tile Address", ppu.Layers[0].ChrAddress, Format.X16),
				new RegEntry("$210B.4-6", "BG2 Tile Address", ppu.Layers[1].ChrAddress, Format.X16),
				new RegEntry("$210C.0-2", "BG3 Tile Address", ppu.Layers[2].ChrAddress, Format.X16),
				new RegEntry("$210C.4-6", "BG4 Tile Address", ppu.Layers[3].ChrAddress, Format.X16),

				new RegEntry("$210D - $2114", "H/V Scroll Offsets"),
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

				new RegEntry("$2115 - $2117", "VRAM"),
				new RegEntry("$2115.0-1", "Increment Value", ppu.VramIncrementValue),
				new RegEntry("$2115.2-3", "Address Mapping", ppu.VramAddressRemapping),
				new RegEntry("$2115.7", "Increment on $2119", ppu.VramAddrIncrementOnSecondReg),
				new RegEntry("$2116/7", "VRAM Address", ppu.VramAddress, Format.X16),

				new RegEntry("$211A - $2120", "Mode 7"),
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

				new RegEntry("$2121", "CGRAM"),
				new RegEntry("$2121", "CGRAM Address", ppu.CgramAddress, Format.X16),
				new RegEntry("", "CGRAM next write to MSB", ppu.CgramAddressLatch),

				new RegEntry("$2123 - $212B", "Windows"),
				new RegEntry("", "BG1 Windows"),
				new RegEntry("$2123.0", "BG1 Window 1 Inverted", ppu.Window[0].InvertedLayers[0] != 0),
				new RegEntry("$2123.1", "BG1 Window 1 Active", ppu.Window[0].ActiveLayers[0] != 0),
				new RegEntry("$2123.2", "BG1 Window 2 Inverted", ppu.Window[1].InvertedLayers[0] != 0),
				new RegEntry("$2123.3", "BG1 Window 2 Active", ppu.Window[1].ActiveLayers[0] != 0),

				new RegEntry("", "BG2 Windows"),
				new RegEntry("$2123.4", "BG2 Window 1 Inverted", ppu.Window[0].InvertedLayers[1] != 0),
				new RegEntry("$2123.5", "BG2 Window 1 Active", ppu.Window[0].ActiveLayers[1] != 0),
				new RegEntry("$2123.6", "BG2 Window 2 Inverted", ppu.Window[1].InvertedLayers[1] != 0),
				new RegEntry("$2123.7", "BG2 Window 2 Active", ppu.Window[1].ActiveLayers[1] != 0),

				new RegEntry("", "BG3 Windows"),
				new RegEntry("$2124.0", "BG3 Window 1 Inverted", ppu.Window[0].InvertedLayers[2] != 0),
				new RegEntry("$2124.1", "BG3 Window 1 Active", ppu.Window[0].ActiveLayers[2] != 0),
				new RegEntry("$2124.2", "BG3 Window 2 Inverted", ppu.Window[1].InvertedLayers[2] != 0),
				new RegEntry("$2124.3", "BG3 Window 2 Active", ppu.Window[1].ActiveLayers[2] != 0),

				new RegEntry("", "BG4 Windows"),
				new RegEntry("$2124.4", "BG4 Window 1 Inverted", ppu.Window[0].InvertedLayers[3] != 0),
				new RegEntry("$2124.5", "BG4 Window 1 Active", ppu.Window[0].ActiveLayers[3] != 0),
				new RegEntry("$2124.6", "BG4 Window 2 Inverted", ppu.Window[1].InvertedLayers[3] != 0),
				new RegEntry("$2124.7", "BG4 Window 2 Active", ppu.Window[1].ActiveLayers[3] != 0),

				new RegEntry("", "OAM Windows"),
				new RegEntry("$2125.0", "OAM Window 1 Inverted", ppu.Window[0].InvertedLayers[4] != 0),
				new RegEntry("$2125.1", "OAM Window 1 Active", ppu.Window[0].ActiveLayers[4] != 0),
				new RegEntry("$2125.2", "OAM Window 2 Inverted", ppu.Window[1].InvertedLayers[4] != 0),
				new RegEntry("$2125.3", "OAM Window 2 Active", ppu.Window[1].ActiveLayers[4] != 0),

				new RegEntry("", "Color Windows"),
				new RegEntry("$2125.4", "Color Window 1 Inverted", ppu.Window[0].InvertedLayers[5] != 0),
				new RegEntry("$2125.5", "Color Window 1 Active", ppu.Window[0].ActiveLayers[5] != 0),
				new RegEntry("$2125.6", "Color Window 2 Inverted", ppu.Window[1].InvertedLayers[5] != 0),
				new RegEntry("$2125.7", "Color Window 2 Active", ppu.Window[1].ActiveLayers[5] != 0),

				new RegEntry("", "Window Position"),
				new RegEntry("$2126", "Window 1 Left", ppu.Window[0].Left),
				new RegEntry("$2127", "Window 1 Right", ppu.Window[0].Right),
				new RegEntry("$2128", "Window 2 Left", ppu.Window[1].Left),
				new RegEntry("$2129", "Window 2 Right", ppu.Window[1].Right),

				new RegEntry("", "Window Masks"),
				new RegEntry("$212A.0-1", "BG1 Window Mask", ppu.MaskLogic[0]),
				new RegEntry("$212A.2-3", "BG2 Window Mask", ppu.MaskLogic[1]),
				new RegEntry("$212A.4-5", "BG3 Window Mask", ppu.MaskLogic[2]),
				new RegEntry("$212A.6-7", "BG4 Window Mask", ppu.MaskLogic[3]),
				new RegEntry("$212B.6-7", "OAM Window Mask", ppu.MaskLogic[4]),
				new RegEntry("$212B.6-7", "Color Window Mask", ppu.MaskLogic[5]),

				new RegEntry("$212C", "Main Screen Layers"),
				new RegEntry("$212C.0", "BG1 Enabled", (ppu.MainScreenLayers & 0x01) != 0),
				new RegEntry("$212C.1", "BG2 Enabled", (ppu.MainScreenLayers & 0x02) != 0),
				new RegEntry("$212C.2", "BG3 Enabled", (ppu.MainScreenLayers & 0x04) != 0),
				new RegEntry("$212C.3", "BG4 Enabled", (ppu.MainScreenLayers & 0x08) != 0),
				new RegEntry("$212C.4", "OAM Enabled", (ppu.MainScreenLayers & 0x10) != 0),

				new RegEntry("$212D", "Sub Screen Layers"),
				new RegEntry("$212D.0", "BG1 Enabled", (ppu.SubScreenLayers & 0x01) != 0),
				new RegEntry("$212D.1", "BG2 Enabled", (ppu.SubScreenLayers & 0x02) != 0),
				new RegEntry("$212D.2", "BG3 Enabled", (ppu.SubScreenLayers & 0x04) != 0),
				new RegEntry("$212D.3", "BG4 Enabled", (ppu.SubScreenLayers & 0x08) != 0),
				new RegEntry("$212D.4", "OAM Enabled", (ppu.SubScreenLayers & 0x10) != 0),

				new RegEntry("$212E", "Main Screen Windows"),
				new RegEntry("$212E.0", "BG1 Mainscreen Window Enabled", ppu.WindowMaskMain[0] != 0),
				new RegEntry("$212E.1", "BG2 Mainscreen Window Enabled", ppu.WindowMaskMain[1] != 0),
				new RegEntry("$212E.2", "BG3 Mainscreen Window Enabled", ppu.WindowMaskMain[2] != 0),
				new RegEntry("$212E.3", "BG4 Mainscreen Window Enabled", ppu.WindowMaskMain[3] != 0),
				new RegEntry("$212E.4", "OAM Mainscreen Window Enabled", ppu.WindowMaskMain[4] != 0),

				new RegEntry("$212F", "Sub Screen Windows"),
				new RegEntry("$212F.0", "BG1 Subscreen Window Enabled", ppu.WindowMaskSub[0] != 0),
				new RegEntry("$212F.1", "BG2 Subscreen Window Enabled", ppu.WindowMaskSub[1] != 0),
				new RegEntry("$212F.2", "BG3 Subscreen Window Enabled", ppu.WindowMaskSub[2] != 0),
				new RegEntry("$212F.3", "BG4 Subscreen Window Enabled", ppu.WindowMaskSub[3] != 0),
				new RegEntry("$212F.4", "OAM Subscreen Window Enabled", ppu.WindowMaskSub[4] != 0),

				new RegEntry("$2130 - $2131", "Color Math"),
				new RegEntry("$2130.0", "Direct Color Mode", ppu.DirectColorMode),
				new RegEntry("$2130.1", "CM - Add Subscreen", ppu.ColorMathAddSubscreen),
				new RegEntry("$2130.4-5", "CM - Prevent Mode", ppu.ColorMathPreventMode),
				new RegEntry("$2130.6-7", "CM - Clip Mode", ppu.ColorMathClipMode),

				new RegEntry("$2131.0", "CM - BG1 Enabled", (ppu.ColorMathEnabled & 0x01) != 0),
				new RegEntry("$2131.1", "CM - BG2 Enabled", (ppu.ColorMathEnabled & 0x02) != 0),
				new RegEntry("$2131.2", "CM - BG3 Enabled", (ppu.ColorMathEnabled & 0x04) != 0),
				new RegEntry("$2131.3", "CM - BG4 Enabled", (ppu.ColorMathEnabled & 0x08) != 0),
				new RegEntry("$2131.4", "CM - OAM Enabled", (ppu.ColorMathEnabled & 0x10) != 0),
				new RegEntry("$2131.5", "CM - Background Enabled", (ppu.ColorMathEnabled & 0x20) != 0),
				new RegEntry("$2131.6", "CM - Half Mode", ppu.ColorMathHalveResult),
				new RegEntry("$2131.7", "CM - Subtract Mode", ppu.ColorMathSubtractMode),

				new RegEntry("$2132 - $2133", "Misc."),
				new RegEntry("$2132", "Fixed Color - BGR", ppu.FixedColor, Format.X16),

				new RegEntry("$2133.0", "Screen Interlace", ppu.ScreenInterlace),
				new RegEntry("$2133.1", "OAM Interlace", ppu.ObjInterlace),
				new RegEntry("$2133.2", "Overscan Mode", ppu.OverscanMode),
				new RegEntry("$2133.3", "High Resolution Mode", ppu.HiResMode),
				new RegEntry("$2133.4", "Ext. BG Enabled", ppu.ExtBgEnabled),
			};

			return new RegisterViewerTab("PPU", entries, Config, CpuType.Snes, MemoryType.SnesRegister);
		}

		private RegisterViewerTab GetSnesDspTab(ref SnesState state)
		{
			DspState dsp = state.Dsp;
			List<RegEntry> entries = new List<RegEntry>();

			void AddReg(int i, string name, bool signed = false)
			{
				entries.Add(new RegEntry("$" + i.ToString("X2"), name, signed ? (sbyte)dsp.Regs[i] : dsp.Regs[i], Format.X8));
			}

			AddReg(0x0C, "Main Volume (MVOL) - Left", true);
			AddReg(0x1C, "Main Volume (MVOL) - Right", true);
			AddReg(0x2C, "Echo Volume (EVOL) - Left", true);
			AddReg(0x3C, "Echo Volume (EVOL) - Right", true);

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

			entries.Add(new RegEntry("$6C", "Flags (FLG)"));
			entries.Add(new RegEntry("$6C.0-4", "Noise Clock", dsp.Regs[0x6C] & 0x1F, Format.X8));
			entries.Add(new RegEntry("$6C.5", "Echo Disabled", (dsp.Regs[0x6C] & 0x20) != 0));
			entries.Add(new RegEntry("$6C.6", "Mute", (dsp.Regs[0x6C] & 0x40) != 0));
			entries.Add(new RegEntry("$6C.7", "Reset", (dsp.Regs[0x6C] & 0x80) != 0));

			entries.Add(new RegEntry("$xF", "Coefficients"));
			for(int i = 0; i < 8; i++) {
				AddReg((i << 4) | 0x0F, "Coefficient " + i);
			}

			for(int i = 0; i < 8; i++) {
				entries.Add(new RegEntry("Voice #" + i.ToString(), ""));

				int voice = i << 4;
				AddReg(voice | 0x00, "Left Volume (VOL)", true);
				AddReg(voice | 0x01, "Right Volume (VOL)", true);
				entries.Add(new RegEntry("$" + i + "2 + $" + i + "3", "Pitch (P)", dsp.Regs[voice | 0x02] | (dsp.Regs[voice | 0x03] << 8), Format.X16));
				AddReg(voice | 0x04, "Source (SRCN)");
				AddReg(voice | 0x05, "ADSR1");
				AddReg(voice | 0x06, "ADSR2");
				AddReg(voice | 0x07, "GAIN");
				AddReg(voice | 0x08, "ENVX");
				AddReg(voice | 0x09, "OUTX");
			}

			return new RegisterViewerTab("DSP", entries, Config);
		}

		private RegisterViewerTab GetSnesSpcTab(ref SnesState state)
		{
			string GetTimerFrequency(double baseFreq, int divider)
			{
				return (divider == 0 ? (baseFreq / 256) : (baseFreq / divider)).ToString(".00") + " Hz";
			}

			SpcState spc = state.Spc;
			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("$F0", "Test"),
				new RegEntry("$F0.0", "Timers Disabled", spc.TimersDisabled),
				new RegEntry("$F0.1", "RAM Write Enabled", spc.WriteEnabled),
				new RegEntry("$F0.3", "Timers Enabled", spc.TimersEnabled),
				new RegEntry("$F0.4-5", "External Speed", spc.ExternalSpeed),
				new RegEntry("$F0.6-7", "Internal Speed", spc.InternalSpeed),

				new RegEntry("$F1", "Control"),
				new RegEntry("$F1.0", "Timer 0 Enabled", spc.Timer0.Enabled),
				new RegEntry("$F1.1", "Timer 1 Enabled", spc.Timer1.Enabled),
				new RegEntry("$F1.2", "Timer 2 Enabled", spc.Timer2.Enabled),
				new RegEntry("$F1.7", "IPL ROM Enabled", spc.RomEnabled),

				new RegEntry("$F2", "DSP"),
				new RegEntry("$F2", "DSP Register", spc.DspReg, Format.X8),

				new RegEntry("$F4 - $F7", "CPU<->SPC Ports"),
				new RegEntry("$F4", "Port 0 (CPU read)", spc.OutputReg[0], Format.X8),
				new RegEntry("$F4", "Port 0 (SPC read)", spc.CpuRegs[0], Format.X8),
				new RegEntry("$F5", "Port 1 (CPU read)", spc.OutputReg[1], Format.X8),
				new RegEntry("$F5", "Port 1 (SPC read)", spc.CpuRegs[1], Format.X8),
				new RegEntry("$F6", "Port 2 (CPU read)", spc.OutputReg[2], Format.X8),
				new RegEntry("$F6", "Port 2 (SPC read)", spc.CpuRegs[2], Format.X8),
				new RegEntry("$F7", "Port 3 (CPU read)", spc.OutputReg[3], Format.X8),
				new RegEntry("$F7", "Port 3 (SPC read)", spc.CpuRegs[3], Format.X8),

				new RegEntry("$F8 - $F9", "RAM Registers"),
				new RegEntry("$F8", "RAM Reg 0", spc.RamReg[0], Format.X8),
				new RegEntry("$F9", "RAM Reg 1", spc.RamReg[1], Format.X8),

				new RegEntry("$FA - $FF", "Timers"),
				new RegEntry("$FA", "Timer 0 Divider", spc.Timer0.Target, Format.X8),
				new RegEntry("$FA", "Timer 0 Frequency", GetTimerFrequency(8000, spc.Timer0.Target), spc.Timer0.Target),
				new RegEntry("$FB", "Timer 1 Divider", spc.Timer1.Target, Format.X8),
				new RegEntry("$FB", "Timer 1 Frequency", GetTimerFrequency(8000, spc.Timer1.Target), spc.Timer1.Target),
				new RegEntry("$FC", "Timer 2 Divider", spc.Timer2.Target, Format.X8),
				new RegEntry("$FC", "Timer 2 Frequency", GetTimerFrequency(64000, spc.Timer2.Target), spc.Timer2.Target),

				new RegEntry("$FD", "Timer 0 Output", spc.Timer0.Output, Format.X8),
				new RegEntry("$FE", "Timer 1 Output", spc.Timer1.Output, Format.X8),
				new RegEntry("$FF", "Timer 2 Output", spc.Timer2.Output, Format.X8),
			};

			return new RegisterViewerTab("SPC", entries, Config, CpuType.Spc, MemoryType.SpcMemory);
		}

		private RegisterViewerTab GetSnesDmaTab(ref SnesState state)
		{
			List<RegEntry> entries = new List<RegEntry>();

			for(int i = 0; i < 8; i++) {
				DmaChannelConfig ch = state.Dma.Channels[i];
				entries.Add(new RegEntry("DMA Channel " + i.ToString(), ""));
				entries.Add(new RegEntry("$420B." + i.ToString(), "Channel Enabled", ch.DmaActive));
				entries.Add(new RegEntry("$420C." + i.ToString(), "HDMA Enabled", (state.Dma.HdmaChannels & (1 << i)) != 0));

				entries.Add(new RegEntry("$43" + i.ToString() + "0.0-2", "Transfer Mode", ch.TransferMode));
				entries.Add(new RegEntry("$43" + i.ToString() + "0.3", "Fixed", ch.FixedTransfer));
				entries.Add(new RegEntry("$43" + i.ToString() + "0.4", "Decrement", ch.Decrement));
				entries.Add(new RegEntry("$43" + i.ToString() + "0.6", "Indirect HDMA", ch.HdmaIndirectAddressing));
				entries.Add(new RegEntry("$43" + i.ToString() + "0.7", "Direction", ch.InvertDirection ? "B -> A" : "A -> B", ch.InvertDirection));

				entries.Add(new RegEntry("$43" + i.ToString() + "1", "B Bus Address", ch.DestAddress, Format.X8));
				entries.Add(new RegEntry("$43" + i.ToString() + "2/3", "A Bus Address", ch.SrcAddress, Format.X16));
				entries.Add(new RegEntry("$43" + i.ToString() + "4", "A Bus Bank", ch.SrcBank, Format.X8));
				entries.Add(new RegEntry("$43" + i.ToString() + "5/6", "Size", ch.TransferSize, Format.X16));

				entries.Add(new RegEntry("$43" + i.ToString() + "7", "HDMA Bank", ch.HdmaBank, Format.X8));
				entries.Add(new RegEntry("$43" + i.ToString() + "8/9", "HDMA Address", ch.HdmaTableAddress, Format.X16));
				entries.Add(new RegEntry("$43" + i.ToString() + "A", "HDMA Line Counter", ch.HdmaLineCounterAndRepeat, Format.X8));
				entries.Add(new RegEntry("$43" + i.ToString() + "B", "Unused register", ch.UnusedRegister, Format.X8));
			}

			return new RegisterViewerTab("DMA", entries, Config, CpuType.Snes, MemoryType.SnesRegister);
		}

		private RegisterViewerTab GetSnesCpuTab(ref SnesState state)
		{
			InternalRegisterState regs = state.InternalRegs;
			AluState alu = state.Alu;

			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("$2181 - $2183", "Work RAM Position", state.WramPosition, Format.X24),

				new RegEntry("$4200 - $4201", "IRQ/NMI/Autopoll Enabled"),
				new RegEntry("$4200.0", "Auto Joypad Poll", regs.EnableAutoJoypadRead),
				new RegEntry("$4200.4", "H IRQ Enabled", regs.EnableHorizontalIrq),
				new RegEntry("$4200.5", "V IRQ Enabled", regs.EnableVerticalIrq),
				new RegEntry("$4200.7", "NMI Enabled", regs.EnableNmi),

				new RegEntry("$4201", "IO Port", regs.IoPortOutput, Format.X8),

				new RegEntry("$4202 - $4206", "Mult/Div Registers (Input)"),
				new RegEntry("$4202", "Multiplicand", alu.MultOperand1, Format.X8),
				new RegEntry("$4203", "Multiplier", alu.MultOperand2, Format.X8),
				new RegEntry("$4204/5", "Dividend", alu.Dividend, Format.X16),
				new RegEntry("$4206", "Divisor", alu.Divisor, Format.X8),

				new RegEntry("$4207 - $420A", "H/V IRQ Timers"),
				new RegEntry("$4207/8", "H Timer", regs.HorizontalTimer, Format.X16),
				new RegEntry("$4209/A", "V Timer", regs.VerticalTimer, Format.X16),

				new RegEntry("$420D - $4212", "Misc. Flags"),

				new RegEntry("$420D.0", "FastROM Enabled", regs.EnableFastRom),
				new RegEntry("$4210.7", "NMI Flag", (_snesReg4210 & 0x80) != 0),
				new RegEntry("$4211.7", "IRQ Flag", (_snesReg4211 & 0x80) != 0),

				new RegEntry("$4212.0", "Auto Joypad Read Active", (_snesReg4212 & 0x01) != 0),
				new RegEntry("$4212.6", "H-Blank Flag", (_snesReg4212 & 0x40) != 0),
				new RegEntry("$4212.7", "V-Blank Flag", (_snesReg4212 & 0x80) != 0),

				new RegEntry("$4214 - $4217", "Mult/Div Registers (Result)"),
				new RegEntry("$4214/5", "Quotient", alu.DivResult, Format.X16),
				new RegEntry("$4216/7", "Product / Remainder", alu.MultOrRemainderResult, Format.X16),

				new RegEntry("$4218 - $421F", "Input Data"),
				new RegEntry("$4218/9", "P1 Data", regs.ControllerData[0], Format.X16),
				new RegEntry("$421A/B", "P2 Data", regs.ControllerData[1], Format.X16),
				new RegEntry("$421C/D", "P3 Data", regs.ControllerData[2], Format.X16),
				new RegEntry("$421E/F", "P4 Data", regs.ControllerData[3], Format.X16),
			};

			return new RegisterViewerTab("CPU", entries, Config, CpuType.Snes, MemoryType.SnesRegister);
		}

		private RegisterViewerTab GetNesPpuTab(ref NesState state)
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

			return new RegisterViewerTab("PPU", entries, Config, CpuType.Nes, MemoryType.NesMemory);
		}

		private RegisterViewerTab GetNesApuTab(ref NesState state)
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
				new RegEntry("$4008.1", "Length Counter - Halted", trg.LengthCounter.Halt),

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

			return new RegisterViewerTab("APU", entries, Config, CpuType.Nes, MemoryType.NesMemory);
		}

		private RegisterViewerTab GetNesCartTab(ref NesState state)
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

			return new RegisterViewerTab("Cart", entries, Config, CpuType.Nes, MemoryType.NesMemory);
		}

		private RegisterViewerTab GetPceVceTab(ref PceState state)
		{
			ref PceVceState vce = ref state.Video.Vce;

			List<RegEntry> entries = new List<RegEntry>() {
				new RegEntry("$00.0-1", "CR - Clock Speed", vce.ClockDivider == 4 ? "5.37 MHz" : vce.ClockDivider == 3 ? "7.16 MHz" : "10.74 MHz", vce.ClockDivider),
				new RegEntry("$00.2", "CR - Number of Scanlines", vce.ScanlineCount),
				new RegEntry("$00.7", "CR - Grayscale", vce.Grayscale),
				new RegEntry("$01.0-8", "CTA - Color Table Address", vce.PalAddr, Format.X16),
			};

			return new RegisterViewerTab("VCE", entries, Config, CpuType.Pce, MemoryType.PceMemory);
		}

		private RegisterViewerTab GetPceVpcTab(ref PceState state)
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

			return new RegisterViewerTab("VPC", entries, Config, CpuType.Pce, MemoryType.PceMemory);
		}

		private RegisterViewerTab GetPceVdcTab(ref PceVdcState vdc, string suffix = "")
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
				new RegEntry("$09.0-1", "VRAM Access Mode", vdc.VramAccessMode),
				new RegEntry("$09.2-3", "Sprite Access Mode", vdc.SpriteAccessMode),
				new RegEntry("$09.4-5", "Column Count", vdc.ColumnCount),
				new RegEntry("$09.6", "Row Count", vdc.RowCount),
				new RegEntry("$09.7", "CG Mode", vdc.CgMode),

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

			return new RegisterViewerTab("VDC" + suffix, entries, Config);
		}

		private RegisterViewerTab GetPceCpuTab(ref PceState state)
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

			return new RegisterViewerTab("CPU", entries, Config, CpuType.Pce, MemoryType.PceMemory);
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
					new RegEntry("", "Channel " + (i + 1)),
					new RegEntry("$802-$803", "Frequency", ch.Frequency, Format.X16),
					new RegEntry("$804.0-4", "Amplitude", ch.Amplitude, Format.X8),
					new RegEntry("$804.6", "DDA Enabled", ch.DdaEnabled),
					new RegEntry("$804.7", "Channel Enabled", ch.Enabled),
					new RegEntry("$805.0-3", "Right Amplitude", ch.RightVolume, Format.X8),
					new RegEntry("$805.4-7", "Left Amplitude", ch.LeftVolume, Format.X8),
					new RegEntry("", "Timer", ch.Timer),
				});

				if(i >= 4) {
					entries.Add(new RegEntry("$806.7", "Noise Enabled", ch.NoiseEnabled));
					entries.Add(new RegEntry("$806.0-4", "Noise Frequency", ch.NoiseFrequency, Format.X8));
					entries.Add(new RegEntry("", "Noise Timer", ch.NoiseTimer));
					entries.Add(new RegEntry("", "Noise Output", ch.NoiseOutput == 0x0F ? 1 : 0));
					entries.Add(new RegEntry("", "Noise LSFR", ch.NoiseLfsr, Format.X24));
				}
			}

			return new RegisterViewerTab("PSG", entries, Config, CpuType.Pce, MemoryType.PceMemory);
		}

		private RegisterViewerTab GetPceCdRomTab(ref PceState pceState)
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

			return new RegisterViewerTab("CD-ROM", entries, Config, CpuType.Pce, MemoryType.PceMemory);
		}

		private RegisterViewerTab GetPceArcadeCardTab(ref PceState pceState)
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

			return new RegisterViewerTab("Arcade Card", entries, Config, CpuType.Pce, MemoryType.PceMemory);
		}


		private RegisterViewerTab GetSmsVdpTab(ref SmsState sms)
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
				new RegEntry("$01.1", "8x16 sprites", vdp.UseLargeSprites),
				new RegEntry("$01.3", "M3 - 240-line output", vdp.M3_Use240LineMode),
				new RegEntry("$01.4", "M1 - 224-line output", vdp.M1_Use224LineMode),
				new RegEntry("$01.5", "Vertical blank IRQ enabled", vdp.EnableVerticalBlankIrq),
				new RegEntry("$01.6", "Rendering enabled", vdp.RenderingEnabled),
				new RegEntry("$01.7", "SG-1000 - 16K VRAM Mode", vdp.RenderingEnabled),

				new RegEntry("$02", "Nametable address", vdp.NametableAddress),
				new RegEntry("$05", "Sprite table address", vdp.SpriteTableAddress),
				new RegEntry("$06", "Sprite tileset address", vdp.SpritePatternSelector),
				new RegEntry("$07", "Background color index", vdp.BackgroundColorIndex),
				new RegEntry("$08", "Horizontal scroll", vdp.HorizontalScroll),
				new RegEntry("$09", "Vertical scroll", vdp.VerticalScroll),
				new RegEntry("$0A", "Scanline IRQ reload value", vdp.ScanlineCounter),
				new RegEntry("--", "Scanline IRQ counter", vdp.ScanlineCounterLatch),
			});

			return new RegisterViewerTab("VDP", entries, Config, CpuType.Sms);
		}

		private RegisterViewerTab GetSmsPsgTab(ref SmsState sms)
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

			if(_romInfo.Format == RomFormat.GameGear) {
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

			return new RegisterViewerTab("PSG", entries, Config, CpuType.Sms);
		}


		private RegisterViewerTab GetSmsMiscTab(ref SmsState sms)
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

			return new RegisterViewerTab("Ports", entries, Config, CpuType.Sms);
		}

		public void OnGameLoaded()
		{
			UpdateRomInfo();
			RefreshData();
		}
	}

	public class RegisterViewerTab : ReactiveObject
	{
		private string _name;
		private List<RegEntry> _data;

		public string TabName { get => _name; set => this.RaiseAndSetIfChanged(ref _name, value); }
		public List<RegEntry> Data { get => _data; set => this.RaiseAndSetIfChanged(ref _data, value); }
		public SelectionModel<RegEntry?> Selection { get; set; } = new();
		public List<int> ColumnWidths { get; }

		public CpuType? CpuType { get; }
		public MemoryType? MemoryType { get; }

		public RegisterViewerTab(string name, List<RegEntry> data, RegisterViewerConfig config, CpuType? cpuType = null, MemoryType? memoryType = null)
		{
			_name = name;
			_data = data;
			ColumnWidths = config.ColumnWidths;
			CpuType = cpuType;
			MemoryType = memoryType;
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

	public class RegEntry : INotifyPropertyChanged
	{
		private static ISolidColorBrush HeaderBgBrush = new SolidColorBrush(0x40B0B0B0);

		public string Address { get; private set; }
		public string Name { get; private set; }
		public bool IsEnabled { get; private set; }
		public IBrush Background { get; private set; }

		private string _value;
		private string _valueHex;

		public event PropertyChangedEventHandler? PropertyChanged;

		public string Value
		{ 
			get => _value;
			set {
				if(_value != value) {
					_value = value;
					PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Value)));
				}
			}
		}

		public string ValueHex
		{
			get => _valueHex;
			set
			{
				if(_valueHex != value) {
					_valueHex = value;
					PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ValueHex)));
				}
			}
		}

		public RegEntry(string reg, string name, ISpanFormattable? value, Format format = Format.None)
		{
			Init(reg, name, value, format);
		}

		public RegEntry(string reg, string name, bool value)
		{
			Init(reg, name, value, Format.None);
		}

		public RegEntry(string reg, string name, Enum value)
		{
			Init(reg, name, value, Format.X8);
		}

		public RegEntry(string reg, string name)
		{
			Init(reg, name, null, Format.None);
		}

		public RegEntry(string reg, string name, string textValue, IConvertible? rawValue)
		{
			Init(reg, name, textValue, Format.None);
			if(rawValue != null) {
				_valueHex = GetHexValue(rawValue, Format.X8);
			}
		}

		[MemberNotNull(nameof(Address), nameof(Name), nameof(Background), nameof(_value), nameof(_valueHex))]
		private void Init(string reg, string name, object? value, Format format)
		{
			if(format == Format.None && !(value is bool)) {
				//Display hex values for everything except booleans
				format = Format.X8;
			}

			Address = reg;
			Name = name;

			_value = GetValue(value);

			if(value is Enum) {
				_valueHex = GetHexValue(Convert.ToInt64(value), Format.X8);
			} else {
				_valueHex = GetHexValue(value, format);
			}

			Background = value == null ? RegEntry.HeaderBgBrush : Brushes.Transparent;
			IsEnabled = value != null;
		}

		private string GetValue(object? value)
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

		private string GetHexValue(object? value, Format format)
		{
			if(value == null || value is string) {
				return "";
			}

			if(value is bool boolValue && format != Format.None) {
				return boolValue ? "$01" : "$00";
			}

			switch(format) {
				default: return "";
				case Format.X8: return "$" + ((IFormattable)value).ToString("X2", null);
				case Format.X16: return "$" + ((IFormattable)value).ToString("X4", null);

				case Format.X24: {
					string str = ((IFormattable)value).ToString("X6", null);
					return "$" + (str.Length > 7 ? str.Substring(str.Length - 6) : str);
				}

				case Format.X28: {
					string str = ((IFormattable)value).ToString("X7", null);
					return "$" + (str.Length > 7 ? str.Substring(str.Length - 7) : str);
				}
				case Format.X32: return "$" + ((IFormattable)value).ToString("X8", null);
			}
		}

		public int StartAddress
		{
			get
			{
				string addr = Address;
				if(addr.StartsWith("$")) {
					addr = addr.Substring(1);
				}
				int separator = addr.IndexOfAny(new char[] { '.', '-', '/' });
				if(separator >= 0) {
					addr = addr.Substring(0, separator);
				}

				if(Int32.TryParse(addr.Trim(), System.Globalization.NumberStyles.HexNumber, null, out int startAddress)) {
					return startAddress;
				}

				return -1;
			}
		}

		public int EndAddress
		{
			get
			{
				string addr = Address;
				int separator = addr.IndexOfAny(new char[] { '.', '-', '/' });
				if(separator >= 0) {
					if(addr[separator] == '.') {
						//First separator is a dot for bits, assume there is no end address
						return StartAddress;
					}
					addr = addr.Substring(separator + 1).Trim();
				} else {
					return StartAddress;
				}

				int lastRangeAddr = addr.LastIndexOf('/');
				if(lastRangeAddr >= 0) {
					addr = addr.Substring(lastRangeAddr + 1);
				}

				if(addr.StartsWith("$")) {
					addr = addr.Substring(1);
				}

				separator = addr.IndexOfAny(new char[] { '.', '-', '/' });
				if(separator >= 0) {
					addr = addr.Substring(0, separator);
				}

				if(Int32.TryParse(addr.Trim(), System.Globalization.NumberStyles.HexNumber, null, out int endAddress)) {
					if(addr.Length == 1) {
						return (StartAddress & ~0xF) | endAddress;
					} else {
						return endAddress;
					}
				}

				return StartAddress;
			}
		}

		public enum Format
		{
			None,
			X8,
			X16,
			X24,
			X28,
			X32
		}
	}
}
