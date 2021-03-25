using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Media.TextFormatting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Controls
{
	public class HexEditor : Control
	{
		public static readonly StyledProperty<byte[]> DataProperty = AvaloniaProperty.Register<HexEditor, byte[]>(nameof(Data));
		public static readonly StyledProperty<int> TopRowProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(TopRow), 0);
		public static readonly StyledProperty<int> BytesPerRowProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(BytesPerRow), 16);
		public static readonly StyledProperty<IByteColorProvider> ByteColorProviderProperty = AvaloniaProperty.Register<HexEditor, IByteColorProvider>(nameof(ByteColorProvider));

		public static readonly StyledProperty<int> SelectionStartProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(SelectionStart), 0);
		public static readonly StyledProperty<int> SelectionLengthProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(SelectionLength), 0);

		public static readonly StyledProperty<Brush> SelectedRowColumnColorProperty = AvaloniaProperty.Register<HexEditor, Brush>(nameof(SelectedRowColumnColor), new SolidColorBrush(0xFFE7E7E7));

		public static readonly StyledProperty<Brush> HeaderBackgroundProperty = AvaloniaProperty.Register<HexEditor, Brush>(nameof(HeaderBackground), new SolidColorBrush(Color.FromRgb(235, 235, 235)));
		public static readonly StyledProperty<Brush> HeaderForegroundProperty = AvaloniaProperty.Register<HexEditor, Brush>(nameof(HeaderForeground), new SolidColorBrush(Colors.Gray));
		public static readonly StyledProperty<Brush> HeaderHighlightProperty = AvaloniaProperty.Register<HexEditor, Brush>(nameof(HeaderHighlight), new SolidColorBrush(Colors.White));

		public byte[] Data
		{
			get { return GetValue(DataProperty); }
			set { SetValue(DataProperty, value); this.InvalidateVisual(); }
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

		public IByteColorProvider ByteColorProvider
		{
			get { return GetValue(ByteColorProviderProperty); }
			set { SetValue(ByteColorProviderProperty, value); }
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
			this.SelectionStart = Math.Min(Math.Max(0, pos), this.Data.Length);
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
				this.SelectionLength = Math.Min(len, this.Data.Length - this.SelectionStart - 1);

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
						_newByteValue = Data[SelectionStart];
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
			this.TopRow = Math.Min(this.Data.Length - 1, Math.Max(0, this.TopRow - (int)(e.Delta.Y * 3)));
		}

		private GridPoint? GetGridPosition(Point p)
		{
			if(p.X >= RowHeaderWidth && p.Y >= ColumnHeaderHeight) {
				double column = (p.X - RowHeaderWidth + LetterSize.Width) / (LetterSize.Width * 3);
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
			} else if(byteIndex >= Data.Length) {
				TopRow = (Data.Length / BytesPerRow) - VisibleRows;
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
		
		private int HeaderCharLength => (Data.Length - 1).ToString(HexFormat).Length;
		private double RowHeaderWidth => HeaderCharLength * LetterSize.Width + 5;
		private double ColumnHeaderHeight => LetterSize.Height + 5;
		private int VisibleRows => (int)((Bounds.Height - ColumnHeaderHeight) / RowHeight) - 1;
		private string HexFormat => "X2";

		public override void Render(DrawingContext context)
		{
			base.Render(context);

			byte[] data = this.Data;
			if(data == null) {
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
			IByteColorProvider? colorProvider = this.ByteColorProvider;
			colorProvider?.Prepare(position, position + bytesPerRow * 100);

			//Draw selected column background color
			context.DrawRectangle(selectedRowColumnColor, null, new Rect(letterWidth * (3 * selectedColumn), 0, letterWidth * 2, bounds.Height));

			//Draw data
			StringBuilder sbHexView = new StringBuilder();
			StringBuilder sbStringView = new StringBuilder();
			var text = new FormattedText("A", this.Font, 14, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			double y = 0;
			double x = 0;
			double stringViewX = 0;
			List<byte> dataToDraw = new List<byte>();
			ByteColors newColors = new ByteColors();
			ByteColors colors = new ByteColors();

			void DrawText(bool endOfLine)
			{
				if(dataToDraw.Count == 0) {
					return;
				}

				foreach(byte b in dataToDraw) {
					sbHexView.Append(b.ToString(HexFormat));
					sbHexView.Append(' ');

					sbStringView.Append(ConvertByteToChar(b));
				}

				text.Text = sbHexView.ToString().Trim();
				sbHexView.Clear();
				dataToDraw.Clear();

				//Draw hexadecimal view
				if(colors.BackColor.Equals(Colors.Transparent) == false) {
					bool extend = !endOfLine && colors.BackColor == newColors.BackColor;
					Rect bgBounds = new Rect(x, y, text.Bounds.Width + (extend ? LetterSize.Width : 0), text.Bounds.Height);

					SolidColorBrush bgBrush = new SolidColorBrush(colors.BackColor);
					context.FillRectangle(bgBrush, bgBounds);
				}

				if(colors.Selected) {
					bool extend = !endOfLine && colors.Selected == newColors.Selected;
					Rect bgBounds = new Rect(x, y, text.Bounds.Width + (extend ? LetterSize.Width : 0), text.Bounds.Height);

					SolidColorBrush bgBrush = new SolidColorBrush(Colors.DodgerBlue, 0.75);
					context.FillRectangle(bgBrush, bgBounds);
				}

				SolidColorBrush fontBrush = new SolidColorBrush(colors.ForeColor);
				context.DrawText(fontBrush, new Point(x, y), text);
				x += text.Bounds.Width + LetterSize.Width;

				//Draw string view
				TextLayout textLayout = new TextLayout(sbStringView.ToString(), Font, 14, fontBrush);
				sbStringView.Clear();

				using var columnHeaderTranslation = context.PushPostTransform(Matrix.CreateTranslation(stringViewX + rowWidth + 30, y));

				double width = textLayout.Size.Width;
				if(colors.BackColor.Equals(Colors.Transparent) == false) {
					SolidColorBrush bgBrush = new SolidColorBrush(colors.BackColor);
					context.FillRectangle(bgBrush, new Rect(0, 0, width, LetterSize.Height));
				}

				if(colors.Selected) {
					SolidColorBrush bgBrush = new SolidColorBrush(Colors.DodgerBlue, 0.75);
					context.FillRectangle(bgBrush, new Rect(0, 0, width, LetterSize.Height));
				}

				textLayout.Draw(context);
				stringViewX += textLayout.Size.Width;
			}

			while(y < bounds.Height) {
				if(position / bytesPerRow == selectedRow) {
					//Draw background color for current row
					context.DrawRectangle(selectedRowColumnColor, null, new Rect(0, y, rowWidth, RowHeight));

					//Draw selected character/byte cursor
					context.DrawRectangle(cursorPen, new Rect(letterWidth * selectedColumn * 3 + (_lastNibble ? letterWidth : 0), y, 1, RowHeight));
				}

				for(int i = 0; i < bytesPerRow; i++) {
					if(data.Length <= position) {
						break;
					}

					newColors = colorProvider?.GetByteColor(position) ?? new ByteColors();
					newColors.Selected = selectionLength > 0 && position >= selectionStart && position < selectionStart + selectionLength;

					if(position == SelectionStart && _newByteValue >= 0) {
						//About to draw the selected byte, draw anything that's pending, and then the current byte
						DrawText(false);
						dataToDraw.Add((byte)_newByteValue);
						colors.ForeColor = Colors.DarkOrange;
						DrawText(false);
					} else {
						if(!newColors.Equals(colors) && i > 0) {
							//Colors are changing, draw built up text
							DrawText(false);
						}
						dataToDraw.Add(data[position]);
					}
					colors = newColors;
					position++;
				}

				DrawText(true);

				x = 0;
				stringViewX = 0;

				y += RowHeight;
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
			byte[] data = Data;
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
			while(y < bounds.Height && headerByte < data.Length) {
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

		private char ConvertByteToChar(byte b)
		{
			if(b < 32 || b >= 128) {
				return 'あ';
			}
			return (char)b;
		}
	}

	public interface IByteColorProvider
	{
		void Prepare(long firstByteIndex, long lastByteIndex);
		ByteColors GetByteColor(long byteIndex);
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

	public struct ByteColors
	{
		public Color ForeColor { get; set; }
		public Color BackColor { get; set; }
		public Color BorderColor { get; set; }
		public bool Selected { get; set; }

		public override bool Equals(object? obj)
		{
			if(obj == null || !(obj is ByteColors)) {
				return false;
			}

			ByteColors a = (ByteColors)obj;
			return this.BackColor == a.BackColor && this.BorderColor == a.BorderColor && this.ForeColor == a.ForeColor && this.Selected == a.Selected;
		}

		public override int GetHashCode()
		{
			return base.GetHashCode();
		}
	}
}
