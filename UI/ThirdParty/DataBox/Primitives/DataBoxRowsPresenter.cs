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

public class DataBoxRowsPresenter : ListBox, IStyleable
{
    internal DataBox? DataBox { get; set; }

    Type IStyleable.StyleKey => typeof(DataBoxRowsPresenter);

	protected override void OnApplyTemplate(TemplateAppliedEventArgs e)
	{
		base.OnApplyTemplate(e);
		ScrollViewer? viewer = this.FindDescendantOfType<ScrollViewer>();
		if(viewer != null) {
			viewer.AddHandler<PointerWheelEventArgs>(ScrollViewer.PointerWheelChangedEvent, DataBoxRowsPresenter_PointerWheelChanged, RoutingStrategies.Tunnel);
		}
	}

	private void DataBoxRowsPresenter_PointerWheelChanged(object? sender, PointerWheelEventArgs e)
	{
		if(sender is ScrollViewer viewer && e.KeyModifiers != KeyModifiers.Shift) {
			double offset = viewer.Offset.Y - e.Delta.Y * 3;
			if(offset < 0) {
				viewer.Offset = viewer.Offset.WithY(0);
			} else if(offset > viewer.Extent.Height) {
				viewer.Offset = viewer.Offset.WithY(viewer.Extent.Height);
			} else {
				viewer.Offset = viewer.Offset.WithY(offset);
			}
			e.Handled = true;
		}
	}

	protected override IItemContainerGenerator CreateItemContainerGenerator()
    {
        var generator = new ItemContainerGenerator<DataBoxRow>(
            this,
            ContentControl.ContentProperty,
            ContentControl.ContentTemplateProperty);

        generator.Materialized += (_, args) =>
        {
            foreach (var container in args.Containers)
            {
                if (container.ContainerControl is DataBoxRow row)
                {
                    row.DataBox = DataBox;
                }
            }
        };

        generator.Dematerialized += (_, args) =>
        {
            foreach (var container in args.Containers)
            {
                if (container.ContainerControl is DataBoxRow row)
                {
                    row.DataBox = null;
                }
            }
        };

        generator.Recycled += (_, args) =>
        {
            foreach (var container in args.Containers)
            {
                if (container.ContainerControl is DataBoxRow row)
                {
                    row.DataBox = DataBox;
                }
            }
        };

        return generator;
    }
}
