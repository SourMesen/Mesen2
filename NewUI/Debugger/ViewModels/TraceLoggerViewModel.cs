using Avalonia.Controls;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Text;

namespace Mesen.Debugger.ViewModels
{
	public class TraceLoggerViewModel : ViewModelBase
	{
		[Reactive] public TraceLoggerCodeDataProvider DataProvider { get; set; } = new TraceLoggerCodeDataProvider();
		[Reactive] public TraceLoggerStyleProvider StyleProvider { get; set; } = new TraceLoggerStyleProvider();
		[Reactive] public int ScrollPosition { get; set; } = 0;
		[Reactive] public int MaxScrollPosition { get; set; } = 0;

		[Reactive] public List<TraceLoggerOptionTab> Tabs { get; set; } = new List<TraceLoggerOptionTab>();

		public TraceLoggerViewModel()
		{
			if(Design.IsDesignMode) {
				return;
			}

			UpdateAvailableTabs();
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
					Options = type switch {
						CpuType.Cpu => ConfigManager.Config.Debug.TraceLogger.SnesCpu.Clone(),
						CpuType.Spc => ConfigManager.Config.Debug.TraceLogger.SpcCpu.Clone(),
						CpuType.Gameboy => ConfigManager.Config.Debug.TraceLogger.GameboyCpu.Clone(),
						CpuType.Nes => ConfigManager.Config.Debug.TraceLogger.NesCpu.Clone(),
						CpuType.Sa1 => ConfigManager.Config.Debug.TraceLogger.Sa1Cpu.Clone(),
						CpuType.Gsu => ConfigManager.Config.Debug.TraceLogger.GsuCpu.Clone(),
						CpuType.Cx4 => ConfigManager.Config.Debug.TraceLogger.Cx4Cpu.Clone(),
						CpuType.NecDsp => ConfigManager.Config.Debug.TraceLogger.NecDspCpu.Clone(),
						_ => throw new Exception("Unsupported cpu type")
					}
				});
			}

			Tabs = tabs;
		}

		public void UpdateLog()
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

			DataProvider = new TraceLoggerCodeDataProvider();
			MaxScrollPosition = DataProvider.GetLineCount();
			ScrollPosition = MaxScrollPosition - 0x20;
		}
	}

	public class TraceLoggerOptionTab
	{
		[Reactive] public string TabName { get; set; } = "";
		[Reactive] public CpuType CpuType { get; set; } = CpuType.Cpu;

		public TraceLoggerCpuConfig Options { get; set; } = new TraceLoggerCpuConfig();
	}

	public class TraceLoggerCodeDataProvider : ICodeDataProvider
	{
		public TraceLoggerCodeDataProvider()
		{
		}

		public bool UseOptimizedSearch { get { return false; } }

		public CpuType CpuType => CpuType.Cpu;

		public int GetLineCount()
		{
			return 30000;
		}

		public int GetNextResult(string searchString, int startPosition, int endPosition, bool searchBackwards)
		{
			throw new NotImplementedException();
		}

		public CodeLineData[] GetCodeLines(int startIndex, int rowCount)
		{
			TraceRow[] rows = DebugApi.GetExecutionTrace((uint)(30000 - startIndex), (uint)rowCount);

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
				lines.Insert(0, new CodeLineData(CpuType.Cpu));
			}

			return lines.ToArray();
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
