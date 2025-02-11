using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Data;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Layout;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Avalonia.Metadata;
using Avalonia.Threading;
using Mesen.Utilities;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.Controls
{
	public class ScrollPictureViewer : UserControl
	{
		public static readonly StyledProperty<Vector> ScrollOffsetProperty = AvaloniaProperty.Register<ScrollPictureViewer, Vector>(nameof(ScrollOffset), defaultBindingMode: BindingMode.TwoWay);

		public static readonly StyledProperty<IImage> SourceProperty = AvaloniaProperty.Register<ScrollPictureViewer, IImage>(nameof(Source));
		public static readonly StyledProperty<double> ZoomProperty = AvaloniaProperty.Register<ScrollPictureViewer, double>(nameof(Zoom), 1, defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<int> GridSizeXProperty = AvaloniaProperty.Register<ScrollPictureViewer, int>(nameof(GridSizeX), 8);
		public static readonly StyledProperty<int> GridSizeYProperty = AvaloniaProperty.Register<ScrollPictureViewer, int>(nameof(GridSizeY), 8);
		public static readonly StyledProperty<bool> ShowGridProperty = AvaloniaProperty.Register<ScrollPictureViewer, bool>(nameof(ShowGrid), false);

		public static readonly StyledProperty<int> LeftClipSizeProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(LeftClipSize), 0);
		public static readonly StyledProperty<int> RightClipSizeProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(RightClipSize), 0);
		public static readonly StyledProperty<int> TopClipSizeProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(TopClipSize), 0);
		public static readonly StyledProperty<int> BottomClipSizeProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(BottomClipSize), 0);

		public static readonly StyledProperty<List<GridDefinition>?> CustomGridsProperty = AvaloniaProperty.Register<PictureViewer, List<GridDefinition>?>(nameof(CustomGrids), null);

		public static readonly StyledProperty<bool> AllowSelectionProperty = AvaloniaProperty.Register<ScrollPictureViewer, bool>(nameof(AllowSelection), true);
		public static readonly StyledProperty<bool> AllowClickDragProperty = AvaloniaProperty.Register<ScrollPictureViewer, bool>(nameof(AllowClickDrag), true);
		
		public static readonly StyledProperty<GridRowColumn?> GridHighlightProperty = AvaloniaProperty.Register<ScrollPictureViewer, GridRowColumn?>(nameof(GridHighlight), null);

		public static readonly StyledProperty<bool> ShowMousePositionProperty = AvaloniaProperty.Register<ScrollPictureViewer, bool>(nameof(ShowMousePosition), true);
		public static readonly StyledProperty<Rect?> MouseOverRectProperty = AvaloniaProperty.Register<ScrollPictureViewer, Rect?>(nameof(MouseOverRect), null, defaultBindingMode: BindingMode.OneWay);

		public static readonly StyledProperty<Rect> SelectionRectProperty = AvaloniaProperty.Register<ScrollPictureViewer, Rect>(nameof(SelectionRect), default, defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<Rect> OverlayRectProperty = AvaloniaProperty.Register<ScrollPictureViewer, Rect>(nameof(OverlayRect), default);
		
		public static readonly StyledProperty<ScrollBarVisibility> ScrollBarVisibilityProperty = AvaloniaProperty.Register<ScrollPictureViewer, ScrollBarVisibility>(nameof(ScrollBarVisibility), ScrollBarVisibility.Auto);

		public static readonly StyledProperty<List<PictureViewerLine>?> OverlayLinesProperty = AvaloniaProperty.Register<PictureViewer, List<PictureViewerLine>?>(nameof(OverlayLines), null);

		public Vector ScrollOffset
		{
			get { return GetValue(ScrollOffsetProperty); }
			set { SetValue(ScrollOffsetProperty, value); }
		}

		public IImage Source
		{
			get { return GetValue(SourceProperty); }
			set { SetValue(SourceProperty, value); }
		}

		public double Zoom
		{
			get { return GetValue(ZoomProperty); }
			set { SetValue(ZoomProperty, value); }
		}

		public bool AllowSelection
		{
			get { return GetValue(AllowSelectionProperty); }
			set { SetValue(AllowSelectionProperty, value); }
		}

		public bool ShowMousePosition
		{
			get { return GetValue(ShowMousePositionProperty); }
			set { SetValue(ShowMousePositionProperty, value); }
		}

		public int GridSizeX
		{
			get { return GetValue(GridSizeXProperty); }
			set { SetValue(GridSizeXProperty, value); }
		}

		public int GridSizeY
		{
			get { return GetValue(GridSizeYProperty); }
			set { SetValue(GridSizeYProperty, value); }
		}

		public bool ShowGrid
		{
			get { return GetValue(ShowGridProperty); }
			set { SetValue(ShowGridProperty, value); }
		}

		public List<GridDefinition>? CustomGrids
		{
			get { return GetValue(CustomGridsProperty); }
			set { SetValue(CustomGridsProperty, value); }
		}

		public bool AllowClickDrag
		{
			get { return GetValue(AllowClickDragProperty); }
			set { SetValue(AllowClickDragProperty, value); }
		}

		public Rect SelectionRect
		{
			get { return GetValue(SelectionRectProperty); }
			set { SetValue(SelectionRectProperty, value); }
		}

		public Rect OverlayRect
		{
			get { return GetValue(OverlayRectProperty); }
			set { SetValue(OverlayRectProperty, value); }
		}

		public Rect? MouseOverRect
		{
			get { return GetValue(MouseOverRectProperty); }
			set { SetValue(MouseOverRectProperty, value); }
		}

		public List<PictureViewerLine>? OverlayLines
		{
			get { return GetValue(OverlayLinesProperty); }
			set { SetValue(OverlayLinesProperty, value); }
		}

		public GridRowColumn? GridHighlight
		{
			get { return GetValue(GridHighlightProperty); }
			set { SetValue(GridHighlightProperty, value); }
		}

		public int TopClipSize
		{
			get { return GetValue(TopClipSizeProperty); }
			set { SetValue(TopClipSizeProperty, value); }
		}

		public int BottomClipSize
		{
			get { return GetValue(BottomClipSizeProperty); }
			set { SetValue(BottomClipSizeProperty, value); }
		}

		public int LeftClipSize
		{
			get { return GetValue(LeftClipSizeProperty); }
			set { SetValue(LeftClipSizeProperty, value); }
		}

		public int RightClipSize
		{
			get { return GetValue(RightClipSizeProperty); }
			set { SetValue(RightClipSizeProperty, value); }
		}

		public ScrollBarVisibility ScrollBarVisibility
		{
			get { return GetValue(ScrollBarVisibilityProperty); }
			set { SetValue(ScrollBarVisibilityProperty, value); }
		}

		private Point _lastPosition;

		static ScrollPictureViewer()
		{
			BoundsProperty.Changed.AddClassHandler<ScrollPictureViewer>((x, e) => x.UpdateScrollBarVisibility());
			SelectionRectProperty.Changed.AddClassHandler<ScrollPictureViewer>((x, e) => x.ScrollToSelection());
		}

		public ScrollPictureViewer()
		{
			InitializeComponent();
			Focusable = true;
			Background = new SolidColorBrush(0xFF202020);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public PictureViewer InnerViewer => this.GetControl<PictureViewer>("picViewer");

		private void Viewer_PointerPressed(object? sender, PointerPressedEventArgs e)
		{
			_lastPosition = e.GetCurrentPoint(this).Position;
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);
			InnerViewer.ProcessKeyDown(e);
		}

		private void UpdateScrollBarVisibility()
		{
			Size scrollViewerSize = this.GetControl<ScrollViewer>("scrollViewer").Bounds.Size;
			Size pictureViewerSize = InnerViewer.Bounds.Size;

			if(pictureViewerSize.Width <= scrollViewerSize.Width && pictureViewerSize.Height <= scrollViewerSize.Height) {
				//If picture is supposed to fit without a scrollbar, toggle scrollbar visibility to force Avalonia to hide the
				//scrollbar properly when the scrollbars themselves are preventing the scrollbars from being hidden
				ScrollBarVisibility = ScrollBarVisibility.Hidden;
				Dispatcher.UIThread.Post(() => {
					ScrollBarVisibility = ScrollBarVisibility.Auto;
				});
			}
		}

		protected override void OnPointerWheelChanged(PointerWheelEventArgs e)
		{
			base.OnPointerWheelChanged(e);
			if(e.KeyModifiers == KeyModifiers.Control) {
				double delta = e.GetDeltaY();
				if(delta > 0) {
					InnerViewer.ZoomIn();
				} else if(delta < 0) {
					InnerViewer.ZoomOut();
				}
				e.Handled = true;
			}
		}

		private void Viewer_PointerMoved(object? sender, PointerEventArgs e)
		{
			if(AllowClickDrag && e.GetCurrentPoint(this).Properties.IsLeftButtonPressed) {
				Vector offset = ScrollOffset;
				offset -= e.GetPosition(this) - _lastPosition;
				if(offset.X < 0) {
					offset = offset.WithX(0);
				}
				if(offset.Y < 0) {
					offset = offset.WithY(0);
				}

				Size extent = this.GetControl<ScrollViewer>("scrollViewer").Extent;
				Size viewport = this.GetControl<ScrollViewer>("scrollViewer").Viewport;
				Size maxOffsets = extent - viewport;

				if(offset.X > maxOffsets.Width) {
					offset = offset.WithX(maxOffsets.Width);
				}
				if(offset.Y > maxOffsets.Height) {
					offset = offset.WithY(maxOffsets.Height);
				}

				ScrollOffset = offset;
				_lastPosition = e.GetPosition(this);
			}
		}

		public void ScrollToSelection()
		{
			if(SelectionRect == default) {
				return;
			}

			ScrollViewer viewer = this.GetControl<ScrollViewer>("scrollViewer");

			Size viewport = viewer.Viewport;
			Vector offset = ScrollOffset;

			double dpiScale = 1 / LayoutHelper.GetLayoutScale(this);
			Rect rect = SelectionRect * Zoom * dpiScale;

			Rect visibleWindow = new Rect(new Point(offset.X, offset.Y), viewport);

			if(!visibleWindow.Contains(rect)) {
				Size extent = viewer.Extent;
				Size maxOffsets = extent - viewport;

				double xLeftGap = rect.Left - visibleWindow.Left;
				double xRightGap = rect.Right - visibleWindow.Right;
				double yTopGap = rect.Top - visibleWindow.Top;
				double yBottomGap = rect.Bottom - visibleWindow.Bottom;

				double xPadding = GridSizeX / 2 * Zoom;
				double yPadding = GridSizeY / 2 * Zoom;
				double x = offset.X + (xRightGap > 0 ? (xRightGap + xPadding): 0) + (xLeftGap < 0 ? (xLeftGap - xPadding) : 0);
				double y = offset.Y + (yBottomGap > 0 ? (yBottomGap + yPadding): 0) + (yTopGap < 0 ? (yTopGap - yPadding) : 0);

				ScrollOffset = new Vector(
					Math.Max(0, Math.Min(x, maxOffsets.Width)),
					Math.Max(0, Math.Min(y, maxOffsets.Height))
				);
			}
		}
	}
}
