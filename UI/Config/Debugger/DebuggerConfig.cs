using Avalonia;
using Avalonia.Media;
using Mesen.Debugger;
using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System.Reactive.Linq;
using System.Reactive;
using System.Collections.Generic;
using ReactiveUI;
using System;

namespace Mesen.Config
{
	public class DebuggerConfig : BaseWindowConfig<DebuggerConfig>
	{
		public DockEntryDefinition? SavedDockLayout { get; set; } = null;

		[Reactive] public bool ShowSettingsPanel { get; set; } = true;

		[Reactive] public bool ShowByteCode { get; set; } = false;
		[Reactive] public bool ShowMemoryValues { get; set; } = true;
		[Reactive] public bool UseLowerCaseDisassembly { get; set; } = false;
		
		[Reactive] public bool ShowJumpLabels { get; set; } = false;
		[Reactive] public AddressDisplayType AddressDisplayType { get; set; } = AddressDisplayType.CpuAddress;

		[Reactive] public bool DrawPartialFrame { get; set; } = false;

		[Reactive] public SnesDebuggerConfig Snes { get; set; } = new();
		[Reactive] public NesDebuggerConfig Nes { get; set; } = new();
		[Reactive] public GbDebuggerConfig Gameboy { get; set; } = new();
		[Reactive] public GbaDebuggerConfig Gba { get; set; } = new();
		[Reactive] public PceDebuggerConfig Pce { get; set; } = new();
		[Reactive] public SmsDebuggerConfig Sms { get; set; } = new();
		[Reactive] public WsDebuggerConfig Ws { get; set; } = new();

		[Reactive] public bool BreakOnUninitRead { get; set; } = false;
		[Reactive] public bool BreakOnOpen { get; set; } = true;
		[Reactive] public bool BreakOnPowerCycleReset { get; set; } = true;

		[Reactive] public bool AutoResetCdl { get; set; } = true;
		[Reactive] public bool DisableDefaultLabels { get; set; } = false;

		[Reactive] public bool UsePredictiveBreakpoints { get; set; } = true;
		[Reactive] public bool SingleBreakpointPerInstruction { get; set; } = true;

		[Reactive] public bool CopyAddresses { get; set; } = true;
		[Reactive] public bool CopyByteCode { get; set; } = true;
		[Reactive] public bool CopyComments { get; set; } = true;
		[Reactive] public bool CopyBlockHeaders { get; set; } = true;

		[Reactive] public bool KeepActiveStatementInCenter { get; set; } = false;

		[Reactive] public bool ShowMemoryMappings { get; set; } = true;
		
		[Reactive] public bool RefreshWhileRunning { get; set; } = false;

		[Reactive] public bool BringToFrontOnBreak { get; set; } = true;
		[Reactive] public bool BringToFrontOnPause { get; set; } = false;
		[Reactive] public bool FocusGameOnResume { get; set; } = false;

		[Reactive] public CodeDisplayMode UnidentifiedBlockDisplay { get; set; } = CodeDisplayMode.Hide;
		[Reactive] public CodeDisplayMode VerifiedDataDisplay { get; set; } = CodeDisplayMode.Hide;

		[Reactive] public int BreakOnValue { get; set; } = 0;
		[Reactive] public int BreakInCount { get; set; } = 1;
		[Reactive] public BreakInMetric BreakInMetric { get; set; } = BreakInMetric.CpuInstructions;

		[Reactive] public bool ShowSelectionLength { get; set; } = false;
		[Reactive] public WatchFormatStyle WatchFormat { get; set; } = WatchFormatStyle.Hex;

		[Reactive] public UInt32 CodeOpcodeColor  { get; set; } = Color.FromRgb(22, 37, 37).ToUInt32();
		[Reactive] public UInt32 CodeLabelDefinitionColor { get; set; } = Colors.Blue.ToUInt32();
		[Reactive] public UInt32 CodeImmediateColor { get; set; } = Colors.Chocolate.ToUInt32();
		[Reactive] public UInt32 CodeAddressColor { get; set; } = Colors.DarkRed.ToUInt32();
		[Reactive] public UInt32 CodeCommentColor { get; set; } = Colors.Green.ToUInt32();
		[Reactive] public UInt32 CodeEffectiveAddressColor { get; set; } = Colors.SteelBlue.ToUInt32();

		[Reactive] public UInt32 CodeVerifiedDataColor { get; set; } = Color.FromRgb(255, 252, 236).ToUInt32();
		[Reactive] public UInt32 CodeUnidentifiedDataColor { get; set; } = Color.FromRgb(255, 242, 242).ToUInt32();
		[Reactive] public UInt32 CodeUnexecutedCodeColor { get; set; } = Color.FromRgb(225, 244, 228).ToUInt32();

		[Reactive] public UInt32 CodeExecBreakpointColor { get; set; } = Color.FromRgb(140, 40, 40).ToUInt32();
		[Reactive] public UInt32 CodeWriteBreakpointColor { get; set; } = Color.FromRgb(40, 120, 80).ToUInt32();
		[Reactive] public UInt32 CodeReadBreakpointColor { get; set; } = Color.FromRgb(40, 40, 200).ToUInt32();
		[Reactive] public UInt32 ForbidBreakpointColor { get; set; } = Color.FromRgb(115, 115, 115).ToUInt32();
		
		[Reactive] public UInt32 CodeActiveStatementColor { get; set; } = Colors.Yellow.ToUInt32();
		[Reactive] public UInt32 CodeActiveMidInstructionColor { get; set; } = Color.FromRgb(255, 220, 40).ToUInt32();

		[Reactive] public List<int> LabelListColumnWidths { get; set; } = new();
		[Reactive] public List<int> FunctionListColumnWidths { get; set; } = new();
		[Reactive] public List<int> BreakpointListColumnWidths { get; set; } = new();
		[Reactive] public List<int> WatchListColumnWidths { get; set; } = new();
		[Reactive] public List<int> CallStackColumnWidths { get; set; } = new();
		[Reactive] public List<int> FindResultColumnWidths { get; set; } = new();

		public DebuggerConfig()
		{
		}
	}

	public class CfgColor : ReactiveObject
	{
		[Reactive] public UInt32 ColorCode { get; set; }
	}

	public enum BreakInMetric
	{
		CpuInstructions,
		PpuCycles,
		Scanlines,
		Frames
	}

	public enum CodeDisplayMode
	{
		Hide,
		Show,
		Disassemble
	}

	public enum AddressDisplayType
	{
		CpuAddress,
		AbsAddress,
		Both,
		BothCompact
	}
}
