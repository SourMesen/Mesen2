using Avalonia;
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
		[Reactive] public bool Visible { get; set; }
		[Reactive] public string? Flags { get; set; }
		
		[Reactive] public bool HorizontalMirror { get; set; }
		[Reactive] public bool VerticalMirror { get; set; }
		[Reactive] public bool UseExtendedVram { get; set; }
		
		public UInt32 TileCount { get; set; }
		public UInt32[] TileAddresses { get; set; } = Array.Empty<UInt32>();

		[Reactive] public NullableBoolean UseSecondTable { get; set; }

		[Reactive] public DynamicBitmap? SpritePreview { get; set; }
		[Reactive] public double SpritePreviewZoom { get; set; }

		public unsafe void Init(ref DebugSpriteInfo sprite, DebugSpritePreviewInfo previewInfo)
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

			TileCount = sprite.TileCount;
			fixed(UInt32* p = sprite.TileAddresses) {
				if(TileAddresses == null || TileAddresses.Length < TileCount) {
					TileAddresses = new UInt32[TileCount];
				}
				for(int i = 0; i < sprite.TileCount; i++) {
					TileAddresses[i] = p[i];
				}
			}

			fixed(UInt32* p = sprite.SpritePreview) {
				if(SpritePreview == null || SpritePreview.PixelSize.Width != sprite.Width || SpritePreview.PixelSize.Height != sprite.Height) {
					SpritePreview = new DynamicBitmap(new PixelSize(Width, Height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
				}

				int spriteSize = Width * Height * sizeof(UInt32);
				using(var bitmapLock = SpritePreview.Lock()) {
					Buffer.MemoryCopy(p, (void*)bitmapLock.FrameBuffer.Address, spriteSize, spriteSize);
				}
			}
			
			SpritePreviewZoom = 32.0 / Math.Max(Width, Height);

			Visible = sprite.Visible;

			HorizontalMirror = sprite.HorizontalMirror;
			VerticalMirror = sprite.VerticalMirror;

			string flags = sprite.HorizontalMirror ? "H" : "";
			flags += sprite.VerticalMirror ? "V" : "";
			flags += sprite.UseSecondTable == NullableBoolean.True ? "N" : "";
			Flags = flags;
		}

		public Rect GetPreviewRect()
		{
			return new Rect(PreviewX, PreviewY, Width, Height);
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
			dst.Visible = Visible;
			dst.Flags = Flags;
			dst.HorizontalMirror = HorizontalMirror;
			dst.VerticalMirror = VerticalMirror;
			dst.UseExtendedVram = UseExtendedVram;
			dst.UseSecondTable = UseSecondTable;
		}
	}
}
