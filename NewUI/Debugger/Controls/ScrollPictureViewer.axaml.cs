using Avalonia;
using Avalonia.Controls;
using Avalonia.Data;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Avalonia.Metadata;
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

		public static readonly StyledProperty<int> AltGridSizeXProperty = AvaloniaProperty.Register<ScrollPictureViewer, int>(nameof(AltGridSizeX), 8);
		public static readonly StyledProperty<int> AltGridSizeYProperty = AvaloniaProperty.Register<ScrollPictureViewer, int>(nameof(AltGridSizeY), 8);
		public static readonly StyledProperty<bool> ShowAltGridProperty = AvaloniaProperty.Register<ScrollPictureViewer, bool>(nameof(ShowAltGrid), false);
		public static readonly StyledProperty<bool> AllowSelectionProperty = AvaloniaProperty.Register<ScrollPictureViewer, bool>(nameof(AllowSelection), true);
		public static readonly StyledProperty<bool> AllowClickDragProperty = AvaloniaProperty.Register<ScrollPictureViewer, bool>(nameof(AllowClickDrag), true);
		
		public static readonly StyledProperty<GridRowColumn?> GridHighlightProperty = AvaloniaProperty.Register<ScrollPictureViewer, GridRowColumn?>(nameof(GridHighlight), null);

		public static readonly StyledProperty<bool> ShowMousePositionProperty = AvaloniaProperty.Register<ScrollPictureViewer, bool>(nameof(ShowMousePosition), true);
		public static readonly StyledProperty<Rect?> MouseOverRectProperty = AvaloniaProperty.Register<ScrollPictureViewer, Rect?>(nameof(MouseOverRect), null, defaultBindingMode: BindingMode.OneWay);

		public static readonly StyledProperty<Rect> SelectionRectProperty = AvaloniaProperty.Register<ScrollPictureViewer, Rect>(nameof(SelectionRect), Rect.Empty, defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<Rect> OverlayRectProperty = AvaloniaProperty.Register<ScrollPictureViewer, Rect>(nameof(OverlayRect), Rect.Empty);

		public static readonly StyledProperty<List<Rect>?> HighlightRectsProperty = AvaloniaProperty.Register<ScrollPictureViewer, List<Rect>?>(nameof(HighlightRects), null);

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

		public int AltGridSizeX
		{
			get { return GetValue(AltGridSizeXProperty); }
			set { SetValue(AltGridSizeXProperty, value); }
		}

		public int AltGridSizeY
		{
			get { return GetValue(AltGridSizeYProperty); }
			set { SetValue(AltGridSizeYProperty, value); }
		}

		public bool ShowAltGrid
		{
			get { return GetValue(ShowAltGridProperty); }
			set { SetValue(ShowAltGridProperty, value); }
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

		public List<Rect>? HighlightRects
		{
			get { return GetValue(HighlightRectsProperty); }
			set { SetValue(HighlightRectsProperty, value); }
		}

		public GridRowColumn? GridHighlight
		{
			get { return GetValue(GridHighlightProperty); }
			set { SetValue(GridHighlightProperty, value); }
		}

		private Point _lastPosition;

		static ScrollPictureViewer()
		{
		}

		public ScrollPictureViewer()
		{
			InitializeComponent();
			Background = new SolidColorBrush(0xFF202020);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public PictureViewer InnerViewer => this.FindControl<PictureViewer>("picViewer");

		private void Viewer_PointerPressed(object? sender, PointerPressedEventArgs e)
		{
			_lastPosition = e.GetCurrentPoint(this).Position;
		}

		protected override void OnPointerWheelChanged(PointerWheelEventArgs e)
		{
			base.OnPointerWheelChanged(e);
			if(e.KeyModifiers == KeyModifiers.Control) {
				if(e.Delta.Y > 0) {
					InnerViewer.ZoomIn();
				} else {
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

				Size extent = this.FindControl<ScrollViewer>("scrollViewer").Extent;
				Size viewport = this.FindControl<ScrollViewer>("scrollViewer").Viewport;
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
	}
}
