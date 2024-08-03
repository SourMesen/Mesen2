using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Windows.Input;
using Avalonia;
using Avalonia.Collections;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Controls.Selection;
using Avalonia.Data;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml.MarkupExtensions;
using Avalonia.Media;
using Avalonia.Metadata;
using Avalonia.Threading;
using DataBoxControl.Controls;
using DataBoxControl.Primitives;
using DynamicData;
using Mesen.Utilities;
using ReactiveUI;

namespace DataBoxControl;

public class DataBox : TemplatedControl
{
    public static readonly DirectProperty<DataBox, IEnumerable?> ItemsProperty =
		  AvaloniaProperty.RegisterDirect<DataBox, IEnumerable?>(
			  nameof(Items),
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
		 AvaloniaProperty.Register<DataBoxColumn, SortState>(nameof(SortState));

	public static readonly StyledProperty<List<int>> ColumnWidthsProperty =
		 AvaloniaProperty.Register<DataBoxColumn, List<int>>(nameof(ColumnWidths));

	public static readonly StyledProperty<bool> CanUserResizeColumnsProperty = 
        AvaloniaProperty.Register<DataBox, bool>(nameof(CanUserResizeColumns));

	public static readonly StyledProperty<bool> DisableSearchProperty =
		  AvaloniaProperty.Register<DataBox, bool>(nameof(DisableSearch));

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

	public List<int> ColumnWidths
	{
		get => GetValue(ColumnWidthsProperty);
		set => SetValue(ColumnWidthsProperty, value);
	}

	public bool DisableSearch
	{
		get => GetValue(DisableSearchProperty);
		set => SetValue(DisableSearchProperty, value);
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
        SortState = new();
        ColumnWidths = new();

        this.AddHandler(InputElement.KeyDownEvent, OnPreviewKeyDown, RoutingStrategies.Tunnel, true);
    }

    protected override void OnApplyTemplate(TemplateAppliedEventArgs e)
    {
        base.OnApplyTemplate(e);

        _headersPresenterScrollViewer = e.NameScope.Find<ScrollViewer>("PART_HeadersPresenterScrollViewer");
        _headersPresenter = e.NameScope.Find<DataBoxColumnHeadersPresenter>("PART_HeadersPresenter");
        _rowsPresenter = e.NameScope.Get<DataBoxRowsPresenter>("PART_RowsPresenter");
        _rowsPresenter.AutoScrollToSelectedItem = true;
        if(_columns.Count > ColumnWidths.Count) {
            for(int i = ColumnWidths.Count; i < _columns.Count; i++) {
                ColumnWidths.Add(_columns[i].InitialWidth);
            }
        }
		  Attach();
    }

	protected override void OnPropertyChanged(AvaloniaPropertyChangedEventArgs change)
	{
		if(change.Property == ColumnsProperty) {
			if(change.NewValue is AvaloniaList<DataBoxColumn> columns && columns.Count > ColumnWidths.Count) {
				 for(int i = ColumnWidths.Count; i < columns.Count; i++) {
                ColumnWidths.Add(columns[i].InitialWidth);
            }
			}
		}
		base.OnPropertyChanged(change);
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

            _rowsPresenter[!!ItemsControl.ItemsSourceProperty] = this[!!ItemsProperty];
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

	internal void OnCellDoubleTapped(object? sender, RoutedEventArgs e)
	{
		if(sender is DataBoxCell cell && !(cell.Column is DataBoxCheckBoxColumn)) {
			Dispatcher.UIThread.Post(() => {
				CellDoubleClick?.Invoke(cell);
			});
		}
	}

	private DataBoxCell? _pressedCell = null;
	internal void OnCellPointerPressed(object? sender, RoutedEventArgs e)
	{
		if(sender is DataBoxCell cell) {
			if(cell.Column is DataBoxCheckBoxColumn && Selection.SelectedItems.IndexOf(cell.DataContext) >= 0) {
				//Prevent selection change when clicking checkbox column when multiple items are selected
				e.Handled = true;
			}

			_pressedCell = cell;
		}
	}

	internal void OnCellPointerReleased(object? sender, PointerReleasedEventArgs e)
	{
		if(sender is DataBoxCell cell && _pressedCell == cell) {
			Dispatcher.UIThread.Post(() => {
				CellClick?.Invoke(cell);
			});
		}
		_pressedCell = null;
	}

	protected override void OnPointerPressed(PointerPressedEventArgs e)
	{
		base.OnPointerPressed(e);
		_searchString = "";
		_resetTimer.Restart();
	}

	private Stopwatch _resetTimer = Stopwatch.StartNew();
	private string _searchString = "";
	private string _searchStringHex = "";

	protected override void OnTextInput(TextInputEventArgs e)
	{
		base.OnTextInput(e);

		if(e.Text == null) {
			return;
		}

		ProcessKeyPress(e.Text);
	}

	private void OnPreviewKeyDown(object? sender, KeyEventArgs e)
	{
		if(e.Key == Key.Space && !DisableSearch) {
			ProcessKeyPress(" ");
			e.Handled = true;
		} else if(IsKeyboardFocusWithin && TopLevel.GetTopLevel(this)?.FocusManager?.GetFocusedElement() is CheckBox) {
			//Allow up/down arrow keys to work properly when focus is on a checkbox column
			_rowsPresenter?.ContainerFromIndex(Selection.SelectedIndex)?.Focus();
		}
	}

	private void ProcessKeyPress(string keyText)
	{
		if(Items == null || _rowsPresenter == null || DisableSearch) {
			return;
		}

		if(_resetTimer.ElapsedMilliseconds > 1000) {
			_searchString = "";
		}

		_searchString += keyText;
		_searchStringHex = "$" + _searchString; //allow searching for hex values without typing leading $ sign
		_resetTimer.Restart();

		foreach(var sort in SortState.SortOrder) {
			DataBoxColumn column = _columns.First(c => sort.Item1 == c.ColumnName);
			if(SearchColumn(column)) {
				return;
			}
		}

		for(int i = 0; i < _columns.Count; i++) {
			if(SearchColumn(_columns[i])) {
				return;
			}
		}
	}

	private bool SearchColumn(DataBoxColumn column)
	{
		if(Items == null || _rowsPresenter == null) {
			return false;
		}

		if(column is DataBoxTextColumn textColumn && textColumn.Binding is CompiledBindingExtension columnBinding) {
#pragma warning disable IL2026 // Members annotated with 'RequiresUnreferencedCodeAttribute' require dynamic access otherwise can break functionality when trimming application code
			Binding binding = new Binding(columnBinding.Path.ToString(), BindingMode.OneTime);
#pragma warning restore IL2026 // Members annotated with 'RequiresUnreferencedCodeAttribute' require dynamic access otherwise can break functionality when trimming application code
			int i = 0;
			foreach(object item in Items) {
				binding.Source = item;
				ValueGetter getter = new ValueGetter();
				getter.Bind(ValueGetter.ValueProperty, binding);

				string value = getter.Value;
				if(value.StartsWith(_searchString, StringComparison.OrdinalIgnoreCase) || value.StartsWith(_searchStringHex, StringComparison.OrdinalIgnoreCase)) {
					_rowsPresenter.SelectedItem = item;
					_rowsPresenter.ScrollIntoView(item);
					_rowsPresenter.GetRow(i)?.Focus();
					return true;
				}
				i++;
			}
		}

		return false;
	}

	private string ConvertToText()
	{
		if(Items == null || _rowsPresenter == null) {
			return string.Empty;
		}

		StringBuilder sb = new();

		List<Binding?> bindings = new();
		for(int i = 0; i < Columns.Count; i++) {
			DataBoxColumn column = Columns[i];
			if(column is DataBoxTextColumn textColumn && textColumn.Binding is CompiledBindingExtension columnBinding) {
#pragma warning disable IL2026 // Members annotated with 'RequiresUnreferencedCodeAttribute' require dynamic access otherwise can break functionality when trimming application code
				bindings.Add(new Binding(columnBinding.Path.ToString(), BindingMode.OneTime));
#pragma warning restore IL2026 // Members annotated with 'RequiresUnreferencedCodeAttribute' require dynamic access otherwise can break functionality when trimming application code

				sb.Append(textColumn.Header);
				if(i < Columns.Count - 1) {
					sb.Append(",");
				}
			} else {
				bindings.Add(null);
			}
		}
		sb.Append(Environment.NewLine);

		foreach(object item in Items) {
			for(int i = 0; i < Columns.Count; i++) {
				DataBoxColumn column = Columns[i];
				if(column is DataBoxTextColumn textColumn && textColumn.Binding is CompiledBindingExtension columnBinding) {
					Binding? binding = bindings[i];
					if(binding == null) {
						continue;
					}

					binding.Source = item;
					ValueGetter getter = new ValueGetter();
					getter.Bind(ValueGetter.ValueProperty, binding);

					string value = getter.Value;
					sb.Append(value);
					if(i < Columns.Count - 1) {
						sb.Append(",");
					}
				}
			}
			sb.Append(Environment.NewLine);
		}

		return sb.ToString();
	}

	public void CopyToClipboard()
	{
		ApplicationHelper.GetMainWindow()?.Clipboard?.SetTextAsync(ConvertToText());
	}

	public DataBoxRow? GetRow(int index)
	{
		return _rowsPresenter?.GetRow(index);
	}

	public T? GetCellControl<T>(int row, int column) where T : class
	{
		return _rowsPresenter?.GetRow(row)?.CellsPresenter?.GetControl<T>(column);
	}
}

public class ValueGetter : AvaloniaObject
{
	public static readonly StyledProperty<string> ValueProperty =
	 AvaloniaProperty.Register<ValueGetter, string>(nameof(Value));

	public string Value
	{
		get => GetValue(ValueProperty);
		set => SetValue(ValueProperty, value);
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