using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels
{
	public class TraceLoggerViewModel : DisposableViewModel, ISelectableModel
	{
		public TraceLoggerConfig Config { get; }
		[Reactive] public TraceLoggerStyleProvider StyleProvider { get; set; }
		[Reactive] public CodeLineData[] TraceLogLines { get; set; } = Array.Empty<CodeLineData>();
		[Reactive] public int VisibleRowCount { get; set; } = 100;
		[Reactive] public int ScrollPosition { get; set; } = 0;
		[Reactive] public int MinScrollPosition { get; set; } = 0;
		[Reactive] public int MaxScrollPosition { get; set; } = DebugApi.TraceLogBufferSize;
		[Reactive] public bool IsLoggingToFile { get; set; } = false;

		[Reactive] public List<TraceLoggerOptionTab> Tabs { get; set; } = new List<TraceLoggerOptionTab>();
		[Reactive] public TraceLoggerOptionTab SelectedTab { get; set; } = null!;

		[Reactive] public string? TraceFile { get; set; } = null;
		[Reactive] public bool AllowOpenTraceFile {get; private set; } = false;

		[Reactive] public int SelectionStart { get; private set; }
		[Reactive] public int SelectionEnd { get; private set; }
		[Reactive] public int SelectionAnchor { get; private set; }
		[Reactive] public int SelectedRow { get; private set; }

		[Reactive] public List<ContextMenuAction> ToolbarItems { get; private set; } = new();

		[Reactive] public List<ContextMenuAction> FileMenuItems { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> DebugMenuItems { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> SearchMenuItems { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> ViewMenuItems { get; private set; } = new();

		public QuickSearchViewModel QuickSearch { get; } = new();
		
		private DisassemblyViewer? _viewer = null;

		public TraceLoggerViewModel()
		{
			Config = ConfigManager.Config.Debug.TraceLogger;
			StyleProvider = new TraceLoggerStyleProvider(this);

			if(Design.IsDesignMode) {
				return;
			}

			QuickSearch.OnFind += QuickSearch_OnFind;
			
			this.WhenAnyValue(x => x.QuickSearch.IsSearchBoxVisible).Subscribe(x => {
				if(!QuickSearch.IsSearchBoxVisible) {
					_viewer?.Focus();
				}
			});

			UpdateAvailableTabs();

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => UpdateCoreOptions()));

			AddDisposable(this.WhenAnyValue(x => x.ScrollPosition).Subscribe(x => {
				ScrollPosition = Math.Max(MinScrollPosition, Math.Min(x, MaxScrollPosition));
				UpdateLog();
			}));

			AddDisposable(this.WhenAnyValue(x => x.MinScrollPosition).Subscribe(x => {
				ScrollPosition = Math.Max(MinScrollPosition, Math.Min(x, MaxScrollPosition));
			}));

			AddDisposable(this.WhenAnyValue(x => x.MaxScrollPosition).Subscribe(x => {
				ScrollPosition = Math.Max(MinScrollPosition, Math.Min(x, MaxScrollPosition));
			}));

			AddDisposable(this.WhenAnyValue(x => x.IsLoggingToFile).Subscribe(x => {
				AllowOpenTraceFile = !IsLoggingToFile && TraceFile != null;
			}));

			AddDisposable(this.WhenAnyValue(x => x.SelectionStart, x => x.SelectionEnd, x => x.SelectedRow, x => x.SelectionAnchor).Subscribe(x => {
				SelectionStart = Math.Max(MinScrollPosition, Math.Min(DebugApi.TraceLogBufferSize - 1, SelectionStart));
				SelectionEnd = Math.Max(MinScrollPosition, Math.Min(DebugApi.TraceLogBufferSize - 1, SelectionEnd));
				SelectedRow = Math.Max(MinScrollPosition, Math.Min(DebugApi.TraceLogBufferSize - 1, SelectedRow));
				SelectionAnchor = Math.Max(MinScrollPosition, Math.Min(DebugApi.TraceLogBufferSize - 1, SelectionAnchor));
			}));
		}

		public void SetViewer(DisassemblyViewer viewer)
		{
			_viewer = viewer;
		}

		private void QuickSearch_OnFind(OnFindEventArgs e)
		{
			CodeLineData[] lines = GetCodeLines(0, DebugApi.TraceLogBufferSize);
			string needle = e.SearchString.ToLowerInvariant().Trim();
			
			int startRow = SelectedRow;
			if(e.Direction == SearchDirection.Backward) {
				startRow--;
			} else if(e.SkipCurrent) {
				startRow++;
			}
			int sign = e.Direction == SearchDirection.Backward ? -1 : 1;

			for(int i = 0; i < lines.Length; i++) {
				int lineIndex = (i * sign + startRow) % lines.Length;
				if(lineIndex < 0) {
					lineIndex += lines.Length;
				}
				if(lines[lineIndex].Text.Contains(needle, StringComparison.OrdinalIgnoreCase)) {
					Dispatcher.UIThread.Post(() => {
						ScrollToRowNumber(lineIndex);
						SelectedRow = lineIndex;
						SelectionStart = lineIndex;
						SelectionEnd = lineIndex;
						InvalidateVisual();
					});
					e.Success = true;
					return;
				}
			}
			e.Success = false;
		}

		public void InitializeMenu(Window wnd)
		{
			FileMenuItems = AddDisposables(new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd.Close()
				}
			});

			DebugMenuItems.AddRange(AddDisposables(DebugSharedActions.GetStepActions(wnd, () => MainWindowViewModel.Instance.RomInfo.ConsoleType.GetMainCpuType())));
			ToolbarItems.AddRange(AddDisposables(DebugSharedActions.GetStepActions(wnd, () => MainWindowViewModel.Instance.RomInfo.ConsoleType.GetMainCpuType())));

			ViewMenuItems = AddDisposables(new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.Refresh,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Refresh),
					OnClick = () => UpdateLog()
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.EnableAutoRefresh,
					IsSelected = () => Config.AutoRefresh,
					OnClick = () => Config.AutoRefresh = !Config.AutoRefresh
				},
				new ContextMenuAction() {
					ActionType = ActionType.RefreshOnBreakPause,
					IsSelected = () => Config.RefreshOnBreakPause,
					OnClick = () => Config.RefreshOnBreakPause = !Config.RefreshOnBreakPause
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.ShowToolbar,
					IsSelected = () => Config.ShowToolbar,
					OnClick = () => Config.ShowToolbar = !Config.ShowToolbar
				}
			});

			SearchMenuItems = AddDisposables(new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.Find,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Find),
					OnClick = () => QuickSearch.Open()
				},
				new ContextMenuAction() {
					ActionType = ActionType.FindPrev,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FindPrev),
					OnClick = () => QuickSearch.FindPrev()
				},
				new ContextMenuAction() {
					ActionType = ActionType.FindNext,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FindNext),
					OnClick = () => QuickSearch.FindNext()
				},
			});

			DebugShortcutManager.RegisterActions(wnd, FileMenuItems);
			DebugShortcutManager.RegisterActions(wnd, DebugMenuItems);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuItems);
			DebugShortcutManager.RegisterActions(wnd, SearchMenuItems);
		}

		public void InvalidateVisual()
		{
			TraceLogLines = (CodeLineData[])TraceLogLines.Clone();
		}

		public void UpdateAvailableTabs()
		{
			List<TraceLoggerOptionTab> tabs = new();
			RomInfo romInfo = EmuApi.GetRomInfo();
			StyleProvider.SetConsoleType(romInfo.ConsoleType);
			foreach(CpuType type in romInfo.CpuTypes) {
				tabs.Add(new TraceLoggerOptionTab(type) {
					Options = Config.GetCpuConfig(type)
				});
			}

			Tabs = tabs;
			SelectedTab = tabs[0];

			UpdateCoreOptions();
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

			UpdateLog();
		}

		public void UpdateLog(bool scrollToBottom = false)
		{
			int traceSize = (int)DebugApi.GetExecutionTraceSize();
			CodeLineData[] lines = GetCodeLines(ScrollPosition, VisibleRowCount);

			Dispatcher.UIThread.Post(() => {
				MinScrollPosition = Math.Min(MaxScrollPosition, DebugApi.TraceLogBufferSize - traceSize);
				TraceLogLines = lines;

				if(scrollToBottom) {
					ScrollToBottom();
				}
			});
		}

		public void SetSelectedRow(int rowNumber)
		{
			rowNumber += ScrollPosition;
			SelectionStart = rowNumber;
			SelectionEnd = rowNumber;
			SelectedRow = rowNumber;
			SelectionAnchor = rowNumber;
			InvalidateVisual();
		}

		public bool IsSelected(int rowNumber)
		{
			rowNumber += ScrollPosition;
			return rowNumber >= SelectionStart && rowNumber <= SelectionEnd;
		}

		public void MoveCursor(int rowOffset, bool extendSelection)
		{
			int rowNumber = SelectedRow + rowOffset - ScrollPosition;
			if(extendSelection) {
				ResizeSelectionTo(rowNumber);
			} else {
				SetSelectedRow(rowNumber);
				ScrollToRowNumber(rowNumber + ScrollPosition, rowOffset < 0 ? ScrollDisplayPosition.Top : ScrollDisplayPosition.Bottom);
			}
		}

		public void ResizeSelectionTo(int rowNumber)
		{
			rowNumber += ScrollPosition;

			if(SelectedRow == rowNumber) {
				return;
			}

			bool anchorTop = SelectionAnchor == SelectionStart;
			if(anchorTop) {
				if(rowNumber < SelectionStart) {
					SelectionEnd = SelectionStart;
					SelectionStart = rowNumber;
				} else {
					SelectionEnd = rowNumber;
				}
			} else {
				if(rowNumber < SelectionEnd) {
					SelectionStart = rowNumber;
				} else {
					SelectionStart = SelectionEnd;
					SelectionEnd = rowNumber;
				}
			}

			ScrollDisplayPosition displayPos = SelectedRow < rowNumber ? ScrollDisplayPosition.Bottom : ScrollDisplayPosition.Top;
			SelectedRow = rowNumber;
			ScrollToRowNumber(rowNumber, displayPos);

			InvalidateVisual();
		}

		public void Scroll(int offset)
		{
			ScrollPosition += offset;
		}

		public void ScrollToTop()
		{
			ScrollPosition = 0;
			SetSelectedRow(0);
		}

		public void ScrollToBottom()
		{
			ScrollPosition = MaxScrollPosition;
			SetSelectedRow(DebugApi.TraceLogBufferSize - 1);
		}

		public void SelectAll()
		{
			SelectionStart = 0;
			SelectionEnd = DebugApi.TraceLogBufferSize - 1;
			InvalidateVisual();
		}

		public void CopySelection()
		{
			StringBuilder sb = new();

			int len = SelectionEnd - SelectionStart + 1;
			CodeLineData[] lines = GetCodeLines(SelectionStart, len);

			for(int i = 0; i < len; i++) {
				string addrFormat = "X" + lines[i].CpuType.GetAddressSize();
				sb.AppendLine(lines[i].GetAddressText(AddressDisplayType.CpuAddress, addrFormat).PadRight(6) + " " + lines[i].Text);
			}
			Application.Current?.Clipboard?.SetTextAsync(sb.ToString());
		}

		private bool IsRowVisible(int rowNumber)
		{
			return rowNumber > ScrollPosition && rowNumber < ScrollPosition + VisibleRowCount;
		}

		private void ScrollToRowNumber(int rowNumber, ScrollDisplayPosition position = ScrollDisplayPosition.Center)
		{
			if(IsRowVisible(rowNumber)) {
				//Row is already visible, don't scroll
				return;
			}

			switch(position) {
				case ScrollDisplayPosition.Top: ScrollPosition = rowNumber; break;
				case ScrollDisplayPosition.Center: ScrollPosition = rowNumber - (VisibleRowCount / 2) + 1; break;
				case ScrollDisplayPosition.Bottom: ScrollPosition = rowNumber - VisibleRowCount + 1; break;
			}
		}

		private CodeLineData[] GetCodeLines(int startIndex, int rowCount)
		{
			TraceRow[] rows = DebugApi.GetExecutionTrace((uint)(DebugApi.TraceLogBufferSize - startIndex - rowCount), (uint)rowCount);

			List<CodeLineData> lines = new(rowCount);

			CodeLineData emptyLine = new CodeLineData(CpuType.Snes);
			int emptyLineCount = rowCount - rows.Length;
			for(int i = 0; i < emptyLineCount; i++) {
				lines.Add(emptyLine);
			}

			for(int i = rows.Length - 1; i >= 0; i--) {
				lines.Add(new CodeLineData(rows[i].Type) {
					Address = (int)rows[i].ProgramCounter,
					Text = rows[i].GetOutput(),
					ByteCode = rows[i].GetByteCode(),
					EffectiveAddress = -1
				});
			}

			return lines.ToArray();
		}

		public AddressInfo? GetSelectedRowAddress()
		{
			TraceRow[] rows = DebugApi.GetExecutionTrace(DebugApi.TraceLogBufferSize - (uint)SelectedRow - 1, 1);
			if(rows.Length > 0) {
				return new AddressInfo() {
					Address = (int)rows[0].ProgramCounter,
					Type = rows[0].Type.ToMemoryType()
				};
			}

			return null;
		}
	}

	public class TraceLoggerOptionTab
	{
		public string TabName { get; set; } = "";
		public Control? HelpTooltip => ExpressionTooltipHelper.GetHelpTooltip(CpuType, false);
		public CpuType CpuType { get; set; } = CpuType.Snes;

		public TraceLoggerCpuConfig Options { get; set; } = new TraceLoggerCpuConfig();

		public TraceLoggerOptionTab(CpuType cpuType)
		{
			CpuType = cpuType;
			TabName = ResourceHelper.GetEnumText(cpuType);
		}
	}

	public class TraceLoggerStyleProvider : ILineStyleProvider
	{
		private ConsoleType _consoleType = ConsoleType.Snes;
		private TraceLoggerViewModel _model;

		public TraceLoggerStyleProvider(TraceLoggerViewModel model)
		{
			_model = model;
		}

		public int AddressSize { get; private set; } = 6;
		public int ByteCodeSize { get; private set; } = 4;
		private LineProperties GetMainCpuStyle() { return new LineProperties() { AddressColor = null, LineBgColor = null }; }
		private LineProperties GetSecondaryCpuStyle() { return new LineProperties() { AddressColor = Color.FromRgb(30, 145, 30), LineBgColor = Color.FromRgb(230, 245, 230) }; }
		private LineProperties GetCoprocessorStyle() { return new LineProperties() { AddressColor = Color.FromRgb(30, 30, 145), LineBgColor = Color.FromRgb(230, 230, 245) }; }

		public List<CodeColor> GetCodeColors(CodeLineData lineData, bool highlightCode, string addressFormat, Color? textColor, bool showMemoryValues)
		{
			return CodeHighlighting.GetCpuHighlights(lineData, highlightCode, addressFormat, textColor, showMemoryValues);
		}

		public LineProperties GetLineStyle(CodeLineData lineData, int lineIndex)
		{
			LineProperties props = lineData.CpuType switch {
				CpuType.Spc => GetSecondaryCpuStyle(),
				CpuType.NecDsp or CpuType.Sa1 or CpuType.Gsu or CpuType.Cx4 => GetCoprocessorStyle(),
				CpuType.Gameboy => _consoleType == ConsoleType.Snes ? GetCoprocessorStyle() : GetMainCpuStyle(),
				_ => GetMainCpuStyle(),
			};

			if(_model != null && lineData.HasAddress) {
				int lineNumber = _model.ScrollPosition + lineIndex;
				props.IsSelectedRow = lineNumber >= _model.SelectionStart && lineNumber <= _model.SelectionEnd;
				props.IsActiveRow = _model.SelectedRow == lineNumber;
			}

			return props;
		}

		internal void SetConsoleType(ConsoleType consoleType)
		{
			_consoleType = consoleType;
			switch(consoleType) {
				case ConsoleType.Snes:
					AddressSize = 6;
					ByteCodeSize = 4;
					break;

				default:
					AddressSize = 4;
					ByteCodeSize = 3;
					break;
			}
		}
	}

}
