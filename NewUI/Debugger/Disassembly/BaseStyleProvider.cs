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
using Mesen.Debugger.ViewModels;

namespace Mesen.Debugger.Disassembly
{
	public class BaseStyleProvider : ILineStyleProvider
	{
		private DisassemblyViewModel? _model { get; set; }

		public BaseStyleProvider(DisassemblyViewModel? model = null)
		{
			_model = model;
		}

		private static void ConfigureActiveStatement(LineProperties props)
		{
			props.FgColor = Colors.Black;
			props.TextBgColor = ConfigManager.Config.Debug.Debugger.CodeActiveStatementColor;
			props.Symbol |= LineSymbol.Arrow;
		}

		public LineProperties GetLineStyle(CodeLineData lineData, int lineIndex)
		{
			DebuggerConfig cfg = ConfigManager.Config.Debug.Debugger;
			LineProperties props = new LineProperties();

			if(lineData.Address >= 0) {
				GetBreakpointLineProperties(props, lineData.Address, lineData.CpuType);
			}

			if(_model != null) {
				bool isActiveStatement = _model.ActiveAddress.HasValue && _model.ActiveAddress.Value == lineData.Address;
				if(isActiveStatement) {
					ConfigureActiveStatement(props);
				}

				if(!lineData.Flags.HasFlag(LineFlags.Empty)) {
					props.IsSelectedRow = lineData.Address >= _model.SelectionStart && lineData.Address <= _model.SelectionEnd;
					props.IsActiveRow = _model.SelectedRowAddress == lineData.Address;
				}
			}

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

		public void GetBreakpointLineProperties(LineProperties props, int cpuAddress, CpuType cpuType)
		{
			MemoryType relMemoryType = cpuType.ToMemoryType();
			AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = cpuAddress, Type = relMemoryType });
			foreach(Breakpoint breakpoint in BreakpointManager.Breakpoints) {
				if(breakpoint.Matches((uint)cpuAddress, relMemoryType, cpuType) || (absAddress.Address >= 0 && breakpoint.Matches((uint)absAddress.Address, absAddress.Type, cpuType))) {
					SetBreakpointLineProperties(props, breakpoint);
				}
			}
		}

		protected void SetBreakpointLineProperties(LineProperties props, Breakpoint breakpoint)
		{
			DebuggerConfig config = ConfigManager.Config.Debug.Debugger;
			Color fgColor = Colors.White;
			Color? bgColor = null;
			Color bpColor = breakpoint.BreakOnExec ? config.CodeExecBreakpointColor : (breakpoint.BreakOnWrite ? config.CodeWriteBreakpointColor : config.CodeReadBreakpointColor);
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
