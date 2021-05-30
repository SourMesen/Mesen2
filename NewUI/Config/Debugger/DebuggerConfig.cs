using Avalonia;
using Avalonia.Media;
using Mesen.Debugger;
using Mesen.Interop;

namespace Mesen.Config
{
	public class DebuggerConfig
	{
		public static string MonospaceFontFamily = "Consolas";
		public static int DefaultFontSize = 14;

		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;
		public int? SplitterDistance = null;

		public bool ShowByteCode = false;
		public bool UseLowerCaseDisassembly = false;

		public bool BreakOnBrk = false;
		public bool BreakOnCop = false;
		public bool BreakOnWdm = false;
		public bool BreakOnStp = false;
		public bool BreakOnUninitRead = false;

		public bool GbBreakOnInvalidOamAccess = false;
		public bool GbBreakOnInvalidVramAccess = false;
		public bool GbBreakOnDisableLcdOutsideVblank = false;
		public bool GbBreakOnInvalidOpCode = false;
		public bool GbBreakOnNopLoad = false;
		public bool GbBreakOnOamCorruption = false;

		public bool BreakOnOpen = true;
		public bool BreakOnPowerCycleReset = true;
		
		public bool AutoResetCdl = true;

		public bool ShowMemoryMappings = true;

		public bool BringToFrontOnBreak = true;
		public bool BringToFrontOnPause = false;

		public CodeDisplayMode UnidentifiedBlockDisplay = CodeDisplayMode.Hide;
		public CodeDisplayMode VerifiedDataDisplay = CodeDisplayMode.Hide;

		public bool UseAltSpcOpNames = false;

		public int BreakOnValue = 0;
		public int BreakInCount = 1;
		public BreakInMetric BreakInMetric = BreakInMetric.CpuInstructions;

		public string FontFamily = DebuggerConfig.MonospaceFontFamily;
		public FontStyle FontStyle = FontStyle.Normal;
		public float FontSize = DebuggerConfig.DefaultFontSize;
		public int TextZoom = 100;

		public bool ShowSelectionLength = false;
		public WatchFormatStyle WatchFormat = WatchFormatStyle.Hex;

		public bool ShowCommentsInLabelList = true;

		public Color CodeOpcodeColor = Color.FromRgb(22, 37, 37);
		public Color CodeLabelDefinitionColor = Colors.Blue;
		public Color CodeImmediateColor = Colors.Chocolate;
		public Color CodeAddressColor = Colors.DarkRed;
		public Color CodeCommentColor = Colors.Green;
		public Color CodeEffectiveAddressColor = Colors.SteelBlue;

		public Color CodeVerifiedDataColor = Color.FromRgb(255, 252, 236);
		public Color CodeUnidentifiedDataColor = Color.FromRgb(255, 242, 242);
		public Color CodeUnexecutedCodeColor = Color.FromRgb(225, 244, 228);

		public Color CodeExecBreakpointColor = Color.FromRgb(140, 40, 40);
		public Color CodeWriteBreakpointColor = Color.FromRgb(40, 120, 80);
		public Color CodeReadBreakpointColor = Color.FromRgb(40, 40, 200);
		public Color CodeActiveStatementColor = Colors.Yellow;

		public void ApplyConfig()
		{
			ConfigApi.SetDebuggerFlag(DebuggerFlags.BreakOnBrk, BreakOnBrk);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.BreakOnCop, BreakOnCop);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.BreakOnWdm, BreakOnWdm);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.BreakOnStp, BreakOnStp);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.BreakOnUninitRead, BreakOnUninitRead);

			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnInvalidOamAccess, GbBreakOnInvalidOamAccess);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnInvalidVramAccess, GbBreakOnInvalidVramAccess);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnDisableLcdOutsideVblank, GbBreakOnDisableLcdOutsideVblank);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnInvalidOpCode, GbBreakOnInvalidOpCode);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnNopLoad, GbBreakOnNopLoad);
			ConfigApi.SetDebuggerFlag(DebuggerFlags.GbBreakOnOamCorruption, GbBreakOnOamCorruption);

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
