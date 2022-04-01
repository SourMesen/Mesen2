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
		[Reactive] public SpritePreviewModel? SelectedItem { get; set; } = null;

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

			AddDisposable(this.WhenAnyValue(x => x.SelectedItem).Subscribe(x => {
				if(x != null) {
					SpriteViewer.SelectSprite(x.SpriteIndex);
				}
			}));
		}

		public void SelectSprite(int spriteIndex)
		{
			SelectedItem = SpritePreviews?.Find(x => x.SpriteIndex == spriteIndex);
		}

		private int GetOrder(ListSortDirection? direction)
		{
			return direction == ListSortDirection.Ascending ? 1 : -1;
		}

		private DateTime _lastRefresh = DateTime.MinValue;

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

			int? selectedIndex = SelectedItem?.SpriteIndex;

			List<SpritePreviewModel> newList = new(SpriteViewer.SpritePreviews.Select(x => x.Clone()).ToList());

			newList.Sort((a, b) => {
				if(SortIndex.HasValue) {
					int result = a.SpriteIndex.CompareTo(b.SpriteIndex) * GetOrder(SortIndex);
					if(result != 0) {
						return result;
					}
				}

				if(SortX.HasValue) {
					int result = a.RawX.CompareTo(b.RawX) * GetOrder(SortX);
					if(result != 0) {
						return result;
					}
				}

				if(SortY.HasValue) {
					int result = a.RawY.CompareTo(b.RawY) * GetOrder(SortY);
					if(result != 0) {
						return result;
					}
				}

				if(SortSize.HasValue) {
					int result = a.Width.CompareTo(b.Width) * GetOrder(SortSize);
					if(result != 0) {
						return result;
					} else {
						result = a.Height.CompareTo(b.Height) * GetOrder(SortSize);
						if(result != 0) {
							return result;
						}
					}
				}

				if(SortTileIndex.HasValue) {
					int result = a.TileIndex.CompareTo(b.TileIndex) * GetOrder(SortTileIndex);
					if(result != 0) {
						return result;
					}
				}

				if(SortPalette.HasValue) {
					int result = a.Palette.CompareTo(b.Palette) * GetOrder(SortPalette);
					if(result != 0) {
						return result;
					}
				}

				if(SortPriority.HasValue) {
					int result = a.Priority.CompareTo(b.Priority) * GetOrder(SortPriority);
					if(result != 0) {
						return result;
					}
				}

				if(SortFlags.HasValue && a.Flags != null) {
					int result = a.Flags.CompareTo(b.Flags) * GetOrder(SortFlags);
					if(result != 0) {
						return result;
					}
				}

				return a.SpriteIndex.CompareTo(b.SpriteIndex);
			});

			for(int i = 0; i < newList.Count; i++) {
				newList[i].CopyTo(SpritePreviews[i]);
			}

			if(selectedIndex != null && SelectedItem?.SpriteIndex != selectedIndex) {
				SelectedItem = null;
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
