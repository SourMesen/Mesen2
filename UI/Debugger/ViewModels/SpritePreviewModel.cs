using Avalonia;
using Avalonia.Controls;
using Avalonia.Platform;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Debugger.ViewModels
{
	public class SpritePreviewModel : ViewModelBase
	{
		[Reactive] public int SpriteIndex { get; set; }
		[Reactive] public int X { get; set; }
		[Reactive] public int Y { get; set; }
		[Reactive] public int RawX { get; set; }
		[Reactive] public int RawY { get; set; }
		[Reactive] public int PreviewX { get; set; }
		[Reactive] public int PreviewY { get; set; }
		[Reactive] public int Width { get; set; }
		[Reactive] public int Height { get; set; }
		[Reactive] public int TileIndex { get; set; }
		[Reactive] public int TileAddress { get; set; }
		[Reactive] public DebugSpritePriority Priority { get; set; }
		[Reactive] public int Bpp { get; set; }
		[Reactive] public TileFormat Format { get; set; }
		[Reactive] public int Palette { get; set; }
		[Reactive] public int PaletteAddress { get; set; }
		[Reactive] public SpriteVisibility Visibility { get; set; }
		[Reactive] public string? Flags { get; set; }
		
		[Reactive] public NullableBoolean HorizontalMirror { get; set; }
		[Reactive] public NullableBoolean VerticalMirror { get; set; }
		[Reactive] public NullableBoolean MosaicEnabled { get; set; }
		[Reactive] public NullableBoolean BlendingEnabled { get; set; }
		[Reactive] public NullableBoolean WindowMode { get; set; }
		[Reactive] public NullableBoolean TransformEnabled { get; set; }
		[Reactive] public NullableBoolean DoubleSize { get; set; }
		[Reactive] public sbyte TransformParamIndex { get; set; }
		[Reactive] public bool UseExtendedVram { get; set; }
		[Reactive] public NullableBoolean UseSecondTable { get; set; }
		
		public UInt32 TileCount { get; set; }
		public UInt32 WrapWidth { get; set; }
		public UInt32 WrapHeight { get; set; }
		public UInt32[] TileAddresses { get; set; } = Array.Empty<UInt32>();

		private UInt32[] _rawPreview = new UInt32[128*128];

		[Reactive] public DynamicBitmap? SpritePreview { get; set; }
		[Reactive] public double SpritePreviewZoom { get; set; }
		[Reactive] public bool FadePreview { get; set; }

		public int RealWidth => Width / (DoubleSize == NullableBoolean.True ? 2 : 1);
		public int RealHeight => Height / (DoubleSize == NullableBoolean.True ? 2 : 1);

		public unsafe void Init(ref DebugSpriteInfo sprite, UInt32[] spritePreviews, DebugSpritePreviewInfo previewInfo)
		{
			SpriteIndex = sprite.SpriteIndex;
			
			X = sprite.X;
			Y = sprite.Y;
			RawX = sprite.RawX;
			RawY = sprite.RawY;
			PreviewX = sprite.X + previewInfo.CoordOffsetX;
			PreviewY = sprite.Y + previewInfo.CoordOffsetY;

			Width = sprite.Width;
			Height = sprite.Height;
			TileIndex = sprite.TileIndex;
			Priority = sprite.Priority;
			Bpp = sprite.Bpp;
			Format = sprite.Format;
			Palette = sprite.Palette;
			TileAddress = sprite.TileAddress;
			PaletteAddress = sprite.PaletteAddress;
			UseSecondTable = sprite.UseSecondTable;
			UseExtendedVram = sprite.UseExtendedVram;

			WrapWidth = previewInfo.WrapRightToLeft ? previewInfo.Width : 0;
			WrapHeight = previewInfo.WrapBottomToTop ? previewInfo.Height : 0;

			TileCount = sprite.TileCount;
			fixed(UInt32* p = sprite.TileAddresses) {
				if(TileAddresses == null || TileAddresses.Length != TileCount) {
					TileAddresses = new UInt32[TileCount];
				}
				for(int i = 0; i < sprite.TileCount; i++) {
					TileAddresses[i] = p[i];
				}
			}

			fixed(UInt32* p = spritePreviews) {
				bool needUpdate = false;

				UInt32* spritePreview = p + (sprite.SpriteIndex * 128 * 128);
				
				if(SpritePreview == null || SpritePreview.PixelSize.Width != sprite.Width || SpritePreview.PixelSize.Height != sprite.Height) {
					SpritePreview = new DynamicBitmap(new PixelSize(Width, Height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
					needUpdate = true;
				}

				int len = Width * Height;
				for(int i = 0; i < len; i++) {
					if(_rawPreview[i] != spritePreview[i]) {
						needUpdate = true;
					}
					_rawPreview[i] = spritePreview[i];
				}

				if(needUpdate) {
					int spriteSize = len * sizeof(UInt32);
					using(var bitmapLock = SpritePreview.Lock()) {
						Buffer.MemoryCopy(spritePreview, (void*)bitmapLock.FrameBuffer.Address, spriteSize, spriteSize);
					}
				}
			}
			
			SpritePreviewZoom = 32.0 / Math.Max(Width, Height);

			Visibility = sprite.Visibility;
			FadePreview = sprite.Visibility != SpriteVisibility.Visible;

			HorizontalMirror = sprite.HorizontalMirror;
			VerticalMirror = sprite.VerticalMirror;
			MosaicEnabled = sprite.MosaicEnabled;
			BlendingEnabled = sprite.BlendingEnabled;
			WindowMode = sprite.WindowMode;
			TransformEnabled = sprite.TransformEnabled;
			DoubleSize = sprite.DoubleSize;
			TransformParamIndex = sprite.TransformParamIndex;

			string flags = sprite.HorizontalMirror == NullableBoolean.True ? "H" : "";
			flags += sprite.VerticalMirror == NullableBoolean.True ? "V" : "";
			flags += sprite.Visibility == SpriteVisibility.Disabled ? "D" : "";
			flags += sprite.TransformEnabled == NullableBoolean.True ? "T" : "";
			flags += sprite.BlendingEnabled == NullableBoolean.True ? "B" : "";
			flags += sprite.WindowMode == NullableBoolean.True ? "W" : "";
			flags += sprite.MosaicEnabled == NullableBoolean.True ? "M" : "";
			flags += sprite.UseSecondTable == NullableBoolean.True ? "N" : "";
			Flags = flags;
		}

		public ValueTuple<Rect,Rect,Rect,Rect> GetPreviewRect()
		{
			Rect mainRect = new Rect(PreviewX, PreviewY, Width, Height);
			Rect wrapTopRect = default;
			Rect wrapLeftRect = default;
			Rect wrapBothRect = default;

			bool wrapVertical = WrapHeight > 0 && PreviewY + Height > WrapHeight;
			bool wrapHorizontal = WrapWidth > 0 && PreviewX + Width > WrapWidth;
			if(wrapVertical) {
				//Generate a 2nd rect for the portion of the sprite that wraps
				//around to the top of the screen when it goes past the screen's
				//bottom, when the WrapBottomToTop flag is set (used by SNES/SMS/GBA)
				wrapTopRect = new Rect(PreviewX, PreviewY - WrapHeight, Width, Height);
			}
			if(wrapHorizontal) {
				//Generate a 2nd rect for the portion of the sprite that wraps
				//around to the left of the screen when it goes past the screen's
				//right side, when the WrapRightToLeft flag is set (used by GBA)
				wrapLeftRect = new Rect(PreviewX - WrapWidth, PreviewY, Width, Height);
			}
			if(wrapVertical && wrapHorizontal) {
				wrapBothRect = new Rect(PreviewX - WrapWidth, PreviewY - WrapHeight, Width, Height);
			}
			return (mainRect, wrapTopRect, wrapLeftRect, wrapBothRect);
		}

		public SpritePreviewModel Clone()
		{
			SpritePreviewModel copy = new SpritePreviewModel();
			CopyTo(copy);
			return copy;
		}

		public void CopyTo(SpritePreviewModel dst)
		{
			dst.SpriteIndex = SpriteIndex;
			dst.X = X;
			dst.Y = Y;
			dst.RawX = RawX;
			dst.RawY = RawY;
			dst.PreviewX = PreviewX;
			dst.PreviewY = PreviewY;
			dst.Width = Width;
			dst.Height = Height;
			dst.TileIndex = TileIndex;
			dst.TileAddress = TileAddress;
			dst.Priority = Priority;
			dst.Bpp = Bpp;
			dst.Format = Format;
			dst.Palette = Palette;
			dst.PaletteAddress = PaletteAddress;
			dst.Visibility = Visibility;
			dst.Flags = Flags;
			dst.HorizontalMirror = HorizontalMirror;
			dst.VerticalMirror = VerticalMirror;
			dst.FadePreview = FadePreview;

			dst.MosaicEnabled = MosaicEnabled;
			dst.BlendingEnabled = BlendingEnabled;
			dst.WindowMode = WindowMode;
			dst.TransformEnabled = TransformEnabled;
			dst.DoubleSize = DoubleSize;
			dst.TransformParamIndex = TransformParamIndex;

			dst.UseExtendedVram = UseExtendedVram;
			dst.UseSecondTable = UseSecondTable;
		}
	}
}
