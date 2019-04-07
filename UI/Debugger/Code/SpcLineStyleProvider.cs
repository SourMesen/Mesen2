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
	public class SpcLineStyleProvider : BaseStyleProvider
	{
		public SpcLineStyleProvider()
		{
		}

		public override string GetLineComment(int lineNumber)
		{
			return null;
		}

		public static void ConfigureActiveStatement(LineProperties props)
		{
			props.FgColor = Color.Black;
			props.TextBgColor = ConfigManager.Config.Debug.Debugger.CodeActiveStatementColor;
			props.Symbol |= LineSymbol.Arrow;
		}

		public override LineProperties GetLineStyle(CodeLineData lineData, int lineIndex)
		{
			DebuggerInfo cfg = ConfigManager.Config.Debug.Debugger;
			LineProperties props = new LineProperties();

			if(lineData.Address >= 0) {
				GetBreakpointLineProperties(props, lineData.Address);
			}

			bool isActiveStatement = ActiveAddress.HasValue && ActiveAddress.Value == lineData.Address;
			if(isActiveStatement) {
				ConfigureActiveStatement(props);
			}
			
			return props;
		}

		private void GetBreakpointLineProperties(LineProperties props, int cpuAddress)
		{
			foreach(Breakpoint breakpoint in BreakpointManager.Breakpoints) {
				if(breakpoint.Matches((uint)cpuAddress, SnesMemoryType.SpcMemory)) {
					SetBreakpointLineProperties(props, breakpoint);
					return;
				}
			}
		}
	}
}
