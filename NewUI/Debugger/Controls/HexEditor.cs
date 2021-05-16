using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Media.TextFormatting;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Controls
{
	public partial class HexEditor : Control
	{
		//TODO Customizable font/styles
		//TODO Fix keyboard selection
		//TODO Support TBL files
		//TODO Copy+paste
		public static readonly StyledProperty<IHexEditorDataProvider> DataProviderProperty = AvaloniaProperty.Register<HexEditor, IHexEditorDataProvider>(nameof(DataProvider));
		public static readonly StyledProperty<int> TopRowProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(TopRow), 0, false, Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<int> BytesPerRowProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(BytesPerRow), 16);

		public static readonly StyledProperty<int> SelectionStartProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(SelectionStart), 0);
		public static readonly StyledProperty<int> SelectionLengthProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(SelectionLength), 0);

		public static readonly StyledProperty<SolidColorBrush> SelectedRowColumnColorProperty = AvaloniaProperty.Register<HexEditor, SolidColorBrush>(nameof(SelectedRowColumnColor), new SolidColorBrush(0xFFE7E7E7));

		public static readonly StyledProperty<SolidColorBrush> HeaderBackgroundProperty = AvaloniaProperty.Register<HexEditor, SolidColorBrush>(nameof(HeaderBackground), new SolidColorBrush(Color.FromRgb(235, 235, 235)));
		public static readonly StyledProperty<SolidColorBrush> HeaderForegroundProperty = AvaloniaProperty.Register<HexEditor, SolidColorBrush>(nameof(HeaderForeground), new SolidColorBrush(Colors.Gray));
		public static readonly StyledProperty<SolidColorBrush> HeaderHighlightProperty = AvaloniaProperty.Register<HexEditor, SolidColorBrush>(nameof(HeaderHighlight), new SolidColorBrush(Colors.White));

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

		public SolidColorBrush SelectedRowColumnColor
		{
			get { return GetValue(SelectedRowColumnColorProperty); }
			set { SetValue(SelectedRowColumnColorProperty, value); }
		}

		public SolidColorBrush HeaderBackground
		{
			get { return GetValue(HeaderBackgroundProperty); }
			set { SetValue(HeaderBackgroundProperty, value); }
		}

		public SolidColorBrush HeaderForeground
		{
			get { return GetValue(HeaderForegroundProperty); }
			set { SetValue(HeaderForegroundProperty, value); }
		}

		public SolidColorBrush HeaderHighlight
		{
			get { return GetValue(HeaderHighlightProperty); }
			set { SetValue(HeaderHighlightProperty, value); }
		}

		public event EventHandler<ByteUpdatedEventArgs>? ByteUpdated;

		private Typeface Font { get; set; }
		private Size LetterSize { get; set; }
		private double RowHeight => this.LetterSize.Height;

		private int HeaderCharLength => (DataProvider.Length - 1).ToString(HexFormat).Length;
		private double RowHeaderWidth => HeaderCharLength * LetterSize.Width + 5;
		private double RowWidth => LetterSize.Width * (3 * BytesPerRow - 1);
		private double StringViewMargin => 20;
		private double ColumnHeaderHeight => LetterSize.Height + 5;
		private int VisibleRows => (int)((Bounds.Height - ColumnHeaderHeight) / RowHeight) - 1;
		private string HexFormat => "X2";

		private int _dragStartPos = -1;
		private int _newByteValue = -1;
		private bool _lastNibble = false;
		private bool _inStringView = false;
		private List<float> _startPositionByByte = new List<float>();
		private List<float> _endPositionByByte = new List<float>();

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

				if(_inStringView) {
					SelectionLength = 0;
					_newByteValue = ConvertCharToByte(c);
					CommitByteChanges();
					SelectionStart++;
				} else {
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

		private GridPoint? GetGridPosition(Point p)
		{
			if(p.X >= RowHeaderWidth && p.Y >= ColumnHeaderHeight) {
				int row = (int)((p.Y - ColumnHeaderHeight) / RowHeight);

				if(p.X >= RowHeaderWidth + RowWidth + StringViewMargin) {
					//String view
					try {
						int rowStart = row * BytesPerRow;
						List<float> startPos = _startPositionByByte;
						float endPos = _endPositionByByte[rowStart + BytesPerRow - 1];

						double x = p.X - RowHeaderWidth - RowWidth - StringViewMargin;

						if(x > endPos) {
							return null;
						}

						int column = 0;
						for(int i = BytesPerRow - 1; i >= 0; i--) {
							if(startPos[rowStart + i] < x) {
								column = i;
								break;
							}
						}

						return new GridPoint { X = column, Y = row, LastNibble = false, InStringView = true };
					} catch {
						return null;
					}
				} else {
					double column = (p.X - RowHeaderWidth + LetterSize.Width) / (LetterSize.Width * 3);
					if(column > BytesPerRow || column < 0) {
						return null;
					}

					bool middle = (column - Math.Floor(column)) >= 0.5;

					return new GridPoint { X = (int)column, Y = row, LastNibble = middle, InStringView = false };
				}
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

		protected override void OnPointerWheelChanged(PointerWheelEventArgs e)
		{
			base.OnPointerWheelChanged(e);
			this.TopRow = Math.Min((DataProvider.Length / BytesPerRow) - 1, Math.Max(0, this.TopRow - (int)(e.Delta.Y * 3)));
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
				_inStringView = gridPos.Value.InStringView;
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
					if(gridPos.Value.InStringView != _inStringView) {
						return;
					}

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

		public override void Render(DrawingContext context)
		{
			base.Render(context);

			IHexEditorDataProvider dataProvider = this.DataProvider;
			if(dataProvider == null) {
				return;
			}

			context.DrawRectangle(ColorHelper.GetBrush(Colors.White), null, this.Bounds);

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

			Brush selectedRowColumnColor = ColorHelper.GetBrush(SelectedRowColumnColor);
			Pen cursorPen = ColorHelper.GetPen(Colors.Black);

			int selectionStart = this.SelectionStart;
			int selectionLength = this.SelectionLength;

			int selectedColumn = selectionStart % bytesPerRow;
			int selectedRow = selectionStart / bytesPerRow;

			double rowWidth = RowWidth;
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
				double xPos;
				if(_inStringView) {
					xPos = RowWidth + StringViewMargin + _startPositionByByte[(selectedRow - TopRow) * bytesPerRow + selectedColumn];
				} else {
					xPos = letterWidth * selectedColumn * 3 + (_lastNibble ? letterWidth : 0);
				}

				context.DrawRectangle(cursorPen, new Rect(xPos, (selectedRow - TopRow) * RowHeight, 1, RowHeight));
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
			context.Custom(new HexViewDrawOperation(this, dataToDraw, fgColors));
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

		private byte ConvertCharToByte(char c)
		{
			if(c > 32 || c < 127) {
				return (byte)c;
			}
			return (byte)'.';
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
			context.DrawRectangle(ColorHelper.GetBrush(HeaderBackground), null, new Rect(0, 0, rowHeaderWidth, bounds.Height));

			int headerByte = TopRow * bytesPerRow;
			double y = 0;

			//Draw row headers for each row
			var text = new FormattedText("", this.Font, 14, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			while(y < bounds.Height && headerByte < dataLength) {
				text.Text = headerByte.ToString("X" + headerCharLength);
				context.DrawText(ColorHelper.GetBrush(HeaderForeground), new Point(xOffset, y), text);
				y += RowHeight;
				headerByte += bytesPerRow;
			}
		}

		private void DrawColumnHeaders(DrawingContext context)
		{
			context.DrawRectangle(ColorHelper.GetBrush(HeaderBackground), null, new Rect(0, 0, Bounds.Width, this.ColumnHeaderHeight));

			StringBuilder sb = new StringBuilder();
			for(int i = 0, len = BytesPerRow; i < len ; i++) {
				sb.Append(i.ToString(HexFormat) + " ");
			}

			var text = new FormattedText(sb.ToString(), this.Font, 14, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			context.DrawText(ColorHelper.GetBrush(HeaderForeground), new Point(RowHeaderWidth, (this.ColumnHeaderHeight - this.LetterSize.Height) / 2), text);
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
		public bool InStringView;
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
