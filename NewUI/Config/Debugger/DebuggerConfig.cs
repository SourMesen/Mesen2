using Avalonia;
using Avalonia.Media;
using Mesen.Debugger;
using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System.Reactive.Linq;
using System.Reactive;

namespace Mesen.Config
{
	public class DebuggerConfig : BaseWindowConfig<DebuggerConfig>
	{
		public DockEntryDefinition? SavedDockLayout { get; set; } = null;

		[Reactive] public bool ShowSettingsPanel { get; set; } = true;

		[Reactive] public bool ShowByteCode { get; set; } = false;
		[Reactive] public bool UseLowerCaseDisassembly { get; set; } = false;
		
		[Reactive] public bool ShowJumpLabels { get; set; } = false;
		[Reactive] public AddressDisplayType AddressDisplayType { get; set; } = AddressDisplayType.CpuAddress;

		[Reactive] public bool DrawPartialFrame { get; set; } = false;

		[Reactive] public SnesDebuggerConfig Snes { get; set; } = new();
		[Reactive] public NesDebuggerConfig Nes { get; set; } = new();
		[Reactive] public GbDebuggerConfig Gameboy { get; set; } = new();
		[Reactive] public PceDebuggerConfig Pce { get; set; } = new();

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

		[Reactive] public CodeDisplayMode UnidentifiedBlockDisplay { get; set; } = CodeDisplayMode.Hide;
		[Reactive] public CodeDisplayMode VerifiedDataDisplay { get; set; } = CodeDisplayMode.Hide;

		[Reactive] public int BreakOnValue { get; set; } = 0;
		[Reactive] public int BreakInCount { get; set; } = 1;
		[Reactive] public BreakInMetric BreakInMetric { get; set; } = BreakInMetric.CpuInstructions;

		[Reactive] public bool ShowSelectionLength { get; set; } = false;
		[Reactive] public WatchFormatStyle WatchFormat { get; set; } = WatchFormatStyle.Hex;

		[Reactive] public Color CodeOpcodeColor  { get; set; } = Color.FromRgb(22, 37, 37);
		[Reactive] public Color CodeLabelDefinitionColor { get; set; } = Colors.Blue;
		[Reactive] public Color CodeImmediateColor { get; set; } = Colors.Chocolate;
		[Reactive] public Color CodeAddressColor { get; set; } = Colors.DarkRed;
		[Reactive] public Color CodeCommentColor { get; set; } = Colors.Green;
		[Reactive] public Color CodeEffectiveAddressColor { get; set; } = Colors.SteelBlue;

		[Reactive] public Color CodeVerifiedDataColor { get; set; } = Color.FromRgb(255, 252, 236);
		[Reactive] public Color CodeUnidentifiedDataColor { get; set; } = Color.FromRgb(255, 242, 242);
		[Reactive] public Color CodeUnexecutedCodeColor { get; set; } = Color.FromRgb(225, 244, 228);

		[Reactive] public Color CodeExecBreakpointColor { get; set; } = Color.FromRgb(140, 40, 40);
		[Reactive] public Color CodeWriteBreakpointColor { get; set; } = Color.FromRgb(40, 120, 80);
		[Reactive] public Color CodeReadBreakpointColor { get; set; } = Color.FromRgb(40, 40, 200);
		
		[Reactive] public Color CodeActiveStatementColor { get; set; } = Colors.Yellow;
		[Reactive] public Color CodeActiveMidInstructionColor { get; set; } = Color.FromRgb(255, 220, 40);

		public DebuggerConfig()
		{
		}
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
