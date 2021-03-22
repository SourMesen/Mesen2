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

		public byte[] Data
		{
			get { return GetValue(DataProperty); }
			set { SetValue(DataProperty, value); }
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

		public HexEditor()
		{
			
		}

		public override void Render(DrawingContext context)
		{
			base.Render(context);

			var text = new FormattedText("Test", Typeface.Default, 12, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			context.DrawText(Brushes.Black, new Point(0, 0), text);
		}
	}

	public interface IByteColorProvider
	{
		void Prepare(long firstByteIndex, long lastByteIndex);
		ByteColors GetByteColor(long firstByteIndex, long byteIndex);
	}

	public class ByteColors
	{
		public Color ForeColor { get; set; }
		public Color BackColor { get; set; }
		public Color BorderColor { get; set; }
	}
}
