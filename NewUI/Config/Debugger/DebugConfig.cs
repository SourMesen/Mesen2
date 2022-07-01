using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Mesen.Debugger;
using Mesen.Interop;

namespace Mesen.Config
{
	public class DebugConfig
	{
		public DebuggerShortcutsConfig Shortcuts { get; set; } = new DebuggerShortcutsConfig();
		public TraceLoggerConfig TraceLogger { get; set; } = new TraceLoggerConfig();
		public HexEditorConfig HexEditor { get; set; } = new HexEditorConfig();
		public EventViewerConfig EventViewer { get; set; } = new EventViewerConfig();
		public DebuggerConfig Debugger { get; set; } = new DebuggerConfig();
		public TilemapViewerConfig TilemapViewer { get; set; } = new TilemapViewerConfig();
		public TileViewerConfig TileViewer { get; set; } = new TileViewerConfig();
		public PaletteViewerConfig PaletteViewer { get; set; } = new PaletteViewerConfig();
		public TileEditorConfig TileEditor { get; set; } = new TileEditorConfig();
		public RegisterViewerConfig RegisterViewer { get; set; } = new RegisterViewerConfig();
		public SpriteViewerConfig SpriteViewer { get; set; } = new SpriteViewerConfig();
		public IntegrationConfig Integration { get; set; } = new IntegrationConfig();
		public ScriptWindowConfig ScriptWindow { get; set; } = new ScriptWindowConfig();
		public ProfilerConfig Profiler { get; set; } = new ProfilerConfig();
		public WatchWindowConfig WatchWindow { get; set; } = new WatchWindowConfig();
		public AssemblerConfig Assembler { get; set; } = new AssemblerConfig();
		public DebugLogConfig DebugLog { get; set; } = new DebugLogConfig();
		public FontConfig Font { get; set; } = new FontConfig();

		public DebugConfig()
		{		
		}

		public void ApplyConfig()
		{
			ConfigApi.SetDebugConfig(new InteropDebugConfig() {
				BreakOnUninitRead = Debugger.BreakOnUninitRead,
				ShowJumpLabels = Debugger.ShowJumpLabels,
				DrawPartialFrame = Debugger.DrawPartialFrame,
				ShowVerifiedData = Debugger.VerifiedDataDisplay == CodeDisplayMode.Show,
				DisassembleVerifiedData = Debugger.VerifiedDataDisplay == CodeDisplayMode.Disassemble,
				ShowUnidentifiedData = Debugger.UnidentifiedBlockDisplay == CodeDisplayMode.Show,
				DisassembleUnidentifiedData = Debugger.UnidentifiedBlockDisplay == CodeDisplayMode.Disassemble,

				UseLowerCaseDisassembly = Debugger.UseLowerCaseDisassembly,

				AutoResetCdl = Debugger.AutoResetCdl,

				ScriptAllowIoOsAccess = ScriptWindow.AllowIoOsAccess,
				ScriptAllowNetworkAccess = ScriptWindow.AllowNetworkAccess,

				UsePredictiveBreakpoints = Debugger.UsePredictiveBreakpoints,
				SingleBreakpointPerInstruction = Debugger.SingleBreakpointPerInstruction,

				SnesBreakOnBrk = Debugger.Snes.BreakOnBrk,
				SnesBreakOnCop = Debugger.Snes.BreakOnCop,
				SnesBreakOnWdm = Debugger.Snes.BreakOnWdm,
				SnesBreakOnStp = Debugger.Snes.BreakOnStp,
				UseAltSpcOpNames = Debugger.Snes.UseAltSpcOpNames,

				GbBreakOnInvalidOamAccess = Debugger.Gameboy.GbBreakOnInvalidOamAccess,
				GbBreakOnInvalidVramAccess = Debugger.Gameboy.GbBreakOnInvalidVramAccess,
				GbBreakOnDisableLcdOutsideVblank = Debugger.Gameboy.GbBreakOnDisableLcdOutsideVblank,
				GbBreakOnInvalidOpCode = Debugger.Gameboy.GbBreakOnInvalidOpCode,
				GbBreakOnNopLoad = Debugger.Gameboy.GbBreakOnNopLoad,
				GbBreakOnOamCorruption = Debugger.Gameboy.GbBreakOnOamCorruption,

				NesBreakOnBrk = Debugger.Nes.BreakOnBrk,
				NesBreakOnUnofficialOpCode = Debugger.Nes.BreakOnUnofficialOpCode,
				NesBreakOnCpuCrash = Debugger.Nes.BreakOnCpuCrash,
				NesBreakOnBusConflict = Debugger.Nes.BreakOnBusConflict,
				NesBreakOnDecayedOamRead = Debugger.Nes.BreakOnDecayedOamRead,
				NesBreakOnPpu2006ScrollGlitch = Debugger.Nes.BreakOnPpu2006ScrollGlitch,

				PceBreakOnBrk = Debugger.Pce.BreakOnBrk,
				PceBreakOnUnofficialOpCode = Debugger.Pce.BreakOnUnofficialOpCode
			});
		}
	}

	public struct InteropDebugConfig
	{
		[MarshalAs(UnmanagedType.I1)] public bool BreakOnUninitRead;

		[MarshalAs(UnmanagedType.I1)] public bool ShowJumpLabels;
		[MarshalAs(UnmanagedType.I1)] public bool DrawPartialFrame;

		[MarshalAs(UnmanagedType.I1)] public bool ShowVerifiedData;
		[MarshalAs(UnmanagedType.I1)] public bool DisassembleVerifiedData;

		[MarshalAs(UnmanagedType.I1)] public bool ShowUnidentifiedData;
		[MarshalAs(UnmanagedType.I1)] public bool DisassembleUnidentifiedData;

		[MarshalAs(UnmanagedType.I1)] public bool UseLowerCaseDisassembly;

		[MarshalAs(UnmanagedType.I1)] public bool AutoResetCdl;

		[MarshalAs(UnmanagedType.I1)] public bool ScriptAllowIoOsAccess;
		[MarshalAs(UnmanagedType.I1)] public bool ScriptAllowNetworkAccess;

		[MarshalAs(UnmanagedType.I1)] public bool UsePredictiveBreakpoints;
		[MarshalAs(UnmanagedType.I1)] public bool SingleBreakpointPerInstruction;

		[MarshalAs(UnmanagedType.I1)] public bool SnesBreakOnBrk;
		[MarshalAs(UnmanagedType.I1)] public bool SnesBreakOnCop;
		[MarshalAs(UnmanagedType.I1)] public bool SnesBreakOnWdm;
		[MarshalAs(UnmanagedType.I1)] public bool SnesBreakOnStp;
		[MarshalAs(UnmanagedType.I1)] public bool UseAltSpcOpNames;

		[MarshalAs(UnmanagedType.I1)] public bool GbBreakOnInvalidOamAccess;
		[MarshalAs(UnmanagedType.I1)] public bool GbBreakOnInvalidVramAccess;
		[MarshalAs(UnmanagedType.I1)] public bool GbBreakOnDisableLcdOutsideVblank;
		[MarshalAs(UnmanagedType.I1)] public bool GbBreakOnInvalidOpCode;
		[MarshalAs(UnmanagedType.I1)] public bool GbBreakOnNopLoad;
		[MarshalAs(UnmanagedType.I1)] public bool GbBreakOnOamCorruption;

		[MarshalAs(UnmanagedType.I1)] public bool NesBreakOnBrk;
		[MarshalAs(UnmanagedType.I1)] public bool NesBreakOnUnofficialOpCode;
		[MarshalAs(UnmanagedType.I1)] public bool NesBreakOnCpuCrash;
		[MarshalAs(UnmanagedType.I1)] public bool NesBreakOnBusConflict;
		[MarshalAs(UnmanagedType.I1)] public bool NesBreakOnDecayedOamRead;
		[MarshalAs(UnmanagedType.I1)] public bool NesBreakOnPpu2006ScrollGlitch;

		[MarshalAs(UnmanagedType.I1)] public bool PceBreakOnBrk;
		[MarshalAs(UnmanagedType.I1)] public bool PceBreakOnUnofficialOpCode;
	}

	public enum RefreshSpeed
	{
		Off = 0,
		Low = 1,
		Normal = 2,
		High = 3
	}

}
