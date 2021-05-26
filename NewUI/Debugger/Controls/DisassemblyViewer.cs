using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Media;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.Controls
{
	public class DisassemblyViewer : Control
	{
		public static readonly StyledProperty<ICodeDataProvider> DataProviderProperty = AvaloniaProperty.Register<DisassemblyViewer, ICodeDataProvider>(nameof(DataProvider));
		public static readonly StyledProperty<ILineStyleProvider> StyleProviderProperty = AvaloniaProperty.Register<DisassemblyViewer, ILineStyleProvider>(nameof(StyleProviderProperty));
		public static readonly StyledProperty<int> ScrollPositionProperty = AvaloniaProperty.Register<DisassemblyViewer, int>(nameof(ScrollPosition), 0, false, Avalonia.Data.BindingMode.TwoWay);

		private static readonly PolylineGeometry ArrowShape = new PolylineGeometry(new List<Point> {
			new Point(0, 5), new Point(8, 5), new Point(8, 0), new Point(15, 7), new Point(15, 8), new Point(8, 15), new Point(8, 10), new Point(0, 10),
		}, true);

		public ICodeDataProvider DataProvider
		{
			get { return GetValue(DataProviderProperty); }
			set { SetValue(DataProviderProperty, value); }
		}

		public ILineStyleProvider StyleProvider
		{
			get { return GetValue(StyleProviderProperty); }
			set { SetValue(StyleProviderProperty, value); }
		}

		public int ScrollPosition
		{
			get { return GetValue(ScrollPositionProperty); }
			set { SetValue(ScrollPositionProperty, value); }
		}

		private Typeface Font { get; set; }
		private Size LetterSize { get; set; }
		private double RowHeight => this.LetterSize.Height;
		private int VisibleRows => (int)(Bounds.Height / RowHeight) - 1;
		
		static DisassemblyViewer()
		{
			AffectsRender<DisassemblyViewer>(DataProviderProperty, ScrollPositionProperty, StyleProviderProperty);
		}

		protected override void OnPointerWheelChanged(PointerWheelEventArgs e)
		{
			base.OnPointerWheelChanged(e);
			ScrollPosition = Math.Max(0, Math.Min(ScrollPosition - (int)(e.Delta.Y * 3), DataProvider.GetLineCount() - 1));
		}

		private void InitFontAndLetterSize()
		{
			this.Font = new Typeface(new FontFamily("Consolas"));
			var text = new FormattedText("A", this.Font, 14, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			this.LetterSize = text.Bounds.Size;
		}

		private List<CodeLineData> _lines = new List<CodeLineData>();
		public void Refresh()
		{
			ICodeDataProvider dp = this.DataProvider;
			int scrollPosition = ScrollPosition;

			InitFontAndLetterSize();

			List<CodeLineData> lines = new List<CodeLineData>();
			if(dp != null) {
				for(int i = scrollPosition, len = scrollPosition + VisibleRows + 1; i < len; i++) {
					if(i < dp.GetLineCount()) {
						lines.Add(dp.GetCodeLineData(i));
					}
				}
			}
			_lines = lines;
		}

		public override void Render(DrawingContext context)
		{
			if(DataProvider == null) {
				return;
			}

			Refresh();

			List<CodeLineData> lines = _lines;
			double y = 0;
			var text = new FormattedText("", this.Font, 14, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			var smallText = new FormattedText("", this.Font, 12, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);

			context.FillRectangle(ColorHelper.GetBrush(Colors.White), Bounds);

			ILineStyleProvider styleProvider = this.StyleProvider;

			string addrFormat = "X" + DataProvider.CpuType.GetAddressSize();
			double symbolMargin = 20;
			double addressMargin = Math.Floor(LetterSize.Width * DataProvider.CpuType.GetAddressSize() + symbolMargin) + 0.5;
			double byteCodeMargin = Math.Floor(LetterSize.Width * (4 * DataProvider.CpuType.GetByteCodeSize()));
			double codeIndent = Math.Floor(LetterSize.Width * 2) + 0.5;

			//Draw margin (address)
			context.FillRectangle(ColorHelper.GetBrush(Color.FromRgb(235, 235, 235)), new Rect(0, 0, addressMargin, Bounds.Height));
			context.DrawLine(ColorHelper.GetPen(Colors.LightGray), new Point(addressMargin, 0), new Point(addressMargin, Bounds.Height));

			//Draw byte code
			context.FillRectangle(ColorHelper.GetBrush(Color.FromRgb(251, 251, 251)), new Rect(addressMargin, 0, byteCodeMargin, Bounds.Height));
			context.DrawLine(ColorHelper.GetPen(Colors.LightGray), new Point(addressMargin + byteCodeMargin, 0), new Point(addressMargin + byteCodeMargin, Bounds.Height));

			//Draw code
			foreach(CodeLineData line in lines) {
				List<CodeColor> lineParts = styleProvider.GetCodeColors(line, true, addrFormat, null, true);
				LineProperties lineStyle = styleProvider.GetLineStyle(line, 0);
				
				double x = 0;

				//Draw symbol in margin
				switch(lineStyle.Symbol) {
					case LineSymbol.Arrow:
						if(lineStyle.TextBgColor.HasValue) {
							double scaleFactor = LetterSize.Height / 16.0;
							using var scale =  context.PushPostTransform(Matrix.CreateScale(scaleFactor * 0.85, scaleFactor * 0.85));
							using var translation =  context.PushPostTransform(Matrix.CreateTranslation(2.5, y + (LetterSize.Height * 0.15 / 2)));
							context.DrawGeometry(new SolidColorBrush(lineStyle.TextBgColor.Value), new Pen(Brushes.Black), DisassemblyViewer.ArrowShape);
						}
						break;
				}

				//Draw address in margin
				text.Text = line.Address >= 0 ? line.Address.ToString(addrFormat) : "..";
				context.DrawText(ColorHelper.GetBrush(Colors.Gray), new Point(symbolMargin, y), text);
				x += addressMargin;

				//Draw byte code
				text.Text = line.ByteCode;
				context.DrawText(ColorHelper.GetBrush(Colors.Gray), new Point(x + LetterSize.Width / 2, y), text);
				x += byteCodeMargin;

				if(line.Flags.HasFlag(LineFlags.BlockStart) || line.Flags.HasFlag(LineFlags.BlockEnd) || line.Flags.HasFlag(LineFlags.SubStart)) {
					//Draw line to mark block start/end
					double lineHeight = Math.Floor(y + LetterSize.Height / 2) + 0.5;
					context.DrawLine(ColorHelper.GetPen(Colors.LightGray), new Point(x, lineHeight), new Point(Bounds.Width, lineHeight));

					if(!string.IsNullOrWhiteSpace(line.Text)) {
						//Draw block title (when set)
						smallText.Text = line.Text;
						double width = Bounds.Width - x;
						double textPosX = Math.Floor(x + (width / 2) - (smallText.Bounds.Width / 2)) + 0.5;
						double textPosY = Math.Floor(y + (LetterSize.Height / 2) - (smallText.Bounds.Height / 2)) + 0.5;
						double rectHeight = Math.Floor(smallText.Bounds.Height);
						double rectWidth = Math.Floor(smallText.Bounds.Width + 10);
						context.DrawRectangle(ColorHelper.GetBrush(Colors.White), ColorHelper.GetPen(Colors.Gray), new Rect(textPosX - 5, textPosY, rectWidth, rectHeight));
						context.DrawText(ColorHelper.GetBrush(Colors.Black), new Point(textPosX, textPosY), smallText);
					}
				} else {
					foreach(CodeColor part in lineParts) {
						text.Text = part.Text;
						if(lineStyle.TextBgColor.HasValue) {
							context.DrawRectangle(new SolidColorBrush(lineStyle.TextBgColor.Value.ToUint32()), null, new Rect(x + codeIndent, y, text.Bounds.Width, text.Bounds.Height));
						}
						context.DrawText(ColorHelper.GetBrush(part.Color), new Point(x + codeIndent, y), text);
						x += text.Bounds.Width;
					}
				}
				y += LetterSize.Height;
			}
		}
	}

	public interface ICodeDataProvider
	{
		CpuType CpuType { get; }

		CodeLineData GetCodeLineData(int lineIndex);
		int GetLineCount();
		int GetNextResult(string searchString, int startPosition, int endPosition, bool searchBackwards);
		bool UseOptimizedSearch { get; }

		int GetLineAddress(int lineIndex);
		int GetLineIndex(UInt32 address);
	}

	public interface ILineStyleProvider
	{
		LineProperties GetLineStyle(CodeLineData lineData, int lineIndex);
		string? GetLineComment(int lineIndex);

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

		public LineProgress? Progress;
	}

	public class LineProgress
	{
		public int Current;
		public int Maxixum;
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

	public class CodeColor
	{
		public string Text = "";
		public Color Color;
	}
}
