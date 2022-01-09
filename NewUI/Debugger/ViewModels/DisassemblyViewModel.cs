using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Linq;
using System.Reactive;

namespace Mesen.Debugger.ViewModels
{
	public class DisassemblyViewModel : ViewModelBase
	{
		[Reactive] public ICodeDataProvider? DataProvider { get; set; } = null;
		[Reactive] public BaseStyleProvider StyleProvider { get; set; } = new BaseStyleProvider();
		[Reactive] public int ScrollPosition { get; set; } = 0;
		[Reactive] public int MaxScrollPosition { get; private set; } = 10000;
		[Reactive] public int TopAddress { get; private set; } = 0;
		[Reactive] public CodeLineData[] Lines { get; private set; } = Array.Empty<CodeLineData>();

		public DebugConfig Config { get; private set; }
		public int VisibleRowCount { get; set; } = 100;

		private int _ignoreScrollUpdates = 0;

		[Obsolete("For designer only")]
		public DisassemblyViewModel(): this(new DebugConfig(), CpuType.Cpu) { }

		public DisassemblyViewModel(DebugConfig config, CpuType cpuType)
		{
			Config = config;

			if(Design.IsDesignMode) {
				return;
			}

			DataProvider = new CodeDataProvider(cpuType);

			this.WhenAnyValue(x => x.DataProvider).Subscribe(x => Refresh());
			this.WhenAnyValue(x => x.TopAddress).Subscribe(x => Refresh());

			int lastValue = ScrollPosition;
			this.WhenAnyValue(x => x.ScrollPosition).Subscribe(scrollPos => {
				int gap = scrollPos - lastValue;
				lastValue = scrollPos;
				if(_ignoreScrollUpdates > 0) {
					return;
				}

				if(gap != 0) {
					if(Math.Abs(gap) <= 10) {
						Scroll(gap);
					} else {
						int lineCount = DataProvider?.GetLineCount() ?? 0;
						TopAddress = Math.Max(0, Math.Min(lineCount - 1, (int)((double)lineCount / MaxScrollPosition * ScrollPosition)));
					}
				}
			});
		}

		public void Scroll(int lineNumberOffset)
		{
			ICodeDataProvider? dp = DataProvider;
			if(dp == null) {
				return;
			}
			
			SetTopAddress(dp.GetRowAddress(TopAddress, lineNumberOffset));
		}

		public void ScrollToTop()
		{
			SetTopAddress(0);
		}

		public void ScrollToBottom()
		{
			SetTopAddress((DataProvider?.GetLineCount() ?? 0) - 1);
		}

		public void InvalidateVisual()
		{
			//Force DisassemblyViewer to refresh
			Lines = Lines.ToArray();
		}

		public void Refresh()
		{
			ICodeDataProvider? dp = DataProvider;
			if(dp == null) {
				return;
			}

			CodeLineData[] lines = dp.GetCodeLines(TopAddress, VisibleRowCount);
			Lines = lines;

			if(lines.Length > 0 && lines[0].Address >= 0) {
				SetTopAddress(lines[0].Address);
			}
		}

		private void SetTopAddress(int address)
		{
			int lineCount = DataProvider?.GetLineCount() ?? 0;
			address = Math.Max(0, Math.Min(lineCount - 1, address));

			_ignoreScrollUpdates++;
			TopAddress = address;
			ScrollPosition = (int)(TopAddress / (double)lineCount * MaxScrollPosition);
			_ignoreScrollUpdates--;
		}

		public void SetActiveAddress(uint pc)
		{
			StyleProvider.ActiveAddress = (int)pc;

			for(int i = 1; i < VisibleRowCount - 2 && i < Lines.Length; i++) {
				if(Lines[i].Address == (int)pc) {
					//Row is already visible, don't scroll
					return;
				}
			}

			ICodeDataProvider? dp = DataProvider;
			if(dp == null) {
				return;
			}
			TopAddress = dp.GetRowAddress((int)pc, -VisibleRowCount / 2 + 1);
		}
	}
}
