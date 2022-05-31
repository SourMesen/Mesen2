using System;
using Avalonia.Controls;
using Avalonia.Controls.Generators;
using Avalonia.Styling;

namespace DataBoxControl.Primitives;

public class DataBoxRowsPresenter : ListBox, IStyleable
{
    internal DataBox? DataBox { get; set; }

    Type IStyleable.StyleKey => typeof(DataBoxRowsPresenter);

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
