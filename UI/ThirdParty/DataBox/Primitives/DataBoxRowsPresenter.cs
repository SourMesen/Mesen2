using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Generators;
using Avalonia.Controls.Primitives;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Styling;
using Avalonia.VisualTree;

namespace DataBoxControl.Primitives;

public class DataBoxRowsPresenter : ListBox
{
	internal DataBox? DataBox { get; set; }

	protected override Type StyleKeyOverride => typeof(DataBoxRowsPresenter);

	protected override Control CreateContainerForItemOverride(object? item, int index, object? recycleKey)
	{
		return new DataBoxRow {
			DataBox = DataBox
		};
	}

	protected override void PrepareContainerForItemOverride(Control element, object? item, int index)
	{
		base.PrepareContainerForItemOverride(element, item, index);

		if(element is DataBoxRow row) {
			row.DataBox = DataBox;
		}
	}

	protected override void ClearContainerForItemOverride(Control element)
	{
		base.ClearContainerForItemOverride(element);

		if(element is DataBoxRow row) {
			row.DataBox = null;
		}
	}

	public DataBoxRow? GetRow(int index)
	{
		return this.ContainerFromIndex(index) as DataBoxRow;
	}
}