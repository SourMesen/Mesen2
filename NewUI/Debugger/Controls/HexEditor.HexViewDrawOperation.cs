using Avalonia;
using Avalonia.Media;
using Avalonia.Platform;
using Avalonia.Rendering.SceneGraph;
using Avalonia.Skia;
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
			private double _rowHeight;
			private string _hexFormat;
			private double _stringViewPosition;
			private Dictionary<Color, SKPaint> _skPaints = new Dictionary<Color, SKPaint>();
			private Color _selectedColor = ColorHelper.GetColor(Colors.LightSkyBlue);

			public HexViewDrawOperation(HexEditor he, List<ByteInfo> dataToDraw, HashSet<Color> fgColors)
			{
				_he = he;
				Bounds = _he.Bounds;
				_bytesPerRow = _he.BytesPerRow;
				_hexFormat = _he.HexFormat;
				_rowHeight = _he.RowHeight;
				_dataToDraw = dataToDraw;
				_fgColors = fgColors;
				_letterSize = _he.LetterSize;
				_stringViewPosition = _he.RowWidth + _he.StringViewMargin;

				foreach(ByteInfo byteInfo in dataToDraw) {
					if(!_skPaints.ContainsKey(byteInfo.BackColor)) {
						_skPaints[byteInfo.BackColor] = new SKPaint() { Color = new SKColor(ColorHelper.GetColor(byteInfo.BackColor).ToUint32()) };
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

					canvas.Translate((float)_stringViewPosition, 0);
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
				paint.Color = new SKColor(ColorHelper.GetColor(color).ToUint32());

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

				List<float> startPositionByByte = new List<float>(_dataToDraw.Count);
				List<float> endPositionByByte = new List<float>(_dataToDraw.Count);
				while(pos < _dataToDraw.Count) {
					double xPos = 0;
					for(int i = 0; i < _bytesPerRow; i++) {
						if(pos + i >= _dataToDraw.Count) {
							break;
						}

						ByteInfo byteInfo = _dataToDraw[pos + i];
						string str = _he.ConvertByteToString(byteInfo.Value);
						int codepoint = Char.ConvertToUtf32(str, 0);

						if(codepoint > 0x024F) {
							altFont.GetGlyphs(str, measureBuffer.GetGlyphSpan());
							startPositionByByte.Add((float)xPos);
							xPos += altFont.MeasureText(measureBuffer.GetGlyphSpan());
							endPositionByByte.Add((float)xPos);
						} else {
							startPositionByByte.Add((float)xPos);
							xPos += _letterSize.Width * str.Length;
							endPositionByByte.Add((float)xPos);
						}
					}
					pos += _bytesPerRow;
				}

				_he._startPositionByByte = startPositionByByte;
				_he._endPositionByByte = endPositionByByte;
			}

			private void DrawStringView(SKCanvas canvas, Color color)
			{
				SKPaint paint = new SKPaint();
				paint.Color = new SKColor(ColorHelper.GetColor(color).ToUint32());

				SKTypeface typeface = SKTypeface.FromFamilyName("Consolas");
				SKFont monoFont = new SKFont(typeface, 14);
				SKFont altFont = new SKFont(SKFontManager.Default.MatchCharacter('あ'), 14);
				
				using var builder = new SKTextBlobBuilder();

				int pos = 0;
				int row = 0;

				SKPaint selectedPaint = new SKPaint() { Color = new SKColor(_selectedColor.R, _selectedColor.G, _selectedColor.B, 255) };

				SKRect GetRect(int i) => new SKRect(
					(float)_he._startPositionByByte[i],
					(float)(row * _rowHeight) - 13,
					(float)_he._endPositionByByte[i],
					(float)((row + 1) * _rowHeight) - 13
				);

				while(pos < _dataToDraw.Count) {
					for(int i = 0; i < _bytesPerRow; i++) {
						if(pos + i >= _dataToDraw.Count) {
							break;
						}

						ByteInfo byteInfo = _dataToDraw[pos + i];
						string str = _he.ConvertByteToString(byteInfo.Value);

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
							var buffer = builder.AllocateRun(currentFont, count, _he._startPositionByByte[pos+i], (float)(row * _rowHeight));
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

				SKPaint selectedPaint = new SKPaint() { Color = new SKColor(_selectedColor.R, _selectedColor.G, _selectedColor.B, 255) };

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
		}
	}
}
