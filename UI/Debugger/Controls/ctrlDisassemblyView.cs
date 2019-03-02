using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Controls;
using Mesen.GUI.Config;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlDisassemblyView : BaseControl
	{
		private LineStyleProvider _styleProvider;

		public ctrlDisassemblyView()
		{
			InitializeComponent();

			_styleProvider = new LineStyleProvider();
			ctrlCode.StyleProvider = _styleProvider;
			ctrlCode.ShowContentNotes = true;
			ctrlCode.ShowMemoryValues = true;
		}

		public void SetActiveAddress(int? address)
		{
			UpdateCode();

			_styleProvider.ActiveAddress = address;
			if(address.HasValue && address.Value >= 0) {
				ctrlCode.ScrollToAddress(address.Value);
			}
		}

		private void UpdateCode()
		{
			CodeDataProvider provider = new CodeDataProvider();

			int centerLineIndex = ctrlCode.GetLineIndexAtPosition(0) + ctrlCode.GetNumberVisibleLines() / 2;
			int centerLineAddress;
			int scrollOffset = -1;
			do {
				//Save the address at the center of the debug view
				centerLineAddress = provider.GetLineAddress(centerLineIndex);
				centerLineIndex--;
				scrollOffset++;
			} while(centerLineAddress < 0 && centerLineIndex > 0);

			ctrlCode.DataProvider = provider;

			if(centerLineAddress >= 0) {
				//Scroll to the same address as before, to prevent the code view from changing due to setting or banking changes, etc.
				int lineIndex = provider.GetLineIndex((UInt32)centerLineAddress) + scrollOffset;
				ctrlCode.ScrollToLineIndex(lineIndex, eHistoryType.None, false, true);
			}
		}

		public class CodeDataProvider : ICodeDataProvider
		{
			private int _lineCount;

			public CodeDataProvider()
			{
				_lineCount = (int)DebugApi.GetDisassemblyLineCount();
			}

			public CodeLineData GetCodeLineData(int lineIndex)
			{
				return DebugApi.GetDisassemblyLineData((UInt32)lineIndex);
			}

			public int GetLineAddress(int lineIndex)
			{
				return DebugApi.GetDisassemblyLineData((UInt32)lineIndex).Address;
			}

			public int GetLineCount()
			{
				return _lineCount;
			}

			public int GetLineIndex(uint cpuAddress)
			{
				return (int)DebugApi.GetDisassemblyLineIndex(cpuAddress);
			}

			public bool UseOptimizedSearch { get { return true; } }

			public int GetNextResult(string searchString, int startPosition, int endPosition, bool searchBackwards)
			{
				return DebugApi.SearchDisassembly(searchString, startPosition, endPosition, searchBackwards);
			}
		}

		public class LineStyleProvider : ctrlTextbox.ILineStyleProvider
		{
			public LineStyleProvider()
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
}
