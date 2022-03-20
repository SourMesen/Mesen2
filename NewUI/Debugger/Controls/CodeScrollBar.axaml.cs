using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Controls.Shapes;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.Windows;
using System;

namespace Mesen.Debugger.Controls
{
	public class CodeScrollBar : UserControl
	{
		public static readonly StyledProperty<int> ValueProperty = AvaloniaProperty.Register<CodeScrollBar, int>(nameof(Value), 0, defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<int> MaximumProperty = AvaloniaProperty.Register<CodeScrollBar, int>(nameof(Maximum), 0);
		
		public static readonly StyledProperty<bool> ShowMarkersProperty = AvaloniaProperty.Register<CodeScrollBar, bool>(nameof(ShowMarkers), true);
		public static readonly StyledProperty<int[]> MarkerTopProperty = AvaloniaProperty.Register<CodeScrollBar, int[]>(nameof(MarkerTop), new int[7]);
		
		public static readonly StyledProperty<Control?> BreakpointBarProperty = AvaloniaProperty.Register<CodeScrollBar, Control?>(nameof(BreakpointBar), null);

		public int Value
		{
			get { return GetValue(ValueProperty); }
			set { SetValue(ValueProperty, value); }
		}

		public int Maximum
		{
			get { return GetValue(MaximumProperty); }
			set { SetValue(MaximumProperty, value); }
		}

		public bool ShowMarkers
		{
			get { return GetValue(ShowMarkersProperty); }
			set { SetValue(ShowMarkersProperty, value); }
		}

		public int[] MarkerTop
		{
			get { return GetValue(MarkerTopProperty); }
			set { SetValue(MarkerTopProperty, value); }
		}

		public Control? BreakpointBar
		{
			get { return GetValue(BreakpointBarProperty); }
			set { SetValue(BreakpointBarProperty, value); }
		}

		static CodeScrollBar()
		{
			ValueProperty.Changed.AddClassHandler<CodeScrollBar>((x, e) => {
				x.Value = Math.Max(0, Math.Min(x.Value, x.Maximum));
				x.UpdatePosition();
			});

			BoundsProperty.Changed.AddClassHandler<CodeScrollBar>((x, e) => {
				x.UpdatePosition();

				double height = x._panel.Bounds.Height;
				int[] markerTop = new int[7];
				for(int i = 1; i < 8; i++) {
					markerTop[i - 1] = (int)((height / 8) * i) - 6;
				}

				x.MarkerTop = markerTop;
			});
		}

		private Panel _panel;
		private Rectangle _thumb;

		public CodeScrollBar()
		{
			InitializeComponent();

			_panel = this.FindControl<Panel>("Panel");
			_thumb = this.FindControl<Rectangle>("Thumb");

			_panel.AddHandler(Panel.PointerPressedEvent, (s, e) => {
				ScrollToPosition(e.GetPosition(_panel));
			}, RoutingStrategies.Bubble, true);

			_panel.AddHandler(Panel.PointerMovedEvent, (s, e) => {
				if(e.GetCurrentPoint(null).Properties.IsLeftButtonPressed) {
					ScrollToPosition(e.GetPosition(_panel));
				}
			}, RoutingStrategies.Bubble, true);
		}

		private void ScrollToPosition(Point p)
		{
			double height = _panel.Bounds.Height;
			double ratio = p.Y / height;
			Value = Math.Max(0, Math.Min(Maximum, (int)(Maximum * ratio)));
		}

		private void UpdatePosition()
		{
			if(Maximum > 0) {
				_thumb.Margin = new Thickness(0, (((double)Value / Maximum) * _panel.Bounds.Height) - 10, 0, 0);
			}
		}

		protected override void OnPointerWheelChanged(PointerWheelEventArgs e)
		{
			base.OnPointerWheelChanged(e);
			Value = Math.Max(0, Math.Min(Maximum, Value - (int)(e.Delta.Y * 3)));
		}

		private void IncrementClick(object? sender, RoutedEventArgs e)
		{
			Value = Math.Max(0, Math.Min(Maximum, Value + 1));
		}

		private void DecrementClick(object? sender, RoutedEventArgs e)
		{
			Value = Math.Max(0, Math.Min(Maximum, Value - 1));
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
