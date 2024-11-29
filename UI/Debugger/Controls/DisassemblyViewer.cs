using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Media;
using Avalonia.Media.Immutable;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Reactive.Disposables;

namespace Mesen.Debugger.Controls
{
	public class DisassemblyViewer : Control
	{
		public static readonly StyledProperty<CodeLineData[]> LinesProperty = AvaloniaProperty.Register<DisassemblyViewer, CodeLineData[]>(nameof(Lines));
		public static readonly StyledProperty<ILineStyleProvider> StyleProviderProperty = AvaloniaProperty.Register<DisassemblyViewer, ILineStyleProvider>(nameof(StyleProviderProperty));

		public static readonly StyledProperty<string> SearchStringProperty = AvaloniaProperty.Register<DisassemblyViewer, string>(nameof(SearchString), string.Empty);

		public static readonly StyledProperty<FontFamily> FontFamilyProperty = AvaloniaProperty.Register<DisassemblyViewer, FontFamily>(nameof(FontFamily), new FontFamily(FontManager.Current.DefaultFontFamily.Name));
		public static readonly StyledProperty<double> FontSizeProperty = AvaloniaProperty.Register<DisassemblyViewer, double>(nameof(FontSize), 12);
		public static readonly StyledProperty<bool> ShowByteCodeProperty = AvaloniaProperty.Register<DisassemblyViewer, bool>(nameof(ShowByteCode), false);
		public static readonly StyledProperty<AddressDisplayType> AddressDisplayTypeProperty = AvaloniaProperty.Register<DisassemblyViewer, AddressDisplayType>(nameof(AddressDisplayType), AddressDisplayType.CpuAddress);
		
		public static readonly StyledProperty<int> VisibleRowCountProperty = AvaloniaProperty.Register<DisassemblyViewer, int>(nameof(VisibleRowCount), 0);
		public static readonly StyledProperty<double> HorizontalScrollPositionProperty = AvaloniaProperty.Register<DisassemblyViewer, double>(nameof(HorizontalScrollPosition), 0, defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<double> HorizontalScrollMaxPositionProperty = AvaloniaProperty.Register<DisassemblyViewer, double>(nameof(HorizontalScrollMaxPosition), 0, defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);

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

		public int VisibleRowCount
		{
			get { return GetValue(VisibleRowCountProperty); }
			set { SetValue(VisibleRowCountProperty, value); }
		}

		public double HorizontalScrollPosition
		{
			get { return GetValue(HorizontalScrollPositionProperty); }
			set { SetValue(HorizontalScrollPositionProperty, value); }
		}

		public double HorizontalScrollMaxPosition
		{
			get { return GetValue(HorizontalScrollMaxPositionProperty); }
			set { SetValue(HorizontalScrollMaxPositionProperty, value); }
		}

		public ILineStyleProvider StyleProvider
		{
			get { return GetValue(StyleProviderProperty); }
			set { SetValue(StyleProviderProperty, value); }
		}

		public FontFamily FontFamily
		{
			get { return GetValue(FontFamilyProperty); }
			set { SetValue(FontFamilyProperty, value); }
		}

		public double FontSize
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
		private Dictionary<CodeLineData, List<TextFragment>> _textFragments = new();
		private Point _previousPointerPos;
		private CodeSegmentInfo? _prevPointerOverSegment = null;
		private CompositeDisposable _disposables = new();

		static DisassemblyViewer()
		{
			AffectsRender<DisassemblyViewer>(
				FontFamilyProperty, FontSizeProperty, StyleProviderProperty, ShowByteCodeProperty,
				LinesProperty, SearchStringProperty, AddressDisplayTypeProperty, HorizontalScrollPositionProperty
			);
		}

		public DisassemblyViewer()
		{
			Focusable = true;
			ClipToBounds = true;

			ColorHelper.InvalidateControlOnThemeChange(this);
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			base.OnAttachedToVisualTree(e);
			_disposables.Add(FontSizeProperty.Changed.Subscribe(_ => InitFontAndLetterSize()));
			_disposables.Add(FontFamilyProperty.Changed.Subscribe(_ => InitFontAndLetterSize()));
			_disposables.Add(BoundsProperty.Changed.Subscribe(_ => InitFontAndLetterSize()));
		}

		protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
		{
			_disposables.Dispose();
			base.OnDetachedFromVisualTree(e);
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
						List<TextFragment>? fragments = null;
						TextFragment? fragment = null;
						if(lineData != null) {
							_textFragments.TryGetValue(lineData, out fragments);
							if(fragments != null) {
								fragment = fragments.Where(frag => p.X >= frag.XPosition && p.X < frag.XPosition + frag.Width).FirstOrDefault();
							}
						}
						CodePointerMoved?.Invoke(this, new CodePointerMovedEventArgs(rowNumber, e, lineData, codeSegment, fragments, fragment));
						_prevPointerOverSegment = codeSegment;
					}
					return;
				}
			}

			_prevPointerOverSegment = null;
			CodePointerMoved?.Invoke(this, new CodePointerMovedEventArgs(rowNumber, e, lineData, null));
		}

		protected override void OnPointerExited(PointerEventArgs e)
		{
			base.OnPointerExited(e);
			_previousPointerPos = new Point(0, 0);
			_prevPointerOverSegment = null;
			CodePointerMoved?.Invoke(this, new CodePointerMovedEventArgs(-1, e, null, null));
		}

		private void InitFontAndLetterSize()
		{
			this.Font = new Typeface(FontFamily);
			var text = FormatText("A");
			this.LetterSize = new Size(text.Width, text.Height);
			this.VisibleRowCount = (int)Math.Ceiling(Bounds.Height / RowHeight);
		}

		private FormattedText FormatText(string text, IBrush? foreground = null, int fontSizeOffset = 0)
		{
			return new FormattedText(text, CultureInfo.CurrentCulture, FlowDirection.LeftToRight, Font, FontSize + fontSizeOffset, foreground);
		}

		public override void Render(DrawingContext context)
		{
			try {
				InternalRender(context);
			} catch(Exception ex) {
				Dispatcher.UIThread.Post(() => {
					MesenMsgBox.ShowException(ex);
				});
			}
		}

		private void InternalRender(DrawingContext context)
		{
			CodeLineData[] lines = Lines;
			if(lines.Length == 0) {
				return;
			}

			double y = 0;
			FormattedText text;
			FormattedText smallText;

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
			_textFragments.Clear();

			bool useLowerCase = ConfigManager.Config.Debug.Debugger.UseLowerCaseDisassembly;
			string baseFormat = useLowerCase ? "x" : "X";

			//Draw code
			int lineCount = Math.Min(VisibleRowCount, lines.Length);
			double maxWidth = 0;

			for(int i = 0; i < lineCount; i++) {
				CodeLineData line = lines[i];
				_textFragments[line] = new();

				string addrFormat = baseFormat + line.CpuType.GetAddressSize();
				LineProperties lineStyle = styleProvider.GetLineStyle(line, i);
				List<CodeColor> lineParts = styleProvider.GetCodeColors(line, true, addrFormat, lineStyle.TextBgColor != null ? ColorHelper.GetContrastTextColor(lineStyle.TextBgColor.Value) : null, true);

				double x = 0;

				//Draw symbol in margin
				DrawLineSymbol(context, y, lineStyle);

				//Draw address in margin
				string addressText = line.HasAddress ? line.Address.ToString(addrFormat) : "";
				text = FormatText(line.GetAddressText(addressDisplayType, addrFormat), ColorHelper.GetBrush(Colors.Gray));

				Point marginAddressPos = new Point(addressMargin - text.Width - 1, y);
				context.DrawText(text, marginAddressPos);
				_visibleCodeSegments.Add(new CodeSegmentInfo(addressText, CodeSegmentType.MarginAddress, new Rect(marginAddressPos.X, marginAddressPos.Y, text.Width, text.Height), line));
				x += addressMargin;

				if(showByteCode) {
					//Draw byte code
					text = FormatText(line.GetByteCode(styleProvider.ByteCodeSize), ColorHelper.GetBrush(Colors.Gray));
					context.DrawText(text, new Point(x + LetterSize.Width / 2, y));
					x += byteCodeMargin;
				}

				if(lineStyle.LineBgColor.HasValue) {
					SolidColorBrush brush = ColorHelper.GetBrush(lineStyle.LineBgColor.Value);
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
						smallText = FormatText(line.Text, ColorHelper.GetBrush(Colors.Black), -2);
						double width = Bounds.Width - x;
						double textPosX = Math.Round(x + (width / 2) - (smallText.Width / 2)) + 0.5;
						double textPosY = Math.Round(y + (LetterSize.Height / 2) - (smallText.Height / 2)) - 0.5;
						double rectHeight = Math.Round(smallText.Height);
						double rectWidth = Math.Round(smallText.Width + 10);
						context.DrawRectangle(ColorHelper.GetBrush(Colors.White), ColorHelper.GetPen(Colors.Gray), new Rect(textPosX - 5, textPosY, rectWidth, rectHeight));
						context.DrawText(smallText, new Point(textPosX, textPosY));
					}
				} else {
					using(var clip = context.PushClip(new Rect(x, 0, Bounds.Width, Bounds.Height))) {
						using(var translation = context.PushTransform(Matrix.CreateTranslation(-HorizontalScrollPosition*10, 0))) {
							if(lineStyle.TextBgColor.HasValue || lineStyle.OutlineColor.HasValue) {
								text = FormatText(GetHighlightedText(line, lineParts, out double leftMargin));

								Brush? b = lineStyle.TextBgColor.HasValue ? new SolidColorBrush(lineStyle.TextBgColor.Value.ToUInt32()) : null;
								Pen? p = lineStyle.OutlineColor.HasValue ? new Pen(lineStyle.OutlineColor.Value.ToUInt32()) : null;
								if(b != null) {
									context.DrawRectangle(b, null, new Rect(Math.Round(x + codeIndent + leftMargin) - 0.5, Math.Round(y) - 0.5, Math.Round(text.WidthIncludingTrailingWhitespace) + 1, Math.Round(LetterSize.Height)));
								}
								if(p != null) {
									context.DrawRectangle(p, new Rect(Math.Round(x + codeIndent + leftMargin) - 0.5, Math.Round(y) - 0.5, Math.Round(text.WidthIncludingTrailingWhitespace) + 1, Math.Round(LetterSize.Height)));
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
								SolidColorBrush brush = part.Type switch {
									CodeSegmentType.Comment or CodeSegmentType.EffectiveAddress or CodeSegmentType.MemoryValue => ColorHelper.GetBrush(part.Color),
									_ => lineStyle.TextBgColor.HasValue ? new SolidColorBrush(part.Color) : ColorHelper.GetBrush(part.Color)
								};
								text = FormatText(part.Text, brush);
								context.DrawText(text, pos);
								_visibleCodeSegments.Add(new CodeSegmentInfo(part.Text, part.Type, new Rect(pos, new Size(text.WidthIncludingTrailingWhitespace, text.Height)), line, part.OriginalIndex));
								GenerateTextFragments(line, part, x, indent);
								x += text.WidthIncludingTrailingWhitespace;
							}

							maxWidth = Math.Max(maxWidth, x + indent);

							if(!string.IsNullOrWhiteSpace(searchString)) {
								DrawSearchHighlight(context, y, searchString, line, lineParts, xStart);
							}
						}
					}

					if(lineStyle.Progress != null) {
						string progressText = lineStyle.Progress.Current + " " + lineStyle.Progress.Text;
						smallText = FormatText(progressText, ColorHelper.GetBrush(Colors.Black), -2);
						Point textPos = new Point(
							Math.Round(Bounds.Width - smallText.Width - 8) - 0.5,
							Math.Round(Math.Round(y) + (RowHeight - smallText.Height) / 2)
						);

						Rect rect = new(Math.Round(textPos.X - 4) + 0.5, Math.Round(y) + 1.5, Math.Round(smallText.Width + 8), Math.Round(LetterSize.Height) - 4);
						context.FillRectangle(ColorHelper.GetBrush(lineStyle.Progress.Color), rect);
						context.DrawRectangle(ColorHelper.GetPen(Colors.Black), rect);
						context.DrawText(smallText, textPos);

						_visibleCodeSegments.Add(new CodeSegmentInfo(progressText, CodeSegmentType.InstructionProgress, rect, line, -1, lineStyle.Progress));
					}
				}
				y += LetterSize.Height;
			}

			Dispatcher.UIThread.Post(() => {
				HorizontalScrollMaxPosition = Math.Max(1, (maxWidth - Bounds.Width + 10) / 10);
			});
		}

		private void GenerateTextFragments(CodeLineData line, CodeColor part, double x, double indent)
		{
			//Calculate x-axis start/end offsets for all text fragments (each invididual word, etc.)
			for(int j = 0; j < part.Text.Length; j++) {
				int start = j;

				char c = part.Text[j];
				int startCharType = (char.IsLetterOrDigit(c) || c == '_' || c == '@' || c == '.') ? 1 : (char.IsWhiteSpace(c) ? 2 : 3);
				int charType;
				do {
					j++;
					if(j >= part.Text.Length) {
						break;
					}

					c = part.Text[j];
					charType = (char.IsLetterOrDigit(c) || c == '_' || c == '@' || c == '.') ? 1 : (char.IsWhiteSpace(c) ? 2 : 3);
				} while(charType == startCharType);
				j--;

				int len = j - start + 1;
				string fragment = part.Text.Substring(start, len);
				FormattedText text = FormatText(fragment);
				double width = text.WidthIncludingTrailingWhitespace;
				_textFragments[line].Add(new TextFragment { Text = fragment, StartIndex = start, TextLength = len, XPosition = x + indent, Width = width });
				x += width;
			}
		}

		private string GetHighlightedText(CodeLineData line, List<CodeColor> lineParts, out double leftMargin)
		{
			leftMargin = 0;
			if(line.Text.Length == 0) {
				return string.Empty;
			}

			int skipCharCount = 0;
			int highlightCharCount = 0;
			bool foundCodeStart = false;
			CodeSegmentType prevType = CodeSegmentType.None;
			foreach(CodeColor part in lineParts) {
				CodeSegmentType type = part.Type;
				//Find the first part that's not a whitespace or label definition (or the : after the label def)
				if(!foundCodeStart && (type == CodeSegmentType.None || type == CodeSegmentType.LabelDefinition || (type == CodeSegmentType.Syntax && prevType == CodeSegmentType.LabelDefinition))) {
					FormattedText text = FormatText(part.Text);
					leftMargin += text.WidthIncludingTrailingWhitespace;
					skipCharCount += part.Text.Length;
				} else {
					foundCodeStart = true;
					if(part.OriginalIndex < 0 || type == CodeSegmentType.Comment) {
						break;
					}
					highlightCharCount += part.Text.Length;
				}

				prevType = type;
			}

			if(skipCharCount + highlightCharCount > line.Text.Length) {
				return string.Empty;
			}

			return line.Text.Substring(skipCharCount, highlightCharCount).Trim();
		}

		private void DrawSearchHighlight(DrawingContext context, double y, string searchString, CodeLineData line, List<CodeColor> lineParts, double xStart)
		{
			DrawHighlightedText(context, line.Text, searchString, y, xStart);
			for(int j = 0; j < lineParts.Count; j++) {
				CodeColor part = lineParts[j];
				switch(part.Type) {
					case CodeSegmentType.EffectiveAddress:
					case CodeSegmentType.MemoryValue:
					case CodeSegmentType.Comment:
					case CodeSegmentType.Label:
						DrawHighlightedText(context, part.Text, searchString, y, _visibleCodeSegments[^(lineParts.Count - j)].Bounds.Left);
						break;
				}
			}
		}

		private void DrawHighlightedText(DrawingContext context, string hay, string needle, double y, double startX)
		{
			int result = hay.IndexOf(needle, StringComparison.OrdinalIgnoreCase);
			if(result >= 0) {
				double highlightPos = startX;
				FormattedText formattedText;
				if(result > 0) {
					formattedText = FormatText(hay.Substring(0, result));
					highlightPos += formattedText.WidthIncludingTrailingWhitespace;
				}
				formattedText = FormatText(hay.Substring(result, needle.Length), ColorHelper.GetBrush(Colors.White));
				Point p = new Point(highlightPos, y);
				SolidColorBrush selectBgBrush = new(Colors.CornflowerBlue);
				context.FillRectangle(selectBgBrush, new Rect(p, new Size(formattedText.WidthIncludingTrailingWhitespace, formattedText.Height)));
				context.DrawText(formattedText, p);
			}
		}

		private void DrawLineSymbol(DrawingContext context, double y, LineProperties lineStyle)
		{
			bool showOutline = lineStyle.Symbol.HasFlag(LineSymbol.CircleOutline) || lineStyle.Symbol.HasFlag(LineSymbol.Forbid) || lineStyle.Symbol.HasFlag(LineSymbol.ForbidDotted);
			if(showOutline || lineStyle.Symbol.HasFlag(LineSymbol.Circle)) {
				if(lineStyle.SymbolColor.HasValue) {
					using var translation = context.PushTransform(Matrix.CreateTranslation(2.5, y + (LetterSize.Height * 0.15 / 2)));
					using var scale = context.PushTransform(Matrix.CreateScale(0.85, 0.85));

					IDashStyle? dashStyle = lineStyle.Symbol.HasFlag(LineSymbol.ForbidDotted) ? new ImmutableDashStyle(new double[] { 1, 1 }, 0.5) : null;
					EllipseGeometry geometry = new EllipseGeometry(new Rect(0, 0, LetterSize.Height, LetterSize.Height));
					IBrush? b = lineStyle.Symbol.HasFlag(LineSymbol.Circle) ? new SolidColorBrush(lineStyle.SymbolColor.Value.ToUInt32()) : null;
					IPen? p = showOutline ? new Pen(lineStyle.SymbolColor.Value.ToUInt32(), 1, dashStyle) : null;
					context.DrawGeometry(b, p, geometry);

					if(lineStyle.Symbol.HasFlag(LineSymbol.Forbid) || lineStyle.Symbol.HasFlag(LineSymbol.ForbidDotted)) {
						p = new Pen(lineStyle.SymbolColor.Value.ToUInt32(), 1, dashStyle);
						double xPos = LetterSize.Height / 2 - (Math.Cos(Math.PI / 4) * LetterSize.Height/2);
						double yPos = LetterSize.Height / 2 - (Math.Sin(Math.PI / 4) * LetterSize.Height/2);
						double xPos2 = LetterSize.Height / 2 + (Math.Cos(Math.PI / 4) * LetterSize.Height/2);
						double yPos2 = LetterSize.Height / 2 + (Math.Sin(Math.PI / 4) * LetterSize.Height/2);
						context.DrawLine(p, new Point(xPos, yPos), new Point(xPos2, yPos2));
					}

					if(lineStyle.Symbol.HasFlag(LineSymbol.Plus)) {
						Color c = showOutline ? lineStyle.SymbolColor.Value : Colors.White;
						p = new Pen(c.ToUInt32(), 2, dashStyle);
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
					using var translation = context.PushTransform(Matrix.CreateTranslation(2.5, y + (LetterSize.Height * 0.15 / 2)));
					using var scale = context.PushTransform(Matrix.CreateScale(scaleFactor * 0.85, scaleFactor * 0.85));
					context.DrawGeometry(new SolidColorBrush(lineStyle.TextBgColor.Value), new Pen(Brushes.Black), DisassemblyViewer.ArrowShape);
				}
			}
		}
	}

	public class TextFragment
	{
		public string Text { get; set; } = "";
		public int StartIndex { get; set; }
		public int TextLength { get; set; }
		public double XPosition { get; set; }
		public double Width { get; set; }
	}

	public class CodePointerMovedEventArgs : EventArgs
	{
		public CodePointerMovedEventArgs(int rowNumber, PointerEventArgs pointerEvent, CodeLineData? lineData, CodeSegmentInfo? codeSegment, List<TextFragment>? fragments = null, TextFragment? fragment = null)
		{
			RowNumber = rowNumber;
			Data = lineData;
			CodeSegment = codeSegment;
			Fragments = fragments;
			Fragment = fragment;
			PointerEvent = pointerEvent;
		}

		public PointerEventArgs PointerEvent { get; }
		public CodeLineData? Data { get; }
		public CodeSegmentInfo? CodeSegment { get; }
		public List<TextFragment>? Fragments { get; }
		public TextFragment? Fragment { get; }
		public int RowNumber { get; }
	}

	public class CodeSegmentInfo
	{
		public CodeSegmentInfo(string text, CodeSegmentType type, Rect bounds, CodeLineData data, int originalTextIndex = -1, LineProgress? progress = null)
		{
			Text = text;
			Type = type;
			Bounds = bounds;
			Data = data;
			OriginalTextIndex = originalTextIndex;
			Progress = progress;
		}

		public string Text { get; }
		public CodeSegmentType Type { get; }
		public Rect Bounds { get; }
		public CodeLineData Data { get; }
		public LineProgress? Progress { get; }
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
		public Color? OutlineColor;
		public Color? SymbolColor;
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
		public CpuInstructionProgress CpuProgress;
	}

	[Flags]
	public enum LineSymbol
	{
		None = 0,
		Circle = 1,
		CircleOutline = 2,
		Arrow = 4,
		Mark = 8,
		Plus = 16,
		Forbid = 32,
		ForbidDotted = 64,
	}

	public enum CodeSegmentType
	{
		OpCode,
		Token,
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
		InstructionProgress,
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
