using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Metadata;
using Avalonia.Controls.Primitives;
using Avalonia.Input;
using Avalonia.Media;
using Avalonia.Styling;
using Avalonia.VisualTree;

namespace DataBoxControl;

[PseudoClasses(":pressed", ":sortascending", ":sortdescending")]
public class DataBoxColumnHeader : ContentControl, IStyleable
{
    public static readonly StyledProperty<IBrush?> SeparatorBrushProperty =
        AvaloniaProperty.Register<DataBoxColumnHeader, IBrush?>(nameof(SeparatorBrush));

    public static readonly StyledProperty<bool> AreSeparatorsVisibleProperty =
        AvaloniaProperty.Register<DataBoxColumnHeader, bool>(
            nameof(AreSeparatorsVisible),
            defaultValue: true);

    internal static readonly StyledProperty<bool> IsPressedProperty =
        AvaloniaProperty.Register<DataBoxColumnHeader, bool>(nameof(IsPressed));

    public DataBoxColumnHeader()
    {
        UpdatePseudoClassesIsPressed(IsPressed);
    }

    Type IStyleable.StyleKey => typeof(DataBoxColumnHeader);

    internal DataBox? DataBox { get; set; }

    public IBrush? SeparatorBrush
    {
        get => GetValue(SeparatorBrushProperty);
        set => SetValue(SeparatorBrushProperty, value);
    }

    public bool AreSeparatorsVisible
    {
        get => GetValue(AreSeparatorsVisibleProperty);
        set => SetValue(AreSeparatorsVisibleProperty, value);
    }

    internal bool IsPressed
    {
        get => GetValue(IsPressedProperty);
        private set => SetValue(IsPressedProperty, value);
    }

    internal DataBoxColumn? Column { get; set; }

    internal IReadOnlyList<DataBoxColumnHeader>? ColumnHeaders { get; set; }

    protected override void OnApplyTemplate(TemplateAppliedEventArgs e)
    {
        base.OnApplyTemplate(e);

        UpdatePseudoClassesSortingState(Column?.SortingState);
    }

    protected override void OnPointerPressed(PointerPressedEventArgs e)
    {
        base.OnPointerPressed(e);
            
        if (e.GetCurrentPoint(this).Properties.IsLeftButtonPressed)
        {
            IsPressed = true;
            e.Handled = true;
        }
    }

    protected override void OnPointerReleased(PointerReleasedEventArgs e)
    {
        base.OnPointerReleased(e);

        if (IsPressed && e.InitialPressMouseButton == MouseButton.Left)
        {
            IsPressed = false;
            e.Handled = true;

            if (this.GetVisualsAt(e.GetPosition(this)).Any(c => this == c || this.IsVisualAncestorOf(c)))
            {
                OnClick(e.KeyModifiers);
            }
        }
    }

    protected override void OnPointerCaptureLost(PointerCaptureLostEventArgs e)
    {
        IsPressed = false;
    }

    protected override void OnPropertyChanged<T>(AvaloniaPropertyChangedEventArgs<T> change)
    {
        base.OnPropertyChanged(change);

        if (change.Property == IsPressedProperty)
        {
            UpdatePseudoClassesIsPressed(change.NewValue.GetValueOrDefault<bool>());
        }
    }

    private void OnClick(KeyModifiers keyModifiers)
    {
        if (DataBox is null)
        {
            return;
        }
            
        if (Column is null || Column.SortCommand is null || ColumnHeaders is null)
        {
            return;
        }

        if (!Column.CanUserSort || !DataBox.CanUserSortColumns)
        {
            return;
        }

        var ctrl = (keyModifiers & KeyModifiers.Control) == KeyModifiers.Control;
        var shift = (keyModifiers & KeyModifiers.Shift) == KeyModifiers.Shift;

        if (!shift)
        {
            foreach (var columnHeader in ColumnHeaders)
            {
                if (!Equals(columnHeader, this))
                {
                    if (columnHeader.Column is { } column)
                    {
                        column.SortingState = null;
                        columnHeader.UpdatePseudoClassesSortingState(column.SortingState);
                    }
                }
            }
        }

        string? sortMemberPath = ctrl ? null : Column.SortMemberPath;
        ListSortDirection? sortingState =  ctrl ? null : (Column.SortingState == ListSortDirection.Ascending
            ? ListSortDirection.Descending
            : ListSortDirection.Ascending);

        Column.SortingState = sortingState;

        UpdatePseudoClassesSortingState(sortingState);

        if (Column.SortCommand is { } command)
        {
            if (command.CanExecute(sortMemberPath))
            {
                command.Execute(sortMemberPath);
            }
        }
    }

    private void UpdatePseudoClassesIsPressed(bool isPressed)
    {
        PseudoClasses.Set(":pressed", isPressed);
    }

    private void UpdatePseudoClassesSortingState(ListSortDirection? sortingState)
    {
        PseudoClasses.Set(":sortascending", sortingState == ListSortDirection.Ascending);
        PseudoClasses.Set(":sortdescending", sortingState == ListSortDirection.Descending);
    }
}
