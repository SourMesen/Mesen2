using Avalonia.Collections;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.ComponentModel;
using System.Linq;
using System.Reactive.Linq;
using System.Windows.Input;

namespace Mesen.Debugger.ViewModels
{
	public class EventViewerListViewModel : DisposableViewModel
	{
		private DebugEventInfo[] _debugEvents = new DebugEventInfo[0];

		public AvaloniaList<DebugEventViewModel> DebugEvents { get; }
		public EventViewerViewModel EventViewer { get; }

		[Reactive] public ListSortDirection? SortProgramCounter { get; set; }
		[Reactive] public ListSortDirection? SortScanline { get; set; }
		[Reactive] public ListSortDirection? SortCycle { get; set; }
		[Reactive] public ListSortDirection? SortType { get; set; }
		[Reactive] public ListSortDirection? SortAddress { get; set; }
		[Reactive] public ListSortDirection? SortValue { get; set; }
		
		public ICommand SortCommand { get; }

		public EventViewerListViewModel(EventViewerViewModel eventViewer)
		{
			EventViewer = eventViewer;
			DebugEvents = new();

			SortScanline = ListSortDirection.Ascending;
			SortCycle = ListSortDirection.Ascending;
			
			SortCommand = ReactiveCommand.Create<string?>(sortMemberPath => {
				RefreshList();
			});
		}

		public void RefreshList()
		{
			_debugEvents = DebugApi.GetDebugEvents(EventViewer.CpuType);

			Array.Sort(_debugEvents, (a, b) => {
				int result = 0;

				void Compare(ListSortDirection? order, Func<int> compare)
				{
					if(order.HasValue && result == 0) {
						result = compare() * (order == ListSortDirection.Ascending ? 1 : -1);
					}
				}

				Compare(SortProgramCounter, () => a.ProgramCounter.CompareTo(b.ProgramCounter));
				Compare(SortScanline, () => a.Scanline.CompareTo(b.Scanline));
				Compare(SortCycle, () => a.Cycle.CompareTo(b.Cycle));
				Compare(SortType, () => a.Type.CompareTo(b.Type));
				Compare(SortAddress, () => a.Operation.Address.CompareTo(b.Operation.Address));
				Compare(SortValue, () => a.Operation.Value.CompareTo(b.Operation.Value));

				if(result == 0) {
					result = a.Scanline.CompareTo(b.Scanline);
					if(result == 0) {
						result = a.Cycle.CompareTo(b.Cycle);
					}
				}
				
				return result;
			});

			if(DebugEvents.Count < _debugEvents.Length) {
				for(int i = 0; i < DebugEvents.Count; i++) {
					DebugEvents[i].Update(_debugEvents, i);
				}
				DebugEvents.AddRange(Enumerable.Range(DebugEvents.Count, _debugEvents.Length - DebugEvents.Count).Select(i => new DebugEventViewModel(_debugEvents, i)));
			} else if(DebugEvents.Count > _debugEvents.Length) {
				for(int i = 0; i < _debugEvents.Length; i++) {
					DebugEvents[i].Update(_debugEvents, i);
				}
				DebugEvents.RemoveRange(_debugEvents.Length, DebugEvents.Count - _debugEvents.Length);
			} else {
				for(int i = 0; i < DebugEvents.Count; i++) {
					DebugEvents[i].Update(_debugEvents, i);
				}
			}
		}
	}
}
