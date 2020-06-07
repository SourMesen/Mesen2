using Mesen.GUI.Controls;
using Mesen.GUI.Debugger;
using Mesen.GUI.Utilities;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public class DebuggerInfo
	{
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

		public string FontFamily = BaseControl.MonospaceFontFamily;
		public FontStyle FontStyle = FontStyle.Regular;
		public float FontSize = BaseControl.DefaultFontSize;
		public int TextZoom = 100;

		public bool ShowSelectionLength = false;
		public WatchFormatStyle WatchFormat = WatchFormatStyle.Hex;

		public bool ShowCommentsInLabelList = true;

		public XmlColor CodeOpcodeColor = Color.FromArgb(22, 37, 37);
		public XmlColor CodeLabelDefinitionColor = Color.Blue;
		public XmlColor CodeImmediateColor = Color.Chocolate;
		public XmlColor CodeAddressColor = Color.DarkRed;
		public XmlColor CodeCommentColor = Color.Green;
		public XmlColor CodeEffectiveAddressColor = Color.SteelBlue;

		public XmlColor CodeVerifiedDataColor = Color.FromArgb(255, 252, 236);
		public XmlColor CodeUnidentifiedDataColor = Color.FromArgb(255, 242, 242);
		public XmlColor CodeUnexecutedCodeColor = Color.FromArgb(225, 244, 228);

		public XmlColor CodeExecBreakpointColor = Color.FromArgb(140, 40, 40);
		public XmlColor CodeWriteBreakpointColor = Color.FromArgb(40, 120, 80);
		public XmlColor CodeReadBreakpointColor = Color.FromArgb(40, 40, 200);
		public XmlColor CodeActiveStatementColor = Color.Yellow;

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
