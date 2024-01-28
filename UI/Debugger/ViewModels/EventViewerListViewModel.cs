using Avalonia.Collections;
using Avalonia.Controls.Selection;
using DataBoxControl;
using Mesen.Config;
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
		public DebugEventInfo[] RawDebugEvents => _debugEvents;
		private DebugEventInfo[] _debugEvents = new DebugEventInfo[0];

		public MesenList<DebugEventViewModel> DebugEvents { get; }
		public SelectionModel<DebugEventViewModel?> Selection { get; set; } = new();
		public EventViewerViewModel EventViewer { get; }

		[Reactive] public SortState SortState { get; set; } = new();
		public List<int> ColumnWidths { get; } = ConfigManager.Config.Debug.EventViewer.ColumnWidths;

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
			{ "Color", (a, b) => a.Color.CompareTo(b.Color) },
			{ "ProgramCounter", (a, b) => a.ProgramCounter.CompareTo(b.ProgramCounter) },
			{ "Scanline", (a, b) => a.Scanline.CompareTo(b.Scanline) },
			{ "Cycle", (a, b) => a.Cycle.CompareTo(b.Cycle) },
			{ "Type", (a, b) => a.Type.CompareTo(b.Type) },
			{ "Address", (a, b) => {
				int result = a.Operation.Address.CompareTo(b.Operation.Address);
				return result != 0 ? result : a.RegisterId.CompareTo(b.RegisterId);
			} },
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
					DebugEvents[i].Update(_debugEvents, i, EventViewer.CpuType);
				}
				DebugEvents.AddRange(Enumerable.Range(DebugEvents.Count, _debugEvents.Length - DebugEvents.Count).Select(i => new DebugEventViewModel(_debugEvents, i, EventViewer.CpuType)));
			} else if(DebugEvents.Count > _debugEvents.Length) {
				for(int i = 0; i < _debugEvents.Length; i++) {
					DebugEvents[i].Update(_debugEvents, i, EventViewer.CpuType);
				}
				DebugEvents.RemoveRange(_debugEvents.Length, DebugEvents.Count - _debugEvents.Length);
			} else {
				for(int i = 0; i < DebugEvents.Count; i++) {
					DebugEvents[i].Update(_debugEvents, i, EventViewer.CpuType);
				}
			}
		}
	}
}
