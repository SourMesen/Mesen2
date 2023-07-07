using System;
using System.Collections.Generic;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Styling;
using DataBoxControl.Primitives.Layout;

namespace DataBoxControl.Primitives;

public class DataBoxColumnHeadersPresenter : Panel
{
    private List<IDisposable>? _columnActualWidthDisposables;
    private List<DataBoxColumnHeader>? _columnHeaders;

	protected override Type StyleKeyOverride => typeof(DataBoxColumnHeadersPresenter);
        
    internal DataBox? DataBox { get; set; }

    protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
    {
        base.OnDetachedFromVisualTree(e);

        Detach();
    }

    internal void Attach()
    {
        if (DataBox is null)
        {
            return;
        }

        _columnHeaders = new List<DataBoxColumnHeader>();
        _columnActualWidthDisposables = new List<IDisposable>();

        for (var c = 0; c < DataBox.Columns.Count; c++)
        {
            var column = DataBox.Columns[c];

            var columnHeader = new DataBoxColumnHeader
            {
                [!ContentControl.ContentProperty] = column[!DataBoxColumn.HeaderProperty],
                HorizontalAlignment = HorizontalAlignment.Stretch,
                VerticalAlignment = VerticalAlignment.Stretch,
                Column = column,
                ColumnHeaders = _columnHeaders,
                DataBox = DataBox
            };

            Children.Add(columnHeader);
            _columnHeaders.Add(columnHeader);

            var disposable = column.GetObservable(DataBoxColumn.MeasureWidthProperty).Subscribe(_ =>
            {
                InvalidateMeasure();
                InvalidateVisual();
            });
            _columnActualWidthDisposables.Add(disposable);
        }

			DataBox.SortState.UpdateColumnHeaders(_columnHeaders);
		}

    internal void Detach()
    {
        if (_columnActualWidthDisposables is { })
        {
            foreach (var disposable in _columnActualWidthDisposables)
            {
                disposable.Dispose();
            }

            _columnActualWidthDisposables.Clear();
            _columnActualWidthDisposables = null;
        }

        _columnHeaders?.Clear();
        _columnHeaders = null;
    }

    protected override Size MeasureOverride(Size availableSize)
    {
        return DataBoxCellsLayout.Measure(availableSize, DataBox, Children);
    }

    protected override Size ArrangeOverride(Size arrangeSize)
    {
        return DataBoxCellsLayout.Arrange(arrangeSize, DataBox, Children);
    }
}
