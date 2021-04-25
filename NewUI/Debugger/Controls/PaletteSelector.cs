using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Media;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Controls
{
	public partial class PaletteSelector : Control
	{
		public static readonly StyledProperty<int> SelectedPaletteProperty = AvaloniaProperty.Register<PaletteSelector, int>(
			nameof(SelectedPalette),
			defaultBindingMode: Avalonia.Data.BindingMode.TwoWay,
			coerce: CoerceSelectedPalette
		);

		public static readonly StyledProperty<PaletteSelectionMode> SelectionModeProperty = AvaloniaProperty.Register<PaletteSelector, PaletteSelectionMode>(nameof(SelectionMode));
		public static readonly StyledProperty<int> ColumnCountProperty = AvaloniaProperty.Register<PaletteSelector, int>(nameof(ColumnCount), 16);
		public static readonly StyledProperty<int> BlockSizeProperty = AvaloniaProperty.Register<PaletteSelector, int>(nameof(BlockSize), 0);
		public static readonly StyledProperty<UInt32[]> PaletteColorsProperty = AvaloniaProperty.Register<PaletteSelector, UInt32[]>(nameof(PaletteColors));
		public static readonly StyledProperty<bool> ShowIndexesProperty = AvaloniaProperty.Register<PaletteSelector, bool>(nameof(ShowIndexes));

		public static readonly RoutedEvent<ColorClickEventArgs> ColorClickEvent = RoutedEvent.Register<PaletteSelector, ColorClickEventArgs>(nameof(ColorClick), RoutingStrategies.Direct);

		public event EventHandler<ColorClickEventArgs> ColorClick
		{
			add => AddHandler(ColorClickEvent, value);
			remove => RemoveHandler(ColorClickEvent, value);
		}

		public int SelectedPalette
		{
			get { return GetValue(SelectedPaletteProperty); }
			set { SetValue(SelectedPaletteProperty, value); }
		}

		public PaletteSelectionMode SelectionMode
		{
			get { return GetValue(SelectionModeProperty); }
			set { SetValue(SelectionModeProperty, value); }
		}

		public UInt32[] PaletteColors
		{
			get { return GetValue(PaletteColorsProperty); }
			set { SetValue(PaletteColorsProperty, value); }
		}
		
		public int ColumnCount
		{
			get { return GetValue(ColumnCountProperty); }
			set { SetValue(ColumnCountProperty, value); }
		}

		public int BlockSize
		{
			get { return GetValue(BlockSizeProperty); }
			set { SetValue(BlockSizeProperty, value); }
		}

		public bool ShowIndexes
		{
			get { return GetValue(ShowIndexesProperty); }
			set { SetValue(ShowIndexesProperty, value); }
		}

		static PaletteSelector()
		{
			AffectsRender<PaletteSelector>(SelectionModeProperty, SelectedPaletteProperty, PaletteColorsProperty, ColumnCountProperty, ShowIndexesProperty);
			AffectsMeasure<PaletteSelector>(ColumnCountProperty, BlockSizeProperty, PaletteColorsProperty);
		}

		public PaletteSelector()
		{
			this.GetObservable(SelectionModeProperty).Subscribe((mode) => {
				this.CoerceValue<int>(SelectedPaletteProperty);
			});

			this.GetObservable(PaletteColorsProperty).Subscribe((mode) => {
				this.CoerceValue<int>(SelectedPaletteProperty);
			});

			Focusable = true;
			ClipToBounds = true;
		}

		protected override Size MeasureOverride(Size availableSize)
		{
			if(PaletteColors != null) {
				return new Size(ColumnCount * BlockSize, (PaletteColors.Length / ColumnCount) * BlockSize);
			} else {
				return availableSize;
			}
		}

		private static int CoerceSelectedPalette(IAvaloniaObject o, int value)
		{
			if(o is PaletteSelector selector) {
				int maxPalette = 0;
				int colorCount = selector.PaletteColors?.Length ?? 256;
				switch(selector.SelectionMode) {
					default: maxPalette = 0; break;
					case PaletteSelectionMode.SingleColor: maxPalette = colorCount - 1; break;
					case PaletteSelectionMode.FourColors: maxPalette = (colorCount / 4) - 1; break;
					case PaletteSelectionMode.SixteenColors: maxPalette = (colorCount / 16) -1; break;
				}

				return Math.Max(0, Math.Min(value, maxPalette));
			}

			return value;
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			switch(e.Key) {
				case Key.Left: SelectedPalette--; break;
				case Key.Right: SelectedPalette++; break;
				
				case Key.Up:
					switch(SelectionMode) {
						case PaletteSelectionMode.SingleColor: SelectedPalette -= ColumnCount; break;
						case PaletteSelectionMode.FourColors: SelectedPalette -= ColumnCount / 4; break;
						case PaletteSelectionMode.SixteenColors: SelectedPalette -= ColumnCount / 16; break;
					}
					break;

				case Key.Down:
					switch(SelectionMode) {
						case PaletteSelectionMode.SingleColor: SelectedPalette += ColumnCount; break;
						case PaletteSelectionMode.FourColors: SelectedPalette += ColumnCount / 4; break;
						case PaletteSelectionMode.SixteenColors: SelectedPalette += ColumnCount / 16; break;
					}
					break;
			}
		}

		public override void Render(DrawingContext context)
		{
			UInt32[] paletteColors = PaletteColors;
			
			if(paletteColors == null) {
				return;
			}

			Size size = Bounds.Size;
			int columnCount = ColumnCount;
			int rowCount = paletteColors.Length / columnCount;
			double width = size.Width / columnCount;
			double height = size.Height / rowCount;
			if(BlockSize > 0) {
				width = BlockSize;
				height = BlockSize;
			}

			Typeface typeface = new Typeface("Consolas");
			FormattedText text = new FormattedText("", typeface, 11, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			for(int y = 0, max = paletteColors.Length / columnCount; y < max; y++) {
				for(int x = 0; x < columnCount; x++) {
					Rect rect = new Rect(x * width, y * height, width, height);
					int index = y * columnCount + x;
					context.FillRectangle(new SolidColorBrush(paletteColors[y * columnCount + x]), rect);

					if(ShowIndexes) {
						rect = rect.Translate(new Vector(2, 0));
						text.Text = index.ToString("X2");
						context.DrawText(Brushes.Black, rect.Translate(new Vector(-1, 0)).Position, text);
						context.DrawText(Brushes.Black, rect.Translate(new Vector(1, 0)).Position, text);
						context.DrawText(Brushes.Black, rect.Translate(new Vector(0, -1)).Position, text);
						context.DrawText(Brushes.Black, rect.Translate(new Vector(0, 1)).Position, text);
						context.DrawText(Brushes.White, rect.Position, text);
					}
				}
			}

			Pen pen = new Pen(Colors.LightGray.ToUint32(), 2, DashStyle.Dash);
			if(SelectionMode == PaletteSelectionMode.SingleColor) {
				int selectedRow = SelectedPalette / columnCount;
				context.DrawRectangle(pen, new Rect((SelectedPalette % columnCount) * width, selectedRow * height, width, height));
			} else if(SelectionMode == PaletteSelectionMode.FourColors) {
				int selectedRow = (SelectedPalette * 4) / columnCount;
				context.DrawRectangle(pen, new Rect((SelectedPalette % (columnCount / 4)) * width * 4, selectedRow * height, width * 4, height));
			} else if(SelectionMode == PaletteSelectionMode.SixteenColors) {
				int selectedRow = (SelectedPalette * 16) / columnCount;
				if(columnCount >= 16) {
					context.DrawRectangle(pen, new Rect((SelectedPalette % (columnCount / 16)) * width, selectedRow * height, width * 16, height));
				}
			}
		}

		protected override void OnPointerPressed(PointerPressedEventArgs e)
		{
			base.OnPointerPressed(e);

			Point p = e.GetCurrentPoint(this).Position;

			Size size = Bounds.Size;
			int columnCount = ColumnCount;
			int rowCount = PaletteColors.Length / columnCount;
			double cellWidth = size.Width / columnCount;
			double cellHeight = size.Height / rowCount;
			if(BlockSize > 0) {
				cellWidth = BlockSize;
				cellHeight = BlockSize;
			}

			int clickedRow = (int)(p.Y / cellHeight);
			int clickedColumn = (int)(p.X / cellWidth);

			int paletteIndex = clickedRow * columnCount + clickedColumn;

			RaiseEvent(new ColorClickEventArgs() { ColorIndex = paletteIndex, Color = Color.FromUInt32(PaletteColors[paletteIndex]) });

			if(SelectionMode == PaletteSelectionMode.SingleColor) {
				paletteIndex /= 1;
			} else if(SelectionMode == PaletteSelectionMode.FourColors) {
				paletteIndex /= 4;
			} else if(SelectionMode == PaletteSelectionMode.SixteenColors) {
				paletteIndex /= 16;
			}
			SelectedPalette = paletteIndex;
		}

		public class ColorClickEventArgs : RoutedEventArgs
		{
			public int ColorIndex;
			public Color Color;

			public ColorClickEventArgs()
			{
				this.RoutedEvent = PaletteSelector.ColorClickEvent;
			}
		}
	}

	public enum PaletteSelectionMode
	{
		None,
		SingleColor,
		FourColors,
		SixteenColors
	}
}