using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Controls.Shapes;
using Avalonia.Styling;

namespace DataBoxControl;

public class DataBoxCell : ContentControl, IStyleable
{
    private Rectangle? _rightGridLine;

    internal DataBox? DataBox { get; set; }
    internal DataBoxColumn? Column { get; set; }

	internal double MeasuredWidth { get; set; }

    Type IStyleable.StyleKey => typeof(DataBoxCell);

    protected override void OnApplyTemplate(TemplateAppliedEventArgs e)
    {
        base.OnApplyTemplate(e);

        _rightGridLine = e.NameScope.Find<Rectangle>("PART_RightGridLine");

        InvalidateRightGridLine();
    }

    protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
    {
        base.OnAttachedToVisualTree(e);

        InvalidateRightGridLine();
    }

    private void InvalidateRightGridLine()
    {
        if (_rightGridLine is null || DataBox is null)
        {
            return;
        }

        bool newVisibility =
            DataBox.GridLinesVisibility == DataBoxGridLinesVisibility.Vertical
            || DataBox.GridLinesVisibility == DataBoxGridLinesVisibility.All;

        if (newVisibility != _rightGridLine.IsVisible)
        {
            _rightGridLine.IsVisible = newVisibility;
        }

        _rightGridLine.Fill = DataBox.VerticalGridLinesBrush;
    }
}
