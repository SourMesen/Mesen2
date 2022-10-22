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
using Mesen.Utilities;

namespace Mesen.Debugger.Disassembly
{
	public class BaseStyleProvider : ILineStyleProvider
	{
		public int AddressSize { get; }
		public int ByteCodeSize { get; }
		public CpuType CpuType { get; }

		public BaseStyleProvider(CpuType cpuType)
		{
			CpuType = cpuType;
			AddressSize = cpuType.GetAddressSize();
			ByteCodeSize = cpuType.GetByteCodeSize();
		}

		private void ConfigureActiveStatement(LineProperties props)
		{
			DebuggerFeatures features = DebugApi.GetDebuggerFeatures(CpuType);

			if(features.ChangeProgramCounter) {
				props.TextBgColor = ConfigManager.Config.Debug.Debugger.CodeActiveStatementColor;
			} else {
				props.TextBgColor = ConfigManager.Config.Debug.Debugger.CodeActiveMidInstructionColor;
			}
			props.FgColor = ColorHelper.GetContrastTextColor(props.TextBgColor.Value);
			props.Symbol |= LineSymbol.Arrow;

			if(!features.ChangeProgramCounter && features.CpuCycleStep) {
				CpuInstructionProgress instProgress = DebugApi.GetInstructionProgress(CpuType);
				if(instProgress.CurrentCycle > instProgress.StartCycle) {
					LineProgress progress = new() {
						Current = (int)Math.Max(1, instProgress.CurrentCycle - instProgress.StartCycle),
						CpuProgress = instProgress
					};

					switch(instProgress.LastMemOperation.Type) {
						case MemoryOperationType.Read: progress.Color = Color.FromRgb(150, 176, 255); progress.Text = "R"; break;
						case MemoryOperationType.Write: progress.Color = Color.FromRgb(255, 171, 150); progress.Text = "W"; break;
						case MemoryOperationType.DummyRead: progress.Color = Color.FromRgb(184, 160, 255); progress.Text = "DR"; break;
						case MemoryOperationType.DummyWrite: progress.Color = Color.FromRgb(255, 245, 137); progress.Text = "DW"; break;
						case MemoryOperationType.DmaRead: progress.Color = Color.FromRgb(255, 160, 221); progress.Text = "DMAR"; break;
						case MemoryOperationType.DmaWrite: progress.Color = Color.FromRgb(255, 160, 221); progress.Text = "DMAW"; break;
						case MemoryOperationType.Idle: progress.Color = Color.FromRgb(240, 240, 240); progress.Text = "I"; break;
						default: progress.Color = Color.FromRgb(143, 255, 173); progress.Text = "X"; break;
					}

					props.Progress = progress;
				}
			}
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

			if((lineData.HasAddress || lineData.AbsoluteAddress.Address >= 0) && !lineData.IsAddressHidden) {
				GetBreakpointLineProperties(props, lineData.Address, lineData.CpuType, lineData.AbsoluteAddress, lineData.Flags.HasFlag(LineFlags.ShowAsData));
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

		public void GetBreakpointLineProperties(LineProperties props, int cpuAddress, CpuType cpuType, AddressInfo absAddress, bool showSymbolOnly)
		{
			MemoryType relMemoryType = cpuType.ToMemoryType();
			if(absAddress.Address < 0) {
				absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = cpuAddress, Type = relMemoryType });
			}
			foreach(Breakpoint breakpoint in BreakpointManager.Breakpoints) {
				if(breakpoint.Matches((uint)cpuAddress, relMemoryType, cpuType) || (absAddress.Address >= 0 && breakpoint.Matches((uint)absAddress.Address, absAddress.Type, cpuType))) {
					SetBreakpointLineProperties(props, breakpoint, showSymbolOnly);
				}
			}
		}

		protected void SetBreakpointLineProperties(LineProperties props, Breakpoint breakpoint, bool showSymbolOnly)
		{
			Color? bgColor = null;
			Color bpColor = breakpoint.GetColor();
			Color outlineColor = bpColor;
			LineSymbol symbol;
			if(breakpoint.Enabled) {
				bgColor = bpColor;
				symbol = LineSymbol.Circle;
			} else {
				symbol = LineSymbol.CircleOutline;
			}

			if(breakpoint.MarkEvent) {
				symbol |= LineSymbol.Mark;
			}

			if(!string.IsNullOrWhiteSpace(breakpoint.Condition)) {
				symbol |= LineSymbol.Plus;
			}

			if(!showSymbolOnly) {
				props.TextBgColor = bgColor;
				props.FgColor = bgColor != null ? ColorHelper.GetContrastTextColor(bgColor.Value) : null;
			}
			props.OutlineColor = outlineColor;
			props.Symbol = symbol;
		}

		public List<CodeColor> GetCodeColors(CodeLineData lineData, bool highlightCode, string addressFormat, Color? textColor, bool showMemoryValues)
		{
			return CodeHighlighting.GetCpuHighlights(lineData, highlightCode, addressFormat, textColor, showMemoryValues);
		}
	}
}
