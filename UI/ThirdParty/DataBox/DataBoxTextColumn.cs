using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Avalonia.Layout;
using Avalonia.Markup.Xaml.MarkupExtensions;

namespace DataBoxControl;

public class DataBoxTextColumn : DataBoxBoundColumn
{
	public static readonly StyledProperty<bool> ShowToolTipProperty =
	  AvaloniaProperty.Register<DataBoxBoundColumn, bool>(nameof(ShowToolTip), false);

	public bool ShowToolTip
	{
		get => GetValue(ShowToolTipProperty);
		set => SetValue(ShowToolTipProperty, value);
	}

	public DataBoxTextColumn()
	{
		CellTemplate = new FuncDataTemplate(
			_=> true,
			(_, _) => {
				var textBlock = new TextBlock {
					[!Layoutable.MarginProperty] = new DynamicResourceExtension("DataGridTextColumnCellTextBlockMargin"),
					VerticalAlignment = VerticalAlignment.Top
				};

				if(Binding is { }) {
					textBlock.Bind(TextBlock.TextProperty, Binding);
					if(ShowToolTip) {
						textBlock.Bind(ToolTip.TipProperty, Binding);
					}
				}

				if(IsVisible is { }) {
					textBlock.Bind(TextBlock.IsVisibleProperty, IsVisible);
				}

				return textBlock;
			},
			supportsRecycling: true);
	}
}