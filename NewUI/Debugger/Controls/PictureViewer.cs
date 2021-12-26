using Avalonia;
using Avalonia.Controls;
using Avalonia.Data;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using Mesen.Debugger;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Controls
{
	public class PictureViewer : Control
	{
		public static readonly StyledProperty<Bitmap> SourceProperty = AvaloniaProperty.Register<PictureViewer, Bitmap>(nameof(Source));
		public static readonly StyledProperty<int> ZoomProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(Zoom), 1, defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<int> GridSizeXProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(GridSizeX), 8);
		public static readonly StyledProperty<int> GridSizeYProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(GridSizeY), 8);
		public static readonly StyledProperty<bool> ShowGridProperty = AvaloniaProperty.Register<PictureViewer, bool>(nameof(ShowGrid), false);

		public static readonly StyledProperty<int> AltGridSizeXProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(AltGridSizeX), 8);
		public static readonly StyledProperty<int> AltGridSizeYProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(AltGridSizeY), 8);
		public static readonly StyledProperty<bool> ShowAltGridProperty = AvaloniaProperty.Register<PictureViewer, bool>(nameof(ShowAltGrid), false);
		public static readonly StyledProperty<bool> AllowSelectionProperty = AvaloniaProperty.Register<PictureViewer, bool>(nameof(AllowSelection), true);
		
		public static readonly StyledProperty<Rect> SelectionRectProperty = AvaloniaProperty.Register<PictureViewer, Rect>(nameof(SelectionRect), Rect.Empty);

		public static readonly RoutedEvent<PositionClickedEventArgs> PositionClickedEvent = RoutedEvent.Register<PictureViewer, PositionClickedEventArgs>(nameof(PositionClicked), RoutingStrategies.Bubble);
		public event EventHandler<PositionClickedEventArgs> PositionClicked
		{
			add => AddHandler(PositionClickedEvent, value);
			remove => RemoveHandler(PositionClickedEvent, value);
		}

		private delegate void PositionClickedHandler(Point p);

		private Stopwatch _stopWatch = Stopwatch.StartNew();
		private DispatcherTimer _timer = new DispatcherTimer();

		public Bitmap Source
		{
			get { return GetValue(SourceProperty); }
			set { SetValue(SourceProperty, value); }
		}

		public int Zoom
		{
			get { return GetValue(ZoomProperty); }
			set { SetValue(ZoomProperty, value); }
		}
		
		public bool AllowSelection
		{
			get { return GetValue(AllowSelectionProperty); }
			set { SetValue(AllowSelectionProperty, value); }
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

		public Rect SelectionRect
		{
			get { return GetValue(SelectionRectProperty); }
			set { SetValue(SelectionRectProperty, value); }
		}

		private Point? _mousePosition = null;

		static PictureViewer()
		{
			AffectsRender<PictureViewer>(SourceProperty, ZoomProperty, GridSizeXProperty, GridSizeYProperty, ShowGridProperty, SelectionRectProperty);
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			base.OnAttachedToVisualTree(e);
			_timer.Interval = TimeSpan.FromMilliseconds(50);
			_timer.Tick += timer_Tick;
			_timer.Start();
		}

		private void timer_Tick(object? sender, EventArgs e)
		{
			InvalidateVisual();
		}

		protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
		{
			base.OnDetachedFromVisualTree(e);
			_timer.Stop();
		}

		protected override void OnPointerWheelChanged(PointerWheelEventArgs e)
		{
			base.OnPointerWheelChanged(e);
			if(e.KeyModifiers == KeyModifiers.Control) {
				if(e.Delta.Y > 0) {
					ZoomIn();
				} else {
					ZoomOut();
				}
				UpdateSize();
				e.Handled = true;
			}
		}

		public void ZoomIn()
		{
			Zoom = Math.Min(20, Math.Max(1, Zoom + 1));
		}

		public void ZoomOut()
		{
			Zoom = Math.Min(20, Math.Max(1, Zoom - 1));
		}

		private void UpdateSize()
		{
			MinWidth = Source.PixelSize.Width * Zoom;
			MinHeight = Source.PixelSize.Height * Zoom;
		}

		protected override void OnPropertyChanged<T>(AvaloniaPropertyChangedEventArgs<T> change)
		{
			base.OnPropertyChanged(change);
			if(change.Property == SourceProperty) {
				UpdateSize();
			}
		}

		protected override void OnPointerMoved(PointerEventArgs e)
		{
			base.OnPointerMoved(e);
			_mousePosition = e.GetCurrentPoint(this).Position;
			InvalidateVisual();
		}

		protected override void OnPointerLeave(PointerEventArgs e)
		{
			base.OnPointerLeave(e);
			_mousePosition = null;
			InvalidateVisual();
		}

		protected override void OnPointerPressed(PointerPressedEventArgs e)
		{
			base.OnPointerPressed(e);
			Point p = e.GetCurrentPoint(this).Position;
			p = new Point(Math.Min(p.X, MinWidth - 1) / Zoom, Math.Min(p.Y, MinHeight - 1) / Zoom);

			PositionClickedEventArgs args = new() { RoutedEvent = PositionClickedEvent, Position = p };
			RaiseEvent(args);

			if(!args.Handled && AllowSelection) {
				Rect selection = new Rect(
					(int)p.X / GridSizeX * GridSizeX,
					(int)p.Y / GridSizeY * GridSizeY,
					GridSizeX,
					GridSizeY
				);
				SelectionRect = selection;
			}
		}

		private void DrawGrid(DrawingContext context, bool show, int gridX, int gridY, Color color)
		{
			if(show) {
				int width = Source.PixelSize.Width * Zoom;
				int height = Source.PixelSize.Height * Zoom;
				int gridSizeX = gridX * Zoom;
				int gridSizeY = gridY * Zoom;

				Pen pen = new Pen(color.ToUint32(), 1 + (Zoom / 2));
				double offset = 0;
				if(Zoom % 2 == 1) {
					offset = 0.5;
				}
				for(int i = 1; i < width / gridSizeX; i++) {
					context.DrawLine(pen, new Point(i * gridSizeX + offset, 0), new Point(i * gridSizeX + offset, height));
				}
				for(int i = 1; i < height / gridSizeY; i++) {
					context.DrawLine(pen, new Point(0, i * gridSizeY + offset), new Point(width, i * gridSizeY + offset));
				}
			}
		}

		public override void Render(DrawingContext context)
		{
			if(Source == null) {
				return;
			}

			int width = Source.PixelSize.Width * Zoom;
			int height = Source.PixelSize.Height * Zoom;

			context.FillRectangle(new SolidColorBrush(0xFFFFFFFF), new Rect(Bounds.Size));

			context.DrawImage(
				Source,
				new Rect(0, 0, Source.PixelSize.Width, Source.PixelSize.Height),
				new Rect(0, 0, width, height),
				Avalonia.Visuals.Media.Imaging.BitmapInterpolationMode.Default
			);

			DrawGrid(context, ShowGrid, GridSizeX, GridSizeY, Color.FromArgb(128, Colors.LightBlue.R, Colors.LightBlue.G, Colors.LightBlue.B));
			DrawGrid(context, ShowAltGrid, AltGridSizeX, AltGridSizeY, Color.FromArgb(128, Colors.Red.R, Colors.Red.G, Colors.Red.B));

			if(SelectionRect != Rect.Empty) {
				Rect rect = new Rect(
					SelectionRect.X * Zoom - 0.5,
					SelectionRect.Y * Zoom - 0.5,
					SelectionRect.Width * Zoom + 1,
					SelectionRect.Height * Zoom + 1
				);
				
				DashStyle dashes = new DashStyle(DashStyle.Dash.Dashes, (double)(_stopWatch.ElapsedMilliseconds / 50) % 100 / 5);
				context.DrawRectangle(new Pen(0x40000000, 2), rect.Inflate(0.5));
				context.DrawRectangle(new Pen(Brushes.White, 2, dashes), rect.Inflate(0.5));
			}

			//TODO delete?
			/*if(_mousePosition.HasValue) {
				Point p = new Point(Math.Floor(_mousePosition.Value.X) + 0.5, Math.Floor(_mousePosition.Value.Y) + 0.5);
				context.DrawLine(new Pen(Brushes.Black), new Point(p.X, 0), new Point(p.X, Bounds.Height));
				context.DrawLine(new Pen(Brushes.Black), new Point(0, p.Y), new Point(Bounds.Width, p.Y));
			}*/
		}
	}

	public class PositionClickedEventArgs : RoutedEventArgs
	{
		public Point Position;
	}
}
