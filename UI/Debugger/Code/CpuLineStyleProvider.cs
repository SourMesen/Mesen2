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
	public class CpuLineStyleProvider : BaseStyleProvider
	{
		public CpuLineStyleProvider()
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

			if(lineData.Flags.HasFlag(LineFlags.PrgRom)) {
				props.AddressColor = Color.Gray;
			} else if(lineData.Flags.HasFlag(LineFlags.WorkRam)) {
				props.AddressColor = Color.DarkBlue;
			} else if(lineData.Flags.HasFlag(LineFlags.SaveRam)) {
				props.AddressColor = Color.DarkRed;
			}

			if(lineData.Flags.HasFlag(LineFlags.VerifiedData)) {
				props.LineBgColor = cfg.CodeVerifiedDataColor;
			} else if(lineData.Flags.HasFlag(LineFlags.UnexecutedCode)) {
				props.LineBgColor = cfg.CodeUnexecutedCodeColor;
			} else if(!lineData.Flags.HasFlag(LineFlags.VerifiedCode)) {
				props.LineBgColor = cfg.CodeUnidentifiedDataColor;
			}

			return props;
		}

		public void GetBreakpointLineProperties(LineProperties props, int cpuAddress)
		{
			DebuggerInfo config = ConfigManager.Config.Debug.Debugger;
			AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = cpuAddress, Type = SnesMemoryType.CpuMemory });
			foreach(Breakpoint breakpoint in BreakpointManager.Breakpoints) {
				if(breakpoint.Matches((uint)cpuAddress, SnesMemoryType.CpuMemory, CpuType.Cpu) || (absAddress.Address >= 0 && breakpoint.Matches((uint)absAddress.Address, absAddress.Type, CpuType.Cpu))) {
					SetBreakpointLineProperties(props, breakpoint);
				}
			}
		}
	}
}
