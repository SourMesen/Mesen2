using System;
using Avalonia;
using Avalonia.Collections;
using Avalonia.Controls;

namespace DataBoxControl.Primitives.Layout;

internal static class DataBoxRowsLayout
{
    private static double SetColumnsActualWidth(DataBox dataBox)
    {
        var accumulatedWidth = 0.0;
        var actualWidths = new double[dataBox.Columns.Count];

        for (var c = 0; c < dataBox.Columns.Count; c++)
        {
            var column = dataBox.Columns[c];
            int actualWidth = dataBox.ColumnWidths[c];

            actualWidth = Math.Max(column.MinWidth, actualWidth);
            actualWidth = Math.Min(column.MaxWidth, actualWidth);
                        
            actualWidths[c] = actualWidth;
            accumulatedWidth += actualWidths[c];

            column.MeasureWidth = actualWidths[c];

        }

        return accumulatedWidth;
    }

    private static double AdjustAccumulatedWidth(double accumulatedWidth, double availableWidth)
    {
        if (double.IsPositiveInfinity(availableWidth))
        {
            return accumulatedWidth;
        }
        return accumulatedWidth < availableWidth ? availableWidth : accumulatedWidth;
    }

    public static Size Measure(Size availableSize, DataBox dataBox, Func<Size, Size> measureOverride, AvaloniaList<Control> rows)
    {
        var availableSizeWidth = availableSize.Width;

        dataBox.AvailableWidth = availableSize.Width;
        dataBox.AvailableHeight = availableSize.Height;

        var accumulatedWidth = SetColumnsActualWidth(dataBox);
        var panelSize = availableSize.WithWidth(accumulatedWidth);

        panelSize = measureOverride(panelSize);
        panelSize = panelSize.WithWidth(AdjustAccumulatedWidth(accumulatedWidth, availableSizeWidth));

        return panelSize;
    }

    public static Size Arrange(Size finalSize, DataBox dataBox, Func<Size, Size> arrangeOverride, AvaloniaList<Control> rows)
    {
        var finalSizeWidth = finalSize.Width;

        dataBox.AvailableWidth = finalSize.Width;
        dataBox.AvailableHeight = finalSize.Height;
        dataBox.AccumulatedWidth = SetColumnsActualWidth(dataBox);
        var panelSize = finalSize.WithWidth(dataBox.AccumulatedWidth);

        panelSize = arrangeOverride(panelSize);
        panelSize = panelSize.WithWidth(AdjustAccumulatedWidth(dataBox.AccumulatedWidth, finalSizeWidth));

        return panelSize;
    }
}
