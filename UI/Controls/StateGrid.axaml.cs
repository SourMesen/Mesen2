using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.ViewModels;
using System;
using Avalonia.Interactivity;
using ReactiveUI;
using System.Collections.Generic;
using Mesen.Interop;
using Mesen.Config;
using Avalonia.Threading;
using System.Linq;

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
		public static readonly StyledProperty<int> SelectedIndexProperty = AvaloniaProperty.Register<StateGrid, int>(nameof(SelectedIndex));

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

		public int SelectedIndex
		{
			get { return GetValue(SelectedIndexProperty); }
			set { SetValue(SelectedIndexProperty, value); }
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
		private DispatcherTimer _timerInput = new DispatcherTimer();

		private int ElementsPerPage => _rowCount * _colCount;
		private int PageCount => (int)Math.Ceiling((double)Entries.Count / ElementsPerPage);

		static StateGrid()
		{
			BoundsProperty.Changed.AddClassHandler<StateGrid>((x, e) => x.InitGrid());
			EntriesProperty.Changed.AddClassHandler<StateGrid>((x, e) => {
				x.SelectedPage = 0;
				x.SelectedIndex = 0;
				x.InitGrid(true);
			});
			SelectedPageProperty.Changed.AddClassHandler<StateGrid>((x, e) => x.InitGrid(true));
			SelectedIndexProperty.Changed.AddClassHandler<StateGrid>((x, e) => x.UpdateSelectedEntry());

			IsVisibleProperty.Changed.AddClassHandler<StateGrid>((x, e) => {
				if(x.IsVisible) {
					x.Focus();
				}
			});
		}

		private void UpdateSelectedEntry()
		{
			if(SelectedIndex < SelectedPage * ElementsPerPage || SelectedIndex >= (SelectedPage + 1) * ElementsPerPage) {
				//Change page
				SelectedPage = SelectedIndex / ElementsPerPage;
				return;
			} else {
				Grid grid = this.GetControl<Grid>("Grid");
				int startIndex = ElementsPerPage * SelectedPage; 
				for(int i = 0; i < grid.Children.Count; i++) {
					if(grid.Children[i] is StateGridEntry entry) {
						entry.IsActiveEntry = startIndex + i == SelectedIndex;
					}
				}
			}
		}

		public StateGrid()
		{
			InitializeComponent();
			Focusable = true;
			_timerInput.Interval = TimeSpan.FromMilliseconds(50);
			_timerInput.Tick += timerInput_Tick;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			base.OnAttachedToVisualTree(e);
			_timerInput.Start();
			Focus();
		}

		protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
		{
			base.OnDetachedFromVisualTree(e);
			_timerInput.Stop();
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
			if(Entries == null) {
				return;
			}

			Grid grid = this.GetControl<Grid>("Grid");
			Size size = grid.Bounds.Size;

			int colCount = Math.Min(4, Math.Max(1, (int)(size.Width / 205)));
			int rowCount = Math.Min(3, Math.Max(1, (int)(size.Height / 200)));

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

			ColumnDefinitions columnDefinitions = new ColumnDefinitions();
			for(int i = 0; i < colCount; i++) {
				columnDefinitions.Add(new ColumnDefinition(1, GridUnitType.Star));
			}
			grid.ColumnDefinitions = columnDefinitions;

			RowDefinitions rowDefinitions = new RowDefinitions();
			for(int i = 0; i < rowCount; i++) {
				rowDefinitions.Add(new RowDefinition(1, GridUnitType.Star));
			}
			grid.RowDefinitions = rowDefinitions;

			int elementsPerPage = ElementsPerPage;
			int startIndex = elementsPerPage * SelectedPage;

			ShowArrows = Entries.Count > elementsPerPage;
			ShowClose = Mode != GameScreenMode.RecentGames;

			List<StateGridEntry> entries = new();
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
					ctrl.IsActiveEntry = index == SelectedIndex;
					ctrl.Init();

					entries.Add(ctrl);
				}
			}
			grid.Children.AddRange(entries);
		}

		private bool _loadRequested = false;
		private HashSet<ushort> _pressedKeyCodes = new();

		private void timerInput_Tick(object? sender, EventArgs e)
		{
			if(!IsEffectivelyVisible || !IsKeyboardFocusWithin || Entries == null || Entries.Count == 0) {
				_loadRequested = false;
				return;
			}

			List<KeyMapping> mappings = new List<ControllerConfig>() {
				ConfigManager.Config.Nes.Port1, ConfigManager.Config.Snes.Port1, ConfigManager.Config.PcEngine.Port1,
				ConfigManager.Config.Gameboy.Controller, ConfigManager.Config.Gba.Controller, ConfigManager.Config.Sms.Port1,
				ConfigManager.Config.Ws.ControllerHorizontal
			}.SelectMany((a) => new List<KeyMapping>() { a.Mapping1, a.Mapping2, a.Mapping3, a.Mapping4 }).ToList();

			List<ushort> keyCodes = InputApi.GetPressedKeys();

			foreach(ushort keyCode in keyCodes) {
				//Use player 1's controls to navigate the recent game selection screen
				if(keyCode > 0 && _pressedKeyCodes.Add(keyCode)) {
					foreach(KeyMapping mapping in mappings) {
						if(mapping.Left == keyCode) {
							if(SelectedIndex == 0) {
								SelectedIndex = Entries.Count - 1;
							} else {
								SelectedIndex--;
							}
							break;
						} else if(mapping.Right == keyCode) {
							SelectedIndex = (SelectedIndex + 1) % Entries.Count;
							break;
						} else if(mapping.Down == keyCode) {
							if(SelectedIndex + _colCount < Entries.Count) {
								SelectedIndex += _colCount;
							} else {
								SelectedIndex = Math.Min(SelectedIndex % _colCount, Entries.Count - 1);
							}
							break;
						} else if(mapping.Up == keyCode) {
							if(SelectedIndex < _colCount) {
								SelectedIndex = Entries.Count - (_colCount - (SelectedIndex % _colCount));
							} else {
								SelectedIndex -= _colCount;
							}
							break;
						} else if(mapping.A == keyCode || mapping.B == keyCode || mapping.X == keyCode || mapping.Y == keyCode || mapping.Select == keyCode || mapping.Start == keyCode) {
							_loadRequested = true;
							break;
						}
					}
				}
			}

			_pressedKeyCodes.Clear();
			_pressedKeyCodes.UnionWith(keyCodes);

			if(_loadRequested && keyCodes.Count == 0 && Entries.Count > 0) {
				//Load game/state once all buttons are released to avoid game processing pressed button
				RecentGameInfo entry = Entries[SelectedIndex % Entries.Count];
				if(entry.IsEnabled() == true) {
					entry.Load();
				}
				_loadRequested = false;
			}
		}
	}
}
