using System;
using System.Collections.Generic;
using System.Linq;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.LogicalTree;
using Avalonia.Styling;
using DataBoxControl.Primitives.Layout;

namespace DataBoxControl.Primitives;

public class DataBoxCellsPresenter : Panel, IStyleable
{
    Type IStyleable.StyleKey => typeof(DataBoxCellsPresenter);

    internal DataBox? DataBox { get; set; }

    internal void Attach()
    {
        if (DataBox is null)
        {
            return;
        }

        List<DataBoxCell> cells = new();
        foreach (var column in DataBox.Columns)
        {
            var cell = new DataBoxCell
            {
                [!ContentControl.ContentProperty] = this[!DataContextProperty],
                [!ContentControl.ContentTemplateProperty] = column[!DataBoxColumn.CellTemplateProperty],
                HorizontalAlignment = HorizontalAlignment.Stretch,
                VerticalAlignment = VerticalAlignment.Stretch,
                DataBox = DataBox,
					 Column = column
            };
			
				cell.Tapped += DataBox.OnCellTapped;
				cell.DoubleTapped += DataBox.OnCellDoubleTapped;
				cell.PointerReleased += DataBox.OnCellPointerReleased;
				cells.Add(cell);
        }
        Children.AddRange(cells);
    }

    protected override Size MeasureOverride(Size availableSize)
    {
        return DataBoxCellsLayout.Measure(availableSize, DataBox, Children);
    }

    protected override Size ArrangeOverride(Size arrangeSize)
    {
        return DataBoxCellsLayout.Arrange(arrangeSize, DataBox, Children);
    }

	public T? GetControl<T>(int index) where T : class
	{
		return ((Control)VisualChildren[0].VisualChildren[0]).GetLogicalDescendants().Where(x => x is T).ElementAtOrDefault(index) as T;
	}
}
