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
		public static string MonospaceFontFamily = "Consolas";
		public static int DefaultFontSize = 14;
		
		[Reactive] public bool ShowSettingsPanel { get; set; } = true;

		[Reactive] public bool ShowByteCode { get; set; } = false;
		[Reactive] public bool UseLowerCaseDisassembly { get; set; } = false;

		[Reactive] public SnesDebuggerConfig Snes { get; set; } = new();
		[Reactive] public NesDebuggerConfig Nes { get; set; } = new();
		[Reactive] public GbDebuggerConfig Gameboy { get; set; } = new();

		[Reactive] public bool BreakOnUninitRead { get; set; } = false;
		[Reactive] public bool BreakOnOpen { get; set; } = true;
		[Reactive] public bool BreakOnPowerCycleReset { get; set; } = true;

		[Reactive] public bool AutoResetCdl { get; set; } = true;

		[Reactive] public bool ShowMemoryMappings { get; set; } = true;

		[Reactive] public bool BringToFrontOnBreak { get; set; } = true;

		[Reactive] public CodeDisplayMode UnidentifiedBlockDisplay { get; set; } = CodeDisplayMode.Hide;
		[Reactive] public CodeDisplayMode VerifiedDataDisplay { get; set; } = CodeDisplayMode.Hide;

		[Reactive] public bool UseAltSpcOpNames { get; set; } = false;

		[Reactive] public int BreakOnValue { get; set; } = 0;
		[Reactive] public int BreakInCount { get; set; } = 1;
		[Reactive] public BreakInMetric BreakInMetric { get; set; } = BreakInMetric.CpuInstructions;

		[Reactive] public FontConfig Font { get; set; } = new FontConfig();

		[Reactive] public bool ShowSelectionLength { get; set; } = false;
		[Reactive] public WatchFormatStyle WatchFormat { get; set; } = WatchFormatStyle.Hex;

		[Reactive] public bool ShowCommentsInLabelList { get; set; } = true;

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

		public DebuggerConfig()
		{
		}

		public void ApplyConfig()
		{
			Snes.ApplyConfig();
			Nes.ApplyConfig();
			Gameboy.ApplyConfig();

			ConfigApi.SetDebuggerFlag(DebuggerFlags.BreakOnUninitRead, BreakOnUninitRead);

			ConfigApi.SetDebuggerFlag(DebuggerFlags.ShowUnidentifiedData, UnidentifiedBlockDisplay == CodeDisplayMode.Show);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.DisassembleUnidentifiedData, UnidentifiedBlockDisplay == CodeDisplayMode.Disassemble);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.ShowVerifiedData, VerifiedDataDisplay == CodeDisplayMode.Show);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.DisassembleVerifiedData, VerifiedDataDisplay == CodeDisplayMode.Disassemble);

			ConfigApi.SetDebuggerFlag(DebuggerFlags.UseAltSpcOpNames, UseAltSpcOpNames);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.UseLowerCaseDisassembly, UseLowerCaseDisassembly);

			ConfigApi.SetDebuggerFlag(DebuggerFlags.AutoResetCdl, AutoResetCdl);
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
}
