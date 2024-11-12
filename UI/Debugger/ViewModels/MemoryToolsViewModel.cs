using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Labels;
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
		[Reactive] public int ScrollPosition { get; set; }
		[Reactive] public HexEditorDataProvider? DataProvider { get; set; }
		[Reactive] public TblByteCharConverter? TblConverter { get; set; }

		[Reactive] public Enum[] AvailableMemoryTypes { get; set; } = Array.Empty<Enum>();

		[Reactive] public int SelectionStart { get; set; }
		[Reactive] public int SelectionLength { get; set; }
		
		[Reactive] public string LocationText { get; private set; } = "";
		[Reactive] public string LengthText { get; private set; } = "";

		[Reactive] public List<ContextMenuAction> FileMenuItems { get; set; } = new();
		[Reactive] public List<ContextMenuAction> ViewMenuItems { get; set; } = new();
		[Reactive] public List<ContextMenuAction> SearchMenuItems { get; set; } = new();
		[Reactive] public List<ContextMenuAction> ToolbarItems { get; set; } = new();

		[Reactive] public int MaxScrollValue { get; private set; }

		private HexEditor _editor;
		public MemoryToolsDisplayOptionsViewModel Options { get; }
		public MemoryViewerFindViewModel Search { get; }

		[Obsolete("For designer only")]
		public MemoryToolsViewModel() : this(null!) { }

		public MemoryToolsViewModel(HexEditor editor)
		{
			Config = ConfigManager.Config.Debug.HexEditor.Clone();

			Search = AddDisposable(new MemoryViewerFindViewModel(this));
			_editor = editor;

			Options = AddDisposable(new MemoryToolsDisplayOptionsViewModel(this));
			ScrollPosition = 0;

			if(Design.IsDesignMode) {
				return;
			}

			if(DebugWorkspaceManager.Workspace.TblMappings.Length > 0) {
				TblConverter = TblLoader.Load(DebugWorkspaceManager.Workspace.TblMappings);
			}

			UpdateAvailableMemoryTypes();

			AddDisposable(this.WhenAnyValue(x => x.SelectionStart, x => x.SelectionLength).Subscribe(((int start, int length) o) => {
				string location;
				string length = "";
				if(o.length <= 1) {
					location = ResourceHelper.GetMessage("HexEditor_Location", o.start.ToString("X2"));
				} else {
					location = ResourceHelper.GetMessage("HexEditor_Selection", o.start.ToString("X2"), (o.start + o.length - 1).ToString("X2"));
					length = ResourceHelper.GetMessage("HexEditor_Length", o.length, o.length.ToString("X2"));
				}

				if(Config.MemoryType.IsWordAddressing()) {
					if(o.length <= 1) {
						location += $" (${o.start / 2:X2}.w)";
					} else {
						location += $" (${o.start / 2:X2}.w - ${(o.start + o.length - 1)/2:X2}.w)";
					}
				}

				CodeLabel? label = LabelManager.GetLabel(new AddressInfo() { Address = o.start, Type = Config.MemoryType });
				if(label != null && (o.length <= 1 || o.length == label.Length)) {
					location += $" ({label.Label})";
				}

				LocationText = location;
				LengthText = length;
			}));

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => UpdateDataProvider()));
			AddDisposable(this.WhenAnyValue(x => x.TblConverter).Subscribe(x => UpdateDataProvider()));

			AddDisposable(this.WhenAnyValue(
				x => x.Config.MemoryType,
				x => x.Config.BytesPerRow
			).Subscribe(((MemoryType memType, int bytesPerRow) o) => {
				MaxScrollValue = (DebugApi.GetMemorySize(o.memType) / o.bytesPerRow) - 1;
				_editor.SetCursorPosition(0, false, true);
			}));
		}

		private void UpdateDataProvider()
		{
			DataProvider = new HexEditorDataProvider(
				Config.MemoryType,
				Config,
				TblConverter
			);

			MaxScrollValue = (DataProvider.Length / Config.BytesPerRow) - 1;
		}

		public void UpdateAvailableMemoryTypes()
		{
			AvailableMemoryTypes = Enum.GetValues<MemoryType>().Where(t => t.SupportsMemoryViewer() && DebugApi.GetMemorySize(t) > 0).Cast<Enum>().ToArray();
			if(!AvailableMemoryTypes.Contains(Config.MemoryType)) {
				Config.MemoryType = (MemoryType)(AvailableMemoryTypes.FirstOrDefault() ?? MemoryType.SnesMemory);
			}

			Options.UpdateAvailableOptions();
			UpdateDataProvider();
		}

		public void NavigateTo(NavType nav)
		{
			switch(nav) {
				case NavType.PrevCode:
				case NavType.NextCode:
				case NavType.PrevData:
				case NavType.NextData:
				case NavType.PrevUnknown:
				case NavType.NextUnknown:
					SearchNextCdlMatch(nav);
					break;

				case NavType.PrevRead:
				case NavType.NextRead:
				case NavType.PrevWrite:
				case NavType.NextWrite:
				case NavType.PrevExec:
				case NavType.NextExec:
					SearchNextAccessMatch(nav);
					break;
			}
		}

		private void SearchNextCdlMatch(NavType nav)
		{
			if(!Config.MemoryType.SupportsCdl()) {
				return;
			}

			int memSize = DebugApi.GetMemorySize(Config.MemoryType);
			VirtualArray<CdlFlags> cdlData = new VirtualArray<CdlFlags>(memSize, (start, end) => {
				return DebugApi.GetCdlData((uint)start, (uint)(end - start + 1), Config.MemoryType);
			});

			Func<int, bool> predicate = nav switch {
				NavType.PrevCode or NavType.NextCode => (x) => cdlData[x].HasFlag(CdlFlags.Code),
				NavType.PrevData or NavType.NextData => (x) => cdlData[x].HasFlag(CdlFlags.Data),
				NavType.PrevUnknown or NavType.NextUnknown => (x) => !cdlData[x].HasFlag(CdlFlags.Code) && !cdlData[x].HasFlag(CdlFlags.Data),
				_ => (x) => false
			};

			SearchNextMatch(nav, predicate, memSize);
		}

		private void SearchNextAccessMatch(NavType nav)
		{
			int memSize = DebugApi.GetMemorySize(Config.MemoryType);
			VirtualArray<AddressCounters> counters = new VirtualArray<AddressCounters>(memSize, (start, end) => {
				return DebugApi.GetMemoryAccessCounts((uint)start, (uint)(end - start + 1), Config.MemoryType);
			});

			TimingInfo timing = EmuApi.GetTimingInfo(Config.MemoryType.ToCpuType());
			double cyclesPerFrame = timing.MasterClockRate / timing.Fps;
			int framesToFade = Config.FadeSpeed.ToFrameCount();
			double targetClock = framesToFade == 0 ? 0 : timing.MasterClock - cyclesPerFrame * framesToFade;

			Func<int, bool> predicate = nav switch {
				NavType.PrevRead or NavType.NextRead => (x) => counters[x].ReadStamp > targetClock,
				NavType.PrevWrite or NavType.NextWrite => (x) => counters[x].WriteStamp > targetClock,
				NavType.PrevExec or NavType.NextExec => (x) => counters[x].ExecStamp > targetClock,
				_ => (x) => false
			};

			SearchNextMatch(nav, predicate, memSize);
		}

		private void SearchNextMatch(NavType nav, Func<int, bool> predicate, int memSize)
		{
			int direction = nav.ToString().Contains("Prev") ? -1 : 1;
			bool skipCurrent = predicate(_editor.SelectionStart);
			for(int i = 0; i < memSize; i++) {
				int index = (i * direction + _editor.SelectionStart) % memSize;
				if(index < 0) {
					index += memSize;
				}

				if(predicate(index)) {
					if(!skipCurrent) {
						_editor.SetCursorPosition(index);
						break;
					}
				} else {
					skipCurrent = false;
				}
			}
		}

		public bool Find(SearchDirection direction)
		{
			SearchData? data = Search.GetSearchData();
			if(data == null) {
				return false;
			}
			
			MemoryType memType = Config.MemoryType;
			TimingInfo timingInfo = Search.IsAccessFiltered && Search.FilterTimeSpanEnabled ? EmuApi.GetTimingInfo(memType.ToCpuType()) : new();

			int offset = direction == SearchDirection.Forward ? 1 : -1;
			int endPos = SelectionStart;
			int memSize = DebugApi.GetMemorySize(memType);
			int searchLen = data.Data.Length;

			int startPos = endPos + ((direction == SearchDirection.Backward || SelectionLength != 0) ? offset : 0);

			//TODO this can be a bit slow in edge cases depending on search+filters.
			//(e.g search 00 00 00 in a code segment with no matches)
			VirtualArray<byte> mem = new VirtualArray<byte>(memSize, (start, end) => {
				return DebugApi.GetMemoryValues(memType, (uint)start, (uint)end);
			});

			bool firstLoop = true;
			for(int i = startPos; ; i += offset) {
				if(i < 0) {
					//Wrap around to the end
					i = memSize - searchLen;
				} else if(i > memSize - searchLen) {
					//Wrap around to the start
					i = 0;
				}

				if(firstLoop) {
					//Adjust startPos based on wrap around calculated above
					startPos = i;
					firstLoop = false;
				} else {
					if(i == startPos) {
						//Break if all start positions were checked
						break;
					}
				}

				int j = 0;
				while(j < searchLen && i + j < memSize) {
					byte val = mem[i + j];
					if(data.Data[j] >= 0 && data.Data[j] != val && (data.DataAlt == null || data.DataAlt[j] != val)) {
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
