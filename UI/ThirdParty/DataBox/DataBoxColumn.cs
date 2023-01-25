using System.ComponentModel;
using System.Windows.Input;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Avalonia.Data;
using Avalonia.Metadata;

namespace DataBoxControl;

public abstract class DataBoxColumn : AvaloniaObject
{
    public static readonly StyledProperty<IDataTemplate?> CellTemplateProperty = 
        AvaloniaProperty.Register<DataBoxColumn, IDataTemplate?>(nameof(CellTemplate));

	public static readonly StyledProperty<object?> HeaderProperty =
		 AvaloniaProperty.Register<DataBoxColumn, object?>(nameof(Header));
	
	public static readonly StyledProperty<string> ColumnNameProperty =
		 AvaloniaProperty.Register<DataBoxColumn, string>(nameof(ColumnName), "");
	
    public static readonly StyledProperty<int> InitialWidthProperty = 
        AvaloniaProperty.Register<DataBoxColumn, int>(nameof(InitialWidth), 50);

    public static readonly StyledProperty<int> MinWidthProperty = 
        AvaloniaProperty.Register<DataBoxColumn, int>(nameof(MinWidth), 22);

    public static readonly StyledProperty<int> MaxWidthProperty = 
        AvaloniaProperty.Register<DataBoxColumn, int>(nameof(MaxWidth), int.MaxValue);

    public static readonly StyledProperty<bool> CanUserSortProperty = 
        AvaloniaProperty.Register<DataBoxColumn, bool>(nameof(CanUserSort), true);

    public static readonly StyledProperty<bool> CanUserResizeProperty = 
        AvaloniaProperty.Register<DataBoxColumn, bool>(nameof(CanUserResize));

    public static readonly StyledProperty<bool> CanUserReorderProperty = 
        AvaloniaProperty.Register<DataBoxColumn, bool>(nameof(CanUserReorder));

    internal static readonly StyledProperty<double> MeasureWidthProperty = 
        AvaloniaProperty.Register<DataBoxColumn, double>(nameof(MeasureWidth), double.NaN);

    [Content]
    public IDataTemplate? CellTemplate
    {
        get => GetValue(CellTemplateProperty);
        set => SetValue(CellTemplateProperty, value);
    }

    public object? Header
    {
        get => GetValue(HeaderProperty);
        set => SetValue(HeaderProperty, value);
    }
	
    public int InitialWidth
	{
        get => GetValue(InitialWidthProperty);
        set => SetValue(InitialWidthProperty, value);
    }

    public int MinWidth
    {
        get => GetValue(MinWidthProperty);
        set => SetValue(MinWidthProperty, value);
    }

    public int MaxWidth
    {
        get => GetValue(MaxWidthProperty);
        set => SetValue(MaxWidthProperty, value);
    }

    public bool CanUserSort
    {
        get => GetValue(CanUserSortProperty);
        set => SetValue(CanUserSortProperty, value);
    }

    public bool CanUserResize
    {
        get => GetValue(CanUserSortProperty);
        set => SetValue(CanUserSortProperty, value);
    }

    public bool CanUserReorder
    {
        get => GetValue(CanUserSortProperty);
        set => SetValue(CanUserSortProperty, value);
    }
	
	public string ColumnName
	{
		get => GetValue(ColumnNameProperty);
		set => SetValue(ColumnNameProperty, value);
	}

	internal double MeasureWidth
    {
        get => GetValue(MeasureWidthProperty);
        set => SetValue(MeasureWidthProperty, value);
    }
}
