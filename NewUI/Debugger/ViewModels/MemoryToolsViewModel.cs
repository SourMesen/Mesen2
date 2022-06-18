using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels
{
	public class MemoryToolsViewModel : DisposableViewModel
	{
		[Reactive] public HexEditorConfig Config { get; set; }
		[Reactive] public FontConfig FontConfig { get; set; }
		[Reactive] public int ScrollPosition { get; set; }
		[Reactive] public HexEditorDataProvider? DataProvider { get; set; }
		[Reactive] public TblByteCharConverter? TblConverter { get; set; }

		[Reactive] public Enum[] AvailableMemoryTypes { get; set; } = Array.Empty<Enum>();

		[Reactive] public int SelectionStart { get; set; }
		[Reactive] public int SelectionLength { get; set; }
		[Reactive] public string StatusBarText { get; private set; } = "";

		[Reactive] public List<ContextMenuAction> FileMenuItems { get; set; } = new();
		[Reactive] public List<ContextMenuAction> SearchMenuItems { get; set; } = new();
		[Reactive] public List<ContextMenuAction> ToolbarItems { get; set; } = new();

		[ObservableAsProperty] public int MaxScrollValue { get; }

		private HexEditor _editor;
		public MemoryToolsDisplayOptionsViewModel Options { get; }
		public MemorySearchViewModel Search { get; }

		[Obsolete("For designer only")]
		public MemoryToolsViewModel() : this(null!) { }

		public MemoryToolsViewModel(HexEditor editor)
		{
			Search = new(this);
			_editor = editor;

			Options = AddDisposable(new MemoryToolsDisplayOptionsViewModel());
			Config = ConfigManager.Config.Debug.HexEditor;
			FontConfig = ConfigManager.Config.Debug.Font;
			ScrollPosition = 0;

			if(Design.IsDesignMode) {
				return;
			}

			if(DebugWorkspaceManager.Workspace.TblMappings.Length > 0) {
				TblConverter = TblLoader.Load(DebugWorkspaceManager.Workspace.TblMappings);
			}

			UpdateAvailableMemoryTypes();

			AddDisposable(this.WhenAnyValue(x => x.SelectionStart, x => x.SelectionLength).Subscribe(((int start, int length) o) => {
				if(o.length <= 1) {
					StatusBarText = ResourceHelper.GetMessage("HexEditor_Location", o.start.ToString("X2"));
				} else {
					StatusBarText = ResourceHelper.GetMessage("HexEditor_Selection", o.start.ToString("X2"), (o.start + o.length - 1).ToString("X2"), o.length);
				}
			}));

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => UpdateDataProvider()));
			AddDisposable(this.WhenAnyValue(x => x.TblConverter).Subscribe(x => UpdateDataProvider()));

			AddDisposable(this.WhenAnyValue(
				x => x.Config.MemoryType,
				x => x.Config.BytesPerRow
			).Select(((MemoryType memType, int bytesPerRow) o) => (DebugApi.GetMemorySize(o.memType) / o.bytesPerRow) - 1).ToPropertyEx(this, x => x.MaxScrollValue));
		}

		private void UpdateDataProvider()
		{
			DataProvider = new HexEditorDataProvider(
				Config.MemoryType,
				Config,
				TblConverter
			);
		}

		public void UpdateAvailableMemoryTypes()
		{
			AvailableMemoryTypes = Enum.GetValues<MemoryType>().Where(t => t.SupportsMemoryViewer() && DebugApi.GetMemorySize(t) > 0).Cast<Enum>().ToArray();
			if(!AvailableMemoryTypes.Contains(Config.MemoryType)) {
				Config.MemoryType = (MemoryType)AvailableMemoryTypes.First();
			}

			Options.UpdateAvailableOptions();
		}

		public bool Find(SearchDirection direction)
		{
			SearchData? data = Search.GetSearchData();
			if(data == null) {
				return false;
			}
			
			TimingInfo timingInfo = Search.IsAccessFiltered && Search.FilterTimeSpanEnabled ? EmuApi.GetTimingInfo() : new();

			int offset = direction == SearchDirection.Forward ? 1 : -1;
			int endPos = SelectionStart;
			int startPos = endPos + offset;

			//TODO, change this to avoid loading all of memory type at once
			MemoryType memType = Config.MemoryType;
			int memSize = DebugApi.GetMemorySize(memType);
			byte[] mem = DebugApi.GetMemoryState(memType);
			int searchLen = data.Data.Length;

			bool firstLoop = true;
			for(int i = startPos; ; i += offset) {
				if(i < 0) {
					//Wrap around to the end
					i = memSize - searchLen;
				} else if(i > memSize - searchLen) {
					//Wrap around to the start
					i = 0;
				}

				if(!firstLoop && i == startPos) {
					break;
				}
				firstLoop = false;

				int j = 0;
				while(j < searchLen) {
					if(data.Data[j] != mem[i+j] && (data.DataAlt == null || data.DataAlt[j] != mem[i+j])) {
						break;
					} else if(j == searchLen - 1) {
						//Match
						if(CheckSearchFilters(memType, i, searchLen, timingInfo)) {
							_editor.SetCursorPosition(i, scrollToTop: true);
							_editor.SelectionLength = searchLen;
							Search.ShowNotFoundError = false;
							return true;
						}
					}

					j++;
				}
			}

			Search.ShowNotFoundError = true;
			return false;
		}

		private bool CheckSearchFilters(MemoryType memType, int pos, int searchLen, TimingInfo timingInfo)
		{
			if(Search.IsDataTypeFiltered) {
				CdlFlags[] cdlData = DebugApi.GetCdlData((uint)pos, (uint)searchLen, memType);
				foreach(CdlFlags flags in cdlData) {
					bool match = false;
					match |= Search.FilterCode && flags.HasFlag(CdlFlags.Code);
					match |= Search.FilterData && flags.HasFlag(CdlFlags.Data);
					match |= Search.FilterUnidentified && (!flags.HasFlag(CdlFlags.Code) && !flags.HasFlag(CdlFlags.Data));
					if(!match) {
						return false;
					}
				}
			}

			if(Search.IsAccessFiltered) {
				AddressCounters[] memCounters = DebugApi.GetMemoryAccessCounts((uint)pos, (uint)searchLen, memType);
				ulong stamp = 0;
				if(Search.FilterTimeSpanEnabled) {
					double clocksPerFrame = timingInfo.MasterClockRate / timingInfo.Fps;
					stamp = timingInfo.MasterClock - (ulong)(clocksPerFrame * Search.FilterTimeSpan);
				}

				foreach(AddressCounters counters in memCounters) {
					bool match = false;
					match |= Search.FilterRead && counters.ReadStamp > stamp;
					match |= Search.FilterWrite && counters.WriteStamp > stamp;
					match |= Search.FilterExec && counters.ExecStamp > stamp;
					match |= Search.FilterNotAccessed && (counters.ReadStamp <= stamp && counters.WriteStamp <= stamp && counters.ExecStamp <= stamp);
					if(!match) {
						return false;
					}
				}
			}

			return true;
		}
	}
}
