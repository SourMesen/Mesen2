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
	public class GbLineStyleProvider : BaseStyleProvider
	{
		public GbLineStyleProvider()
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

			if(lineData.Flags.HasFlag(LineFlags.VerifiedData)) {
				props.LineBgColor = cfg.CodeVerifiedDataColor;
			} else if(!lineData.Flags.HasFlag(LineFlags.VerifiedCode)) {
				props.LineBgColor = cfg.CodeUnidentifiedDataColor;
			}

			return props;
		}

		private void GetBreakpointLineProperties(LineProperties props, int cpuAddress)
		{
			AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = cpuAddress, Type = SnesMemoryType.GameboyMemory });
			foreach(Breakpoint breakpoint in BreakpointManager.Breakpoints) {
				if(breakpoint.Matches((uint)cpuAddress, SnesMemoryType.GameboyMemory, CpuType.Gameboy) || (absAddress.Address >= 0 && breakpoint.Matches((uint)absAddress.Address, absAddress.Type, CpuType.Gameboy))) {
					SetBreakpointLineProperties(props, breakpoint);
					return;
				}
			}
		}
	}
}
