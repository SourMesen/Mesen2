using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
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

		[Reactive] public List<TraceLoggerOptionTab> Tabs { get; set; } = new();
		[Reactive] public TraceLoggerOptionTab SelectedTab { get; set; } = null!;

		[Reactive] public string? TraceFile { get; set; } = null;
		[Reactive] public bool AllowOpenTraceFile { get; private set; } = false;
		[Reactive] public bool IsStartLoggingEnabled { get; set; }
		
		[Reactive] public bool ShowByteCode { get; private set; }

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
				Tabs = new() { new TraceLoggerOptionTab(this, CpuType.Nes, Config.GetCpuConfig(CpuType.Nes), true) };
				SelectedTab = Tabs[0];
				return;
			}

			QuickSearch.OnFind += QuickSearch_OnFind;

			AddDisposable(this.WhenAnyValue(x => x.QuickSearch.IsSearchBoxVisible).Subscribe(x => {
				if(!QuickSearch.IsSearchBoxVisible) {
					_viewer?.Focus();
				}
			}));

			UpdateAvailableTabs();

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
			string needle = e.SearchString.ToLowerInvariant();
			
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
			foreach(TraceLoggerOptionTab tab in Tabs) {
				tab.Dispose();
			}

			List<TraceLoggerOptionTab> tabs = new();
			RomInfo romInfo = EmuApi.GetRomInfo();
			bool showEnableButton = romInfo.CpuTypes.Count > 1;
			StyleProvider.SetConsoleType(romInfo.ConsoleType);
			foreach(CpuType type in romInfo.CpuTypes) {
				tabs.Add(AddDisposable(new TraceLoggerOptionTab(this, type, Config.GetCpuConfig(type), showEnableButton)));
			}

			Tabs = tabs;
			SelectedTab = tabs[0];

			UpdateOptions();
		}

		public void UpdateOptions()
		{
			bool forceEnable = Tabs.Count == 1;
			bool isStartLoggingEnabled = forceEnable;
			bool showByteCode = false;

			RomInfo romInfo = EmuApi.GetRomInfo();
			foreach(CpuType cpuType in romInfo.CpuTypes) {
				showByteCode |= Config.GetCpuConfig(cpuType).ShowByteCode;
				isStartLoggingEnabled |= Config.GetCpuConfig(cpuType).Enabled;
			}

			UpdateCoreOptions();

			IsStartLoggingEnabled = isStartLoggingEnabled;
			ShowByteCode = showByteCode;
			UpdateLog();
		}

		public void UpdateCoreOptions()
		{
			RomInfo romInfo = EmuApi.GetRomInfo();
			foreach(CpuType cpuType in romInfo.CpuTypes) {
				TraceLoggerCpuConfig cfg = Config.GetCpuConfig(cpuType);
				InteropTraceLoggerOptions options = new InteropTraceLoggerOptions() {
					Enabled = romInfo.CpuTypes.Count == 1 || cfg.Enabled,
					UseLabels = cfg.UseLabels,
					IndentCode = cfg.IndentCode,
					Format = Encoding.UTF8.GetBytes(cfg.UseCustomFormat ? cfg.Format : TraceLoggerOptionTab.GetAutoFormat(cfg, cpuType)),
					Condition = Encoding.UTF8.GetBytes(cfg.Condition)
				};

				Array.Resize(ref options.Condition, 1000);
				Array.Resize(ref options.Format, 1000);

				DebugApi.SetTraceOptions(cpuType, options);
			}
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
			ApplicationHelper.GetMainWindow()?.Clipboard?.SetTextAsync(sb.ToString());
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
					AbsoluteAddress = new() { Address = -1 },
					Text = rows[i].GetOutput(),
					OpSize = rows[i].ByteCodeSize,
					ByteCode = rows[i].GetByteCode()
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

	public class TraceLoggerOptionTab : DisposableViewModel
	{
		public string TabName { get; set; } = "";
		public Control HelpTooltip => ExpressionTooltipHelper.GetHelpTooltip(CpuType, false);
		public Control FormatTooltip => GetFormatTooltip();

		public CpuType CpuType { get; set; } = CpuType.Snes;

		public string LogOptionsTitle { get; }
		public bool ShowEnableButton { get; }
		public bool ShowStatusFormat { get; }
		public bool ShowIndentCode { get; }
		public TraceLoggerCpuConfig Options { get; }

		[Reactive] public string Format { get; set; } = "";
		[Reactive] public bool IsConditionValid { get; set; } = true;

		private TraceLoggerViewModel _traceLogger;

		public TraceLoggerOptionTab(TraceLoggerViewModel traceLogger, CpuType cpuType, TraceLoggerCpuConfig options, bool showEnableButton)
		{
			_traceLogger = traceLogger;
			CpuType = cpuType;
			Options = options;
			
			ShowEnableButton = showEnableButton;
			ShowStatusFormat = cpuType != CpuType.Cx4 && cpuType != CpuType.NecDsp;
			ShowIndentCode = cpuType != CpuType.Gsu;

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(options, OnOptionsChanged));
			UpdateFormat();

			if(string.IsNullOrWhiteSpace(Options.Format)) {
				//Set custom format to the default if it's empty
				Options.Format = GetAutoFormat(Options, CpuType);
			}

			TabName = ResourceHelper.GetEnumText(cpuType);
			LogOptionsTitle = string.Format(ResourceHelper.GetViewLabel(nameof(TraceLoggerWindow), "grpLogOptions"), ResourceHelper.GetEnumText(cpuType));
		}

		private void OnOptionsChanged(object? sender, PropertyChangedEventArgs e)
		{
			UpdateFormat();

			if(e.PropertyName == nameof(TraceLoggerCpuConfig.UseCustomFormat) && Options.UseCustomFormat && string.IsNullOrWhiteSpace(Options.Format)) {
				//Set custom format to the default if it's empty
				Options.Format = Format;
			}

			if(Options.Enabled) {
				//Auto-select current tab when enabled
				_traceLogger.SelectedTab = this;
			}

			if(!string.IsNullOrWhiteSpace(Options.Condition)) {
				DebugApi.EvaluateExpression(Options.Condition, CpuType, out EvalResultType result, false);
				IsConditionValid = result == EvalResultType.Numeric || result == EvalResultType.Boolean;
			} else {
				IsConditionValid = true;
			}

			_traceLogger.UpdateOptions();
		}

		private void UpdateFormat()
		{
			if(!Options.UseCustomFormat) {
				Format = GetAutoFormat(Options, CpuType);
			}
		}

		public static string GetAutoFormat(TraceLoggerCpuConfig cfg, CpuType cpuType)
		{
			string format = "";
			int alignValue = cpuType switch {
				CpuType.Gba => 42,
				CpuType.Ws => 36,
				_ => 24
			};

			void addTag(bool condition, string formatText, int align = 0)
			{
				if(condition) {
					format += formatText;
					alignValue += align;
				}
			}

			format += "[Disassembly]";
			addTag(cfg.ShowEffectiveAddresses, "[EffectiveAddress]", 8);
			addTag(cfg.ShowMemoryValues, " [MemoryValue,h]", 6);
			format += "[Align," + alignValue.ToString() + "] ";

			switch(cpuType) {
				case CpuType.Snes:
				case CpuType.Sa1:
					addTag(cfg.ShowRegisters, "A:[A,4h] X:[X,4h] Y:[Y,4h] S:[SP,4h] D:[D,4h] DB:[DB,2h] ");
					addTag(cfg.ShowStatusFlags, cfg.StatusFormat switch {
						StatusFlagFormat.Hexadecimal => "P:[P,h] ",
						StatusFlagFormat.CompactText => "P:[P] ",
						StatusFlagFormat.Text or _ => "P:[P,8] "
					});
					break;

				case CpuType.Spc:
				case CpuType.Nes:
				case CpuType.Pce:
					addTag(cfg.ShowRegisters, "A:[A,2h] X:[X,2h] Y:[Y,2h] S:[SP,2h] ");
					addTag(cfg.ShowStatusFlags, cfg.StatusFormat switch {
						StatusFlagFormat.Hexadecimal => "P:[P,h] ",
						StatusFlagFormat.CompactText => "P:[P] ",
						StatusFlagFormat.Text or _ => "P:[P,8] "
					});
					break;

				case CpuType.Gsu:
					addTag(cfg.ShowRegisters, "SRC:[SRC,2h] DST:[DST,2h] R0:[R0,4h] R1:[R1,4h] R2:[R2,4h] R3:[R3,4h] R4:[R4,4h] R5:[R5,4h] R6:[R6,4h] R7:[R7,4h] R8:[R8,4h] R9:[R9,4h] R10:[R10,4h] R11:[R11,4h] R12:[R12,4h] R13:[R13,4h] R14:[R14,4h] R15:[R15,4h] ");
					addTag(cfg.ShowStatusFlags, cfg.StatusFormat switch {
						StatusFlagFormat.Hexadecimal => "SFR:[SFR,h] ",
						StatusFlagFormat.CompactText => "SFR:[SFR] ",
						StatusFlagFormat.Text or _ => "SFR:[SFR,16] "
					});
					break;

				case CpuType.Cx4:
					addTag(cfg.ShowRegisters, "A:[A,6h] MAR:[MAR,6h] MDR:[MDR,6h] DPR:[DPR,6h] ML:[ML,6h] MH:[MH,6h] P:[P,4h] PB:[PB,4h] R0:[R0,6h] R1:[R1,6h] R2:[R2,6h] R3:[R3,6h] R4:[R4,6h] R5:[R5,6h] R6:[R6,6h] R7:[R7,6h] R8:[R8,6h] R9:[R9,6h] R10:[R10,6h] R11:[R11,6h] R12:[R12,6h] R13:[R13,6h] R14:[R14,6h] R15:[R15,6h] ");
					addTag(cfg.ShowStatusFlags, "PS:[PS] ");
					break;

				case CpuType.NecDsp:
					addTag(cfg.ShowRegisters, "A:[A,4h] ");
					addTag(cfg.ShowStatusFlags, "[FlagsA] ");
					addTag(cfg.ShowRegisters, "B:[B,4h] ");
					addTag(cfg.ShowStatusFlags, "[FlagsB] ");
					addTag(cfg.ShowRegisters, "K:[K,4h] L:[L,4h] M:[M,4h] N:[N,4h] RP:[RP,4h] DP:[DP,4h] DR:[DR,4h] SR:[SR,4h] TR:[TR,4h] TRB:[TRB,4h] ");
					break;

				case CpuType.Gameboy:
					addTag(cfg.ShowRegisters, "A:[A,2h] B:[B,2h] C:[C,2h] D:[D,2h] E:[E,2h] ");
					addTag(cfg.ShowStatusFlags, cfg.StatusFormat switch {
						StatusFlagFormat.Hexadecimal => "F:[PS,h] ",
						StatusFlagFormat.CompactText => "F:[PS] ",
						StatusFlagFormat.Text or _ => "F:[PS,4] "
					});
					addTag(cfg.ShowRegisters, "HL:[H,2h][L,2h] S:[SP,4h] ");
					break;

				case CpuType.Sms:
					addTag(cfg.ShowRegisters, "A:[A,2h] B:[B,2h] C:[C,2h] D:[D,2h] E:[E,2h] ");
					addTag(cfg.ShowStatusFlags, cfg.StatusFormat switch {
						StatusFlagFormat.Hexadecimal => "F:[PS,h] ",
						StatusFlagFormat.CompactText => "F:[PS] ",
						StatusFlagFormat.Text or _ => "F:[PS,8] "
					});
					addTag(cfg.ShowRegisters, "HL:[H,2h][L,2h] IX:[IX,4h] IY:[IY,4h] S:[SP,4h] ");
					break;

				case CpuType.St018:
				case CpuType.Gba:
					addTag(cfg.ShowRegisters, "R0:[R0,8h] R1:[R1,8h] R2:[R2,8h] R3:[R3,8h] R4:[R4,8h] R5:[R5,8h] R6:[R6,8h] R7:[R7,8h] R8:[R8,8h] R9:[R9,8h] R10:[R10,8h] R11:[R11,8h] R12:[R12,8h] R13:[R13,8h] R14:[R14,8h] R15:[R15,8h] ");
					addTag(cfg.ShowStatusFlags, cfg.StatusFormat switch {
						StatusFlagFormat.Hexadecimal => "CPSR:[CPSR,8h] ",
						StatusFlagFormat.CompactText => "CPSR:[CPSR] ",
						StatusFlagFormat.Text or _ => "CPSR:[CPSR,7] "
					});
					addTag(cfg.ShowStatusFlags, "Mode: [Mode,3] ");
					break;

				case CpuType.Ws:
					addTag(cfg.ShowRegisters, "AX:[AX,4h] BX:[BX,4h] CX:[CX,4h] DX:[DX,4h] DS:[DS,4h] ES:[ES,4h] SS:[SS,4h] SP:[SP,4h] BP:[BP,4h] SI:[SI,4h] DI:[DI,4h] ");
					addTag(cfg.ShowStatusFlags, cfg.StatusFormat switch {
						StatusFlagFormat.Hexadecimal => "F:[F,4h] ",
						StatusFlagFormat.CompactText => "F:[F] ",
						StatusFlagFormat.Text or _ => "F:[F,10] "
					});
					break;
			}

			addTag(cfg.ShowFramePosition, "V:[Scanline,3] H:[Cycle,3] ");
			addTag(cfg.ShowFrameCounter, "Fr:[FrameCount] ");
			addTag(cfg.ShowClockCounter, "Cycle:[CycleCount] ");
			addTag(cfg.ShowByteCode, "BC:[ByteCode]");

			return format.Trim();
		}

		private Control GetFormatTooltip()
		{
			StackPanel panel = new();

			void addRow(string text) { panel.Children.Add(new TextBlock() { Text = text }); }
			void addBoldRow(string text) { panel.Children.Add(new TextBlock() { Text = text, FontWeight = Avalonia.Media.FontWeight.Bold }); }

			addBoldRow("Notes");
			addRow("You can customize the output by enabling the 'Use custom format' option and manually editing the format.");
			addRow(" ");
			addRow("Tags can have their display format configured by using a comma and specifying the format options. e.g:");
			addRow("  [Scanline,3] - display scanline in decimal, pad to always be 3 characters wide");
			addRow("  [Scanline,h] - display scanline in hexadecimal");
			addRow("  [Scanline,3h] - display scanline in decimal, pad to always be 3 characters wide");
			addRow(" ");
			addBoldRow("Common tags (all CPUs)");
			addRow("  [ByteCode] - byte code for the instruction (1 to 3 bytes)");
			addRow("  [Disassembly] - disassembly for the current instruction");
			addRow("  [EffectiveAddress] - effective address used for indirect addressing modes");
			addRow("  [MemoryValue] - value stored at the memory location referred to by the instruction");
			addRow("  [PC] - program counter");
			addRow("  [Cycle] - current horizontal cycle (H)");
			addRow("  [HClock] - current horizontal cycle (H, in master clocks)");
			addRow("  [Scanline] - current scanline (V)");
			addRow("  [FrameCount] - current frame number");
			addRow("  [CycleCount] - current CPU cycle (64-bit unsigned value)");
			addRow("  [Align,X] - add spaces to ensure the line is X characters long");
			addRow(" ");

			addBoldRow("CPU-specific tags (" + ResourceHelper.GetEnumText(CpuType) + ")");

			string[] tokens = CpuType switch {
				CpuType.Snes or CpuType.Sa1 => new string[] { "A", "X", "Y", "D", "DB", "P", "SP" },
				CpuType.Spc => new string[] { "A", "X", "Y", "P", "SP" },
				CpuType.NecDsp => new string[] { "A", "B", "FlagsA", "FlagsB", "K", "L", "M", "N", "RP", "DP", "DR", "SR", "TR", "TRB" },
				CpuType.Gsu => new string[] { "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15", "SRC", "DST", "SFR" },
				CpuType.Cx4 => new string[] { "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15", "MAR", "MDR", "DPR", "ML", "MH", "PB", "P", "PS", "A" },
				CpuType.Gameboy => new string[] { "A", "B", "C", "D", "E", "F", "H", "L", "PS", "SP" },
				CpuType.Nes => new string[] { "A", "X", "Y", "P", "SP" },
				CpuType.Pce => new string[] { "A", "X", "Y", "P", "SP" },
				CpuType.Sms => new string[] { "A", "B", "C", "D", "E", "F", "H", "L", "IX", "IY", "A'", "B'", "C'", "D'", "E'", "F'", "H'", "L'", "I", "R", "PS", "SP" },
				CpuType.Gba or CpuType.St018  => new string[] { "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15", "CPSR" },
				CpuType.Ws => new string[] { "AX", "BX", "CX", "DX", "CS", "IP", "SS", "SP", "BP", "DS", "ES", "SI", "DI", "F" },
				_ => throw new Exception("unsupported cpu type")
			};

			Array.Sort(tokens);

			Grid tokenGrid = new Grid() {
				ColumnDefinitions = new("40, 40, 40, 40"),
				RowDefinitions = new(string.Join(",", Enumerable.Repeat("Auto", (tokens.Length / 4) + 1))),
				Margin = new Thickness(5, 0, 0, 0)
			};

			int col = 0;
			int row = 0;
			foreach(string token in tokens) {
				TextBlock txt = new() { Text = $"[{token}]", Padding = new Thickness(0, 0, 5, 0) };
				tokenGrid.Children.Add(txt);
				Grid.SetColumn(txt, col);
				Grid.SetRow(txt, row);
				col++;
				if(col == 4) {
					col = 0;
					row++;
				}
			}

			panel.Children.Add(tokenGrid);

			return panel;
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
			AddressSize = consoleType.GetMainCpuType().GetAddressSize();
			ByteCodeSize = consoleType.GetMainCpuType().GetByteCodeSize();
		}
	}

}
