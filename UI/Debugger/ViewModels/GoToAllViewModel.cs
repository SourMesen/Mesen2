using Avalonia.Controls.Selection;
using Mesen.Debugger.Integration;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Mesen.Debugger.ViewModels;

public class GoToAllViewModel : DisposableViewModel
{
	[Reactive] public string SearchString { get; set; } = "";
	[Reactive] public List<SearchResultInfo> SearchResults { get; set; } = new();
	[Reactive] public SelectionModel<SearchResultInfo?> SelectionModel { get; set; } = new();
	[Reactive] public SearchResultInfo? SelectedItem { get; set; } = null;
	[Reactive] public bool CanSelect { get; set; } = false;

	[Obsolete("For designer only")]
	public GoToAllViewModel() : this(CpuType.Snes, GoToAllOptions.None) { }

	public GoToAllViewModel(CpuType cpuType, GoToAllOptions options, ISymbolProvider? symbolProvider = null)
	{
		AddDisposable(this.WhenAnyValue(x => x.SearchString).Subscribe(x => {
			SearchResults = SearchHelper.GetGoToAllResults(cpuType, SearchString, options, symbolProvider);
			if(SearchResults.Count > 0) {
				SelectionModel.SelectedIndex = 0;
			}
		}));

		AddDisposable(this.WhenAnyValue(x => x.SelectionModel.SelectedItem).Subscribe(item => {
			CanSelect = item?.Disabled == false;
		}));
	}
}
