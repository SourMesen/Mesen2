using Avalonia;
using Avalonia.Data;

namespace DataBoxControl;

public abstract class DataBoxBoundColumn : DataBoxColumn
{
	public static readonly StyledProperty<IBinding?> BindingProperty =
		 AvaloniaProperty.Register<DataBoxBoundColumn, IBinding?>(nameof(Binding));

	public static readonly StyledProperty<IBinding?> IsVisibleProperty =
	 AvaloniaProperty.Register<DataBoxBoundColumn, IBinding?>(nameof(IsVisible));

	[AssignBinding]
	public IBinding? Binding
	{
		get => GetValue(BindingProperty);
		set => SetValue(BindingProperty, value);
	}

	[AssignBinding]
	public IBinding? IsVisible
	{
		get => GetValue(IsVisibleProperty);
		set => SetValue(IsVisibleProperty, value);
	}
}