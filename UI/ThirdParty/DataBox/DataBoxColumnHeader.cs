using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reflection;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Metadata;
using Avalonia.Controls.Primitives;
using Avalonia.Controls.Shapes;
using Avalonia.Input;
using Avalonia.Media;
using Avalonia.Styling;
using Avalonia.VisualTree;
using DataBoxControl.Controls;

namespace DataBoxControl;

[PseudoClasses(":pressed", ":sortascending", ":sortdescending")]
public class DataBoxColumnHeader : ContentControl
{
    public static readonly StyledProperty<IBrush?> SeparatorBrushProperty =
        AvaloniaProperty.Register<DataBoxColumnHeader, IBrush?>(nameof(SeparatorBrush));

    public static readonly StyledProperty<bool> AreSeparatorsVisibleProperty =
        AvaloniaProperty.Register<DataBoxColumnHeader, bool>(
            nameof(AreSeparatorsVisible),
            defaultValue: true);

    internal static readonly StyledProperty<bool> IsPressedProperty =
        AvaloniaProperty.Register<DataBoxColumnHeader, bool>(nameof(IsPressed));
    internal static readonly StyledProperty<bool> IsResizingProperty =
        AvaloniaProperty.Register<DataBoxColumnHeader, bool>(nameof(IsResizingProperty));
   internal static readonly StyledProperty<string> SortNumberProperty =
       AvaloniaProperty.Register<DataBoxColumnHeader, string>(nameof(SortNumber));

   public DataBoxColumnHeader()
    {
        UpdatePseudoClassesIsPressed(IsPressed);
    }

    protected override Type StyleKeyOverride => typeof(DataBoxColumnHeader);

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

    internal bool IsResizing
    {
        get => GetValue(IsResizingProperty);
        private set => SetValue(IsResizingProperty, value);
    }

   public string SortNumber
   {
      get => GetValue(SortNumberProperty);
      set => SetValue(SortNumberProperty, value);
   }

   internal bool ResizePreviousColumn { get; set; }
   internal double ResizePositionX { get; set; }
   internal DataBoxColumn? Column { get; set; }

    internal IReadOnlyList<DataBoxColumnHeader>? ColumnHeaders { get; set; }

    protected override void OnApplyTemplate(TemplateAppliedEventArgs e)
    {
        base.OnApplyTemplate(e);
    }

    protected override void OnPointerPressed(PointerPressedEventArgs e)
    {
        base.OnPointerPressed(e);

        if (e.GetCurrentPoint(this).Properties.IsLeftButtonPressed)
        {
            if((e.Source is Panel p && p.Name == "PanelResize") || (e.Source is Rectangle r && r.Name == "VerticalSeparator")) {
               IsResizing = true;
               ResizePreviousColumn = false;
               ResizePositionX = e.GetCurrentPoint(DataBox).Position.X;
            } else if((e.Source is Panel pRight && pRight.Name == "PanelResizeRight") || (e.Source is Rectangle rRight && rRight.Name == "VerticalSeparatorRight")) {
               IsResizing = true;
               ResizePreviousColumn = true;
               ResizePositionX = e.GetCurrentPoint(DataBox).Position.X;
            }else {
               IsPressed = true;
            }
            e.Handled = true;
        }
    }

    protected override void OnPointerReleased(PointerReleasedEventArgs e)
    {
        base.OnPointerReleased(e);

        if (IsResizing && e.InitialPressMouseButton == MouseButton.Left)
        {
            IsResizing = false;
            e.Handled = true;
        }
        else if (IsPressed && e.InitialPressMouseButton == MouseButton.Left)
        {
            IsPressed = false;
            e.Handled = true;

            if (this.GetVisualsAt(e.GetPosition(this)).Any(c => this == c || this.IsVisualAncestorOf(c)))
            {
                OnClick(e.KeyModifiers);
            }
        }
    }

    protected override void OnPointerMoved(PointerEventArgs e)
    {
        base.OnPointerMoved(e);
        if(IsResizing && Column != null && DataBox != null) {
            double x = e.GetCurrentPoint(DataBox).Position.X;
            DataBoxColumn column;

            int index = DataBox.Columns.IndexOf(Column);
            if(ResizePreviousColumn) {
                index--;
            }
            if(index >= 0) {
                column = DataBox.Columns[index];

                DataBox.ColumnWidths[index] += (int)(x - ResizePositionX);
                DataBox.ColumnWidths[index] = Math.Min(column.MaxWidth, Math.Max(column.MinWidth, DataBox.ColumnWidths[index]));

                ResizePositionX = x;

                DataBoxPanel? panel = DataBox.FindDescendantOfType<DataBoxPanel>();
                panel?.InvalidateMeasure();
                panel?.InvalidateVisual();
                DataBox.InvalidateMeasure();
                DataBox.InvalidateVisual();
            }
        }
    }

    protected override void OnPointerCaptureLost(PointerCaptureLostEventArgs e)
    {
        IsPressed = false;
    }

    protected override void OnPropertyChanged(AvaloniaPropertyChangedEventArgs change)
    {
        base.OnPropertyChanged(change);

        if (change.Property == IsPressedProperty && change.NewValue is bool isPressed)
        {
            UpdatePseudoClassesIsPressed(isPressed);
        }
    }

   private void OnClick(KeyModifiers keyModifiers)
   {
      if(DataBox is null || DataBox.SortMode == SortMode.None || DataBox.SortCommand is null) {
         return;
      }

      if(Column is null || ColumnHeaders is null || !Column.CanUserSort || string.IsNullOrEmpty(Column.ColumnName)) {
         return;
      }

      var ctrl = (keyModifiers & KeyModifiers.Control) == KeyModifiers.Control;
      var shift = (keyModifiers & KeyModifiers.Shift) == KeyModifiers.Shift;

      SortState sortState = DataBox.SortState;

      if(ctrl) {
         sortState.Remove(Column.ColumnName);
      } else if(shift && DataBox.SortMode == SortMode.Multiple) {
         sortState.ToggleSortOrder(Column.ColumnName, false);
      } else {
         sortState.ToggleSortOrder(Column.ColumnName, true);
      }

      sortState.UpdateColumnHeaders(ColumnHeaders);

      if(DataBox.SortCommand.CanExecute(null)) {
         DataBox.SortCommand.Execute(null);
      }
   }

    private void UpdatePseudoClassesIsPressed(bool isPressed)
    {
        PseudoClasses.Set(":pressed", isPressed);
    }

    public void UpdatePseudoClassesSortingState(ListSortDirection? sortingState)
    {
        PseudoClasses.Set(":sortascending", sortingState == ListSortDirection.Ascending);
        PseudoClasses.Set(":sortdescending", sortingState == ListSortDirection.Descending);
    }
}
