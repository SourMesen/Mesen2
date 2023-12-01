using Avalonia;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using System;
using System.Collections.Generic;

namespace Mesen.Utilities
{
	public class DynamicBitmap : WriteableBitmap, IDynamicBitmap
	{
		public event EventHandler? Invalidated;

		public List<Rect>? HighlightRects { get; set; }

		public DynamicBitmap(PixelSize size, Vector dpi, PixelFormat format, AlphaFormat alphaFormat)
			: base(size.Width == 0 && size.Height == 0 ? new PixelSize(100, 100) : size, dpi, format, alphaFormat)
		{
		}

		public void Invalidate()
		{
			Invalidated?.Invoke(this, EventArgs.Empty);
		}

		public DynamicBitmapLock Lock(bool forReadAccess = false)
		{
			return new DynamicBitmapLock(this, () => base.Lock(), forReadAccess);
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
		private bool _forReadAccess;

		public DynamicBitmapLock(DynamicBitmap bitmap, Func<ILockedFramebuffer> lockWriteableBitmap, bool forReadAccess)
		{
			FrameBuffer = lockWriteableBitmap();
			_bitmap = bitmap;
			_forReadAccess = forReadAccess;
		}

		public void Dispose()
		{
			FrameBuffer.Dispose();
			if(!_forReadAccess) {
				_bitmap.Invalidate();
			}
		}
	}
}
