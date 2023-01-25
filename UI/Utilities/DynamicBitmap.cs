using Avalonia;
using Avalonia.Controls;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using System;

namespace Mesen.Utilities
{
	public class DynamicBitmap : WriteableBitmap, IDynamicBitmap
	{
		public event EventHandler? Invalidated;

		public DynamicBitmap(PixelSize size, Vector dpi, PixelFormat format, AlphaFormat alphaFormat)
			: base(size.Width == 0 && size.Height == 0 ? new PixelSize(100, 100) : size, dpi, format, alphaFormat)
		{
		}

		public void Invalidate()
		{
			Invalidated?.Invoke(this, EventArgs.Empty);
		}

		public new DynamicBitmapLock Lock()
		{
			return new DynamicBitmapLock(this, base.Lock());
		}
	}

	public interface IDynamicBitmap
	{
		event EventHandler? Invalidated;
		void Invalidate();
	}

	public class DynamicBitmapLock : IDisposable
	{
		public ILockedFramebuffer FrameBuffer { get; private set; }
		private DynamicBitmap _bitmap;

		public DynamicBitmapLock(DynamicBitmap bitmap, ILockedFramebuffer lockedFramebuffer)
		{
			_bitmap = bitmap;
			FrameBuffer = lockedFramebuffer;
		}

		public void Dispose()
		{
			FrameBuffer.Dispose();
			_bitmap.Invalidate();
		}
	}
}
