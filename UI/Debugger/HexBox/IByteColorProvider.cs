using System;
using System.Drawing;

namespace Be.Windows.Forms
{
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
