using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Controls;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Code
{
	public class CpuLineStyleProvider : ctrlTextbox.ILineStyleProvider
	{
		public CpuLineStyleProvider()
		{
		}

		public int? ActiveAddress { get; set; }

		public string GetLineComment(int lineNumber)
		{
			return null;
		}

		public static void ConfigureActiveStatement(LineProperties props)
		{
			props.FgColor = Color.Black;
			props.TextBgColor = ConfigManager.Config.Debug.CodeActiveStatementColor;
			props.Symbol |= LineSymbol.Arrow;
		}

		public LineProperties GetLineStyle(CodeLineData lineData, int lineIndex)
		{
			DebugInfo info = ConfigManager.Config.Debug;
			LineProperties props = new LineProperties();

			if(lineData.Address >= 0) {
				GetBreakpointLineProperties(props, lineData.Address);
			}

			bool isActiveStatement = ActiveAddress.HasValue && ActiveAddress.Value == lineData.Address;
			if(isActiveStatement) {
				ConfigureActiveStatement(props);
			}

			//TODO
			/* else if(_code._code.UnexecutedAddresses.Contains(lineNumber)) {
				props.LineBgColor = info.CodeUnexecutedCodeColor;
			}*/

			if(lineData.Flags.HasFlag(LineFlags.PrgRom)) {
				props.AddressColor = Color.Gray;
			} else if(lineData.Flags.HasFlag(LineFlags.WorkRam)) {
				props.AddressColor = Color.DarkBlue;
			} else if(lineData.Flags.HasFlag(LineFlags.SaveRam)) {
				props.AddressColor = Color.DarkRed;
			}

			if(lineData.Flags.HasFlag(LineFlags.VerifiedData)) {
				props.LineBgColor = info.CodeVerifiedDataColor;
			} else if(!lineData.Flags.HasFlag(LineFlags.VerifiedCode)) {
				props.LineBgColor = info.CodeUnidentifiedDataColor;
			}

			return props;
		}

		public static void GetBreakpointLineProperties(LineProperties props, int cpuAddress)
		{
			DebugInfo config = ConfigManager.Config.Debug;
			foreach(Breakpoint breakpoint in BreakpointManager.Breakpoints) {
				if(breakpoint.Matches((uint)cpuAddress, SnesMemoryType.CpuMemory)) {
					Color fgColor = Color.White;
					Color? bgColor = null;
					Color bpColor = breakpoint.BreakOnExec ? config.CodeExecBreakpointColor : (breakpoint.BreakOnWrite ? config.CodeWriteBreakpointColor : config.CodeReadBreakpointColor);
					Color outlineColor = bpColor;
					LineSymbol symbol;
					if(breakpoint.Enabled) {
						bgColor = bpColor;
						symbol = LineSymbol.Circle;
					} else {
						fgColor = Color.Black;
						symbol = LineSymbol.CircleOutline;
					}

					if(breakpoint.MarkEvent) {
						symbol |= LineSymbol.Mark;
					}

					if(!string.IsNullOrWhiteSpace(breakpoint.Condition)) {
						symbol |= LineSymbol.Plus;
					}

					props.FgColor = fgColor;
					props.TextBgColor = bgColor;
					props.OutlineColor = outlineColor;
					props.Symbol = symbol;
					return;
				}
			}
		}
	}
}
