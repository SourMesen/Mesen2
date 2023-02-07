using System;
using System.Linq;
using Avalonia;
using Avalonia.Controls;
using Avalonia.LogicalTree;
using Avalonia.Styling;
using DataBoxControl.Primitives.Layout;

namespace DataBoxControl.Controls;

public class DataBoxPanel : VirtualizingStackPanel, IStyleable
{
	Type IStyleable.StyleKey => typeof(DataBoxPanel);

	internal DataBox? DataBox { get; set; }
	
	public override void ApplyTemplate()
	{
		base.ApplyTemplate();

		DataBox = this.GetLogicalAncestors().FirstOrDefault(x => x is DataBox) as DataBox;
	}

	protected override Size MeasureOverride(Size availableSize)
	{
		if(DataBox is null) {
			return availableSize;
		}

		return DataBoxRowsLayout.Measure(availableSize, DataBox, base.MeasureOverride, Children);
	}

	protected override Size ArrangeOverride(Size finalSize)
	{
		if(DataBox is null) {
			return finalSize;
		}

		return DataBoxRowsLayout.Arrange(finalSize, DataBox, base.ArrangeOverride, Children);
	}
}