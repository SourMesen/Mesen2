using System;
using Avalonia;
using Avalonia.Collections;
using Avalonia.Controls;

namespace DataBoxControl.Primitives.Layout;

internal static class DataBoxCellsLayout
{
    public static Size Measure(Size availableSize, DataBox? dataBox, AvaloniaList<Control> children)
    {
        if (dataBox is null || children.Count == 0)
        {
            return availableSize;
        }

        var parentWidth = 0.0;
        var parentHeight = 0.0;

        for (var i = 0; i < children.Count; ++i)
        {
            if (i >= dataBox.Columns.Count)
            {
                break;
            }

            var child = children[i];
            var column = dataBox.Columns[i];
            var width = Math.Max(0.0, double.IsNaN(column.MeasureWidth) ? 0.0 : column.MeasureWidth);

            width = Math.Max(column.MinWidth, width);
            width = Math.Min(column.MaxWidth, width);

            child.Measure(new Size(double.PositiveInfinity, double.PositiveInfinity));

            parentWidth += width;
            parentHeight = Math.Max(parentHeight, child.DesiredSize.Height);
        }

        return new Size(parentWidth, parentHeight);
    }

    public static Size Arrange(Size arrangeSize, DataBox? dataBox, AvaloniaList<Control> children)
    {
        if (dataBox is null || children.Count == 0)
        {
            return arrangeSize;
        }

        var accumulatedWidth = 0.0;
        var accumulatedHeight = 0.0;
        var maxHeight = 0.0;

        for (var i = 0; i < children.Count; ++i)
        {
            var child = children[i];

            maxHeight = Math.Max(maxHeight, child.DesiredSize.Height);
        }

        for (var i = 0; i < children.Count; ++i)
        {
            if (i >= dataBox.Columns.Count)
            {
                break;
            }

            var child = children[i];
            var column = dataBox.Columns[i];
            var width = Math.Max(0.0, double.IsNaN(column.MeasureWidth) ? 0.0 : column.MeasureWidth);
            var height = Math.Max(maxHeight, arrangeSize.Height);
            var rect = new Rect(accumulatedWidth, 0.0, width, height);

            child.Arrange(rect);

            accumulatedWidth += width;
            accumulatedHeight = Math.Max(accumulatedHeight, height);
        }

        return new Size(accumulatedWidth, accumulatedHeight);
    }
}
