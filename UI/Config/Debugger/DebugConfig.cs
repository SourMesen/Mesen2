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
		public DebuggerShortcutsConfig Shortcuts { get; set; } = new();
		public TraceLoggerConfig TraceLogger { get; set; } = new();
		public HexEditorConfig HexEditor { get; set; } = new();
		public EventViewerConfig EventViewer { get; set; } = new();
		public DebuggerConfig Debugger { get; set; } = new();
		public TilemapViewerConfig TilemapViewer { get; set; } = new();
		public TileViewerConfig TileViewer { get; set; } = new();
		public PaletteViewerConfig PaletteViewer { get; set; } = new();
		public TileEditorConfig TileEditor { get; set; } = new();
		public RegisterViewerConfig RegisterViewer { get; set; } = new();
		public SpriteViewerConfig SpriteViewer { get; set; } = new();
		public IntegrationConfig Integration { get; set; } = new();
		public ScriptWindowConfig ScriptWindow { get; set; } = new();
		public ProfilerConfig Profiler { get; set; } = new();
		public MemorySearchConfig MemorySearch { get; set; } = new();
		public WatchWindowConfig WatchWindow { get; set; } = new();
		public AssemblerConfig Assembler { get; set; } = new();
		public DebugLogConfig DebugLog { get; set; } = new();
		public DebuggerFontConfig Fonts { get; set; } = new();

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
				ShowMemoryValues = Debugger.ShowMemoryValues,

				AutoResetCdl = Debugger.AutoResetCdl,

				UsePredictiveBreakpoints = Debugger.UsePredictiveBreakpoints,
				SingleBreakpointPerInstruction = Debugger.SingleBreakpointPerInstruction,

				SnesBreakOnBrk = Debugger.Snes.BreakOnBrk,
				SnesBreakOnCop = Debugger.Snes.BreakOnCop,
				SnesBreakOnWdm = Debugger.Snes.BreakOnWdm,
				SnesBreakOnStp = Debugger.Snes.BreakOnStp,
				SnesUseAltSpcOpNames = Debugger.Snes.UseAltSpcOpNames,
				SnesIgnoreDspReadWrites = Debugger.Snes.IgnoreDspReadWrites,

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
				NesBreakOnPpu2000ScrollGlitch = Debugger.Nes.BreakOnPpu2000ScrollGlitch,
				NesBreakOnPpu2006ScrollGlitch = Debugger.Nes.BreakOnPpu2006ScrollGlitch,
				NesBreakOnExtOutputMode = Debugger.Nes.NesBreakOnExtOutputMode,

				PceBreakOnBrk = Debugger.Pce.BreakOnBrk,
				PceBreakOnUnofficialOpCode = Debugger.Pce.BreakOnUnofficialOpCode,
				PceBreakOnInvalidVramAddress = Debugger.Pce.BreakOnInvalidVramAddress,
				
				SmsBreakOnNopLoad = Debugger.Sms.BreakOnNopLoad,

				GbaBreakOnInvalidOpCode = Debugger.Gba.BreakOnInvalidOpCode,
				GbaBreakOnNopLoad = Debugger.Gba.BreakOnNopLoad,
				GbaBreakOnUnalignedMemAccess = Debugger.Gba.BreakOnUnalignedMemAccess,

				WsBreakOnUndefinedOpCode = Debugger.Ws.BreakOnUndefinedOpCode,

				ScriptAllowIoOsAccess = ScriptWindow.AllowIoOsAccess,
				ScriptAllowNetworkAccess = ScriptWindow.AllowNetworkAccess,
				ScriptTimeout = ScriptWindow.ScriptTimeout
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
		[MarshalAs(UnmanagedType.I1)] public bool ShowMemoryValues;

		[MarshalAs(UnmanagedType.I1)] public bool AutoResetCdl;

		[MarshalAs(UnmanagedType.I1)] public bool UsePredictiveBreakpoints;
		[MarshalAs(UnmanagedType.I1)] public bool SingleBreakpointPerInstruction;

		[MarshalAs(UnmanagedType.I1)] public bool SnesBreakOnBrk;
		[MarshalAs(UnmanagedType.I1)] public bool SnesBreakOnCop;
		[MarshalAs(UnmanagedType.I1)] public bool SnesBreakOnWdm;
		[MarshalAs(UnmanagedType.I1)] public bool SnesBreakOnStp;
		[MarshalAs(UnmanagedType.I1)] public bool SnesUseAltSpcOpNames;
		[MarshalAs(UnmanagedType.I1)] public bool SnesIgnoreDspReadWrites;

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
		[MarshalAs(UnmanagedType.I1)] public bool NesBreakOnPpu2000ScrollGlitch;
		[MarshalAs(UnmanagedType.I1)] public bool NesBreakOnPpu2006ScrollGlitch;
		[MarshalAs(UnmanagedType.I1)] public bool NesBreakOnExtOutputMode;

		[MarshalAs(UnmanagedType.I1)] public bool PceBreakOnBrk;
		[MarshalAs(UnmanagedType.I1)] public bool PceBreakOnUnofficialOpCode;
		[MarshalAs(UnmanagedType.I1)] public bool PceBreakOnInvalidVramAddress;

		[MarshalAs(UnmanagedType.I1)] public bool SmsBreakOnNopLoad;
		
		[MarshalAs(UnmanagedType.I1)] public bool GbaBreakOnNopLoad;
		[MarshalAs(UnmanagedType.I1)] public bool GbaBreakOnInvalidOpCode;
		[MarshalAs(UnmanagedType.I1)] public bool GbaBreakOnUnalignedMemAccess;
		
		[MarshalAs(UnmanagedType.I1)] public bool WsBreakOnUndefinedOpCode;

		[MarshalAs(UnmanagedType.I1)] public bool ScriptAllowIoOsAccess;
		[MarshalAs(UnmanagedType.I1)] public bool ScriptAllowNetworkAccess;
		public UInt32 ScriptTimeout;
	}

	public enum RefreshSpeed
	{
		Off = 0,
		Low = 1,
		Normal = 2,
		High = 3
	}

}
