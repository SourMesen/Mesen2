using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.ViewModels;
using System;
using Avalonia.Interactivity;
using ReactiveUI;
using System.Collections.Generic;
using Mesen.Interop;

namespace Mesen.Controls
{
	public class StateGrid : UserControl
	{
		public static readonly StyledProperty<List<RecentGameInfo>> EntriesProperty = AvaloniaProperty.Register<StateGrid, List<RecentGameInfo>>(nameof(Entries));
		
		public static readonly StyledProperty<string> TitleProperty = AvaloniaProperty.Register<StateGrid, string>(nameof(Title));
		public static readonly StyledProperty<int> SelectedPageProperty = AvaloniaProperty.Register<StateGrid, int>(nameof(SelectedPage));
		public static readonly StyledProperty<bool> ShowArrowsProperty = AvaloniaProperty.Register<StateGrid, bool>(nameof(ShowArrows));
		public static readonly StyledProperty<bool> ShowCloseProperty = AvaloniaProperty.Register<StateGrid, bool>(nameof(ShowClose));
		public static readonly StyledProperty<GameScreenMode> ModeProperty = AvaloniaProperty.Register<StateGrid, GameScreenMode>(nameof(Mode));

		public string Title
		{
			get { return GetValue(TitleProperty); }
			set { SetValue(TitleProperty, value); }
		}

		public int SelectedPage
		{
			get { return GetValue(SelectedPageProperty); }
			set { SetValue(SelectedPageProperty, value); }
		}

		public bool ShowArrows
		{
			get { return GetValue(ShowArrowsProperty); }
			set { SetValue(ShowArrowsProperty, value); }
		}

		public bool ShowClose
		{
			get { return GetValue(ShowCloseProperty); }
			set { SetValue(ShowCloseProperty, value); }
		}

		public GameScreenMode Mode
		{
			get { return GetValue(ModeProperty); }
			set { SetValue(ModeProperty, value); }
		}

		public List<RecentGameInfo> Entries
		{
			get { return GetValue(EntriesProperty); }
			set { SetValue(EntriesProperty, value); }
		}

		private int _colCount = 0;
		private int _rowCount = 0;

		private int ElementsPerPage => _rowCount * _colCount;
		private int PageCount => (int)Math.Ceiling((double)Entries.Count / ElementsPerPage);

		static StateGrid()
		{
			BoundsProperty.Changed.AddClassHandler<StateGrid>((x, e) => x.InitGrid());
			EntriesProperty.Changed.AddClassHandler<StateGrid>((x, e) => {
				x.SelectedPage = 0;
				x.InitGrid(true);
			});
			SelectedPageProperty.Changed.AddClassHandler<StateGrid>((x, e) => x.InitGrid(true));
		}

		public StateGrid()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void OnCloseClick(object sender, RoutedEventArgs e)
		{
			if(DataContext is RecentGamesViewModel model) {
				if(model.NeedResume) {
					EmuApi.Resume();
				}
				model.Visible = false;
			}
		}

		private void OnPrevPageClick(object sender, RoutedEventArgs e)
		{
			int page = SelectedPage - 1;
			if(page < 0) {
				page = PageCount - 1;
			}
			SelectedPage = page;
		}

		private void OnNextPageClick(object sender, RoutedEventArgs e)
		{
			int page = SelectedPage + 1;
			if(page >= PageCount) {
				page = 0;
			}
			SelectedPage = page;
		}

		private void InitGrid(bool forceUpdate = false)
		{
			Grid grid = this.FindControl<Grid>("Grid");
			Size size = grid.Bounds.Size;

			int colCount = Math.Min(4, Math.Max(1, (int)(size.Width / 220)));
			int rowCount = Math.Min(3, Math.Max(1, (int)(size.Height / 210)));

			if(Entries.Count <= 1) {
				colCount = 1;
				rowCount = 1;
			} else if(Entries.Count <= 4) {
				colCount = Math.Min(2, colCount);
				rowCount = colCount;
			}

			if(Mode != GameScreenMode.RecentGames) {
				colCount = 4;
				rowCount = 3;
			}

			bool layoutChanged = _colCount != colCount || _rowCount != rowCount;
			if(!forceUpdate && !layoutChanged) {
				//Grid is already the same size
				return;
			}

			if(layoutChanged) {
				SelectedPage = 0;
			}

			_colCount = colCount;
			_rowCount = rowCount;

			grid.Children.Clear();

			grid.ColumnDefinitions = new ColumnDefinitions();
			for(int i = 0; i < colCount; i++) {
				grid.ColumnDefinitions.Add(new ColumnDefinition(1, GridUnitType.Star));
			}

			grid.RowDefinitions = new RowDefinitions();
			for(int i = 0; i < rowCount; i++) {
				grid.RowDefinitions.Add(new RowDefinition(1, GridUnitType.Star));
			}

			int elementsPerPage = ElementsPerPage;
			int startIndex = elementsPerPage * SelectedPage;

			ShowArrows = Entries.Count > elementsPerPage;
			ShowClose = Mode != GameScreenMode.RecentGames;

			for(int row = 0; row < rowCount; row++) {
				for(int col = 0; col < colCount; col++) {
					int index = startIndex + row * colCount + col;

					if(index >= Entries.Count) {
						break;
					}

					StateGridEntry ctrl = new StateGridEntry();

					ctrl.SetValue(Grid.ColumnProperty, col);
					ctrl.SetValue(Grid.RowProperty, row);
					ctrl.Entry = Entries[index];
					ctrl.Init();

					grid.Children.Add(ctrl);
				}
			}
		}
	}
}
