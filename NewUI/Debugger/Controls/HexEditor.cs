using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Media;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Text;
using Mesen.Config;
using System.Text.RegularExpressions;
using Mesen.Debugger.Utilities;
using System.Globalization;

namespace Mesen.Debugger.Controls
{
	public partial class HexEditor : Control
	{
		public static readonly StyledProperty<IHexEditorDataProvider> DataProviderProperty = AvaloniaProperty.Register<HexEditor, IHexEditorDataProvider>(nameof(DataProvider));
		public static readonly StyledProperty<int> TopRowProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(TopRow), 0, false, Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<int> BytesPerRowProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(BytesPerRow), 16);

		public static readonly StyledProperty<int> SelectionStartProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(SelectionStart), 0, defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<int> SelectionLengthProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(SelectionLength), 0, defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);

		public static readonly StyledProperty<string> FontFamilyProperty = AvaloniaProperty.Register<HexEditor, string>(nameof(FontFamily), DebuggerConfig.MonospaceFontFamily);
		public static readonly StyledProperty<float> FontSizeProperty = AvaloniaProperty.Register<HexEditor, float>(nameof(FontSize), DebuggerConfig.DefaultFontSize);

		public static readonly StyledProperty<bool> ShowStringViewProperty = AvaloniaProperty.Register<HexEditor, bool>(nameof(ShowStringView), false);
		public static readonly StyledProperty<bool> HighDensityModeProperty = AvaloniaProperty.Register<HexEditor, bool>(nameof(HighDensityMode), false);

		public static readonly StyledProperty<SolidColorBrush> SelectedRowColumnColorProperty = AvaloniaProperty.Register<HexEditor, SolidColorBrush>(nameof(SelectedRowColumnColor), new SolidColorBrush(0xFFF0F0F0));

		public static readonly StyledProperty<SolidColorBrush> HeaderBackgroundProperty = AvaloniaProperty.Register<HexEditor, SolidColorBrush>(nameof(HeaderBackground), new SolidColorBrush(Color.FromRgb(235, 235, 235)));
		public static readonly StyledProperty<SolidColorBrush> HeaderForegroundProperty = AvaloniaProperty.Register<HexEditor, SolidColorBrush>(nameof(HeaderForeground), new SolidColorBrush(Colors.Gray));
		public static readonly StyledProperty<SolidColorBrush> HeaderHighlightProperty = AvaloniaProperty.Register<HexEditor, SolidColorBrush>(nameof(HeaderHighlight), new SolidColorBrush(Colors.White));
		
		public static readonly StyledProperty<int> NewByteValueProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(NewByteValue), -1);

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

		public bool ShowStringView
		{
			get { return GetValue(ShowStringViewProperty); }
			set { SetValue(ShowStringViewProperty, value); }
		}

		public bool HighDensityMode
		{
			get { return GetValue(HighDensityModeProperty); }
			set { SetValue(HighDensityModeProperty, value); }
		}

		public string FontFamily
		{
			get { return GetValue(FontFamilyProperty); }
			set { SetValue(FontFamilyProperty, value); }
		}

		public float FontSize
		{
			get { return GetValue(FontSizeProperty); }
			set { SetValue(FontSizeProperty, value); }
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
		
		public int NewByteValue
		{
			get { return GetValue(NewByteValueProperty); }
			set { SetValue(NewByteValueProperty, value); }
		}

		public event EventHandler<ByteUpdatedEventArgs>? ByteUpdated;

		private Typeface Font { get; set; }
		private Size LetterSize { get; set; }
		private double RowHeight => HighDensityMode ? LetterSize.Height * 0.8 : LetterSize.Height;

		private int HeaderCharLength => DataProvider.Length > 0 ? (DataProvider.Length - 1).ToString(HexFormat).Length : 0;
		private double RowHeaderWidth => HeaderCharLength * LetterSize.Width + 5;
		private double RowWidth => LetterSize.Width * (3 * BytesPerRow - 1);
		private double StringViewMargin => 20;
		private double ColumnHeaderHeight => LetterSize.Height + 5;
		private int VisibleRows => Math.Max(0, (int)((Bounds.Height - ColumnHeaderHeight) / RowHeight) - 1);
		private string HexFormat => "X2";

		private int _cursorPosition = 0;
		private int _lastClickedPosition = -1;
		private bool _lastNibble = false;
		private bool _inStringView = false;
		private float[] _startPositionByByte = Array.Empty<float>();
		private float[] _endPositionByByte = Array.Empty<float>();

		static HexEditor()
		{
			AffectsRender<HexEditor>(
				DataProviderProperty, TopRowProperty, BytesPerRowProperty, SelectionStartProperty, SelectionLengthProperty,
				SelectedRowColumnColorProperty, HeaderBackgroundProperty, HeaderForegroundProperty, HeaderHighlightProperty,
				IsFocusedProperty, HighDensityModeProperty, NewByteValueProperty
			);

			FontFamilyProperty.Changed.AddClassHandler<HexEditor>((x, e) => {
				x.InitFontAndLetterSize();
			});

			FontSizeProperty.Changed.AddClassHandler<HexEditor>((x, e) => {
				x.InitFontAndLetterSize();
			});
		}

		public HexEditor()
		{
			Focusable = true;
			InitFontAndLetterSize();
		}

		protected override void OnPointerEntered(PointerEventArgs e)
		{
			base.OnPointerEntered(e);
			Cursor = new Cursor(StandardCursorType.Ibeam);
		}

		protected override void OnPointerExited(PointerEventArgs e)
		{
			base.OnPointerExited(e);
			Cursor = null;
		}

		private void MoveCursor(int offset, bool nibbleMode = false, bool keepNibble = false)
		{
			if(nibbleMode) {
				if(_lastNibble) {
					if(offset > 0) {
						MoveCursor(1);
					} else {
						_lastNibble = false;
					}
				} else {
					if(offset > 0) {
						_lastNibble = true;
					} else {
						MoveCursor(-1);
						_lastNibble = true;
					}
				}
			} else {
				int pos = this.SelectionStart + offset;
				SetCursorPosition(pos, keepNibble);
			}
		}

		public void SetCursorPosition(int pos, bool keepNibble = false, bool scrollToTop = false)
		{
			if(this.DataProvider == null) {
				return;
			}

			this.SelectionStart = Math.Max(0, Math.Min(pos, this.DataProvider.Length - 1));
			this.SelectionLength = 0;
			_cursorPosition = this.SelectionStart;
			_lastClickedPosition = _cursorPosition;
			if(!keepNibble) {
				_lastNibble = false;
			}

			ScrollIntoView(_cursorPosition, scrollToTop);
		}

		private void ChangeSelectionLength(int offset)
		{
			_lastNibble = false;

			int start = this.SelectionStart;
			int end = this.SelectionStart + this.SelectionLength;
			int length = this.SelectionLength;
			bool newAnchorEnd = false;

			if(this.SelectionStart == this._cursorPosition) {
				//Anchored at the start of the selection
				start += offset;
				if(end < start) {
					length = start - end;
					start = end;
					newAnchorEnd = true;
				} else {
					length -= offset;
				}
			} else {
				//Anchored at the end of the selection
				length += offset;
				if(length < 0) {
					start = start + length;
					length = -length;
					newAnchorEnd = false;
				} else {
					newAnchorEnd = true;
				}
			}

			if(start < 0) {
				length += start;
				start = 0;
			}

			this.SelectionStart = Math.Max(0, Math.Min(this.DataProvider.Length - 1, start));
			this.SelectionLength = Math.Max(0, Math.Min(length, this.DataProvider.Length - this.SelectionStart));
			if(newAnchorEnd) {
				this._cursorPosition = this.SelectionStart + this.SelectionLength;
			} else {
				this._cursorPosition = this.SelectionStart;
			}
			ScrollIntoView(this._cursorPosition);
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);

			bool ctrl = e.KeyModifiers.HasFlag(KeyModifiers.Control);

			if(e.KeyModifiers.HasFlag(KeyModifiers.Shift)) {
				switch(e.Key) {
					case Key.Left: ChangeSelectionLength(-1); break;
					case Key.Right: ChangeSelectionLength(1); break;
					case Key.Up: ChangeSelectionLength(-BytesPerRow); break;
					case Key.Down: ChangeSelectionLength(BytesPerRow); break;

					case Key.PageUp: ChangeSelectionLength(-BytesPerRow * VisibleRows); break;
					case Key.PageDown: ChangeSelectionLength(BytesPerRow * VisibleRows); break;

					case Key.Home: {
						int anchor = _cursorPosition == SelectionStart ? _cursorPosition : SelectionStart + SelectionLength;
						ChangeSelectionLength(-(anchor % BytesPerRow));
						break;
					}
					
					case Key.End: {
						int anchor = _cursorPosition == SelectionStart ? _cursorPosition : SelectionStart + SelectionLength;
						ChangeSelectionLength(BytesPerRow - (anchor % BytesPerRow));
						break;
					}
				}
			} else {
				switch(e.Key) {
					case Key.Left: MoveCursor(-1, ctrl); break;
					case Key.Right: MoveCursor(1, ctrl); break;
					case Key.Up: MoveCursor(-BytesPerRow, keepNibble: true); break;
					case Key.Down: MoveCursor(BytesPerRow, keepNibble: true); break;
					
					case Key.PageUp: MoveCursor(-BytesPerRow * VisibleRows, keepNibble: true); break;
					case Key.PageDown: MoveCursor(BytesPerRow * VisibleRows, keepNibble: true); break;
					case Key.Home:
						if(ctrl) {
							SetCursorPosition(0);
						} else {
							MoveCursor(-(SelectionStart % BytesPerRow));
						}
						break;
					case Key.End:
						if(ctrl) {
							SetCursorPosition(DataProvider.Length);
						} else {
							MoveCursor(BytesPerRow - (SelectionStart % BytesPerRow) - 1);
						}
						break;
				}
			}
		}

		protected override void OnTextInput(TextInputEventArgs e)
		{
			if(e.Text != null) {
				char c = e.Text[0];

				if(_inStringView) {
					SelectionLength = 0;
					NewByteValue = DataProvider.ConvertCharToByte(c);
					CommitByteChanges();
					MoveCursor(1);
				} else {
					if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
						int keyValue = Int32.Parse(c.ToString(), System.Globalization.NumberStyles.HexNumber);

						SelectionLength = 0;

						if(NewByteValue < 0) {
							NewByteValue = DataProvider.GetByte(SelectionStart).Value;
						}

						if(_lastNibble) {
							//Commit byte
							NewByteValue &= 0xF0;
							NewByteValue |= keyValue;
							CommitByteChanges();
							MoveCursor(1);
						} else {
							NewByteValue &= 0x0F;
							NewByteValue |= (keyValue << 4);
							_lastNibble = true;
						}
					}
				}
			}
			base.OnTextInput(e);
		}

		public void CopySelection()
		{
			IHexEditorDataProvider dp = DataProvider;
			byte[] data = dp.GetRawBytes(SelectionStart, SelectionLength);
			StringBuilder sb = new StringBuilder();
			int bytesPerRow = BytesPerRow;
			for(int i = 0; i < data.Length; i++) {
				if(_inStringView) {
					UInt64 tblKeyValue = (UInt64)data[i];
					for(int j = 1; j < 8; j++) {
						if(i + j < data.Length) {
							tblKeyValue += (UInt64)data[i + j] << (8 * j);
						}
					}
					sb.Append(dp.ConvertValueToString(tblKeyValue, out int keyLength));
					i += keyLength - 1;
				} else {
					sb.Append(data[i].ToString("X2"));
					if((i + 1) % bytesPerRow == 0) {
						sb.AppendLine();
					} else if(i < data.Length - 1) {
						sb.Append(" ");
					}
				}
			}

			Application.Current?.Clipboard?.SetTextAsync(sb.ToString());
		}

		public async void PasteSelection()
		{
			var clipboard = Application.Current?.Clipboard;
			if(clipboard != null) {
				string text = await clipboard.GetTextAsync();
				text = text.Replace("\n", "").Replace("\r", "");
				if(Regex.IsMatch(text, "^[ a-f0-9]+$", RegexOptions.IgnoreCase)) {
					byte[] pastedData = HexUtilities.HexToArray(text);
					for(int i = 0; i < pastedData.Length; i++) {
						RequestByteUpdate(_cursorPosition + i, pastedData[i]);
					}
					InvalidateVisual();
				}
			}
		}

		public void SelectAll()
		{
			SetCursorPosition(0);
			SelectionLength = DataProvider.Length;
		}

		private void CommitByteChanges()
		{
			if(NewByteValue >= 0) {
				RequestByteUpdate(_cursorPosition, (byte)NewByteValue);
				_lastNibble = false;
				NewByteValue = -1;
			}
		}

		private void RequestByteUpdate(int position, byte value)
		{
			if(position < DataProvider.Length) {
				ByteUpdated?.Invoke(this, new ByteUpdatedEventArgs() { ByteOffset = position, Value = value });
			}
		}

		private GridPoint? GetGridPosition(Point p)
		{
			if(p.X >= RowHeaderWidth && p.Y >= ColumnHeaderHeight) {
				int row = (int)((p.Y - ColumnHeaderHeight) / RowHeight);

				if(ShowStringView && p.X >= RowHeaderWidth + RowWidth + StringViewMargin) {
					//String view
					try {
						int rowStart = row * BytesPerRow;
						float[] startPos = _startPositionByByte;
						float[] endPos = _endPositionByByte;

						double x = p.X - RowHeaderWidth - RowWidth - StringViewMargin;

						int column = 0;
						if(rowStart + BytesPerRow - 1 >= _startPositionByByte.Length) {
							return null;
						} else if(x >= _startPositionByByte[rowStart + BytesPerRow - 1]) {
							column = BytesPerRow - 1;
						} else {
							for(int i = 0, len = BytesPerRow - 1; i < len; i++) {
								if(startPos[rowStart + i] <= x && _endPositionByByte[rowStart + i] >= x) {
									column = i;
									break;
								}
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

		private void ScrollIntoView(int byteIndex, bool scrollToTop = false)
		{
			int topRow = TopRow;
			if(byteIndex < 0) {
				topRow = 0;
			} else if(byteIndex >= DataProvider.Length) {
				topRow = (DataProvider.Length / BytesPerRow) - VisibleRows;
			} else if(byteIndex < TopRow * BytesPerRow || scrollToTop) {
				//scroll up
				topRow = byteIndex / BytesPerRow;
			} else if(byteIndex > (TopRow + VisibleRows) * BytesPerRow) {
				topRow = byteIndex / BytesPerRow - VisibleRows;
			}

			TopRow = Math.Max(0, topRow);
		}

		protected override void OnPointerWheelChanged(PointerWheelEventArgs e)
		{
			base.OnPointerWheelChanged(e);
			this.TopRow = Math.Min((DataProvider.Length / BytesPerRow) - 1, Math.Max(0, this.TopRow - (int)(e.Delta.Y * 3)));
		}

		protected override void OnPointerPressed(PointerPressedEventArgs e)
		{
			base.OnPointerPressed(e);

			PointerPointProperties props = e.GetCurrentPoint(this).Properties;

			Point p = e.GetPosition(this);
			GridPoint? gridPos = GetGridPosition(p);

			if(gridPos == null) {
				return;
			}

			if(props.IsLeftButtonPressed) {
				_inStringView = gridPos.Value.InStringView;
				_lastNibble = gridPos.Value.LastNibble;

				if(e.KeyModifiers.HasFlag(KeyModifiers.Shift)) {
					MoveSelectionWithMouse(gridPos.Value);
				} else {
					int pos = GetByteOffset(gridPos.Value);
					SetCursorPosition(pos);
				}
			} else if(props.IsRightButtonPressed) {
				int pos = GetByteOffset(gridPos.Value);
				if(pos >= SelectionStart && pos < SelectionStart + SelectionLength) {
					_inStringView = gridPos.Value.InStringView;
				} else {
					SetCursorPosition(pos);
				}
			}
		}

		private int GetByteOffset(GridPoint gridPos)
		{
			return Math.Max(0, Math.Min((TopRow + gridPos.Y) * BytesPerRow + gridPos.X, DataProvider.Length - 1));
		}

		private void MoveSelectionWithMouse(GridPoint gridPos)
		{
			int currentPos = GetByteOffset(gridPos);

			if(currentPos < _lastClickedPosition) {
				this.SelectionStart = currentPos;
				this.SelectionLength = _lastClickedPosition - currentPos;
			} else {
				this.SelectionStart = _lastClickedPosition;
				this.SelectionLength = currentPos - _lastClickedPosition + 1;
			}

			this._cursorPosition = currentPos;

			ScrollIntoView(currentPos);
		}

		protected override void OnPointerMoved(PointerEventArgs e)
		{
			base.OnPointerMoved(e);

			if(!e.GetCurrentPoint(this).Properties.IsLeftButtonPressed || _lastClickedPosition < 0) {
				return;
			}

			Point p = e.GetPosition(this);
			if(p.Y < ColumnHeaderHeight) {
				//Allow auto-scroll up when mouse goes over header
				this.TopRow = Math.Max(0, this.TopRow - 1);
				p = p.WithY(ColumnHeaderHeight);
			}

			GridPoint? gridPos = GetGridPosition(p);
			_lastNibble = false;

			if(gridPos != null) {
				if(gridPos.Value.InStringView != _inStringView) {
					return;
				}
				
				MoveSelectionWithMouse(gridPos.Value);
			}
		}

		public override void Render(DrawingContext context)
		{
			base.Render(context);

			IHexEditorDataProvider dataProvider = this.DataProvider;
			if(dataProvider == null) {
				return;
			}

			using var clipRect = context.PushClip(new Rect(this.Bounds.Size));

			context.DrawRectangle(ColorHelper.GetBrush(Colors.White), null, new Rect(Bounds.Size));

			//Draw column headers
			DrawColumnHeaders(context);
			using var columnHeaderTranslation = context.PushPostTransform(Matrix.CreateTranslation(0, ColumnHeaderHeight));

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

			int selectedColumn = this._cursorPosition % bytesPerRow;
			int selectedRow = this._cursorPosition / bytesPerRow;

			double rowWidth = RowWidth;
			double letterWidth = LetterSize.Width;

			//Init byte color information for the data we're about to draw
			dataProvider.Prepare(position, position + bytesPerRow * (VisibleRows + 3));

			//Draw selected column background color
			if(IsFocused) {
				context.DrawRectangle(selectedRowColumnColor, null, new Rect(letterWidth * (3 * selectedColumn), 0, letterWidth * 2, bounds.Height));
			}

			//Draw data
			int visibleRows = VisibleRows + 2;

			if(IsFocused && selectedRow >= TopRow && selectedRow < TopRow + visibleRows) {
				//Draw background color for current row
				context.DrawRectangle(selectedRowColumnColor, null, new Rect(0, (selectedRow - TopRow) * RowHeight, rowWidth, RowHeight));
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

				if(position == SelectionStart && NewByteValue >= 0) {
					//About to draw the selected byte, draw anything that's pending, and then the current byte
					byteInfo.ForeColor = Colors.DarkOrange;
					byteInfo.Value = (byte)NewByteValue;
					dataToDraw.Add(byteInfo);
				} else {
					dataToDraw.Add(byteInfo);
				}

				fgColors.Add(byteInfo.ForeColor);

				position++;
			}

			context.Custom(new HexViewDrawOperation(this, dataToDraw, fgColors));

			if(selectedRow >= TopRow && selectedRow < TopRow + visibleRows) {
				//Draw selected character/byte cursor
				double xPos;
				if(ShowStringView && _inStringView) {
					xPos = RowWidth + StringViewMargin + _startPositionByByte[(selectedRow - TopRow) * bytesPerRow + selectedColumn];
				} else {
					xPos = letterWidth * selectedColumn * 3 + (_lastNibble ? letterWidth : 0);
				}

				xPos = (int)xPos;
				double yPos = (int)((selectedRow - TopRow) * RowHeight);
				context.DrawRectangle(cursorPen, new Rect(xPos, yPos, 1, (int)RowHeight));
			}
		}

		private void InitFontAndLetterSize()
		{
			this.Font = new Typeface(new FontFamily(this.FontFamily));
			var text = new FormattedText("A", CultureInfo.CurrentCulture, FlowDirection.LeftToRight, Font, FontSize, null);
			this.LetterSize = new Size(text.Width, text.Height);
		}

		private void DrawRowHeaders(DrawingContext context)
		{
			//Draw background
			context.DrawRectangle(ColorHelper.GetBrush(HeaderBackground), null, new Rect(0, 0, RowHeaderWidth, Bounds.Height));

			//Draw row addresses
			context.Custom(new HexViewDrawRowHeaderOperation(this));
		}

		private void DrawColumnHeaders(DrawingContext context)
		{
			context.DrawRectangle(ColorHelper.GetBrush(HeaderBackground), null, new Rect(0, 0, Bounds.Width, this.ColumnHeaderHeight));

			StringBuilder sb = new StringBuilder();
			for(int i = 0, len = BytesPerRow; i < len ; i++) {
				sb.Append(i.ToString(HexFormat) + " ");
			}

			var text = new FormattedText(sb.ToString(), CultureInfo.CurrentCulture, FlowDirection.LeftToRight, Font, FontSize, ColorHelper.GetBrush(HeaderForeground));
			context.DrawText(text, new Point(RowHeaderWidth, (this.ColumnHeaderHeight - this.LetterSize.Height) / 2));
		}
	}

	public interface IHexEditorDataProvider
	{
		void Prepare(int firstByteIndex, int lastByteIndex);
		ByteInfo GetByte(int byteIndex);
		int Length { get; }

		byte[] GetRawBytes(int start, int length);

		string ConvertValueToString(UInt64 val, out int keyLength);
		byte ConvertCharToByte(char c);
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

		public int StringValueKeyLength { get; set; }
		public string StringValue { get; set; }
		public bool UseAltFont { get; set; }

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
