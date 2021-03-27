using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Media.TextFormatting;
using Avalonia.Platform;
using Avalonia.Rendering.SceneGraph;
using Avalonia.Skia;
using SkiaSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Controls
{
	public class HexEditor : Control
	{
		public static readonly StyledProperty<IHexEditorDataProvider> DataProviderProperty = AvaloniaProperty.Register<HexEditor, IHexEditorDataProvider>(nameof(DataProvider));
		public static readonly StyledProperty<int> TopRowProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(TopRow), 0, false, Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<int> BytesPerRowProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(BytesPerRow), 16);

		public static readonly StyledProperty<int> SelectionStartProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(SelectionStart), 0);
		public static readonly StyledProperty<int> SelectionLengthProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(SelectionLength), 0);

		public static readonly StyledProperty<Brush> SelectedRowColumnColorProperty = AvaloniaProperty.Register<HexEditor, Brush>(nameof(SelectedRowColumnColor), new SolidColorBrush(0xFFE7E7E7));

		public static readonly StyledProperty<Brush> HeaderBackgroundProperty = AvaloniaProperty.Register<HexEditor, Brush>(nameof(HeaderBackground), new SolidColorBrush(Color.FromRgb(235, 235, 235)));
		public static readonly StyledProperty<Brush> HeaderForegroundProperty = AvaloniaProperty.Register<HexEditor, Brush>(nameof(HeaderForeground), new SolidColorBrush(Colors.Gray));
		public static readonly StyledProperty<Brush> HeaderHighlightProperty = AvaloniaProperty.Register<HexEditor, Brush>(nameof(HeaderHighlight), new SolidColorBrush(Colors.White));

		public IHexEditorDataProvider DataProvider
		{
			get { return GetValue(DataProviderProperty); }
			set { SetValue(DataProviderProperty, value); this.InvalidateVisual(); }
		}

		public int TopRow
		{
			get { return GetValue(TopRowProperty); }
			set { SetValue(TopRowProperty, value); }
		}

		public int BytesPerRow
		{
			get { return GetValue(BytesPerRowProperty); }
			set { SetValue(BytesPerRowProperty, value); }
		}

		public int SelectionStart
		{
			get { return GetValue(SelectionStartProperty); }
			set {
				CommitByteChanges();
				SetValue(SelectionStartProperty, value);
			}
		}

		public int SelectionLength
		{
			get { return GetValue(SelectionLengthProperty); }
			set { SetValue(SelectionLengthProperty, value); }
		}

		public Brush SelectedRowColumnColor
		{
			get { return GetValue(SelectedRowColumnColorProperty); }
			set { SetValue(SelectedRowColumnColorProperty, value); }
		}

		public Brush HeaderBackground
		{
			get { return GetValue(HeaderBackgroundProperty); }
			set { SetValue(HeaderBackgroundProperty, value); }
		}

		public Brush HeaderForeground
		{
			get { return GetValue(HeaderForegroundProperty); }
			set { SetValue(HeaderForegroundProperty, value); }
		}

		public Brush HeaderHighlight
		{
			get { return GetValue(HeaderHighlightProperty); }
			set { SetValue(HeaderHighlightProperty, value); }
		}

		public event EventHandler<ByteUpdatedEventArgs> ByteUpdated;

		private int _dragStartPos = -1;
		private int _newByteValue = -1;
		private bool _lastNibble = false;

		public HexEditor()
		{
			this.Focusable = true;
			this.Cursor = new Cursor(StandardCursorType.Ibeam);
		}

		void MoveSelection(int offset)
		{
			int pos = this.SelectionStart + offset;
			this.SelectionStart = Math.Min(Math.Max(0, pos), this.DataProvider.Length);
			this.SelectionLength = 0;
			_lastNibble = false;

			ScrollIntoView(this.SelectionStart);
		}

		void ChangeSelectionLength(int offset)
		{
			int len = this.SelectionLength + offset;
			_lastNibble = false;

			if(len < 0) {
				this.SelectionStart = Math.Max(0, this.SelectionStart + len);
				this.SelectionLength = -len;
				
				ScrollIntoView(this.SelectionStart);
			} else {
				this.SelectionLength = Math.Min(len, this.DataProvider.Length - this.SelectionStart - 1);

				ScrollIntoView(this.SelectionLength + this.SelectionStart);
			}
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);

			if(e.KeyModifiers.HasFlag(KeyModifiers.Shift)) {
				switch(e.Key) {
					case Key.Left: ChangeSelectionLength(-1); break;
					case Key.Right: ChangeSelectionLength(1); break;
					case Key.Up: ChangeSelectionLength(-BytesPerRow); break;
					case Key.Down: ChangeSelectionLength(BytesPerRow); break;

					case Key.PageUp: ChangeSelectionLength(-BytesPerRow * VisibleRows); break;
					case Key.PageDown: ChangeSelectionLength(BytesPerRow * VisibleRows); break;
					case Key.Home: ChangeSelectionLength(-(SelectionStart % BytesPerRow)); break;
					case Key.End: ChangeSelectionLength(BytesPerRow - (SelectionStart % BytesPerRow) - 1); break;
				}
			} else {
				switch(e.Key) {
					case Key.Left: MoveSelection(-1); break;
					case Key.Right: MoveSelection(1); break;
					case Key.Up: MoveSelection(-BytesPerRow); break;
					case Key.Down: MoveSelection(BytesPerRow); break;
					
					case Key.PageUp: MoveSelection(-BytesPerRow * VisibleRows); break;
					case Key.PageDown: MoveSelection(BytesPerRow * VisibleRows); break;
					case Key.Home: MoveSelection(-(SelectionStart % BytesPerRow)); break;
					case Key.End: MoveSelection(BytesPerRow - (SelectionStart % BytesPerRow) - 1); break;
				}
			}
		}

		protected override void OnTextInput(TextInputEventArgs e)
		{
			if(e.Text != null) {
				char c = e.Text.ToLower()[0];

				if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) {
					int keyValue = Int32.Parse(c.ToString(), System.Globalization.NumberStyles.HexNumber);

					SelectionLength = 0;

					if(_newByteValue < 0) {
						_newByteValue = DataProvider.GetByte(SelectionStart).Value;
					}

					if(_lastNibble) {
						//Commit byte
						_newByteValue &= 0xF0;
						_newByteValue |= keyValue;
						CommitByteChanges();
						SelectionStart++;
					} else {
						_newByteValue &= 0x0F;
						_newByteValue |= (keyValue << 4);
						_lastNibble = true;
					}
				}
			}
			base.OnTextInput(e);
		}

		private void CommitByteChanges()
		{
			if(_newByteValue >= 0) {
				this.ByteUpdated?.Invoke(this, new ByteUpdatedEventArgs() { ByteOffset = SelectionStart, Value = (byte)_newByteValue });
				_lastNibble = false;
				_newByteValue = -1;
			}
		}

		protected override void OnPointerWheelChanged(PointerWheelEventArgs e)
		{
			base.OnPointerWheelChanged(e);
			this.TopRow = Math.Min((DataProvider.Length / BytesPerRow) - 1, Math.Max(0, this.TopRow - (int)(e.Delta.Y * 3)));
		}

		private GridPoint? GetGridPosition(Point p)
		{
			if(p.X >= RowHeaderWidth && p.Y >= ColumnHeaderHeight) {
				double column = (p.X - RowHeaderWidth + LetterSize.Width) / (LetterSize.Width * 3);
				if(column > BytesPerRow || column < 0) {
					return null;
				}

				int row = (int)((p.Y - ColumnHeaderHeight) / RowHeight);
				bool middle = (column - Math.Floor(column)) >= 0.5;

				return new GridPoint { X = (int)column, Y = row, LastNibble = middle };
			}

			return null;
		}

		private void ScrollIntoView(int byteIndex)
		{
			if(byteIndex < 0) {
				TopRow = 0;
			} else if(byteIndex >= DataProvider.Length) {
				TopRow = (DataProvider.Length / BytesPerRow) - VisibleRows;
			} else if(byteIndex < TopRow * BytesPerRow) {
				//scroll up
				TopRow = byteIndex / BytesPerRow;
			} else if(byteIndex > (TopRow + VisibleRows) * BytesPerRow) {
				TopRow = byteIndex / BytesPerRow - VisibleRows;
			}
		}

		protected override void OnPointerPressed(PointerPressedEventArgs e)
		{
			base.OnPointerPressed(e);

			if(!e.GetCurrentPoint(this).Properties.IsLeftButtonPressed) {
				return;
			}

			Point p = e.GetPosition(this);
			GridPoint? gridPos = GetGridPosition(p);

			if(gridPos != null) {
				_lastNibble = gridPos.Value.LastNibble;
				_dragStartPos = (TopRow + gridPos.Value.Y) * BytesPerRow + gridPos.Value.X;

				this.SelectionStart = _dragStartPos;
				this.SelectionLength = 0;
			}
		}

		protected override void OnPointerMoved(PointerEventArgs e)
		{
			base.OnPointerMoved(e);

			if(_dragStartPos >= 0) {
				Point p = e.GetPosition(this);
				GridPoint? gridPos = GetGridPosition(p);
				_lastNibble = false;

				if(gridPos != null) {
					int currentPos = (TopRow + gridPos.Value.Y) * BytesPerRow + gridPos.Value.X;

					if(currentPos < _dragStartPos) {
						this.SelectionStart = currentPos;
						this.SelectionLength = _dragStartPos - currentPos + 1;
					} else {
						this.SelectionStart = _dragStartPos;
						this.SelectionLength = currentPos - _dragStartPos + 1;
					}

					ScrollIntoView(currentPos);
				}
			}
		}

		protected override void OnPointerReleased(PointerReleasedEventArgs e)
		{
			base.OnPointerReleased(e);
			if(e.InitialPressMouseButton == MouseButton.Left) {
				_dragStartPos = -1;
			}
		}

		private Typeface Font { get; set; }
		private Size LetterSize { get; set; }
		private double RowHeight => this.LetterSize.Height;
		
		private int HeaderCharLength => (DataProvider.Length - 1).ToString(HexFormat).Length;
		private double RowHeaderWidth => HeaderCharLength * LetterSize.Width + 5;
		private double ColumnHeaderHeight => LetterSize.Height + 5;
		private int VisibleRows => (int)((Bounds.Height - ColumnHeaderHeight) / RowHeight) - 1;
		private string HexFormat => "X2";

		public override void Render(DrawingContext context)
		{
			base.Render(context);

			IHexEditorDataProvider dataProvider = this.DataProvider;
			if(dataProvider == null) {
				return;
			}

			context.DrawRectangle(Brushes.White, null, this.Bounds);

			//Init font and letter size
			InitFontAndLetterSize();

			//Draw column headers
			DrawColumnHeaders(context);
			using var columnHeaderTranslation = context.PushPostTransform(Matrix.CreateTranslation(0, this.ColumnHeaderHeight));

			//Draw row headers
			DrawRowHeaders(context);
			using var rowHeaderTranslation = context.PushPostTransform(Matrix.CreateTranslation(RowHeaderWidth, 0));

			//Precalculate some values for data draw
			int bytesPerRow = this.BytesPerRow;
			Rect bounds = this.Bounds;
			int position = this.TopRow * this.BytesPerRow;

			Brush selectedRowColumnColor = this.SelectedRowColumnColor;
			Pen cursorPen = new Pen(Brushes.Black, 1);

			int selectionStart = this.SelectionStart;
			int selectionLength = this.SelectionLength;

			int selectedColumn = selectionStart % bytesPerRow;
			int selectedRow = selectionStart / bytesPerRow;

			double rowWidth = LetterSize.Width * (3 * bytesPerRow - 1);
			double letterWidth = LetterSize.Width;

			//Init byte color information for the data we're about to draw
			dataProvider.Prepare(position, position + bytesPerRow * (VisibleRows + 3));

			//Draw selected column background color
			context.DrawRectangle(selectedRowColumnColor, null, new Rect(letterWidth * (3 * selectedColumn), 0, letterWidth * 2, bounds.Height));

			//Draw data
			int visibleRows = VisibleRows + 2;

			if(selectedRow >= TopRow && selectedRow < TopRow + visibleRows) {
				//Draw background color for current row
				context.DrawRectangle(selectedRowColumnColor, null, new Rect(0, (selectedRow - TopRow) * RowHeight, rowWidth, RowHeight));

				//Draw selected character/byte cursor
				context.DrawRectangle(cursorPen, new Rect(letterWidth * selectedColumn * 3 + (_lastNibble ? letterWidth : 0), (selectedRow - TopRow) * RowHeight, 1, RowHeight));
			}

			int bytesToDraw = bytesPerRow * visibleRows;
			List<ByteInfo> dataToDraw = new List<ByteInfo>(bytesToDraw);
			HashSet<Color> fgColors = new HashSet<Color>();

			for(int i = 0; i < bytesToDraw; i++) {
				if(dataProvider.Length <= position) {
					break;
				}

				ByteInfo byteInfo = dataProvider.GetByte(position);
				byteInfo.Selected = selectionLength > 0 && position >= selectionStart && position < selectionStart + selectionLength;

				if(position == SelectionStart && _newByteValue >= 0) {
					//About to draw the selected byte, draw anything that's pending, and then the current byte
					byteInfo.ForeColor = Colors.DarkOrange;
					byteInfo.Value = (byte)_newByteValue;
					dataToDraw.Add(byteInfo);
				} else {
					dataToDraw.Add(byteInfo);
				}

				fgColors.Add(byteInfo.ForeColor);

				position++;
			}

			//DrawHexView(context, dataToDraw, rowWidth);
			context.Custom(new CustomDrawOp(bytesPerRow, HexFormat, RowHeight, bounds, dataToDraw, fgColors, LetterSize));
		}

		class CustomDrawOp : ICustomDrawOperation
		{
			List<ByteInfo> _dataToDraw;
			HashSet<Color> _fgColors;
			Size _letterSize;
			int _bytesPerRow;
			double _rowHeight;
			string _hexFormat;
			Dictionary<Color, SKPaint> _skPaints = new Dictionary<Color, SKPaint>();
			List<float> _startPositionByByte;
			List<float> _endPositionByByte;

			public CustomDrawOp(int bytesPerRow, string hexFormat, double rowHeight, Rect bounds, List<ByteInfo> dataToDraw, HashSet<Color> fgColors, Size letterSize)
			{
				Bounds = bounds;
				_bytesPerRow = bytesPerRow;
				_hexFormat = hexFormat;
				_rowHeight = rowHeight;
				_dataToDraw = dataToDraw;
				_fgColors = fgColors;
				_letterSize = letterSize;

				foreach(ByteInfo byteInfo in dataToDraw) {
					if(!_skPaints.ContainsKey(byteInfo.BackColor)) {
						_skPaints[byteInfo.BackColor] = new SKPaint() { Color = new SKColor(byteInfo.BackColor.ToUint32()) };
					}
				}
			}

			public Rect Bounds { get; private set; }

			public void Dispose()
			{
			}

			public bool Equals(ICustomDrawOperation? other) => false;
			public bool HitTest(Point p) => false;

			public void Render(IDrawingContextImpl context)
			{
				var canvas = (context as ISkiaDrawingContextImpl)?.SkCanvas;
				if(canvas == null) {
					//context.DrawText(Brushes.Black, new Point(), _noSkia.PlatformImpl);
				} else {
					canvas.Save();

					DrawBackground(canvas);

					canvas.Translate(0, 13);
					foreach(Color color in _fgColors) {
						DrawHexView(canvas, color);
					}

					canvas.Translate((float)(_letterSize.Width * _bytesPerRow * 3 + 20), 0);
					PrepareStringView();
					foreach(Color color in _fgColors) {
						DrawStringView(canvas, color);
					}

					canvas.Restore();
				}
			}

			private void DrawHexView(SKCanvas canvas, Color color)
			{
				SKPaint paint = new SKPaint();
				paint.Color = new SKColor(color.ToUint32());

				SKTypeface typeface = SKTypeface.FromFamilyName("Consolas");
				SKFont font = new SKFont(typeface, 14);

				using var builder = new SKTextBlobBuilder();

				int pos = 0;

				StringBuilder sb = new StringBuilder();
				int row = 0;
				while(pos < _dataToDraw.Count) {
					for(int i = 0; i < _bytesPerRow; i++) {
						if(pos + i >= _dataToDraw.Count) {
							break;
						}
						ByteInfo byteInfo = _dataToDraw[pos + i];
						if(byteInfo.ForeColor == color) {
							sb.Append(byteInfo.Value.ToString(_hexFormat));
						} else {
							sb.Append("  ");
						}
						sb.Append(' ');
					}
					pos += _bytesPerRow;

					string rowText = sb.ToString();
					int count = font.CountGlyphs(rowText);
					var buffer = builder.AllocateRun(font, count, 0, (float)(row*_rowHeight));
					font.GetGlyphs(rowText, buffer.GetGlyphSpan());
					row++;
					sb.Clear();
				}

				canvas.DrawText(builder.Build(), 0, 0, paint);
			}

			private void PrepareStringView()
			{
				SKFont altFont = new SKFont(SKFontManager.Default.MatchCharacter('あ'), 14);

				int pos = 0;

				using var measureText = new SKTextBlobBuilder();
				var measureBuffer = measureText.AllocateRun(altFont, 1, 0, 0);

				_startPositionByByte = new List<float>(_dataToDraw.Count);
				_endPositionByByte = new List<float>(_dataToDraw.Count);
				while(pos < _dataToDraw.Count) {
					double xPos = 0;
					for(int i = 0; i < _bytesPerRow; i++) {
						if(pos + i >= _dataToDraw.Count) {
							break;
						}

						ByteInfo byteInfo = _dataToDraw[pos + i];
						string str = ConvertByteToString(byteInfo.Value);
						int codepoint = Char.ConvertToUtf32(str, 0);

						if(codepoint > 0x024F) {
							altFont.GetGlyphs(str, measureBuffer.GetGlyphSpan());
							_startPositionByByte.Add((float)xPos);
							xPos += altFont.MeasureText(measureBuffer.GetGlyphSpan());
							_endPositionByByte.Add((float)xPos);
						} else {
							_startPositionByByte.Add((float)xPos);
							xPos += _letterSize.Width * str.Length;
							_endPositionByByte.Add((float)xPos);
						}
					}
					pos += _bytesPerRow;
				}
			}

			private void DrawStringView(SKCanvas canvas, Color color)
			{
				SKPaint paint = new SKPaint();
				paint.Color = new SKColor(color.ToUint32());

				SKTypeface typeface = SKTypeface.FromFamilyName("Consolas");
				SKFont monoFont = new SKFont(typeface, 14);
				SKFont altFont = new SKFont(SKFontManager.Default.MatchCharacter('あ'), 14);
				
				using var builder = new SKTextBlobBuilder();

				int pos = 0;
				int row = 0;
				
				SKPaint selectedPaint = new SKPaint() { Color = new SKColor(30, 144, 255, 200) };

				SKRect GetRect(int i) => new SKRect(
					(float)_startPositionByByte[i],
					(float)(row * _rowHeight) - 13,
					(float)_endPositionByByte[i],
					(float)((row + 1) * _rowHeight) - 13
				);

				while(pos < _dataToDraw.Count) {
					for(int i = 0; i < _bytesPerRow; i++) {
						if(pos + i >= _dataToDraw.Count) {
							break;
						}

						ByteInfo byteInfo = _dataToDraw[pos + i];
						string str = ConvertByteToString(byteInfo.Value);

						if(byteInfo.ForeColor == color) {
							int codepoint = Char.ConvertToUtf32(str, 0);
							SKFont currentFont = (codepoint > 0x024F) ? altFont : monoFont;

							if(byteInfo.BackColor != Colors.Transparent) {
								canvas.DrawRect(GetRect(pos+i), _skPaints[byteInfo.BackColor]);
							}
							if(byteInfo.Selected) {
								canvas.DrawRect(GetRect(pos+i), selectedPaint);
							}

							int count = currentFont.CountGlyphs(str);
							var buffer = builder.AllocateRun(currentFont, count, _startPositionByByte[pos+i], (float)(row * _rowHeight));
							currentFont.GetGlyphs(str, buffer.GetGlyphSpan());
						}
					}

					pos += _bytesPerRow;
					row++;
				}

				canvas.DrawText(builder.Build(), 0, 0, paint);
			}

			private void DrawBackground(SKCanvas canvas)
			{
				int pos = 0;
				int row = 0;

				SKPaint selectedPaint = new SKPaint() { Color = new SKColor(30, 144, 255, 200) };

				SKRect GetRect(int start, int end) => new SKRect(
					(float)(start * 3 * _letterSize.Width),
					(float)(row * _rowHeight),
					(float)((end * 3 - 1) * _letterSize.Width),
					(float)((row + 1) * _rowHeight)
				);

				while(pos < _dataToDraw.Count) {
					int bgStartPos = -1;
					int selectedStartPos = -1;
					Color bgColor = Colors.Transparent;
					bool selected = false;
					for(int i = 0; i < _bytesPerRow; i++) {
						if(pos + i >= _dataToDraw.Count) {
							break;
						}

						ByteInfo byteInfo = _dataToDraw[pos + i];

						if(byteInfo.BackColor != bgColor) {
							if(bgColor != Colors.Transparent && bgStartPos >= 0) {
								canvas.DrawRect(GetRect(bgStartPos, i), _skPaints[bgColor]);
								bgStartPos = -1;
							}
							if(byteInfo.BackColor != Colors.Transparent) {
								bgStartPos = i;
							}
							bgColor = byteInfo.BackColor;
						}

						if(selected != byteInfo.Selected) {
							if(selectedStartPos >= 0 && selected) {
								canvas.DrawRect(GetRect(selectedStartPos, i), selectedPaint);
								selectedStartPos = -1;
							}
							if(byteInfo.Selected) {
								selectedStartPos = i;
							}
							selected = byteInfo.Selected;
						}
					}
					pos += _bytesPerRow;

					if(bgStartPos >= 0) {
						canvas.DrawRect(GetRect(bgStartPos, _bytesPerRow), _skPaints[bgColor]);
					}
					if(selectedStartPos >= 0) {
						canvas.DrawRect(GetRect(selectedStartPos, _bytesPerRow), selectedPaint);
					}
					row++;
				}
			}

			private string ConvertByteToString(byte b)
			{
				if(b < 32 || b >= 127) {
					return ".";
					/*if(b >= 250) {
						return "[long]";
					}
					if(b % 3 == 0) {
						return "あ";
					}
					if(b % 2 == 0) {
						return "え";
					}
					return "い";*/
				}
				return ((char)b).ToString();
			}
		}

		private void InitFontAndLetterSize()
		{
			this.Font = new Typeface(new FontFamily("Consolas"));
			var text = new FormattedText("A", this.Font, 14, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			this.LetterSize = text.Bounds.Size;
		}

		private void DrawRowHeaders(DrawingContext context)
		{
			Rect bounds = Bounds;
			int dataLength = DataProvider.Length;
			int bytesPerRow = BytesPerRow;

			int headerCharLength = HeaderCharLength;
			double rowHeaderWidth = RowHeaderWidth;
			double textWidth = headerCharLength * LetterSize.Width;
			double xOffset = (rowHeaderWidth - textWidth) / 2;

			//Draw background
			context.DrawRectangle(HeaderBackground, null, new Rect(0, 0, rowHeaderWidth, bounds.Height));

			int headerByte = TopRow * bytesPerRow;
			double y = 0;

			//Draw row headers for each row
			var text = new FormattedText("", this.Font, 14, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			while(y < bounds.Height && headerByte < dataLength) {
				text.Text = headerByte.ToString("X" + headerCharLength);
				context.DrawText(HeaderForeground, new Point(xOffset, y), text);
				y += RowHeight;
				headerByte += bytesPerRow;
			}
		}

		private void DrawColumnHeaders(DrawingContext context)
		{
			context.DrawRectangle(HeaderBackground, null, new Rect(0, 0, Bounds.Width, this.ColumnHeaderHeight));

			StringBuilder sb = new StringBuilder();
			for(int i = 0, len = BytesPerRow; i < len ; i++) {
				sb.Append(i.ToString(HexFormat) + " ");
			}

			var text = new FormattedText(sb.ToString(), this.Font, 14, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			context.DrawText(HeaderForeground, new Point(RowHeaderWidth, (this.ColumnHeaderHeight - this.LetterSize.Height) / 2), text);
		}
	}

	public interface IHexEditorDataProvider
	{
		void Prepare(int firstByteIndex, int lastByteIndex);
		ByteInfo GetByte(int byteIndex);
		int Length { get; }
	}

	public struct GridPoint
	{
		public int X;
		public int Y;
		public bool LastNibble;
	}

	public class ByteUpdatedEventArgs : EventArgs
	{
		public int ByteOffset;
		public byte Value;
	}

	public struct ByteInfo
	{
		public Color ForeColor { get; set; }
		public Color BackColor { get; set; }
		public Color BorderColor { get; set; }
		public bool Selected { get; set; }
		public byte Value { get; set; }

		public override bool Equals(object? obj)
		{
			if(obj == null || !(obj is ByteInfo)) {
				return false;
			}

			ByteInfo a = (ByteInfo)obj;
			return this.BackColor == a.BackColor && this.BorderColor == a.BorderColor && this.ForeColor == a.ForeColor && this.Selected == a.Selected;
		}

		public override int GetHashCode()
		{
			return (int)(this.BackColor.ToUint32() ^ this.BorderColor.ToUint32() ^ this.ForeColor.ToUint32() ^ (this.Selected ? 1 : 0));
		}
	}
}
