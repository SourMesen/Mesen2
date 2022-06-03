using Avalonia.Controls.Selection;
using Mesen.Config;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Windows.Input;

namespace Mesen.Debugger.ViewModels
{
	public class SpriteViewerListViewModel : DisposableViewModel
	{
		[Reactive] public bool ShowListView { get; set; }
		[Reactive] public double MinListViewHeight { get; set; }
		[Reactive] public double ListViewHeight { get; set; }
		[Reactive] public List<SpritePreviewModel>? SpritePreviews { get; set; } = null;
		[Reactive] public SelectionModel<SpritePreviewModel?> Selection { get; set; } = new();

		[Reactive] public ListSortDirection? SortIndex { get; set; }
		[Reactive] public ListSortDirection? SortX { get; set; }
		[Reactive] public ListSortDirection? SortY { get; set; }
		[Reactive] public ListSortDirection? SortSize { get; set; }
		[Reactive] public ListSortDirection? SortTileIndex { get; set; }
		[Reactive] public ListSortDirection? SortPalette { get; set; }
		[Reactive] public ListSortDirection? SortPriority { get; set; }
		[Reactive] public ListSortDirection? SortFlags { get; set; }

		public ICommand SortCommand { get; }
		public SpriteViewerViewModel SpriteViewer { get; }
		public SpriteViewerConfig Config { get; }
		
		private DateTime _lastRefresh = DateTime.MinValue;

		public SpriteViewerListViewModel(SpriteViewerViewModel viewer)
		{
			SpriteViewer = viewer;
			Config = viewer.Config;
			ShowListView = Config.ShowListView;
			ListViewHeight = Config.ShowListView ? Config.ListViewHeight : 0;

			SortIndex = ListSortDirection.Ascending;

			SortCommand = ReactiveCommand.Create<string?>(sortMemberPath => {
				RefreshList(true);
			});

			AddDisposable(this.WhenAnyValue(x => x.Selection.SelectedItem).Subscribe(x => {
				if(x != null) {
					SpriteViewer.SelectSprite(x.SpriteIndex);
				}
			}));
		}

		public void SelectSprite(int spriteIndex)
		{
			Selection.SelectedItem = SpritePreviews?.Find(x => x.SpriteIndex == spriteIndex);
		}

		public void ForceRefresh()
		{
			_lastRefresh = DateTime.MinValue;
		}

		public void RefreshList(bool force = false)
		{
			if(!force && (DateTime.Now - _lastRefresh).TotalMilliseconds < 70) {
				return;
			}

			_lastRefresh = DateTime.Now;

			if(!ShowListView) {
				SpritePreviews = null;
				return;
			}

			if(SpritePreviews == null || SpritePreviews.Count != SpriteViewer.SpritePreviews.Count) {
				SpritePreviews = SpriteViewer.SpritePreviews.Select(x => x.Clone()).ToList();
			}

			int? selectedIndex = Selection.SelectedItem?.SpriteIndex;

			List<SpritePreviewModel> newList = new(SpriteViewer.SpritePreviews.Select(x => x.Clone()).ToList());

			newList.Sort((a, b) => {
				int result = 0;

				void Compare(ListSortDirection? order, Func<int> compare)
				{
					if(order.HasValue && result == 0) {
						result = compare() * (order == ListSortDirection.Ascending ? 1 : -1);
					}
				}

				Compare(SortIndex, () => a.SpriteIndex.CompareTo(b.SpriteIndex));
				Compare(SortX, () => a.RawX.CompareTo(b.RawX));
				Compare(SortY, () => a.RawY.CompareTo(b.RawY));
				Compare(SortIndex, () => a.SpriteIndex.CompareTo(b.SpriteIndex));
				Compare(SortSize, () => a.Width.CompareTo(b.Width));
				Compare(SortSize, () => a.Height.CompareTo(b.Height));
				Compare(SortPalette, () => a.Palette.CompareTo(b.Palette));
				Compare(SortPriority, () => a.Priority.CompareTo(b.Priority));
				Compare(SortFlags, () => a.Flags?.CompareTo(b.Flags) ?? 0);

				return result != 0 ? result : a.SpriteIndex.CompareTo(b.SpriteIndex);
			});

			for(int i = 0; i < newList.Count; i++) {
				newList[i].CopyTo(SpritePreviews[i]);
			}

			if(selectedIndex != null && Selection.SelectedItem?.SpriteIndex != selectedIndex) {
				Selection.SelectedItem = null;
			}
		}

		public void InitListViewObservers()
		{
			//Update list view height based on show list view flag
			AddDisposable(this.WhenAnyValue(x => x.ShowListView).Subscribe(showListView => {
				Config.ShowListView = showListView;
				ListViewHeight = showListView ? Config.ListViewHeight : 0;
				MinListViewHeight = showListView ? 100 : 0;
				RefreshList(true);
			}));

			AddDisposable(this.WhenAnyValue(x => x.SpriteViewer.SpritePreviews).Subscribe(x => {
				RefreshList(true);
			}));

			AddDisposable(this.WhenAnyValue(x => x.ListViewHeight).Subscribe(height => {
				if(ShowListView) {
					Config.ListViewHeight = height;
				} else {
					ListViewHeight = 0;
				}
			}));
		}
	}
}
