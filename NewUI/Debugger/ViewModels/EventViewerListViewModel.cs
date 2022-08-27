using Avalonia.Collections;
using DataBoxControl;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reactive.Linq;
using System.Windows.Input;

namespace Mesen.Debugger.ViewModels
{
	public class EventViewerListViewModel : DisposableViewModel
	{
		private DebugEventInfo[] _debugEvents = new DebugEventInfo[0];

		public MesenList<DebugEventViewModel> DebugEvents { get; }
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

		private Dictionary<string, Func<DebugEventInfo, DebugEventInfo, int>> _comparers = new() {
			{ "ProgramCounter", (a, b) => a.ProgramCounter.CompareTo(b.ProgramCounter) },
			{ "Scanline", (a, b) => a.Scanline.CompareTo(b.Scanline) },
			{ "Cycle", (a, b) => a.Cycle.CompareTo(b.Cycle) },
			{ "Type", (a, b) => a.Type.CompareTo(b.Type) },
			{ "Address", (a, b) => a.Operation.Address.CompareTo(b.Operation.Address) },
			{ "Value", (a, b) => a.Operation.Value.CompareTo(b.Operation.Value)},
			{ "Default", (a, b) => {
				int result = a.Scanline.CompareTo(b.Scanline);
				return result != 0 ? result : a.Cycle.CompareTo(b.Cycle);
			} }
		};

		public void RefreshList()
		{
			_debugEvents = DebugApi.GetDebugEvents(EventViewer.CpuType);

			SortHelper.SortArray(_debugEvents, SortState.SortOrder, _comparers, "Default");

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
