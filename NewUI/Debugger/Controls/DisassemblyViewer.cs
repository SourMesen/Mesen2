using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.Controls
{
	public class DisassemblyViewer : Control
	{
		public static readonly StyledProperty<CodeLineData[]> LinesProperty = AvaloniaProperty.Register<DisassemblyViewer, CodeLineData[]>(nameof(Lines));
		public static readonly StyledProperty<ILineStyleProvider> StyleProviderProperty = AvaloniaProperty.Register<DisassemblyViewer, ILineStyleProvider>(nameof(StyleProviderProperty));

		public static readonly StyledProperty<string> SearchStringProperty = AvaloniaProperty.Register<DisassemblyViewer, string>(nameof(SearchString), string.Empty);

		public static readonly StyledProperty<string> FontFamilyProperty = AvaloniaProperty.Register<DisassemblyViewer, string>(nameof(FontFamily), DebuggerConfig.MonospaceFontFamily);
		public static readonly StyledProperty<float> FontSizeProperty = AvaloniaProperty.Register<DisassemblyViewer, float>(nameof(FontSize), DebuggerConfig.DefaultFontSize);
		public static readonly StyledProperty<bool> ShowByteCodeProperty = AvaloniaProperty.Register<DisassemblyViewer, bool>(nameof(ShowByteCode), false);
		public static readonly StyledProperty<AddressDisplayType> AddressDisplayTypeProperty = AvaloniaProperty.Register<DisassemblyViewer, AddressDisplayType>(nameof(AddressDisplayType), AddressDisplayType.CpuAddress);

		private static readonly PolylineGeometry ArrowShape = new PolylineGeometry(new List<Point> {
			new Point(0, 5), new Point(8, 5), new Point(8, 0), new Point(15, 7), new Point(15, 8), new Point(8, 15), new Point(8, 10), new Point(0, 10),
		}, true);

		public CodeLineData[] Lines
		{
			get { return GetValue(LinesProperty); }
			set { SetValue(LinesProperty, value); }
		}

		public string SearchString
		{
			get { return GetValue(SearchStringProperty); }
			set { SetValue(SearchStringProperty, value); }
		}

		public ILineStyleProvider StyleProvider
		{
			get { return GetValue(StyleProviderProperty); }
			set { SetValue(StyleProviderProperty, value); }
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

		public bool ShowByteCode
		{
			get { return GetValue(ShowByteCodeProperty); }
			set { SetValue(ShowByteCodeProperty, value); }
		}

		public AddressDisplayType AddressDisplayType
		{
			get { return GetValue(AddressDisplayTypeProperty); }
			set { SetValue(AddressDisplayTypeProperty, value); }
		}

		public delegate void RowClickedEventHandler(DisassemblyViewer sender, RowClickedEventArgs args);
		public event RowClickedEventHandler? RowClicked;

		public delegate void CodePointerMovedEventHandler(DisassemblyViewer sender, CodePointerMovedEventArgs args);
		public event CodePointerMovedEventHandler? CodePointerMoved;

		private Typeface Font { get; set; }
		private Size LetterSize { get; set; }
		private double RowHeight => this.LetterSize.Height;
		private List<CodeSegmentInfo> _visibleCodeSegments = new();
		private Point _previousPointerPos;
		private CodeSegmentInfo? _prevPointerOverSegment = null;

		static DisassemblyViewer()
		{
			AffectsRender<DisassemblyViewer>(FontFamilyProperty, FontSizeProperty, StyleProviderProperty, ShowByteCodeProperty, LinesProperty, SearchStringProperty, AddressDisplayTypeProperty);
		}

		public DisassemblyViewer()
		{
			Focusable = true;
			ClipToBounds = true;
		}

		protected override void OnPointerPressed(PointerPressedEventArgs e)
		{
			base.OnPointerPressed(e);
			PointerPoint p = e.GetCurrentPoint(this);
			int rowNumber = (int)(p.Position.Y / LetterSize.Height);
			bool marginClicked = p.Position.X < 20;
			if(rowNumber >= 0 && rowNumber < Lines.Length) {
				RowClicked?.Invoke(this, new RowClickedEventArgs(Lines[rowNumber], rowNumber, marginClicked, e.GetCurrentPoint(this).Properties, e));
			}
		}

		protected override void OnPointerMoved(PointerEventArgs e)
		{
			base.OnPointerMoved(e);

			PointerPoint point = e.GetCurrentPoint(this);
			Point p = point.Position;

			if(_previousPointerPos == p) {
				//Pointer didn't move, don't trigger the pointer event
				return;
			}
			_previousPointerPos = p;

			int rowNumber = (int)(p.Y / LetterSize.Height);
			bool marginClicked = p.X < 20;
			CodeLineData? lineData = null;
			if(rowNumber >= 0 && rowNumber < Lines.Length) {
				lineData = Lines[rowNumber];
			} else {
				rowNumber = -1;
			}

			foreach(var codeSegment in _visibleCodeSegments) {
				if(codeSegment.Bounds.Contains(p)) {
					//Don't trigger an event if this is the same segment
					if(_prevPointerOverSegment != codeSegment) {
						CodePointerMoved?.Invoke(this, new CodePointerMovedEventArgs(rowNumber, e, lineData, codeSegment));
						_prevPointerOverSegment = codeSegment;
					}
					return;
				}
			}

			_prevPointerOverSegment = null;
			CodePointerMoved?.Invoke(this, new CodePointerMovedEventArgs(rowNumber, e, lineData, null));
		}

		protected override void OnPointerLeave(PointerEventArgs e)
		{
			base.OnPointerLeave(e);
			_previousPointerPos = new Point(0, 0);
			_prevPointerOverSegment = null;
			CodePointerMoved?.Invoke(this, new CodePointerMovedEventArgs(-1, e, null, null));
		}

		public int GetVisibleRowCount()
		{
			InitFontAndLetterSize();
			return (int)Math.Ceiling(Bounds.Height / RowHeight);
		}

		private void InitFontAndLetterSize()
		{
			this.Font = new Typeface(new FontFamily(this.FontFamily));
			var text = new FormattedText("A", this.Font, this.FontSize, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			this.LetterSize = text.Bounds.Size;
		}

		public override void Render(DrawingContext context)
		{
			CodeLineData[] lines = Lines;
			if(lines.Length == 0) {
				return;
			}

			InitFontAndLetterSize();

			double y = 0;
			var text = new FormattedText("", this.Font, this.FontSize, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			var smallText = new FormattedText("", this.Font, this.FontSize - 2, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);

			context.FillRectangle(ColorHelper.GetBrush(Colors.White), Bounds);

			ILineStyleProvider styleProvider = this.StyleProvider;

			string searchString = this.SearchString;
			AddressDisplayType addressDisplayType = AddressDisplayType;

			int addressMaxCharCount = addressDisplayType switch {
				AddressDisplayType.CpuAddress => StyleProvider.AddressSize,
				AddressDisplayType.AbsAddress => 6,
				AddressDisplayType.Both => StyleProvider.AddressSize + 6 + 3,
				AddressDisplayType.BothCompact => StyleProvider.AddressSize + 3 + 3,
				_ => throw new NotImplementedException()
			};

			double symbolMargin = 20;
			double addressMargin = Math.Floor(LetterSize.Width * addressMaxCharCount + symbolMargin) + 0.5;
			double byteCodeMargin = Math.Floor(LetterSize.Width * (3 * styleProvider.ByteCodeSize));
			double codeIndent = Math.Floor(LetterSize.Width * 2) + 0.5;

			//Draw margin (address)
			context.FillRectangle(ColorHelper.GetBrush(Color.FromRgb(235, 235, 235)), new Rect(0, 0, addressMargin, Bounds.Height));
			context.DrawLine(ColorHelper.GetPen(Colors.LightGray), new Point(addressMargin, 0), new Point(addressMargin, Bounds.Height));

			bool showByteCode = ShowByteCode;
			if(showByteCode) {
				//Draw byte code
				context.FillRectangle(ColorHelper.GetBrush(Color.FromRgb(251, 251, 251)), new Rect(addressMargin, 0, byteCodeMargin, Bounds.Height));
				context.DrawLine(ColorHelper.GetPen(Colors.LightGray), new Point(addressMargin + byteCodeMargin, 0), new Point(addressMargin + byteCodeMargin, Bounds.Height));
			}

			_visibleCodeSegments.Clear();

			//Draw code
			int lineCount = Math.Min(GetVisibleRowCount(), lines.Length);
			for(int i = 0; i < lineCount; i++) {
				CodeLineData line = lines[i];
				string addrFormat = "X" + line.CpuType.GetAddressSize();
				LineProperties lineStyle = styleProvider.GetLineStyle(line, i);
				List<CodeColor> lineParts = styleProvider.GetCodeColors(line, true, addrFormat, lineStyle.FgColor, true);

				double x = 0;

				//Draw symbol in margin
				DrawLineSymbol(context, y, lineStyle);

				//Draw address in margin
				string addressText = line.HasAddress ? line.Address.ToString(addrFormat) : "";
				string absAddress = line.AbsoluteAddress.Address >= 0 && !line.IsAddressHidden ? line.AbsoluteAddress.Address.ToString(addrFormat) : "";
				string compactAbsAddress = line.AbsoluteAddress.Address >= 0 && !line.IsAddressHidden ? (line.AbsoluteAddress.Address >> 12).ToString("X") : "";
				text.Text = addressDisplayType switch {
					AddressDisplayType.CpuAddress => addressText,
					AddressDisplayType.AbsAddress => absAddress,
					AddressDisplayType.Both => (addressText + (string.IsNullOrEmpty(absAddress) ? "" : " [" + absAddress + "]")).Trim(),
					AddressDisplayType.BothCompact => (addressText + (string.IsNullOrEmpty(compactAbsAddress) ? "" : " [" + compactAbsAddress + "]")).Trim(),
					_ => throw new NotImplementedException()
				};

				Point marginAddressPos = new Point(addressMargin - text.Bounds.Width - 1, y);
				context.DrawText(ColorHelper.GetBrush(Colors.Gray), marginAddressPos, text);
				_visibleCodeSegments.Add(new CodeSegmentInfo(addressText, CodeSegmentType.MarginAddress, text.Bounds.Translate(new Vector(marginAddressPos.X, marginAddressPos.Y)), line));
				x += addressMargin;

				if(showByteCode) {
					//Draw byte code
					text.Text = line.GetByteCode(styleProvider.ByteCodeSize);
					context.DrawText(ColorHelper.GetBrush(Colors.Gray), new Point(x + LetterSize.Width / 2, y), text);
					x += byteCodeMargin;
				}

				if(lineStyle.LineBgColor.HasValue) {
					SolidColorBrush brush = new(lineStyle.LineBgColor.Value.ToUint32());
					context.DrawRectangle(brush, null, new Rect(x, y, Bounds.Width - x, LetterSize.Height));
				}

				if(lineStyle.IsSelectedRow) {
					SolidColorBrush brush = ColorHelper.GetBrush(Color.FromArgb(150, 185, 210, 255));
					context.DrawRectangle(brush, null, new Rect(x, y, Bounds.Width - x, LetterSize.Height));
				}

				if(lineStyle.IsActiveRow) {
					Pen borderPen = ColorHelper.GetPen(Colors.Blue);
					context.DrawRectangle(borderPen, new Rect(x, Math.Round(y) - 0.5, Math.Floor(Bounds.Width) - x - 0.5, Math.Round(LetterSize.Height)));
				}

				if(line.Flags.HasFlag(LineFlags.BlockStart) || line.Flags.HasFlag(LineFlags.BlockEnd) || line.Flags.HasFlag(LineFlags.SubStart)) {
					//Draw line to mark block start/end
					double lineHeight = Math.Floor(y + LetterSize.Height / 2) + 0.5;
					context.DrawLine(ColorHelper.GetPen(Colors.LightGray), new Point(x, lineHeight), new Point(Bounds.Width, lineHeight));

					if(!string.IsNullOrWhiteSpace(line.Text)) {
						//Draw block title (when set)
						smallText.Text = line.Text;
						double width = Bounds.Width - x;
						double textPosX = Math.Round(x + (width / 2) - (smallText.Bounds.Width / 2)) + 0.5;
						double textPosY = Math.Round(y + (LetterSize.Height / 2) - (smallText.Bounds.Height / 2)) - 0.5;
						double rectHeight = Math.Round(smallText.Bounds.Height);
						double rectWidth = Math.Round(smallText.Bounds.Width + 10);
						context.DrawRectangle(ColorHelper.GetBrush(Colors.White), ColorHelper.GetPen(Colors.Gray), new Rect(textPosX - 5, textPosY, rectWidth, rectHeight));
						context.DrawText(ColorHelper.GetBrush(Colors.Black), new Point(textPosX, textPosY), smallText);
					}
				} else {
					if(lineStyle.TextBgColor.HasValue || lineStyle.OutlineColor.HasValue) {
						text.Text = GetHighlightedText(text, line, lineParts, out double leftMargin);

						Brush? b = lineStyle.TextBgColor.HasValue ? new SolidColorBrush(lineStyle.TextBgColor.Value.ToUint32()) : null;
						Pen? p = lineStyle.OutlineColor.HasValue ? new Pen(lineStyle.OutlineColor.Value.ToUint32()) : null;
						if(b != null) {
							context.DrawRectangle(b, null, new Rect(Math.Round(x + codeIndent + leftMargin), Math.Round(y), Math.Round(text.Bounds.Width), Math.Round(LetterSize.Height) - 1));
						}
						if(p != null) {
							context.DrawRectangle(p, new Rect(Math.Round(x + codeIndent + leftMargin), Math.Round(y), Math.Round(text.Bounds.Width), Math.Round(LetterSize.Height) - 1));
						}
					}

					double indent = codeIndent;
					if(lineParts.Count == 1 && (lineParts[0].Type == CodeSegmentType.LabelDefinition || lineParts[0].Type == CodeSegmentType.Comment)) {
						//Don't indent multi-line comments/label definitions
						indent = 0.5;
					}

					double xStart = x + indent;
					foreach(CodeColor part in lineParts) {
						Point pos = new Point(x + indent, y);
						text.Text = part.Text;
						context.DrawText(ColorHelper.GetBrush(part.Color), pos, text);
						_visibleCodeSegments.Add(new CodeSegmentInfo(part.Text, part.Type, text.Bounds.Translate(pos), line, part.OriginalIndex));
						x += text.Bounds.Width;
					}

					if(!string.IsNullOrWhiteSpace(searchString)) {
						DrawSearchHighlight(context, y, text, searchString, line, lineParts, xStart);
					}
				}
				y += LetterSize.Height;
			}
		}

		private static string GetHighlightedText(FormattedText text, CodeLineData line, List<CodeColor> lineParts, out double leftMargin)
		{
			leftMargin = 0;
			int skipCharCount = 0;
			int highlightCharCount = 0;
			bool foundOpCode = false;
			foreach(CodeColor part in lineParts) {
				if(!foundOpCode && part.Type != CodeSegmentType.OpCode) {
					text.Text = part.Text;
					leftMargin += text.Bounds.Width;
					skipCharCount += part.Text.Length;
				} else {
					foundOpCode = true;
					if(part.OriginalIndex < 0 || part.Type == CodeSegmentType.Comment) {
						break;
					}
					highlightCharCount += part.Text.Length;
				}
			}

			return line.Text.Substring(skipCharCount, highlightCharCount).Trim();
		}

		private void DrawSearchHighlight(DrawingContext context, double y, FormattedText text, string searchString, CodeLineData line, List<CodeColor> lineParts, double xStart)
		{
			DrawHighlightedText(context, line.Text, searchString, y, text, xStart);
			for(int j = 0; j < lineParts.Count; j++) {
				CodeColor part = lineParts[j];
				switch(part.Type) {
					case CodeSegmentType.EffectiveAddress:
					case CodeSegmentType.MemoryValue:
					case CodeSegmentType.Comment:
					case CodeSegmentType.Label:
						DrawHighlightedText(context, part.Text, searchString, y, text, _visibleCodeSegments[^(lineParts.Count - j)].Bounds.Left);
						break;
				}
			}
		}

		private void DrawHighlightedText(DrawingContext context, string hay, string needle, double y, FormattedText formattedText, double startX)
		{
			int result = hay.IndexOf(needle, StringComparison.OrdinalIgnoreCase);
			if(result >= 0) {
				double highlightPos = startX;
				if(result > 0) {
					formattedText.Text = hay.Substring(0, result);
					highlightPos += formattedText.Bounds.Width;
				}
				formattedText.Text = hay.Substring(result, needle.Length);
				Point p = new Point(highlightPos, y);
				SolidColorBrush selectBgBrush = new(Colors.CornflowerBlue);
				context.FillRectangle(selectBgBrush, formattedText.Bounds.Translate(p));
				context.DrawText(ColorHelper.GetBrush(Colors.White), p, formattedText);
			}
		}

		private void DrawLineSymbol(DrawingContext context, double y, LineProperties lineStyle)
		{
			if(lineStyle.Symbol.HasFlag(LineSymbol.Circle) || lineStyle.Symbol.HasFlag(LineSymbol.CircleOutline)) {
				if(lineStyle.OutlineColor.HasValue) {
					using var scale = context.PushPostTransform(Matrix.CreateScale(0.85, 0.85));
					using var translation = context.PushPostTransform(Matrix.CreateTranslation(2.5, y + (LetterSize.Height * 0.15 / 2)));
					EllipseGeometry geometry = new EllipseGeometry(new Rect(0, 0, LetterSize.Height, LetterSize.Height));
					IBrush? b = lineStyle.Symbol.HasFlag(LineSymbol.Circle) ? new SolidColorBrush(lineStyle.OutlineColor.Value) : null;
					IPen? p = lineStyle.Symbol.HasFlag(LineSymbol.CircleOutline) ? new Pen(lineStyle.OutlineColor.Value.ToUint32()) : null;
					context.DrawGeometry(b!, p!, geometry);

					if(lineStyle.Symbol.HasFlag(LineSymbol.Plus)) {
						Color c = lineStyle.Symbol.HasFlag(LineSymbol.CircleOutline) ? lineStyle.OutlineColor.Value : Colors.White;
						p = new Pen(c.ToUint32(), 2);
						context.DrawLine(p, new Point(2, LetterSize.Height / 2), new Point(LetterSize.Height - 2, LetterSize.Height / 2));
						context.DrawLine(p, new Point(LetterSize.Height / 2, 2), new Point(LetterSize.Height / 2, LetterSize.Height - 2));
					}

					if(lineStyle.Symbol.HasFlag(LineSymbol.Mark)) {
						EllipseGeometry markGeometry = new EllipseGeometry(new Rect(LetterSize.Height * 0.7, -LetterSize.Height * 0.1, LetterSize.Height / 2, LetterSize.Height / 2));
						context.DrawGeometry(Brushes.LightSkyBlue, new Pen(Brushes.Black), markGeometry);
					}
				}
			}

			if(lineStyle.Symbol.HasFlag(LineSymbol.Arrow)) {
				if(lineStyle.TextBgColor.HasValue) {
					double scaleFactor = LetterSize.Height / 16.0;
					using var scale = context.PushPostTransform(Matrix.CreateScale(scaleFactor * 0.85, scaleFactor * 0.85));
					using var translation = context.PushPostTransform(Matrix.CreateTranslation(2.5, y + (LetterSize.Height * 0.15 / 2)));
					context.DrawGeometry(new SolidColorBrush(lineStyle.TextBgColor.Value), new Pen(Brushes.Black), DisassemblyViewer.ArrowShape);
				}
			}
		}
	}

	public class CodePointerMovedEventArgs : EventArgs
	{
		public CodePointerMovedEventArgs(int rowNumber, PointerEventArgs pointerEvent, CodeLineData? lineData, CodeSegmentInfo? codeSegment)
		{
			this.RowNumber = rowNumber;
			this.Data = lineData;
			this.CodeSegment = codeSegment;
			this.PointerEvent = pointerEvent;
		}

		public PointerEventArgs PointerEvent { get; }
		public CodeLineData? Data { get; }
		public CodeSegmentInfo? CodeSegment { get; }
		public int RowNumber { get; }
	}

	public class CodeSegmentInfo
	{
		public CodeSegmentInfo(string text, CodeSegmentType type, Rect bounds, CodeLineData data, int originalTextIndex = -1)
		{
			Text = text;
			Type = type;
			Bounds = bounds;
			Data = data;
			OriginalTextIndex = originalTextIndex;
		}

		public string Text { get; }
		public CodeSegmentType Type { get; }
		public Rect Bounds { get; }
		public CodeLineData Data { get; }
		public int OriginalTextIndex { get; }
	}

	public class RowClickedEventArgs
	{
		public CodeLineData CodeLineData { get; private set; }
		public int RowNumber { get; private set; }
		public bool MarginClicked { get; private set; }
		public PointerPointProperties Properties { get; private set; }
		public PointerPressedEventArgs PointerEvent { get; private set; }

		public RowClickedEventArgs(CodeLineData codeLineData, int rowNumber, bool marginClicked, PointerPointProperties properties, PointerPressedEventArgs pointerEvent)
		{
			this.CodeLineData = codeLineData;
			this.RowNumber = rowNumber;
			this.MarginClicked = marginClicked;
			this.Properties = properties;
			this.PointerEvent = pointerEvent;
		}
	}

	public interface ICodeDataProvider
	{
		CpuType CpuType { get; }

		CodeLineData[] GetCodeLines(int address, int rowCount);

		int GetRowAddress(int address, int rowOffset);

		int GetLineCount();
	}

	public interface ILineStyleProvider
	{
		int AddressSize { get; }
		int ByteCodeSize { get; }

		LineProperties GetLineStyle(CodeLineData lineData, int lineIndex);

		List<CodeColor> GetCodeColors(CodeLineData lineData, bool highlightCode, string addressFormat, Color? textColor, bool showMemoryValues);
	}

	public class LineProperties
	{
		public Color? LineBgColor;
		public Color? TextBgColor;
		public Color? FgColor;
		public Color? OutlineColor;
		public Color? AddressColor;
		public LineSymbol Symbol;

		public bool IsActiveRow;
		public bool IsSelectedRow;

		public LineProgress? Progress;
	}

	public class LineProgress
	{
		public int Current;
		public int Maximum;
		public string Text = "";
		public Color Color;
	}

	[Flags]
	public enum LineSymbol
	{
		None = 0,
		Circle = 1,
		CircleOutline = 2,
		Arrow = 4,
		Mark = 8,
		Plus = 16
	}

	public enum CodeSegmentType
	{
		OpCode,
		ImmediateValue,
		Address,
		Label,
		EffectiveAddress,
		MemoryValue,
		None,
		Syntax,
		LabelDefinition,
		MarginAddress,
		Comment,
		Directive,
	}

	public class CodeColor
	{
		public string Text { get; }
		public int OriginalIndex { get; }
		public Color Color { get; }
		public CodeSegmentType Type { get; }

		public CodeColor(string text, Color color, CodeSegmentType type, int originalIndex = -1)
		{
			Text = text;
			Color = color;
			Type = type;
			OriginalIndex = originalIndex;
		}
	}
}
