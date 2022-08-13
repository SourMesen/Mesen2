using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Input;
using Avalonia;
using Avalonia.Collections;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Controls.Selection;
using Avalonia.Data;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Metadata;
using DataBoxControl.Controls;
using DataBoxControl.Primitives;

namespace DataBoxControl;

public class DataBox : TemplatedControl
{
    public static readonly DirectProperty<DataBox, IEnumerable?> ItemsProperty =
        ItemsControl.ItemsProperty.AddOwner<DataBox>( 
            o => o.Items, 
            (o, v) => {
					o.Items = v;
					if(o.Selection != null) {
						o.Selection.Source = v;
					}
				});

	public static readonly DirectProperty<DataBox, ISelectionModel> SelectionProperty =
		DataBoxRowsPresenter.SelectionProperty.AddOwner<DataBox>(
		  o => o.Selection,
		  (o, v) => {
			  if(v != null) {
				  v.Source = o.Items;
			     o.Selection = v;
			  }
		  },
		  defaultBindingMode: BindingMode.TwoWay);
		
	public static readonly DirectProperty<DataBox, AvaloniaList<DataBoxColumn>> ColumnsProperty =
        AvaloniaProperty.RegisterDirect<DataBox, AvaloniaList<DataBoxColumn>>(
            nameof(Columns), 
            o => o.Columns);

	public static readonly StyledProperty<SortMode> SortModeProperty = 
        AvaloniaProperty.Register<DataBox, SortMode>(nameof(SortMode), SortMode.None);

	public static readonly StyledProperty<ICommand?> SortCommandProperty =
		 AvaloniaProperty.Register<DataBoxColumn, ICommand?>(nameof(SortCommand));

	public static readonly StyledProperty<SortState> SortStateProperty =
		 AvaloniaProperty.Register<DataBoxColumn, SortState>(nameof(SortState), new SortState());

	public static readonly StyledProperty<bool> CanUserResizeColumnsProperty = 
        AvaloniaProperty.Register<DataBox, bool>(nameof(CanUserResizeColumns));

    public static readonly StyledProperty<DataBoxGridLinesVisibility> GridLinesVisibilityProperty = 
        AvaloniaProperty.Register<DataBox, DataBoxGridLinesVisibility>(nameof(GridLinesVisibility));

	public static readonly StyledProperty<SelectionMode> SelectionModeProperty =
	 AvaloniaProperty.Register<DataBox, SelectionMode>(nameof(SelectionMode));

	public static readonly StyledProperty<bool> IsReadOnlyProperty = 
        AvaloniaProperty.Register<DataBox, bool>(nameof(IsReadOnly));

    public static readonly StyledProperty<IBrush> HorizontalGridLinesBrushProperty =
        AvaloniaProperty.Register<DataBox, IBrush>(nameof(HorizontalGridLinesBrush));
        
    public static readonly StyledProperty<IBrush> VerticalGridLinesBrushProperty =
        AvaloniaProperty.Register<DataBox, IBrush>(nameof(VerticalGridLinesBrush));

	private IEnumerable? _items = Array.Empty<object?>();
	private ISelectionModel _selection = new SelectionModel<object?>();

	private AvaloniaList<DataBoxColumn> _columns;
    private ScrollViewer? _headersPresenterScrollViewer;
    private DataBoxColumnHeadersPresenter? _headersPresenter;
    private DataBoxRowsPresenter? _rowsPresenter;

    public AvaloniaList<DataBoxColumn> Columns
    {
        get => _columns;
        private set => SetAndRaise(ColumnsProperty, ref _columns, value);
    }

    [Content]
    public IEnumerable? Items
    {
        get { return _items; }
        set { SetAndRaise(ItemsProperty, ref _items, value); }
    }

	public ISelectionModel Selection
	{
		get => _selection;
		set => SetAndRaise(SelectionProperty, ref _selection, value);
	}

	public SelectionMode SelectionMode
	{
		get => GetValue(SelectionModeProperty);
		set => SetValue(SelectionModeProperty, value);
	}
	
	public SortMode SortMode
	{
		get => GetValue(SortModeProperty);
		set => SetValue(SortModeProperty, value);
	}

	public ICommand? SortCommand
	{
		get => GetValue(SortCommandProperty);
		set => SetValue(SortCommandProperty, value);
	}

	public SortState SortState
	{
		get => GetValue(SortStateProperty);
		set => SetValue(SortStateProperty, value);
	}

	public bool CanUserResizeColumns
    {
        get => GetValue(CanUserResizeColumnsProperty);
        set => SetValue(CanUserResizeColumnsProperty, value);
    }

    public DataBoxGridLinesVisibility GridLinesVisibility
    {
        get => GetValue(GridLinesVisibilityProperty);
        set => SetValue(GridLinesVisibilityProperty, value);
    }

	public bool IsReadOnly
    {
        get => GetValue(IsReadOnlyProperty);
        set => SetValue(IsReadOnlyProperty, value);
    }

    public IBrush HorizontalGridLinesBrush
    {
        get => GetValue(HorizontalGridLinesBrushProperty);
        set => SetValue(HorizontalGridLinesBrushProperty, value);
    }

    public IBrush VerticalGridLinesBrush
    {
        get => GetValue(VerticalGridLinesBrushProperty);
        set => SetValue(VerticalGridLinesBrushProperty, value);
    }

	public delegate void CellClickHandler(DataBoxCell cell);
	public event CellClickHandler? CellClick;

	public delegate void CellDoubleClickHandler(DataBoxCell cell);
	public event CellDoubleClickHandler? CellDoubleClick;

	internal double AccumulatedWidth { get; set; }
        
    internal double AvailableWidth { get; set; }

    internal double AvailableHeight { get; set; }

	public DataBox()
    {
        _columns = new AvaloniaList<DataBoxColumn>();
    }

    protected override void OnApplyTemplate(TemplateAppliedEventArgs e)
    {
        base.OnApplyTemplate(e);

        _headersPresenterScrollViewer = e.NameScope.Find<ScrollViewer>("PART_HeadersPresenterScrollViewer");
        _headersPresenter = e.NameScope.Find<DataBoxColumnHeadersPresenter>("PART_HeadersPresenter");
        _rowsPresenter = e.NameScope.Get<DataBoxRowsPresenter>("PART_RowsPresenter");
        _rowsPresenter.AutoScrollToSelectedItem = true;

		  Attach();
    }

	protected override void OnPropertyChanged(AvaloniaPropertyChangedEventArgs change)
	{
		if(change.Property == SelectionProperty) {
			if(change.OldValue is ISelectionModel oldModel) {
				oldModel.SelectionChanged -= Selection_SelectionChanged;
			}
			if(change.NewValue is ISelectionModel newModel) {
				newModel.SelectionChanged += Selection_SelectionChanged;
			}
		}
		base.OnPropertyChanged(change);
	}

	private void Selection_SelectionChanged(object? sender, SelectionModelSelectionChangedEventArgs e)
	{
		if(IsKeyboardFocusWithin && Selection?.SelectedIndex >= 0 && Selection.SelectedItems.Count == 1) {
			//When selection is changed and only 1 row is selected, move keyboard focus to that row
			//Only do this if the DataBox already contained the keyboard focus
			_rowsPresenter?.ItemContainerGenerator.ContainerFromIndex(Selection.SelectedIndex)?.Focus();
		}
	}

	internal void Attach()
    {
        if (_headersPresenter is { })
        {
            _headersPresenter.DataBox = this;
            _headersPresenter.Detach();
            _headersPresenter.Attach();
        }

        if (_rowsPresenter is { })
        {
            _rowsPresenter.DataBox = this;

            _rowsPresenter[!!ItemsControl.ItemsProperty] = this[!!ItemsProperty];
            _rowsPresenter[!!ListBox.SelectionProperty] = this[!!SelectionProperty];

            _rowsPresenter.TemplateApplied += (_, _) =>
            {
                if (_rowsPresenter.Scroll is ScrollViewer scrollViewer)
                {
                    scrollViewer.ScrollChanged += (_, _) =>
                    {
                        var (x, _) = scrollViewer.Offset;
                        if (_headersPresenterScrollViewer is { })
                        {
                            _headersPresenterScrollViewer.Offset = new Vector(x, 0);
                        }
                    };
                }
            };
        }
    }

	internal void OnCellTapped(object? sender, RoutedEventArgs e)
	{
		if(sender is DataBoxCell cell) {
			CellClick?.Invoke(cell);
		}
	}

	internal void OnCellDoubleTapped(object? sender, RoutedEventArgs e)
	{
		if(sender is DataBoxCell cell) {
			CellDoubleClick?.Invoke(cell);
		}
	}
}

public class SortState
{
	public List<Tuple<string, ListSortDirection>> SortOrder { get; private set; } = new();

	public void Reset()
	{
		SortOrder.Clear();
	}

	public void Remove(string column)
	{
		Tuple<string, ListSortDirection>? columnInfo = SortOrder.Find(x => x.Item1 == column);
		if(columnInfo != null) {
			SortOrder.Remove(columnInfo);
		}
	}

	public void ToggleSortOrder(string column, bool reset)
	{
		int i = SortOrder.FindIndex(x => x.Item1 == column);
		ListSortDirection order;
		if(i < 0) {
			order = ListSortDirection.Ascending;
		} else {
			order = SortOrder[i].Item2 == ListSortDirection.Descending ? ListSortDirection.Ascending : ListSortDirection.Descending;
		}

		SetColumnSort(column, order, reset);
	}

	public void SetColumnSort(string column, ListSortDirection order, bool reset)
	{
		Tuple<string, ListSortDirection> columnInfo = columnInfo = new(column, order);

		int i = SortOrder.FindIndex(x => x.Item1 == column);
		if(i < 0) {
			SortOrder.Add(columnInfo);
		} else {
			SortOrder[i] = columnInfo;
		}

		if(reset) {
			Reset();
			SortOrder.Add(columnInfo);
		}
	}

	internal void UpdateColumnHeaders(IReadOnlyList<DataBoxColumnHeader> columnHeaders)
	{
		foreach(var columnHeader in columnHeaders) {
			if(columnHeader.Column is DataBoxColumn column) {
				columnHeader.UpdatePseudoClassesSortingState(GetSortState(column.ColumnName));
				columnHeader.SortNumber = GetSortNumber(column.ColumnName);
			}
		}
	}

	private ListSortDirection? GetSortState(string column)
	{
		Tuple<string, ListSortDirection>? columnInfo = SortOrder.Find(x => x.Item1 == column);
		return columnInfo?.Item2;
	}

	private string GetSortNumber(string column)
	{
		if(SortOrder.Count <= 1) {
			return "";
		}
		int i = SortOrder.FindIndex(x => x.Item1 == column);
		return i >= 0 ? (i+1).ToString() : "";
	}
}

public enum SortMode
{
	None,
	Single,
	Multiple
}