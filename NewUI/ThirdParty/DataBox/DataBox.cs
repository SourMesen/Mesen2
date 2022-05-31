using System.Collections;
using Avalonia;
using Avalonia.Collections;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Data;
using Avalonia.Media;
using Avalonia.Metadata;
using DataBoxControl.Primitives;

namespace DataBoxControl;

public class DataBox : TemplatedControl
{
    public static readonly DirectProperty<DataBox, IEnumerable?> ItemsProperty =
        AvaloniaProperty.RegisterDirect<DataBox, IEnumerable?>(
            nameof(Items), 
            o => o.Items, 
            (o, v) => o.Items = v);

    public static readonly DirectProperty<DataBox, object?> SelectedItemProperty =
        AvaloniaProperty.RegisterDirect<DataBox, object?>(
            nameof(SelectedItem), 
            o => o.SelectedItem, 
            (o, v) => o.SelectedItem = v,
            defaultBindingMode: BindingMode.TwoWay);

    public static readonly DirectProperty<DataBox, AvaloniaList<DataBoxColumn>> ColumnsProperty =
        AvaloniaProperty.RegisterDirect<DataBox, AvaloniaList<DataBoxColumn>>(
            nameof(Columns), 
            o => o.Columns);

    public static readonly StyledProperty<bool> CanUserSortColumnsProperty = 
        AvaloniaProperty.Register<DataBox, bool>(nameof(CanUserSortColumns), true);

    public static readonly StyledProperty<bool> CanUserResizeColumnsProperty = 
        AvaloniaProperty.Register<DataBox, bool>(nameof(CanUserResizeColumns));

    public static readonly StyledProperty<DataBoxGridLinesVisibility> GridLinesVisibilityProperty = 
        AvaloniaProperty.Register<DataBox, DataBoxGridLinesVisibility>(nameof(GridLinesVisibility));

    public static readonly StyledProperty<bool> IsReadOnlyProperty = 
        AvaloniaProperty.Register<DataBox, bool>(nameof(IsReadOnly));

    public static readonly StyledProperty<IBrush> HorizontalGridLinesBrushProperty =
        AvaloniaProperty.Register<DataBox, IBrush>(nameof(HorizontalGridLinesBrush));
        
    public static readonly StyledProperty<IBrush> VerticalGridLinesBrushProperty =
        AvaloniaProperty.Register<DataBox, IBrush>(nameof(VerticalGridLinesBrush));

    private IEnumerable? _items = new AvaloniaList<object>();
    private object? _selectedItem;
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

    public object? SelectedItem
    {
        get => _selectedItem;
        set => SetAndRaise(SelectedItemProperty, ref _selectedItem, value);
    }

    public bool CanUserSortColumns
    {
        get => GetValue(CanUserSortColumnsProperty);
        set => SetValue(CanUserSortColumnsProperty, value);
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
        _rowsPresenter = e.NameScope.Find<DataBoxRowsPresenter>("PART_RowsPresenter");

        Attach();
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
            this[!!SelectedItemProperty] = _rowsPresenter[!!SelectingItemsControl.SelectedItemProperty];

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
}
