using Avalonia;
using Avalonia.Media;
using Avalonia.Platform;
using Avalonia.Rendering.SceneGraph;
using Avalonia.Skia;
using Mesen.Config;
using Mesen.Utilities;
using SkiaSharp;
using System;
using System.Collections.Generic;
using System.Text;

namespace Mesen.Debugger.Controls
{
	public partial class HexEditor
	{
		class HexViewDrawOperation : ICustomDrawOperation
		{
			private HexEditor _he;
			private List<ByteInfo> _dataToDraw;
			private HashSet<Color> _fgColors;
			private Size _letterSize;
			private int _bytesPerRow;
			private bool _showStringView;
			private bool _inStringView;
			private double _rowHeight;
			private string _hexFormat;
			private string _fontFamily;
			private float _fontSize;
			private double _stringViewPosition;
			private IHexEditorDataProvider _dataProvider;
			private Dictionary<Color, SKPaint> _skFillPaints = new Dictionary<Color, SKPaint>();
			private Dictionary<Color, SKPaint> _skBorderPaints = new Dictionary<Color, SKPaint>();
			private Color _selectedColorOther = ColorHelper.GetColor(Colors.LightBlue);
			private Color _selectedColor = ColorHelper.GetColor(Colors.LightSkyBlue);
			private bool _highDensityMode;

			private SKFontEdging _skiaEdging;
			private bool _skiaSubpixelSmoothing;

			public HexViewDrawOperation(HexEditor he, List<ByteInfo> dataToDraw, HashSet<Color> fgColors, FontAntialiasing fontAntialiasing)
			{
				_he = he;
				Bounds = _he.Bounds;
				_fontFamily = _he.FontFamily.Name;
				_fontSize = (float)_he.FontSize;
				_bytesPerRow = _he.BytesPerRow;
				_hexFormat = _he.HexFormat;
				_rowHeight = _he.RowHeight;
				_dataProvider = _he.DataProvider;
				_dataToDraw = dataToDraw;
				_fgColors = fgColors;
				_letterSize = _he.LetterSize;
				_showStringView = _he.ShowStringView;
				_inStringView = _he._inStringView;
				_stringViewPosition = _he.RowWidth + _he.StringViewMargin;
				_highDensityMode = _he.HighDensityMode;

				_skiaEdging = fontAntialiasing switch {
					FontAntialiasing.Disabled => SKFontEdging.Alias,
					FontAntialiasing.Antialias => SKFontEdging.Antialias,
					FontAntialiasing.SubPixelAntialias or _ => SKFontEdging.SubpixelAntialias
				};
				_skiaSubpixelSmoothing = fontAntialiasing == FontAntialiasing.SubPixelAntialias;

				foreach(ByteInfo byteInfo in dataToDraw) {
					if(!_skFillPaints.ContainsKey(byteInfo.BackColor)) {
						_skFillPaints[byteInfo.BackColor] = new SKPaint() { Color = new SKColor(ColorHelper.GetColor(byteInfo.BackColor).ToUInt32()) };
					}
					if(!_skBorderPaints.ContainsKey(byteInfo.BorderColor)) {
						_skBorderPaints[byteInfo.BorderColor] = new SKPaint() { Style = SKPaintStyle.Stroke, Color = new SKColor(ColorHelper.GetColor(byteInfo.BorderColor).ToUInt32()) };
					}
				}
			}

			private void SetFontProperties(SKFont font)
			{
				font.Edging = _skiaEdging;
				font.Subpixel = _skiaSubpixelSmoothing;

				//Fixes layout issues (on Linux)
				font.Hinting = SKFontHinting.Full;
				font.LinearMetrics = true;
			}

			public Rect Bounds { get; private set; }

			public void Dispose()
			{
			}

			public bool Equals(ICustomDrawOperation? other) => false;
			public bool HitTest(Point p) => false;

			public void Render(ImmediateDrawingContext context)
			{
				var leaseFeature = context.PlatformImpl.GetFeature<ISkiaSharpApiLeaseFeature>();
				using var lease = leaseFeature?.Lease();
				var canvas = lease?.SkCanvas;
				if(canvas == null) {
					//context.DrawText(Brushes.Black, new Point(), _noSkia.PlatformImpl);
				} else {
					canvas.Save();

					DrawBackground(canvas);

					foreach(Color color in _fgColors) {
						DrawHexView(canvas, color);
					}

					if(_showStringView) {
						canvas.Translate((float)_stringViewPosition, 0);
						PrepareStringView();
						foreach(Color color in _fgColors) {
							DrawStringView(canvas, color);
						}
					}

					canvas.Restore();
				}
			}

			private void DrawHexView(SKCanvas canvas, Color color)
			{
				SKPaint paint = new SKPaint();
				paint.Color = new SKColor(ColorHelper.GetColor(color).ToUInt32());

				SKTypeface typeface = SKTypeface.FromFamilyName(_fontFamily);
				SKFont font = new SKFont(typeface, _fontSize);
				SetFontProperties(font);

				using var builder = new SKTextBlobBuilder();

				int pos = 0;
				double drawOffsetY = _highDensityMode ? -_rowHeight * 0.1 : -_rowHeight * 0.2;

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
					var buffer = builder.AllocateRun(font, count, 0, (float)(row * _rowHeight + drawOffsetY));
					font.GetGlyphs(rowText, buffer.GetGlyphSpan());
					row++;
					sb.Clear();
				}

				SKTextBlob? textToDraw = builder.Build();
				if(textToDraw != null) {
					canvas.DrawText(textToDraw, 0, (float)_rowHeight, paint);
				}
			}

			private void PrepareStringView()
			{
				SKFont altFont = new SKFont(SKFontManager.Default.MatchCharacter('あ'), _fontSize);
				SetFontProperties(altFont);

				int pos = 0;

				using var measureText = new SKTextBlobBuilder();
				float[] startPositionByByte = new float[_dataToDraw.Count];
				float[] endPositionByByte = new float[_dataToDraw.Count];
				while(pos < _dataToDraw.Count) {
					double xPos = 0;
					int i;
					int gap = pos % _bytesPerRow;
					pos -= gap;
					for(i = gap; i < _bytesPerRow; i++) {
						if(pos + i >= _dataToDraw.Count) {
							break;
						}

						int index = pos + i;
						ByteInfo byteInfo = _dataToDraw[index];
						UInt64 tblKeyValue = (UInt64)byteInfo.Value;
						for(int j = 1; j < 8; j++) {
							if(index + j < _dataToDraw.Count) {
								tblKeyValue += (UInt64)_dataToDraw[index + j].Value << (8 * j);
							}
						}

						string str = _dataProvider.ConvertValueToString(tblKeyValue, out int keyLength);
						byteInfo.StringValue = str;
						byteInfo.StringValueKeyLength = keyLength;

						int codepoint = Char.ConvertToUtf32(str, 0);
						if(codepoint > 0x024F) {
							SKRunBuffer measureBuffer = measureText.AllocateRun(altFont, altFont.CountGlyphs(str), 0, 0);
							byteInfo.UseAltFont = true;
							altFont.GetGlyphs(str, measureBuffer.GetGlyphSpan());
							startPositionByByte[index] = (float)xPos;
							xPos += altFont.MeasureText(measureBuffer.GetGlyphSpan());
							endPositionByByte[index] = (float)xPos;
						} else {
							startPositionByByte[index] = (float)xPos;
							xPos += _letterSize.Width * str.Length;
							endPositionByByte[index] = (float)xPos;
						}

						for(int j = 1; j < byteInfo.StringValueKeyLength && index + j < startPositionByByte.Length; j++) {
							startPositionByByte[index + j] = startPositionByByte[index];
							endPositionByByte[index + j] = endPositionByByte[index];
						}

						i += (byteInfo.StringValueKeyLength - 1);
						_dataToDraw[index] = byteInfo;
					}
					pos += i;
				}

				_he._startPositionByByte = startPositionByByte;
				_he._endPositionByByte = endPositionByByte;
			}

			private void DrawStringView(SKCanvas canvas, Color color)
			{
				SKPaint paint = new SKPaint();
				paint.Color = new SKColor(ColorHelper.GetColor(color).ToUInt32());

				SKTypeface typeface = SKTypeface.FromFamilyName(_fontFamily);
				SKFont monoFont = new SKFont(typeface, _fontSize);
				SetFontProperties(monoFont);

				SKFont altFont = new SKFont(SKFontManager.Default.MatchCharacter('あ'), _fontSize);
				SetFontProperties(altFont);

				using var builder = new SKTextBlobBuilder();

				int pos = 0;
				int row = 0;
				double drawOffsetY = _highDensityMode ? -_rowHeight * 0.1 : -_rowHeight * 0.2;

				Color selectedColor = _inStringView ? _selectedColor : _selectedColorOther;
				SKPaint selectedPaint = new SKPaint() { Color = new SKColor(selectedColor.R, selectedColor.G, selectedColor.B, 255) };

				SKRect GetRect(int i) => new SKRect(
					(float)_he._startPositionByByte[i],
					(float)(row * _rowHeight),
					(float)_he._endPositionByByte[i],
					(float)((row + 1) * _rowHeight)
				);

				while(pos < _dataToDraw.Count) {
					int i;
					int gap = pos % _bytesPerRow;
					pos -= gap;
					for(i = gap; i < _bytesPerRow; i++) {
						if(pos + i >= _dataToDraw.Count) {
							break;
						}

						ByteInfo byteInfo = _dataToDraw[pos + i];

						if(byteInfo.ForeColor == color) {
							SKFont currentFont = byteInfo.UseAltFont ? altFont : monoFont;

							SKRect rect = GetRect(pos + i);
							if(byteInfo.BackColor != Colors.Transparent) {
								canvas.DrawRect(rect, _skFillPaints[byteInfo.BackColor]);
							}
							if(byteInfo.Selected) {
								canvas.DrawRect(rect, selectedPaint);
							}
							if(byteInfo.BorderColor != Colors.Transparent) {
								rect.Inflate(0, -1);
								rect.Offset(0, -0.5f);
								canvas.DrawRect(rect, _skBorderPaints[byteInfo.BorderColor]);
							}

							int count = currentFont.CountGlyphs(byteInfo.StringValue);
							var buffer = builder.AllocateRun(currentFont, count, _he._startPositionByByte[pos + i], (float)(row * _rowHeight + drawOffsetY));
							currentFont.GetGlyphs(byteInfo.StringValue, buffer.GetGlyphSpan());
						}

						i += (byteInfo.StringValueKeyLength - 1);
					}

					pos += i;
					row++;
				}

				SKTextBlob? textToDraw = builder.Build();
				if(textToDraw != null) {
					canvas.DrawText(textToDraw, 0, (float)_rowHeight, paint);
				}
			}

			private void DrawBackground(SKCanvas canvas)
			{
				int pos = 0;
				int row = 0;

				Color selectedColor = _inStringView ? _selectedColorOther : _selectedColor;
				SKPaint selectedPaint = new SKPaint() { Color = new SKColor(selectedColor.R, selectedColor.G, selectedColor.B, 255) };

				SKRect GetRect(int start, int end) => new SKRect(
					(float)(start * 3 * _letterSize.Width),
					(float)(row * _rowHeight),
					(float)((end * 3 - 1) * _letterSize.Width),
					(float)((row + 1) * _rowHeight)
				);

				while(pos < _dataToDraw.Count) {
					int bgStartPos = -1;
					int borderStartPos = -1;
					int selectedStartPos = -1;
					Color bgColor = Colors.Transparent;
					Color borderColor = Colors.Transparent;
					bool selected = false;

					for(int i = 0; i < _bytesPerRow; i++) {
						if(pos + i >= _dataToDraw.Count) {
							break;
						}

						ByteInfo byteInfo = _dataToDraw[pos + i];

						if(byteInfo.BackColor != bgColor) {
							if(bgColor != Colors.Transparent && bgStartPos >= 0) {
								canvas.DrawRect(GetRect(bgStartPos, i), _skFillPaints[bgColor]);
								bgStartPos = -1;
							}
							if(byteInfo.BackColor != Colors.Transparent) {
								bgStartPos = i;
							}
							bgColor = byteInfo.BackColor;
						}
					}

					if(bgStartPos >= 0) {
						canvas.DrawRect(GetRect(bgStartPos, _bytesPerRow), _skFillPaints[bgColor]);
					}

					for(int i = 0; i < _bytesPerRow; i++) {
						if(pos + i >= _dataToDraw.Count) {
							break;
						}

						ByteInfo byteInfo = _dataToDraw[pos + i];

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

					if(selectedStartPos >= 0) {
						canvas.DrawRect(GetRect(selectedStartPos, _bytesPerRow), selectedPaint);
					}

					for(int i = 0; i < _bytesPerRow; i++) {
						if(pos + i >= _dataToDraw.Count) {
							break;
						}

						ByteInfo byteInfo = _dataToDraw[pos + i];

						if(byteInfo.BorderColor != borderColor) {
							if(borderColor != Colors.Transparent && borderStartPos >= 0) {
								SKRect rect = GetRect(borderStartPos, i);
								rect.Inflate(0, -1);
								rect.Offset(0, -0.5f);
								canvas.DrawRect(rect, _skBorderPaints[borderColor]);
								borderStartPos = -1;
							}
							if(byteInfo.BorderColor != Colors.Transparent) {
								borderStartPos = i;
							}
							borderColor = byteInfo.BorderColor;
						}

					}

					if(borderStartPos >= 0) {
						SKRect rect = GetRect(borderStartPos, _bytesPerRow);
						rect.Inflate(0, -1);
						rect.Offset(0, -0.5f);
						canvas.DrawRect(rect, _skBorderPaints[borderColor]);
					}

					pos += _bytesPerRow;
					row++;
				}
			}
		}

		class HexViewDrawRowHeaderOperation : ICustomDrawOperation
		{
			private HexEditor _he;
			private Size _letterSize;
			private int _bytesPerRow;
			private double _rowHeight;
			private string _fontFamily;
			private float _fontSize;
			private IHexEditorDataProvider _dataProvider;
			private double _headerCharLength;
			private double _rowHeaderWidth;
			private double _columnHeaderHeight;
			private Color _headerForeground;
			private int _firstByte;
			private bool _highDensityMode;

			private SKFontEdging _skiaEdging;
			private bool _skiaSubpixelSmoothing;

			public HexViewDrawRowHeaderOperation(HexEditor he, FontAntialiasing fontAntialiasing)
			{
				_he = he;
				Bounds = _he.Bounds;
				_fontFamily = _he.FontFamily.Name;
				_fontSize = (float)_he.FontSize;
				_bytesPerRow = _he.BytesPerRow;
				_rowHeight = _he.RowHeight;
				_rowHeaderWidth = _he.RowHeaderWidth;
				_columnHeaderHeight = _he.ColumnHeaderHeight;
				_headerCharLength = _he.HeaderCharLength;
				_headerForeground = _he.HeaderForeground.Color;
				_highDensityMode = _he.HighDensityMode;
				_dataProvider = _he.DataProvider;
				_letterSize = _he.LetterSize;
				_firstByte = _he.TopRow * _bytesPerRow;

				_skiaEdging = fontAntialiasing switch {
					FontAntialiasing.Disabled => SKFontEdging.Alias,
					FontAntialiasing.Antialias => SKFontEdging.Antialias,
					FontAntialiasing.SubPixelAntialias or _ => SKFontEdging.SubpixelAntialias
				};
				_skiaSubpixelSmoothing = fontAntialiasing == FontAntialiasing.SubPixelAntialias;
			}

			public Rect Bounds { get; private set; }

			public void Dispose()
			{
			}

			public bool Equals(ICustomDrawOperation? other) => false;
			public bool HitTest(Point p) => false;

			public void Render(ImmediateDrawingContext context)
			{
				var leaseFeature = context.PlatformImpl.GetFeature<ISkiaSharpApiLeaseFeature>();
				using var lease = leaseFeature?.Lease();
				var canvas = lease?.SkCanvas;
				if(canvas == null) {
					//context.DrawText(Brushes.Black, new Point(), _noSkia.PlatformImpl);
				} else {
					canvas.Save();

					canvas.Translate(0, 0);

					SKPaint paint = new SKPaint();
					paint.Color = new SKColor(ColorHelper.GetColor(_headerForeground).ToUInt32());

					SKTypeface typeface = SKTypeface.FromFamilyName(_fontFamily);
					SKFont font = new SKFont(typeface, _fontSize);
					font.Edging = _skiaEdging;
					font.Subpixel = _skiaSubpixelSmoothing;

					//Fixes layout issues (on Linux)
					font.Hinting = SKFontHinting.Full;
					font.LinearMetrics = true;

					using var builder = new SKTextBlobBuilder();

					int dataLength = _dataProvider.Length;
					int bytesPerRow = _bytesPerRow;

					double rowHeaderWidth = _rowHeaderWidth;
					double textWidth = _headerCharLength * _letterSize.Width;
					double xOffset = (rowHeaderWidth - textWidth) / 2;

					int headerByte = _firstByte;
					double y = 0;
					int row = 0;
					double drawOffsetY = _highDensityMode ? -_rowHeight * 0.1 : -_rowHeight * 0.2;

					//Draw row headers for each row
					while(y < Bounds.Height && headerByte < dataLength) {
						string rowText = headerByte.ToString("X" + _headerCharLength);
						int count = font.CountGlyphs(rowText);
						var buffer = builder.AllocateRun(font, count, (float)xOffset, (float)(row * _rowHeight + drawOffsetY));
						font.GetGlyphs(rowText, buffer.GetGlyphSpan());
						row++;
						y += _rowHeight;
						headerByte += bytesPerRow;
					}

					SKTextBlob? textToDraw = builder.Build();
					if(textToDraw != null) {
						canvas.DrawText(textToDraw, 0, (float)_rowHeight, paint);
					}

					canvas.Restore();
				}
			}
		}
	}
}
