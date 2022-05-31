using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Avalonia.Layout;
using Avalonia.Markup.Xaml.MarkupExtensions;

namespace DataBoxControl;

public class DataBoxTextColumn : DataBoxBoundColumn
{
    public DataBoxTextColumn()
    {
        CellTemplate = new FuncDataTemplate(
            _ => true,
            (_, _) =>
            {
                var textBlock = new TextBlock            
                {
                    [!Layoutable.MarginProperty] = new DynamicResourceExtension("DataGridTextColumnCellTextBlockMargin"),
                    VerticalAlignment = VerticalAlignment.Center
                };

                if (Binding is { })
                {
                    textBlock.Bind(TextBlock.TextProperty, Binding);
                }

                return textBlock;
            },
            supportsRecycling: true);
    }
}