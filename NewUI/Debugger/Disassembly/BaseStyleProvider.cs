using Avalonia.Media;
using Mesen.Debugger.Controls;
using Mesen.Config;
using Mesen.Debugger;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Mesen.Interop;

namespace Mesen.Debugger.Disassembly
{
	public class BaseStyleProvider : ILineStyleProvider
	{
		public int AddressSize { get; }
		public int ByteCodeSize { get; }

		public BaseStyleProvider(CpuType cpuType)
		{
			AddressSize = cpuType.GetAddressSize();
			ByteCodeSize = cpuType.GetByteCodeSize();
		}

		private static void ConfigureActiveStatement(LineProperties props)
		{
			props.FgColor = Colors.Black;
			props.TextBgColor = ConfigManager.Config.Debug.Debugger.CodeActiveStatementColor;
			props.Symbol |= LineSymbol.Arrow;
		}

		public virtual bool IsLineActive(CodeLineData line, int lineIndex)
		{
			return false;
		}

		public virtual bool IsLineSelected(CodeLineData line, int lineIndex)
		{
			return false;
		}

		public virtual bool IsLineFocused(CodeLineData line, int lineIndex)
		{
			return false;
		}

		public LineProperties GetLineStyle(CodeLineData lineData, int lineIndex)
		{
			DebuggerConfig cfg = ConfigManager.Config.Debug.Debugger;
			LineProperties props = new LineProperties();

			if((lineData.HasAddress || lineData.AbsoluteAddress.Address >= 0) && !lineData.Flags.HasFlag(LineFlags.Empty) && !lineData.Flags.HasFlag(LineFlags.Label)) {
				GetBreakpointLineProperties(props, lineData.Address, lineData.CpuType, lineData.AbsoluteAddress);
			}

			if(IsLineActive(lineData, lineIndex)) {
				ConfigureActiveStatement(props);
			}

			props.IsSelectedRow = IsLineSelected(lineData, lineIndex);
			props.IsActiveRow = IsLineFocused(lineData, lineIndex);
			
			if(lineData.Flags.HasFlag(LineFlags.PrgRom)) {
				props.AddressColor = Colors.Gray;
			} else if(lineData.Flags.HasFlag(LineFlags.WorkRam)) {
				props.AddressColor = Colors.DarkBlue;
			} else if(lineData.Flags.HasFlag(LineFlags.SaveRam)) {
				props.AddressColor = Colors.DarkRed;
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

		public void GetBreakpointLineProperties(LineProperties props, int cpuAddress, CpuType cpuType, AddressInfo absAddress)
		{
			MemoryType relMemoryType = cpuType.ToMemoryType();
			if(absAddress.Address < 0) {
				absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = cpuAddress, Type = relMemoryType });
			}
			foreach(Breakpoint breakpoint in BreakpointManager.Breakpoints) {
				if(breakpoint.Matches((uint)cpuAddress, relMemoryType, cpuType) || (absAddress.Address >= 0 && breakpoint.Matches((uint)absAddress.Address, absAddress.Type, cpuType))) {
					SetBreakpointLineProperties(props, breakpoint);
				}
			}
		}

		protected void SetBreakpointLineProperties(LineProperties props, Breakpoint breakpoint)
		{
			Color fgColor = Colors.White;
			Color? bgColor = null;
			Color bpColor = breakpoint.GetColor();
			Color outlineColor = bpColor;
			LineSymbol symbol;
			if(breakpoint.Enabled) {
				bgColor = bpColor;
				symbol = LineSymbol.Circle;
			} else {
				fgColor = Colors.Black;
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
		}

		public List<CodeColor> GetCodeColors(CodeLineData lineData, bool highlightCode, string addressFormat, Color? textColor, bool showMemoryValues)
		{
			return CodeHighlighting.GetCpuHighlights(lineData, highlightCode, addressFormat, textColor, showMemoryValues);
		}
	}
}
