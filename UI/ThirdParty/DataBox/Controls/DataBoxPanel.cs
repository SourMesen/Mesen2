using System;
using System.Linq;
using Avalonia;
using Avalonia.Controls;
using Avalonia.LogicalTree;
using Avalonia.Styling;
using Avalonia.VisualTree;
using DataBoxControl.Primitives.Layout;

namespace DataBoxControl.Controls;

public class DataBoxPanel : VirtualizingStackPanel
{
	protected override Type StyleKeyOverride => typeof(DataBoxPanel);

	internal DataBox? DataBox { get; set; }
	
	public override void ApplyTemplate()
	{
		base.ApplyTemplate();

		DataBox = this.GetLogicalAncestors().FirstOrDefault(x => x is DataBox) as DataBox;
	}

	protected override Size MeasureOverride(Size availableSize)
	{
		if(this.GetVisualRoot() == null) {
			//Prevent issues when measure is called after window was closed
			//Otherwise EstimateViewport in VirtualizingStackPanel uses a 500+k pixel height which causes performance problems
			return default;
		}

		if(DataBox is null) {
			return availableSize;
		}

		return DataBoxRowsLayout.Measure(availableSize, DataBox, base.MeasureOverride, Children);
	}

	protected override Size ArrangeOverride(Size finalSize)
	{
		if(DataBox is null || this.GetVisualRoot() == null) {
			return finalSize;
		}

		return DataBoxRowsLayout.Arrange(finalSize, DataBox, base.ArrangeOverride, Children);
	}
}