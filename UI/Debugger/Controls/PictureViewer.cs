using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Data;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using Avalonia.Rendering.SceneGraph;
using Avalonia.Skia;
using Avalonia.Threading;
using Mesen.Utilities;
using SkiaSharp;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;

namespace Mesen.Debugger.Controls
{
	public class PictureViewer : Control
	{
		public static readonly StyledProperty<IImage> SourceProperty = AvaloniaProperty.Register<PictureViewer, IImage>(nameof(Source));
		public static readonly StyledProperty<double> ZoomProperty = AvaloniaProperty.Register<PictureViewer, double>(nameof(Zoom), 1, defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<int> GridSizeXProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(GridSizeX), 8);
		public static readonly StyledProperty<int> GridSizeYProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(GridSizeY), 8);
		public static readonly StyledProperty<bool> ShowGridProperty = AvaloniaProperty.Register<PictureViewer, bool>(nameof(ShowGrid), false);

		public static readonly StyledProperty<int> LeftClipSizeProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(LeftClipSize), 0);
		public static readonly StyledProperty<int> RightClipSizeProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(RightClipSize), 0);
		public static readonly StyledProperty<int> TopClipSizeProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(TopClipSize), 0);
		public static readonly StyledProperty<int> BottomClipSizeProperty = AvaloniaProperty.Register<PictureViewer, int>(nameof(BottomClipSize), 0);

		public static readonly StyledProperty<bool> AllowSelectionProperty = AvaloniaProperty.Register<PictureViewer, bool>(nameof(AllowSelection), true);

		public static readonly StyledProperty<List<GridDefinition>?> CustomGridsProperty = AvaloniaProperty.Register<PictureViewer, List<GridDefinition>?>(nameof(CustomGrids), null);

		public static readonly StyledProperty<GridRowColumn?> GridHighlightProperty = AvaloniaProperty.Register<PictureViewer, GridRowColumn?>(nameof(GridHighlight), null);

		public static readonly StyledProperty<bool> ShowMousePositionProperty = AvaloniaProperty.Register<PictureViewer, bool>(nameof(ShowMousePosition), true);
		public static readonly StyledProperty<Rect?> MouseOverRectProperty = AvaloniaProperty.Register<PictureViewer, Rect?>(nameof(MouseOverRect), null, defaultBindingMode: BindingMode.OneWay);

		public static readonly StyledProperty<Rect> SelectionRectProperty = AvaloniaProperty.Register<PictureViewer, Rect>(nameof(SelectionRect), default, defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<Rect> OverlayRectProperty = AvaloniaProperty.Register<PictureViewer, Rect>(nameof(OverlayRect), default);

		public static readonly StyledProperty<List<PictureViewerLine>?> OverlayLinesProperty = AvaloniaProperty.Register<PictureViewer, List<PictureViewerLine>?>(nameof(OverlayLines), null);

		public static readonly RoutedEvent<PositionClickedEventArgs> PositionClickedEvent = RoutedEvent.Register<PictureViewer, PositionClickedEventArgs>(nameof(PositionClicked), RoutingStrategies.Bubble);
		public event EventHandler<PositionClickedEventArgs> PositionClicked
		{
			add => AddHandler(PositionClickedEvent, value);
			remove => RemoveHandler(PositionClickedEvent, value);
		}

		private delegate void PositionClickedHandler(Point p);

		private Stopwatch _stopWatch = Stopwatch.StartNew();
		private DispatcherTimer _timer = new DispatcherTimer();

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

		static PictureViewer()
		{
			AffectsRender<PictureViewer>(
				SourceProperty, ZoomProperty, GridSizeXProperty, GridSizeYProperty,
				ShowGridProperty, SelectionRectProperty, OverlayRectProperty,
				MouseOverRectProperty, GridHighlightProperty,
				OverlayLinesProperty, TopClipSizeProperty, LeftClipSizeProperty,
				BottomClipSizeProperty, RightClipSizeProperty, CustomGridsProperty
			);

			SourceProperty.Changed.AddClassHandler<PictureViewer>((x, e) => {
				x.UpdateSize();

				if(e.OldValue is IDynamicBitmap oldSource) {
					oldSource.Invalidated -= x.OnSourceInvalidated;
				}

				if(x.Source is IDynamicBitmap newSource) {
					newSource.Invalidated += x.OnSourceInvalidated;
				}
			});

			ZoomProperty.Changed.AddClassHandler<PictureViewer>((x, e) => {
				x.UpdateSize();
			});

			LeftClipSizeProperty.Changed.AddClassHandler<PictureViewer>((x, e) => x.UpdateSize());
			RightClipSizeProperty.Changed.AddClassHandler<PictureViewer>((x, e) => x.UpdateSize());
			TopClipSizeProperty.Changed.AddClassHandler<PictureViewer>((x, e) => x.UpdateSize());
			BottomClipSizeProperty.Changed.AddClassHandler<PictureViewer>((x, e) => x.UpdateSize());
		}

		public PictureViewer()
		{
			VerticalAlignment = VerticalAlignment.Top;
			HorizontalAlignment = HorizontalAlignment.Left;
			ClipToBounds = true;
			RenderOptions.SetBitmapInterpolationMode(this, BitmapInterpolationMode.None);
		}

		private void OnSourceInvalidated(object? sender, EventArgs e)
		{
			InvalidateVisual();
		}

		protected override void OnUnloaded(RoutedEventArgs e)
		{
			if(Source is IDynamicBitmap src) {
				src.Invalidated -= OnSourceInvalidated;
			}
			base.OnUnloaded(e);
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			base.OnAttachedToVisualTree(e);
			_timer.Interval = TimeSpan.FromMilliseconds(250);
			_timer.Tick += timer_Tick;
			_timer.Start();
			UpdateSize();
		}

		private void timer_Tick(object? sender, EventArgs e)
		{
			if(SelectionRect != default) {
				InvalidateVisual();
			}
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
				double delta = e.GetDeltaY();
				if(delta > 0) {
					ZoomIn();
				} else if(delta < 0) {
					ZoomOut();
				}
				e.Handled = true;
			}
		}

		public void ZoomIn()
		{
			if(Zoom < 1) {
				Zoom = 1;
			} else {
				Zoom = Math.Min(40, Math.Max(1, Zoom + 1));
			}
		}

		public void ZoomOut()
		{
			if(Zoom <= 1) {
				Zoom = 0.5;
			} else {
				Zoom = Math.Min(40, Math.Max(1, Zoom - 1));
			}
		}

		public async void ExportToPng()
		{
			if(Source is Bitmap bitmap) {
				string? filename = await FileDialogHelper.SaveFile(null, null, this.VisualRoot, FileDialogHelper.PngExt);
				if(filename != null) {
					bitmap.Save(filename);
				}
			}
		}

		private void UpdateSize()
		{
			if(Source == null) {
				MinWidth = 0;
				MinHeight = 0;
			} else {
				double dpiScale = LayoutHelper.GetLayoutScale(this);
				MinWidth = Math.Max(0, (int)(Source.Size.Width - LeftClipSize - RightClipSize) * Zoom / dpiScale);
				MinHeight = Math.Max(0, (int)(Source.Size.Height - TopClipSize - BottomClipSize) * Zoom / dpiScale);
			}
		}

		public void ProcessKeyDown(KeyEventArgs e)
		{
			if(!AllowSelection) {
				return;
			}

			double GetMaxX() => Source.Size.Width - GridSizeX;
			double GetMaxY() => Source.Size.Height - GridSizeY;

			if(e.Key == Key.Left) {
				SelectionRect = SelectionRect.WithX(SelectionRect.X <= 0 ? GetMaxX() : (SelectionRect.X - GridSizeX));
				e.Handled = true;
			} else if(e.Key == Key.Right) {
				SelectionRect = SelectionRect.WithX(SelectionRect.X >= GetMaxX() ? 0 : (SelectionRect.X + GridSizeX));
				e.Handled = true;
			} else if(e.Key == Key.Up) {
				SelectionRect = SelectionRect.WithY(SelectionRect.Y <= 0 ? GetMaxY() : (SelectionRect.Y - GridSizeY));
				e.Handled = true;
			} else if(e.Key == Key.Down) {
				SelectionRect = SelectionRect.WithY(SelectionRect.Y >= GetMaxY() ? 0 : (SelectionRect.Y + GridSizeY));
				e.Handled = true;
			}
		}

		protected override void OnPointerMoved(PointerEventArgs e)
		{
			base.OnPointerMoved(e);
			PixelPoint? p = GetGridPointFromMousePoint(e.GetCurrentPoint(this).Position);
			if(p == null) {
				e.Handled = true;
				MouseOverRect = null;
				return;
			}

			if(ShowMousePosition) {
				MouseOverRect = GetTileRect(p.Value);
			}

			PointerPointProperties props = e.GetCurrentPoint(this).Properties;
			if(props.IsLeftButtonPressed || props.IsRightButtonPressed) {
				PositionClickedEventArgs args = new(p.Value, props, e, PositionClickedEvent);
				RaiseEvent(args);
			}
		}

		protected override void OnPointerExited(PointerEventArgs e)
		{
			base.OnPointerExited(e);
			MouseOverRect = null;
		}

		protected override void OnPointerPressed(PointerPressedEventArgs e)
		{
			PixelPoint? p = GetGridPointFromMousePoint(e.GetCurrentPoint(this).Position);
			if(p == null) {
				e.Handled = true;
				return;
			}

			PositionClickedEventArgs args = new(p.Value, e.GetCurrentPoint(this).Properties, e, PositionClickedEvent);
			RaiseEvent(args);

			if(!args.Handled && AllowSelection) {
				SelectionRect = GetTileRect(p.Value);
			}
		}

		public PixelPoint? GetGridPointFromMousePoint(Point p)
		{
			double leftClip = LeftClipSize * Zoom / LayoutHelper.GetLayoutScale(this);
			double topClip = TopClipSize * Zoom / LayoutHelper.GetLayoutScale(this);
			p = new Point(p.X + leftClip, p.Y + topClip);

			if(p.X < 0 || p.Y < 0 || p.X >= MinWidth + leftClip || p.Y >= MinHeight + topClip) {
				return null;
			}

			double scale = LayoutHelper.GetLayoutScale(this) / Zoom;
			PixelPoint point = PixelPoint.FromPoint(p, scale);
			if(point.X < 0 || point.Y < 0 || point.X >= Source.Size.Width || point.Y >= Source.Size.Height) {
				return null;
			}

			return point;
		}

		private Rect GetTileRect(PixelPoint p)
		{
			return new Rect(
				p.X / GridSizeX * GridSizeX,
				p.Y / GridSizeY * GridSizeY,
				GridSizeX,
				GridSizeY
			);
		}

		private Rect ToDrawRect(Rect r)
		{
			return new Rect(
				r.X * Zoom - 0.5,
				r.Y * Zoom - 0.5,
				r.Width * Zoom + 1,
				r.Height * Zoom + 1
			);
		}

		private void DrawGrid(DrawingContext context, bool show, GridDefinition gridDef)
		{
			if(show) {
				int width = (int)(Source.Size.Width * Zoom);
				int height = (int)(Source.Size.Height * Zoom);
				int gridSizeX = (int)(gridDef.SizeX * Zoom);
				int gridSizeY = (int)(gridDef.SizeY * Zoom);
				if(gridSizeX <= 1 || gridSizeY <= 1) {
					return;
				}

				double gridRestartY = (int)(gridDef.RestartY * Zoom) + 0.5;

				Pen pen = new Pen(gridDef.Color.ToUInt32(), 1);
				double offset = 0.5;
				for(int i = 1; i <= width / gridSizeX; i++) {
					double x = i * gridSizeX + offset;
					context.DrawLine(pen, new Point(x, 0), new Point(x, height));
				}
				for(int i = 1; i <= height / gridSizeY; i++) {
					double y = i * gridSizeY + offset;
					if(gridRestartY > 0.5 && y >= gridRestartY) {
						context.DrawLine(pen, new Point(0, gridRestartY), new Point(width, gridRestartY));
						offset += (y - gridRestartY);
						y += (y - gridRestartY);
						gridRestartY = 0;
					}
					context.DrawLine(pen, new Point(0, y), new Point(width, y));
				}
			}
		}

		public override void Render(DrawingContext context)
		{
			if(Source == null) {
				return;
			}
			
			int width = (int)(Source.Size.Width * Zoom);
			int height = (int)(Source.Size.Height * Zoom);
			
			double dpiScale = 1 / LayoutHelper.GetLayoutScale(this);
			using var scale = context.PushTransform(Matrix.CreateScale(dpiScale, dpiScale));

			using var translation = context.PushTransform(Matrix.CreateTranslation(-LeftClipSize * Zoom, -TopClipSize * Zoom));
			using var clip = context.PushClip(new Rect(0, 0, width, height));

			if(Source is DynamicBitmap) {
				context.Custom(new PictureViewerDrawOperation(this));
			} else {
				context.DrawImage(
					Source,
					new Rect(0, 0, (int)Source.Size.Width, (int)Source.Size.Height),
					new Rect(0, 0, width, height)
				);
			}

			DrawGrid(context, ShowGrid, new GridDefinition() { SizeX = GridSizeX, SizeY = GridSizeY, Color = Color.FromArgb(192, Colors.LightBlue.R, Colors.LightBlue.G, Colors.LightBlue.B) });

			if(CustomGrids != null) {
				foreach(GridDefinition gridDef in CustomGrids) {
					DrawGrid(context, true, gridDef);
				}
			}

			if(OverlayRect != default) {
				Rect rect = ToDrawRect(OverlayRect);
				Brush brush = new SolidColorBrush(Colors.Gray, 0.4);
				Pen pen = new Pen(Brushes.White, 2);

				context.FillRectangle(brush, rect);
				context.DrawRectangle(pen, rect.Inflate(0.5));

				if((rect.Top + rect.Height) > height) {
					Rect offsetRect = rect.Translate(new Vector(0, -height));
					context.FillRectangle(brush, offsetRect);
					context.DrawRectangle(pen, offsetRect.Inflate(0.5));
				}

				if((rect.Left + rect.Width) > width) {
					Rect offsetRect = rect.Translate(new Vector(-width, 0));
					context.FillRectangle(brush, offsetRect);
					context.DrawRectangle(pen, offsetRect.Inflate(0.5));

					if((rect.Top + rect.Height) > height) {
						offsetRect = rect.Translate(new Vector(-width, -height));
						context.FillRectangle(brush, offsetRect);
						context.DrawRectangle(pen, offsetRect.Inflate(0.5));
					}
				}
			}

			if(OverlayLines?.Count > 0) {
				foreach(PictureViewerLine line in OverlayLines) {
					Pen pen = new Pen(line.Color.ToUInt32(), line.Width ?? 2, line.DashStyle);
					context.DrawLine(pen, line.Start * Zoom, line.End * Zoom);
				}
			}

			if(MouseOverRect != null && MouseOverRect.Value != default) {
				Rect rect = ToDrawRect(MouseOverRect.Value);
				DashStyle dashes = new DashStyle(DashStyle.Dash.Dashes, 0);
				context.DrawRectangle(new Pen(Brushes.DimGray, 2), rect.Inflate(0.5));
				context.DrawRectangle(new Pen(Brushes.LightYellow, 2, dashes), rect.Inflate(0.5));
			}

			if(SelectionRect != default) {
				Rect rect = ToDrawRect(SelectionRect);
				
				DashStyle dashes = new DashStyle(DashStyle.Dash.Dashes, _stopWatch.ElapsedMilliseconds / 250.0);
				context.DrawRectangle(new Pen(Brushes.Black, 2), rect.Inflate(0.5));
				context.DrawRectangle(new Pen(Brushes.White, 2, dashes), rect.Inflate(0.5));
			}

			if(GridHighlight != null) {
				GridRowColumn point = GridHighlight;
				PixelPoint p = new PixelPoint((int)(point.X * Zoom), (int)(point.Y * Zoom));

				Pen pen = new Pen(0x80FFFFFF, Math.Max(1, Zoom - 1));
				DrawHighlightLines(context, point, p, pen);

				FormattedText text = new FormattedText(point.DisplayValue, CultureInfo.CurrentCulture, FlowDirection.LeftToRight, new Typeface(FontFamily.Default), 14 + Zoom * 2, Brushes.Black);

				Point textPos = new Point(p.X + point.Width * Zoom + pen.Thickness * 2 + 5, p.Y - 5 - text.Height - pen.Thickness * 2);
				if(text.Width + textPos.X >= width) {
					textPos = textPos.WithX(p.X - text.Width - 5 - pen.Thickness * 2);
				}
				if(textPos.Y < 0) {
					textPos = textPos.WithY(p.Y + point.Height * Zoom + 5 + pen.Thickness);
				}

				for(int i = -2; i <= 2; i++) {
					for(int j = -2; j <= 2; j++) {
						context.DrawText(text, textPos + new Point(i, j));
					}
				}
				text.SetForegroundBrush(Brushes.White);
				context.DrawText(text, textPos);
			}
		}

		private void DrawHighlightLines(DrawingContext context, GridRowColumn point, PixelPoint p, Pen pen)
		{
			Rect bounds = Bounds * LayoutHelper.GetLayoutScale(this);
			context.DrawLine(pen, new Point(p.X - pen.Thickness / 2, 0), new Point(p.X - pen.Thickness / 2, bounds.Height));
			context.DrawLine(pen, new Point(p.X + point.Width * Zoom + pen.Thickness / 2, 0), new Point(p.X + point.Width * Zoom + pen.Thickness / 2, bounds.Height));

			context.DrawLine(pen, new Point(0, p.Y - pen.Thickness / 2), new Point(bounds.Width, p.Y - pen.Thickness / 2));
			context.DrawLine(pen, new Point(0, p.Y + point.Height * Zoom + pen.Thickness / 2), new Point(bounds.Width, p.Y + point.Height * Zoom + pen.Thickness / 2));
		}
	}

	class PictureViewerDrawOperation : ICustomDrawOperation
	{
		public Rect Bounds { get; private set; }

		private DynamicBitmap _source;
		private double _zoom;
		private SKBitmap _bitmap;
		private SKPaint _highlightPaint;

		public PictureViewerDrawOperation(PictureViewer viewer)
		{
			//Inflate(1) fixes a refresh issue in tooltips in the sprite viewer:
			//  First pixel in the preview image keeps the same data as the tooltip that was active before it
			//Translate fixes a similar refresh issue when LeftClip/TopClip are not 0 (e.g sprite viewer)
			//  Bottom/right part of the picture were not getting updated
			double scale = LayoutHelper.GetLayoutScale(viewer);
			Bounds = (viewer.Bounds * scale).Inflate(1 * scale).Translate(new Vector(viewer.LeftClipSize * viewer.Zoom, viewer.TopClipSize * viewer.Zoom));
			_source = (DynamicBitmap)viewer.Source;
			_zoom = viewer.Zoom;
			using(var lockedBuffer = ((WriteableBitmap)_source).Lock()) {
				var info = new SKImageInfo(
					lockedBuffer.Size.Width,
					lockedBuffer.Size.Height,
					lockedBuffer.Format.ToSkColorType(),
					SKAlphaType.Premul
				);
				_bitmap = new SKBitmap();
				_bitmap.InstallPixels(info, lockedBuffer.Address);
			}
			_highlightPaint = new SKPaint() { IsStroke = true, Color = new SKColor(Colors.LightSteelBlue.R, Colors.LightSteelBlue.G, Colors.LightSteelBlue.B) };
		}

		public void Dispose()
		{
		}

		public bool Equals(ICustomDrawOperation? other) => false;
		public bool HitTest(Point p) => true;

		private SKRect ToDrawRect(Rect r)
		{
			return new SKRect(
				(float)(r.X * _zoom - 0.5),
				(float)(r.Y * _zoom - 0.5),
				(float)((r.X + r.Width) * _zoom),
				(float)((r.Y + r.Height) * _zoom)
			);
		}

		public void Render(ImmediateDrawingContext context)
		{
			var leaseFeature = context.PlatformImpl.GetFeature<ISkiaSharpApiLeaseFeature>();
			if(leaseFeature != null) {
				using var lease = leaseFeature.Lease();
				var canvas = lease.SkCanvas;
				canvas.Save();

				int width = (int)(_source.Size.Width * _zoom);
				int height = (int)(_source.Size.Height * _zoom);

				using(_source.Lock(true)) {
					canvas.DrawBitmap(_bitmap,
						new SKRect(0, 0, (int)_source.Size.Width, (int)_source.Size.Height),
						new SKRect(0, 0, width, height)
					);

					List<Rect>? highlightRects = _source.HighlightRects;
					if(highlightRects?.Count > 0) {
						foreach(Rect highlightRect in highlightRects) {
							SKRect rect = ToDrawRect(highlightRect);
							canvas.DrawRect(rect, _highlightPaint);
						}
					}
				}
				canvas.Restore();
			}
		}
	}
	
	public class PositionClickedEventArgs : RoutedEventArgs
	{
		public PixelPoint Position { get; }
		public PointerPointProperties Properties { get; }
		public PointerEventArgs OriginalEvent { get; }

		public PositionClickedEventArgs(PixelPoint position, PointerPointProperties properties, PointerEventArgs originalEvent, RoutedEvent evt)
		{
			Position = position;
			Properties = properties;
			OriginalEvent = originalEvent;
			RoutedEvent = evt;
		}
	}

	public class GridRowColumn
	{
		public int X;
		public int Y;
		public int Width;
		public int Height;
		public string DisplayValue = "";
	}

	public struct GridDefinition
	{
		public int SizeX;
		public int SizeY;
		public Color Color;
		public int RestartY;
	}

	public struct PictureViewerLine
	{
		public Point Start;
		public Point End;
		public Color Color;
		public double? Width;
		public IDashStyle? DashStyle;
	}
}
