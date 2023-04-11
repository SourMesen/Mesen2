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
			return new DynamicBitmapLock(this, _dynBitmapLock, () => base.Lock());
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

		public DynamicBitmapLock(DynamicBitmap bitmap, object dynBitmapLock, Func<ILockedFramebuffer> lockWriteableBitmap)
		{
			while(true) {
				FrameBuffer = lockWriteableBitmap();

				//Use TryEnter to avoid deadlocks due to the locks potentially causing the message pump to
				//be processed, which in turn can lead to a deadlock caused by the rendering thread.
				//By using TryEnter, the code waits for 20ms and then unlocks everything (to let other threads continue)
				//and then tries to take both locks again.
				if(Monitor.TryEnter(dynBitmapLock, 20)) {
					_bitmap = bitmap;
					_dynBitmapLock = dynBitmapLock;
					break;
				} else {
					FrameBuffer?.Dispose();
					FrameBuffer = null!;
					Thread.Sleep(10);
				}
			}
		}

		public void Dispose()
		{
			FrameBuffer.Dispose();
			_bitmap.Invalidate();
			Monitor.Exit(_dynBitmapLock);
		}
	}
}
