using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
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
		public static readonly StyledProperty<int> ZoomProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(Zoom), 1);
		public static readonly StyledProperty<int> GridSizeXProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(GridSizeX), 8);
		public static readonly StyledProperty<int> GridSizeYProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(GridSizeY), 8);
		public static readonly StyledProperty<bool> ShowGridProperty = AvaloniaProperty.Register<PictureViewer, bool>(nameof(ShowGrid), false);

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

		private Point? _mousePosition = null;
		private PixelPoint? _selectedTile = null;

		static PictureViewer()
		{
			AffectsRender<PictureViewer>(SourceProperty, ZoomProperty, GridSizeXProperty, GridSizeYProperty, ShowGridProperty);
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
				Zoom = Math.Min(20, Math.Max(1, Zoom + (e.Delta.Y > 0 ? 1 : -1)));
				UpdateSize();
				e.Handled = true;
			}
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
			p = new Point(Math.Min(p.X, MinWidth - 1), Math.Min(p.Y, MinHeight - 1));
			_selectedTile = new PixelPoint((int)(p.X / GridSizeX / Zoom), (int)(p.Y / GridSizeY / Zoom));
			InvalidateVisual();
		}

		public override void Render(DrawingContext context)
		{
			if(Source == null) {
				return;
			}

			int width = Source.PixelSize.Width * Zoom;
			int height = Source.PixelSize.Height * Zoom;

			context.FillRectangle(new SolidColorBrush(0xFF333333), new Rect(Bounds.Size));

			context.DrawImage(
				Source,
				new Rect(0, 0, Source.PixelSize.Width, Source.PixelSize.Height),
				new Rect(0, 0, width, height),
				Avalonia.Visuals.Media.Imaging.BitmapInterpolationMode.Default
			);

			int gridSizeX = GridSizeX * Zoom;
			int gridSizeY = GridSizeY * Zoom;
			if(ShowGrid) {
				Pen pen = new Pen(Color.FromArgb(128, Colors.LightBlue.R, Colors.LightBlue.G, Colors.LightBlue.B).ToUint32());
				for(int i = 1; i < width / gridSizeX; i++) {
					context.DrawLine(pen, new Point(i * gridSizeX + 0.5, 0), new Point(i* gridSizeX + 0.5, height));
				}
				for(int i = 1; i < height / gridSizeY; i++) {
					context.DrawLine(pen, new Point(0, i * gridSizeY + 0.5), new Point(width, i * gridSizeY + 0.5));
				}
			}

			if(_selectedTile.HasValue) {
				Rect rect = new Rect(
					_selectedTile.Value.X * gridSizeX - 0.5,
					_selectedTile.Value.Y * gridSizeY - 0.5,
					gridSizeX + 1,
					gridSizeY + 1
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
}
