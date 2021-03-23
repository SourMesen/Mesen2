using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
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
		public static readonly StyledProperty<int> StartByteProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(StartByte), 0);
		public static readonly StyledProperty<int> BytesPerRowProperty = AvaloniaProperty.Register<HexEditor, int>(nameof(BytesPerRow), 16);
		public static readonly StyledProperty<IByteColorProvider> ByteColorProviderProperty = AvaloniaProperty.Register<HexEditor, IByteColorProvider>(nameof(ByteColorProvider));

		public byte[] Data
		{
			get { return GetValue(DataProperty); }
			set { SetValue(DataProperty, value); this.InvalidateVisual(); }
		}

		public int StartByte
		{
			get { return GetValue(StartByteProperty); }
			set { SetValue(StartByteProperty, value); }
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

		public HexEditor()
		{
			
		}

		public override void Render(DrawingContext context)
		{
			base.Render(context);

			byte[] data = this.Data;
			if(data == null) {
				return;
			}

			int bytesPerRow = this.BytesPerRow;
			Rect bounds = this.Bounds;
			int position = this.StartByte * this.BytesPerRow;
			IByteColorProvider? colorProvider = this.ByteColorProvider;

			colorProvider?.Prepare(position, position + bytesPerRow * 100);

			double totalHeight = 0;
			double x = 0;
			Typeface typeFace = new Typeface(new FontFamily("Consolas"));
			StringBuilder sb = new StringBuilder(bytesPerRow * 3);
			var text = new FormattedText("", typeFace, 14, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);

			while(totalHeight < bounds.Height) {
				ByteColors? colors = null;
				
				void DrawText()
				{
					text.Text = sb.ToString();
					sb.Clear();

					if(colors?.BackColor.Equals(Colors.Transparent) == false) {
						SolidColorBrush bgBrush = new SolidColorBrush(colors.BackColor);
						context.FillRectangle(bgBrush, text.Bounds.Translate(new Vector(x, totalHeight)));
					}

					SolidColorBrush fontBrush = new SolidColorBrush(colors?.ForeColor ?? Colors.Black);
					context.DrawText(fontBrush, new Point(x, totalHeight), text);
					x += text.Bounds.Width;
				}

				for(int i = 0; i < bytesPerRow; i++) {
					if(data.Length <= position) {
						break;
					}

					ByteColors? newColors = colorProvider?.GetByteColor(position);
					if(newColors != colors && i > 0) {
						DrawText();
					}
					colors = newColors;

					sb.Append(data[position].ToString("X2") + " ");
					position++;
				}

				if(sb.Length > 0) {
					DrawText();
				}

				x = 0;
				totalHeight += text.Bounds.Height - 2;
			}
		}
	}

	public interface IByteColorProvider
	{
		void Prepare(long firstByteIndex, long lastByteIndex);
		ByteColors GetByteColor(long byteIndex);
	}

	public class ByteColors
	{
		public Color ForeColor { get; set; }
		public Color BackColor { get; set; }
		public Color BorderColor { get; set; }

		public override bool Equals(object? obj)
		{
			if(obj == null || !(obj is ByteColors)) {
				return false;
			}

			ByteColors a = (ByteColors)obj;
			return this.BackColor == a.BackColor && this.BorderColor == a.BorderColor && this.ForeColor == a.ForeColor;
		}

		public override int GetHashCode()
		{
			return base.GetHashCode();
		}
	}
}
