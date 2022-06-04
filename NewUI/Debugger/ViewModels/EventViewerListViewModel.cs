using Avalonia.Collections;
using DataBoxControl;
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

		[Reactive] public SortState SortState { get; set; } = new();

		public ICommand SortCommand { get; }

		public EventViewerListViewModel(EventViewerViewModel eventViewer)
		{
			EventViewer = eventViewer;
			DebugEvents = new();

			SortState.SetColumnSort("Scanline", ListSortDirection.Ascending, false);
			SortState.SetColumnSort("Cycle", ListSortDirection.Ascending, false);

			SortCommand = ReactiveCommand.Create<string?>(sortMemberPath => {
				RefreshList();
			});
		}

		public void RefreshList()
		{
			_debugEvents = DebugApi.GetDebugEvents(EventViewer.CpuType);

			Array.Sort(_debugEvents, (a, b) => {
				int result = 0;

				foreach((string column, ListSortDirection order) in SortState.SortOrder) {
					void Compare(string name, Func<int> compare)
					{
						if(result == 0 && column == name) {
							result = compare() * (order == ListSortDirection.Ascending ? 1 : -1);
						}
					}

					Compare("ProgramCounter", () => a.ProgramCounter.CompareTo(b.ProgramCounter));
					Compare("Scanline", () => a.Scanline.CompareTo(b.Scanline));
					Compare("Cycle", () => a.Cycle.CompareTo(b.Cycle));
					Compare("Type", () => a.Type.CompareTo(b.Type));
					Compare("Address", () => a.Operation.Address.CompareTo(b.Operation.Address));
					Compare("Value", () => a.Operation.Value.CompareTo(b.Operation.Value));

					if(result != 0) {
						return result;
					}
				}

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
