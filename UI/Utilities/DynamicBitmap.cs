using Avalonia;
using Avalonia.Controls;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using SkiaSharp;
using System;
using System.Collections.Generic;
using System.Threading;

namespace Mesen.Utilities
{
	public class DynamicBitmap : WriteableBitmap, IDynamicBitmap
	{
		public event EventHandler? Invalidated;

		private object _dynBitmapLock = new();
		public List<Rect>? HighlightRects { get; set; }

		public DynamicBitmap(PixelSize size, Vector dpi, PixelFormat format, AlphaFormat alphaFormat)
			: base(size.Width == 0 && size.Height == 0 ? new PixelSize(100, 100) : size, dpi, format, alphaFormat)
		{
		}

		public void Invalidate()
		{
			Invalidated?.Invoke(this, EventArgs.Empty);
		}

		public void Draw(SKCanvas canvas, SKBitmap bitmap, SKRect sourceRect, SKRect destRect)
		{
			lock(_dynBitmapLock) {
				canvas.DrawBitmap(bitmap, sourceRect, destRect);
			}
		}

		public new DynamicBitmapLock Lock()
		{
			return new DynamicBitmapLock(this, _dynBitmapLock, base.Lock());
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
		private object _dynBitmapLock;

		public DynamicBitmapLock(DynamicBitmap bitmap, object dynBitmapLock, ILockedFramebuffer lockedFramebuffer)
		{
			Monitor.Enter(dynBitmapLock);
			_bitmap = bitmap;
			_dynBitmapLock = dynBitmapLock;
			FrameBuffer = lockedFramebuffer;
		}

		public void Dispose()
		{
			FrameBuffer.Dispose();
			_bitmap.Invalidate();
			Monitor.Exit(_dynBitmapLock);
		}
	}
}
