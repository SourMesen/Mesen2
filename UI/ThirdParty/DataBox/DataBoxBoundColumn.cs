using Avalonia;
using Avalonia.Data;

namespace DataBoxControl;

public abstract class DataBoxBoundColumn : DataBoxColumn
{
    public static readonly StyledProperty<IBinding?> BindingProperty = 
        AvaloniaProperty.Register<DataBoxBoundColumn, IBinding?>(nameof(Binding));

    [AssignBinding]
    public IBinding? Binding
    {
        get => GetValue(BindingProperty);
        set => SetValue(BindingProperty, value);
    }
}