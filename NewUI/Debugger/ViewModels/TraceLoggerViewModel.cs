using Avalonia.Controls;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Text;

namespace Mesen.Debugger.ViewModels
{
	public class TraceLoggerViewModel : DisposableViewModel
	{
		public const int TraceLogBufferSize = 30000;

		public TraceLoggerConfig Config { get; }
		[Reactive] public TraceLoggerCodeDataProvider DataProvider { get; set; }
		[Reactive] public TraceLoggerStyleProvider StyleProvider { get; set; } = new TraceLoggerStyleProvider();
		[Reactive] public CodeLineData[] TraceLogLines { get; set; } = Array.Empty<CodeLineData>();
		[Reactive] public int VisibleRowCount { get; set; } = 100;
		[Reactive] public int ScrollPosition { get; set; } = 0;
		[Reactive] public int MaxScrollPosition { get; set; } = TraceLoggerViewModel.TraceLogBufferSize;
		[Reactive] public bool IsLoggingToFile { get; set; } = false;

		[Reactive] public List<TraceLoggerOptionTab> Tabs { get; set; } = new List<TraceLoggerOptionTab>();

		public TraceLoggerViewModel()
		{
			Config = ConfigManager.Config.Debug.TraceLogger;
			DataProvider = new TraceLoggerCodeDataProvider();

			if(Design.IsDesignMode) {
				return;
			}

			UpdateAvailableTabs();
			UpdateCoreOptions();

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => UpdateCoreOptions()));

			AddDisposable(this.WhenAnyValue(x => x.ScrollPosition).Subscribe(x => {
				ScrollPosition = Math.Max(0, Math.Min(x, MaxScrollPosition));
				UpdateTraceLogLines();
			}));

			AddDisposable(this.WhenAnyValue(x => x.MaxScrollPosition).Subscribe(x => {
				ScrollPosition = Math.Min(ScrollPosition, MaxScrollPosition);
			}));
		}

		public void UpdateAvailableTabs()
		{
			List<TraceLoggerOptionTab> tabs = new();
			RomInfo romInfo = EmuApi.GetRomInfo();
			StyleProvider.SetConsoleType(romInfo.ConsoleType);
			foreach(CpuType type in romInfo.CpuTypes) {
				tabs.Add(new TraceLoggerOptionTab() {
					TabName = ResourceHelper.GetEnumText(type),
					CpuType = type,
					Options = Config.CpuConfig[type]
				});
			}

			Tabs = tabs;
		}

		private void UpdateCoreOptions()
		{
			foreach(TraceLoggerOptionTab tab in Tabs) {
				InteropTraceLoggerOptions options = new InteropTraceLoggerOptions() {
					Enabled = tab.Options.Enabled,
					UseLabels = true,
					Format = Encoding.UTF8.GetBytes(tab.Options.Format),
					Condition = Encoding.UTF8.GetBytes(tab.Options.Condition)
				};

				Array.Resize(ref options.Condition, 1000);
				Array.Resize(ref options.Format, 1000);

				DebugApi.SetTraceOptions(tab.CpuType, options);
			}
		}

		public void UpdateLog()
		{
			UpdateTraceLogLines();
		}

		private void UpdateTraceLogLines()
		{
			TraceLogLines = DataProvider.GetCodeLines(ScrollPosition, VisibleRowCount);
		}

		public void Scroll(int offset)
		{
			ScrollPosition += offset;
		}

		public void ScrollToTop()
		{
			ScrollPosition = 0;
		}

		public void ScrollToBottom()
		{
			ScrollPosition = MaxScrollPosition;
		}
	}

	public class TraceLoggerOptionTab
	{
		[Reactive] public string TabName { get; set; } = "";
		[Reactive] public CpuType CpuType { get; set; } = CpuType.Snes;

		public TraceLoggerCpuConfig Options { get; set; } = new TraceLoggerCpuConfig();
	}

	public class TraceLoggerCodeDataProvider : ICodeDataProvider
	{
		public TraceLoggerCodeDataProvider()
		{
		}

		public bool UseOptimizedSearch { get { return false; } }

		public CpuType CpuType => CpuType.Snes;

		public int GetLineCount()
		{
			return TraceLoggerViewModel.TraceLogBufferSize;
		}

		public int GetNextResult(string searchString, int startPosition, int endPosition, bool searchBackwards)
		{
			throw new NotImplementedException();
		}

		public CodeLineData[] GetCodeLines(int startIndex, int rowCount)
		{
			TraceRow[] rows = DebugApi.GetExecutionTrace((uint)(TraceLoggerViewModel.TraceLogBufferSize - startIndex - rowCount), (uint)rowCount);

			List<CodeLineData> lines = new(rowCount);
			for(int i = 0; i < rows.Length; i++) {
				lines.Insert(0, new CodeLineData(rows[i].Type) {
					Address = (int)rows[i].ProgramCounter,
					Text = rows[i].GetOutput(),
					ByteCode = rows[i].GetByteCode(),
					EffectiveAddress = -1
				});
			}

			while(lines.Count < rowCount) {
				lines.Insert(0, new CodeLineData(CpuType.Snes));
			}

			return lines.ToArray();
		}

		public int GetRowAddress(int address, int rowOffset)
		{
			return Math.Max(0, Math.Min(GetLineCount() - 1, address + rowOffset));
		}
	}

	public class TraceLoggerStyleProvider : ILineStyleProvider
	{
		private LineProperties _defaultLine = new LineProperties() { AddressColor = null, LineBgColor = null };
		private LineProperties _spcLine = new LineProperties() { AddressColor = Color.FromRgb(30, 145, 30), LineBgColor = Color.FromRgb(230, 245, 230) };
		private LineProperties _coprocLine = new LineProperties() { AddressColor = Color.FromRgb(30, 30, 145), LineBgColor = Color.FromRgb(230, 230, 245) };
		private ConsoleType _consoleType = ConsoleType.Snes;

		public TraceLoggerStyleProvider()
		{
		}

		public List<CodeColor> GetCodeColors(CodeLineData lineData, bool highlightCode, string addressFormat, Color? textColor, bool showMemoryValues)
		{
			return CodeHighlighting.GetCpuHighlights(lineData, highlightCode, addressFormat, textColor, showMemoryValues);
		}

		public LineProperties GetLineStyle(CodeLineData lineData, int lineIndex)
		{
			switch(lineData.CpuType) {
				case CpuType.Spc: return _spcLine;
				
				case CpuType.NecDsp:
				case CpuType.Sa1:
				case CpuType.Gsu:
				case CpuType.Cx4:
					return _coprocLine;

				case CpuType.Gameboy:
					if(_consoleType == ConsoleType.Snes) {
						return _coprocLine;
					}
					break;

				default: break;
			}

			return _defaultLine;
		}

		internal void SetConsoleType(ConsoleType consoleType)
		{
			_consoleType = consoleType;
		}
	}

}
