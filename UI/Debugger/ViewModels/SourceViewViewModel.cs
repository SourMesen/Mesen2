using Avalonia;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Integration;
using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Linq;
using System.Collections.Generic;
using System.Threading.Tasks;
using Mesen.ViewModels;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Utilities;
using System.Text;
using Mesen.Debugger.Controls;
using Mesen.Utilities;

namespace Mesen.Debugger.ViewModels;

public class SourceViewViewModel : DisposableViewModel, ISelectableModel
{
	public ISymbolProvider SymbolProvider { get; set; }
	public DebugConfig Config { get; }
	public CpuType CpuType { get; }
	public List<SourceFileInfo> SourceFiles { get; }

	public BaseStyleProvider StyleProvider { get; }

	public QuickSearchViewModel QuickSearch { get; } = new();

	[Reactive] public SourceFileInfo? SelectedFile { get; set; }
	[Reactive] public int MaxScrollPosition { get; private set; }
	[Reactive] public int ScrollPosition { get; set; }
	[Reactive] public CodeLineData[] Lines { get; private set; } = Array.Empty<CodeLineData>();

	[Reactive] public int? ActiveAddress { get; set; }
	[Reactive] public int SelectedRow { get; set; }
	[Reactive] public int SelectionAnchor { get; set; }
	[Reactive] public int SelectionStart { get; set; }
	[Reactive] public int SelectionEnd { get; set; }

	[Reactive] public int VisibleRowCount { get; set; } = 100;
	public DebuggerWindowViewModel Debugger { get; }

	public NavigationHistory<SourceCodeLocation> History { get; } = new();
	private DisassemblyViewer? _viewer = null;
	private Action? _refreshScrollbar = null;

	[Obsolete("For designer only")]
	public SourceViewViewModel() : this(new(), new SnesDbgImporter(RomFormat.Sfc), CpuType.Snes) { }

	public SourceViewViewModel(DebuggerWindowViewModel debugger, ISymbolProvider symbolProvider, CpuType cpuType)
	{
		Debugger = debugger;
		Config = ConfigManager.Config.Debug;
		CpuType = cpuType;
		SymbolProvider = symbolProvider;
		StyleProvider = new SourceViewStyleProvider(cpuType, this);

		SourceFiles = SymbolProvider.SourceFiles.Where(f => f.Data.Length > 0 && !f.Name.EndsWith(".chr", StringComparison.OrdinalIgnoreCase)).ToList();
		SourceFiles.Sort((a, b) => {
			int result = a.IsAssembly.CompareTo(b.IsAssembly);
			if(result != 0) {
				return result;
			}
			return a.ToString().CompareTo(b.ToString());
		});
		if(SourceFiles.Count > 0) {
			SelectedFile = SourceFiles[0];
		}

		QuickSearch.OnFind += QuickSearch_OnFind;

		AddDisposable(this.WhenAnyValue(x => x.QuickSearch.IsSearchBoxVisible).Subscribe(x => {
			if(!QuickSearch.IsSearchBoxVisible) {
				_viewer?.Focus();
			}
		}));

		AddDisposable(this.WhenAnyValue(x => x.SelectedFile).Subscribe(x => {
			MaxScrollPosition = (x?.Data.Length ?? 1) - 1;
			ScrollPosition = 0;
			UpdateCodeLines();
		}));

		int lastValue = ScrollPosition;
		AddDisposable(this.WhenAnyValue(x => x.ScrollPosition).Subscribe(x => {
			if(_viewer == null && x == 0) {
				ScrollPosition = lastValue;
				return;
			}
			UpdateCodeLines();
		}));
	}

	private void UpdateCodeLines()
	{
		if(SelectedFile is SourceFileInfo file) {
			List<CodeLineData> lines = new();

			int end = Math.Min(ScrollPosition + VisibleRowCount, file.Data.Length);
			for(int i = ScrollPosition; i < end; i++) {
				lines.Add(GetCodeLineData(file, i));
			}
			Lines = lines.ToArray();
			_refreshScrollbar?.Invoke();
		}
	}

	private CodeLineData GetCodeLineData(SourceFileInfo file, int lineNumber)
	{
		int lineAddr = -1;
		bool showLineAddress = true;
		AddressInfo? address = SymbolProvider.GetLineAddress(file, lineNumber);
		if(address == null) {
			showLineAddress = false;
			int prevLine = lineNumber - 1;
			while(address == null && prevLine >= 0) {
				address = SymbolProvider.GetLineAddress(file, prevLine);
				prevLine--;
			}
		}

		AddressInfo? endAddress = SymbolProvider.GetLineEndAddress(file, lineNumber);
		int opSize = 0;
		if(endAddress == null) {
			int nextLine = lineNumber + 1;
			while(endAddress == null && nextLine < file.Data.Length) {
				endAddress = SymbolProvider.GetLineAddress(file, nextLine);
				nextLine++;
			}

			if(endAddress != null && endAddress.Value.Address >= 1) {
				//Set the end of the current row to the byte before the start of the next row
				endAddress = new() { Address = endAddress.Value.Address - 1, Type = endAddress.Value.Type };
			}
		}

		if(endAddress?.Type == address?.Type && endAddress?.Address >= address?.Address) {
			opSize = Math.Min(endAddress!.Value.Address - address!.Value.Address + 1, 16);
		}

		byte[]? byteCode = null;
		if(showLineAddress && address != null) {
			if(endAddress != null && endAddress.Value.Type == address.Value.Type && endAddress.Value.Address >= address.Value.Address) {
				byteCode = DebugApi.GetMemoryValues(address.Value.Type, (uint)address.Value.Address, (uint)endAddress.Value.Address);
			} else {
				byteCode = new byte[1] { DebugApi.GetMemoryValue(address.Value.Type, (uint)address.Value.Address) };
			}
			lineAddr = DebugApi.GetRelativeAddress(address.Value, CpuType).Address;
		}

		return new CodeLineData(CpuType) {
			Address = lineAddr,
			AbsoluteAddress = address ?? new AddressInfo() { Address = -1 },
			Flags = LineFlags.VerifiedCode | (!showLineAddress ? LineFlags.Empty : LineFlags.None),
			Text = ReplaceTabs(file.Data[lineNumber], ConfigManager.Config.Debug.Integration.TabSize),
			ByteCode = byteCode ?? Array.Empty<byte>(),
			OpSize = (byte)opSize
		};
	}

	private string ReplaceTabs(string line, int tabSize)
	{
		StringBuilder sb = new();
		for(int i = 0; i < line.Length; i++) {
			if(line[i] != '\t') {
				sb.Append(line[i]);
			} else {
				sb.Append(' ', tabSize - sb.Length % tabSize);
			}
		}
		return sb.ToString();
	}

	private void QuickSearch_OnFind(OnFindEventArgs e)
	{
		SourceFileInfo? file = SelectedFile;
		if(file == null) {
			return;
		}

		int findAddress = -1;

		int startRow = SelectedRow;
		if(e.Direction == SearchDirection.Backward) {
			startRow--;
		} else if(e.SkipCurrent) {
			startRow++;
		}
		
		int direction = e.Direction == SearchDirection.Backward ? -1 : 1;
		for(int i = 0; i < file.Data.Length; i++) {
			int index = ((i * direction) + startRow) % file.Data.Length;
			if(index < 0) {
				index += file.Data.Length;
			}

			if(file.Data[index].Contains(e.SearchString, StringComparison.OrdinalIgnoreCase)) {
				findAddress = index;
				break;
			}
		}

		if(findAddress >= 0) {
			Dispatcher.UIThread.Post(() => {
				SetSelectedRow(findAddress, true);
			});
		}
	}

	public void CopySelection()
	{
		if(SelectedFile != null) {
			StringBuilder sb = new();
			for(int i = SelectionStart; i <= SelectionEnd; i++) {
				sb.AppendLine(SelectedFile.Data[i]);
			}
			ApplicationHelper.GetMainWindow()?.Clipboard?.SetTextAsync(sb.ToString());
		}
	}

	public void Refresh()
	{
		UpdateCodeLines();
	}

	public void InvalidateVisual()
	{
		Lines = Lines.ToArray();
	}

	public bool SetActiveAddress(int? activeAddress)
	{
		ActiveAddress = activeAddress;

		if(activeAddress >= 0) {
			return GoToRelativeAddress(activeAddress.Value);
		}

		return true;
	}

	public bool GoToRelativeAddress(int address, bool addToHistory = false)
	{
		AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = address, Type = CpuType.ToMemoryType() });
		SourceCodeLocation? location = SymbolProvider.GetSourceCodeLineInfo(absAddress);
		if(location != null) {
			ScrollToLocation(location.Value, addToHistory);
			return true;
		}

		return false;
	}

	public void ScrollToLocation(SourceCodeLocation loc, bool addToHistory = false)
	{
		SourceCodeLocation? prevLoc = SelectedFile != null ? new SourceCodeLocation(SelectedFile, SelectedRow) : null;
		SelectedFile = loc.File;
		SetSelectedRow(loc.LineNumber);
		ScrollToRowNumber(loc.LineNumber, ScrollDisplayPosition.Center, Config.Debugger.KeepActiveStatementInCenter);

		if(addToHistory) {
			if(prevLoc != null) {
				History.AddHistory(prevLoc.Value);
			}
			History.AddHistory(loc);
		}
	}

	public void ResizeSelectionTo(int rowNumber)
	{
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

	public void MoveCursor(int rowOffset, bool extendSelection)
	{
		int rowNumber = Math.Max(0, Math.Min(SelectedRow + rowOffset, MaxScrollPosition));
		if(extendSelection) {
			ResizeSelectionTo(rowNumber - ScrollPosition);
		} else {
			SetSelectedRow(rowNumber);
			ScrollToRowNumber(rowNumber, rowOffset < 0 ? ScrollDisplayPosition.Top : ScrollDisplayPosition.Bottom);
		}
	}

	private bool IsRowVisible(int rowNumber)
	{
		return rowNumber > ScrollPosition && rowNumber < ScrollPosition + VisibleRowCount - 1;
	}

	public void ScrollToRowNumber(int rowNumber, ScrollDisplayPosition position = ScrollDisplayPosition.Center, bool forceScroll = false)
	{
		if(!forceScroll && IsRowVisible(rowNumber)) {
			//Row is already visible, don't scroll
			return;
		}

		int newPos = position switch {
			ScrollDisplayPosition.Top => rowNumber - 1,
			ScrollDisplayPosition.Center => rowNumber - (VisibleRowCount / 2) + 1,
			ScrollDisplayPosition.Bottom => rowNumber - VisibleRowCount + 2,
			_ => rowNumber
		};

		ScrollPosition = Math.Max(0, Math.Min(newPos, MaxScrollPosition));
	}

	public void GoBack()
	{
		ScrollToLocation(History.GoBack(), false);
	}

	public void GoForward()
	{
		ScrollToLocation(History.GoForward(), false);
	}

	public void ScrollToTop()
	{
		if(SelectedFile != null) {
			ScrollToLocation(new SourceCodeLocation(SelectedFile, 0), true);
		}
	}

	public void ScrollToBottom()
	{
		if(SelectedFile != null) {
			ScrollToLocation(new SourceCodeLocation(SelectedFile, SelectedFile.Data.Length - 1), true);
		}
	}

	public void SetSelectedRow(int rowNumber)
	{
		SelectionStart = rowNumber;
		SelectionEnd = rowNumber;
		SelectedRow = rowNumber;
		SelectionAnchor = rowNumber;
		InvalidateVisual();
	}

	public void SetSelectedRow(int rowNumber, bool scrollToRow)
	{
		SetSelectedRow(rowNumber);
		if(scrollToRow) {
			ScrollToRowNumber(rowNumber);
		}
	}

	public bool IsSelected(int rowNumber)
	{
		return rowNumber >= SelectionStart && rowNumber <= SelectionEnd;
	}

	public void Scroll(int offset)
	{
		ScrollPosition = Math.Max(0, Math.Min(ScrollPosition + offset, MaxScrollPosition));
	}

	public AddressInfo? GetSelectedRowAddress()
	{
		if(SelectedFile == null) {
			return null;
		}

		return SymbolProvider.GetLineAddress(SelectedFile, SelectedRow);
	}

	public void SetViewer(DisassemblyViewer? viewer)
	{
		_viewer = viewer;
	}

	public int? GetActiveLineIndex()
	{
		if(SelectedFile == null || ActiveAddress == null) {
			return null;
		}

		AddressInfo absAddr = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = ActiveAddress.Value, Type = CpuType.ToMemoryType() });
		if(absAddr.Address < 0) {
			return null;
		}

		for(int i = 0, len = SelectedFile.Data.Length; i < len; i++) {
			AddressInfo? address = SymbolProvider.GetLineAddress(SelectedFile, i);
			if(address?.Address == absAddr.Address) {
				return i;
			}
		}
		return null;
	}

	public Breakpoint? GetBreakpoint(int lineNumber)
	{
		if(SelectedFile == null) {
			return null;
		}

		AddressInfo? address = SymbolProvider.GetLineAddress(SelectedFile, lineNumber);
		if(address?.Address >= 0) {
			return BreakpointManager.GetMatchingBreakpoint(address.Value, CpuType);
		}
		return null;
	}

	public void SetRefreshScrollBar(Action? refreshScrollbar)
	{
		_refreshScrollbar = refreshScrollbar;
	}

	public List<FindResultViewModel> FindAllOccurrences(string search, DisassemblySearchOptions options)
	{
		List<FindResultViewModel> results = new();
		foreach(SourceFileInfo file in SourceFiles) {
			for(int i = 0; i < file.Data.Length; i++) {
				if(file.Data[i].Contains(search, StringComparison.OrdinalIgnoreCase)) {
					AddressInfo? absAddress = SymbolProvider.GetLineAddress(file, i);
					LocationInfo loc = new() { AbsAddress = absAddress, SourceLocation = new SourceCodeLocation(file, i) };
					results.Add(new FindResultViewModel(loc, $"{file.Name}:{i}", file.Data[i]));
				}
			}
		}
		return results;
	}
}
