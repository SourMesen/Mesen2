using Avalonia;
using Avalonia.Controls;
using Avalonia.Threading;
using Dock.Model.Core;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Views;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Linq;
using System.Text;
using Tmds.DBus.Protocol;

namespace Mesen.Debugger.ViewModels
{
	public class DisassemblyViewModel : DisposableViewModel, ISelectableModel
	{
		public ICodeDataProvider DataProvider { get; }
		public CpuType CpuType { get; }
		public DebuggerWindowViewModel Debugger { get; }
		public DisassemblyViewStyleProvider StyleProvider { get; }

		[Reactive] public int ScrollPosition { get; set; } = 0;
		[Reactive] public int MaxScrollPosition { get; private set; } = 1000000000;
		[Reactive] public int TopAddress { get; private set; } = 0;
		[Reactive] public CodeLineData[] Lines { get; private set; } = Array.Empty<CodeLineData>();

		[Reactive] public int? ActiveAddress { get; set; }
		[Reactive] public int SelectedRowAddress { get; set; }
		[Reactive] public int SelectionAnchor { get; set; }
		[Reactive] public int SelectionStart { get; set; }
		[Reactive] public int SelectionEnd { get; set; }

		public QuickSearchViewModel QuickSearch { get; } = new QuickSearchViewModel();
		public NavigationHistory<int> History { get; } = new();

		public bool ShowScrollBarMarkers => CpuType != CpuType.NecDsp;

		public DebugConfig Config { get; private set; }
		public int VisibleRowCount { get; set; } = 100;

		private DisassemblyViewer? _viewer = null;

		private int _ignoreScrollUpdates = 0;
		private Action? _refreshScrollbar = null;

		[Obsolete("For designer only")]
		public DisassemblyViewModel(): this(new DebuggerWindowViewModel(), new DebugConfig(), CpuType.Snes) { }

		public DisassemblyViewModel(DebuggerWindowViewModel debugger, DebugConfig config, CpuType cpuType)
		{
			Config = config;
			CpuType = cpuType;
			Debugger = debugger;
			StyleProvider = new DisassemblyViewStyleProvider(cpuType, this);
			DataProvider = new CodeDataProvider(cpuType);

			if(Design.IsDesignMode) {
				return;
			}

			QuickSearch.OnFind += QuickSearch_OnFind;

			AddDisposable(this.WhenAnyValue(x => x.TopAddress).Subscribe(x => Refresh()));

			AddDisposable(this.WhenAnyValue(x => x.QuickSearch.IsSearchBoxVisible).Subscribe(x => {
				if(!QuickSearch.IsSearchBoxVisible) {
					_viewer?.Focus();
				}
			}));

			int lastValue = ScrollPosition;
			AddDisposable(this.WhenAnyValue(x => x.ScrollPosition).Subscribe(scrollPos => {
				if(_viewer == null) {
					ScrollPosition = lastValue;
					return;
				}

				int gap = scrollPos - lastValue;
				lastValue = scrollPos;
				if(_ignoreScrollUpdates > 0) {
					return;
				}

				if(gap != 0) {
					if(Math.Abs(gap) <= 10) {
						Scroll(gap);
					} else {
						int lineCount = DataProvider.GetLineCount();
						TopAddress = Math.Max(0, Math.Min(lineCount - 1, (int)((double)lineCount / MaxScrollPosition * ScrollPosition)));
					}
				}
			}));
		}

		private void QuickSearch_OnFind(OnFindEventArgs e)
		{
			DisassemblySearchOptions options = new() { SearchBackwards = e.Direction == SearchDirection.Backward, SkipFirstLine = e.SkipCurrent };
			int findAddress = DebugApi.SearchDisassembly(CpuType, e.SearchString.ToLowerInvariant(), SelectedRowAddress, options);
			if(findAddress >= 0) {
				Dispatcher.UIThread.Post(() => {
					SetSelectedRow(findAddress, true, true);
				});
				e.Success = true;
			} else {
				e.Success = false;
			}
		}

		public void SetViewer(DisassemblyViewer? viewer)
		{
			_viewer = viewer;
		}

		public void Scroll(int lineNumberOffset)
		{
			if(lineNumberOffset == 0) {
				return;
			}

			SetTopAddress(DataProvider.GetRowAddress(TopAddress, lineNumberOffset));
		}

		public void ScrollToTop()
		{
			SetSelectedRow(0, false, true);
			ScrollToAddress(0, ScrollDisplayPosition.Top);
		}

		public void ScrollToBottom()
		{
			int address = DataProvider.GetLineCount() - 1;
			SetSelectedRow(address, false, true);
			ScrollToAddress((uint)address, ScrollDisplayPosition.Bottom);
		}

		public void InvalidateVisual()
		{
			//Force DisassemblyViewer to refresh
			Lines = Lines.ToArray();
		}

		public void Refresh()
		{
			CodeLineData[] lines = DataProvider.GetCodeLines(TopAddress, VisibleRowCount);
			Lines = lines;

			if(lines.Length > 0 && lines[0].Address >= 0) {
				SetTopAddress(lines[0].Address);
			}

			_refreshScrollbar?.Invoke();
		}

		public void SetRefreshScrollBar(Action? refreshScrollbar)
		{
			_refreshScrollbar = refreshScrollbar;
		}

		private void SetTopAddress(int address)
		{
			int lineCount = DataProvider.GetLineCount();
			address = Math.Max(0, Math.Min(lineCount - 1, address));

			_ignoreScrollUpdates++;
			TopAddress = address;
			ScrollPosition = (int)(TopAddress / (double)lineCount * MaxScrollPosition);
			_ignoreScrollUpdates--;
		}

		public void SetActiveAddress(int? pc)
		{
			ActiveAddress = pc;
			if(pc != null) {
				int currentAddress = SelectedRowAddress;
				SetSelectedRow((int)pc);
				bool scrolled = ScrollToAddress((uint)pc, ScrollDisplayPosition.Center, Config.Debugger.KeepActiveStatementInCenter);
				if(scrolled) {
					if(currentAddress != 0 || History.CanGoBack()) {
						History.AddHistory(currentAddress);
					}
					History.AddHistory((int)pc);
				}
			}
		}

		public bool IsSelected(int address)
		{
			return address >= SelectionStart && address <= SelectionEnd;
		}

		public AddressInfo? GetSelectedRowAddress()
		{
			return new AddressInfo() {
				Address = SelectedRowAddress,
				Type = CpuType.ToMemoryType()
			};
		}

		public void GoBack()
		{
			SetSelectedRow(History.GoBack(), true, false);
		}

		public void GoForward()
		{
			SetSelectedRow(History.GoForward(), true, false);
		}

		public void SetSelectedRow(int address)
		{
			SetSelectedRow(address, false);
		}

		public void SetSelectedRow(int address, bool scrollToRow = false, bool addToHistory = false)
		{
			int currentAddress = SelectedRowAddress;
			SelectionStart = address;
			SelectionEnd = address;
			SelectedRowAddress = address;
			SelectionAnchor = address;

			if(addToHistory) {
				if(currentAddress != 0 || History.CanGoBack()) {
					History.AddHistory(currentAddress);
				}
				History.AddHistory(address);
			}

			InvalidateVisual();

			if(scrollToRow) {
				ScrollToAddress((uint)address);
			}
		}

		public void MoveCursor(int rowOffset, bool extendSelection)
		{
			int address = DataProvider.GetRowAddress(SelectedRowAddress, rowOffset);
			if(extendSelection) {
				ResizeSelectionTo(address);
			} else {
				SetSelectedRow(address);
				ScrollToAddress((uint)address, rowOffset < 0 ? ScrollDisplayPosition.Top : ScrollDisplayPosition.Bottom);
			}
		}

		public void ResizeSelectionTo(int address)
		{
			if(SelectedRowAddress == address) {
				return;
			}

			bool anchorTop = SelectionAnchor == SelectionStart;
			if(anchorTop) {
				if(address < SelectionStart) {
					SelectionEnd = SelectionStart;
					SelectionStart = address;
				} else {
					SelectionEnd = address;
				}
			} else {
				if(address < SelectionEnd) {
					SelectionStart = address;
				} else {
					SelectionStart = SelectionEnd;
					SelectionEnd = address;
				}
			}

			ScrollDisplayPosition displayPos = SelectedRowAddress < address ? ScrollDisplayPosition.Bottom : ScrollDisplayPosition.Top;
			SelectedRowAddress = address;
			ScrollToAddress((uint)address, displayPos);

			InvalidateVisual();
		}

		private bool IsAddressVisible(int address)
		{
			for(int i = 1; i < VisibleRowCount - 2 && i < Lines.Length; i++) {
				if(Lines[i].Address == address) {
					return true;
				}
			}

			return false;
		}

		private bool ScrollToAddress(uint pc, ScrollDisplayPosition position = ScrollDisplayPosition.Center, bool forceScroll = false)
		{
			if(!forceScroll && IsAddressVisible((int)pc)) {
				//Row is already visible, don't scroll
				return false;
			}

			ICodeDataProvider dp = DataProvider;

			switch(position) {
				case ScrollDisplayPosition.Top: TopAddress = dp.GetRowAddress((int)pc, -1); break;
				case ScrollDisplayPosition.Center: TopAddress = dp.GetRowAddress((int)pc, -VisibleRowCount / 2 + 1); break;
				case ScrollDisplayPosition.Bottom: TopAddress = dp.GetRowAddress((int)pc, -VisibleRowCount + 2); break;
			}

			if(!IsAddressVisible((int)pc)) {
				TopAddress = dp.GetRowAddress(TopAddress, TopAddress < pc ? 1 : -1);
			}
			return true;
		}

		public void CopySelection()
		{
			DebuggerConfig cfg = Config.Debugger;
			string code = GetSelection(cfg.CopyAddresses, cfg.CopyByteCode, cfg.CopyComments, cfg.CopyBlockHeaders, out _, false);
			ApplicationHelper.GetMainWindow()?.Clipboard?.SetTextAsync(code);
		}

		public string GetSelection(bool getAddresses, bool getByteCode, bool getComments, bool getHeaders, out int byteCount, bool skipGeneratedJmpSubLabels)
		{
			ICodeDataProvider dp = DataProvider;
			
			const int commentSpacingCharCount = 25;

			int addrSize = dp.CpuType.GetAddressSize();
			string addrFormat = "X" + addrSize;
			StringBuilder sb = new StringBuilder();
			int i = SelectionStart;
			int endAddress = 0;
			CodeLineData? prevLine = null;
			AddressDisplayType addressDisplayType = Config.Debugger.AddressDisplayType;
			do {
				CodeLineData[] data = dp.GetCodeLines(i, 5000);
				for(int j = 0; j < data.Length; j++) {
					CodeLineData lineData = data[j];
					if(prevLine?.Address == lineData.Address && prevLine?.Text == lineData.Text) {
						continue;
					}

					if(lineData.Address > SelectionEnd) {
						i = lineData.Address;
						break;
					}

					string codeString = lineData.Text.Trim();
					if(lineData.Flags.HasFlag(LineFlags.BlockEnd) || lineData.Flags.HasFlag(LineFlags.BlockStart)) {
						if(getHeaders) {
							codeString = "--------" + codeString + "--------";
						} else {
							if(j == data.Length - 1) {
								i = lineData.Address;
							}
							continue;
						}
					}

					prevLine = lineData;

					int padding = Math.Max(commentSpacingCharCount, codeString.Length);
					if(codeString.Length == 0) {
						padding = 0;
					}

					codeString = codeString.PadRight(padding);

					bool indentText = !(lineData.Flags.HasFlag(LineFlags.ShowAsData) || lineData.Flags.HasFlag(LineFlags.BlockStart) || lineData.Flags.HasFlag(LineFlags.BlockEnd) || lineData.Flags.HasFlag(LineFlags.Label) || (lineData.Flags.HasFlag(LineFlags.Comment) && lineData.Text.Length == 0));
					string line = (indentText ? "  " : "") + codeString;
					if(getByteCode) {
						line = lineData.ByteCodeStr.PadRight(13) + line;
					}
					if(getAddresses) {
						string addressText = lineData.GetAddressText(addressDisplayType, addrFormat);
						line = addressText.PadRight(addrSize) + "  " + line;
					}
					if(getComments && !string.IsNullOrWhiteSpace(lineData.Comment)) {
						line = line + lineData.Comment;
					}

					//Skip lines that contain a jump/sub "label" (these aren't 
					bool skipLine = skipGeneratedJmpSubLabels && lineData.Flags.HasFlag(LineFlags.Label) && lineData.Text.StartsWith("$");

					string result = line.TrimEnd();
					if(!skipLine && result.Length > 0) {
						sb.AppendLine(result);
					}

					i = lineData.Address;
					endAddress = lineData.Address + lineData.OpSize - 1;
				}
			} while(i < SelectionEnd);

			if(SelectionStart <= endAddress) {
				byteCount = endAddress - SelectionStart + 1;
			} else {
				byteCount = 0;
			}

			return sb.ToString();
		}
	}

	public enum ScrollDisplayPosition
	{
		Top,
		Center,
		Bottom
	}
}
