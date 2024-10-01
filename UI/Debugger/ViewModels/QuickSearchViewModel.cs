using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Threading;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels;

public class QuickSearchViewModel : ViewModelBase
{
	[Reactive] public bool IsSearchBoxVisible { get; set; }
	[Reactive] public string SearchString { get; set; } = "";
	[Reactive] public bool IsErrorVisible { get; set; } = false;

	public delegate void OnFindEventHandler(OnFindEventArgs e);
	public event OnFindEventHandler? OnFind;

	private string _noMatchSearch = "";
	private bool _searchInProgress = false;
	private OnFindEventArgs? _searchArgs;

	private TextBox? _txtSearch = null;

	public QuickSearchViewModel()
	{
		this.WhenAnyValue(x => x.SearchString).Subscribe(x => {
			if(!string.IsNullOrWhiteSpace(x)) {
				if(!string.IsNullOrEmpty(_noMatchSearch) && x.StartsWith(_noMatchSearch)) {
					//Previous search gave no result, current search starts with the same string, so can't give results either, don't search
					return;
				}

				Find(SearchDirection.Forward, false);
			}
		});
	}

	public void Open()
	{
		IsSearchBoxVisible = true;
		_txtSearch?.FocusAndSelectAll();
	}

	public void Close(object? param = null)
	{
		IsSearchBoxVisible = false;
		SearchString = "";
	}

	public void FindPrev(object? param = null)
	{
		Find(SearchDirection.Backward, false);
	}

	public void FindNext(object? param = null)
	{
		Find(SearchDirection.Forward, true);
	}

	public void Find(SearchDirection dir, bool skipCurrent)
	{
		if(string.IsNullOrWhiteSpace(SearchString)) {
			Open();
			return;
		}

		_searchArgs = new OnFindEventArgs() { SearchString = SearchString, Direction = dir, SkipCurrent = skipCurrent };
		if(_searchInProgress) {
			return;
		}
		
		_searchInProgress = true;

		Task.Run(() => {
			OnFindEventArgs? args;
			OnFindEventArgs? lastArgs = null;
			while((args = Interlocked.Exchange(ref _searchArgs, null)) != null) {
				//Keep searching until the most recent search is processed
				OnFind?.Invoke(args);
				lastArgs = args;
			}

			Dispatcher.UIThread.Post(() => {
				if(lastArgs != null) {
					if(lastArgs.Success) {
						IsErrorVisible = false;
						_noMatchSearch = "";
					} else {
						IsErrorVisible = true;
						_noMatchSearch = SearchString;
					}
				}
			});
			_searchInProgress = false;
		});
	}

	public void SetSearchBox(TextBox txtSearch)
	{
		_txtSearch = txtSearch;
	}
}

public class OnFindEventArgs : EventArgs
{
	public string SearchString { get; init; } = "";
	public SearchDirection Direction { get; init; }
	public bool SkipCurrent { get; init; }
	public bool Success { get; set; } = true;
}