using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.PpuViewer
{
	public class SpriteInfo
	{
		public int Index;
		public int X;
		public int Y;
		public int Width;
		public int Height;

		public int TileIndex;
		public int Palette;
		public int Priority;
		public int Flags;

		public bool LargeSprite;
		public bool HorizontalMirror;
		public bool VerticalMirror;
		public bool UseSecondTable;

		private bool GameboyMode;

		public Rectangle GetBounds()
		{
			return new Rectangle(X, Y, Width, Height);
		}

		public bool IsVisible()
		{
			if(GameboyMode) {
				return !(X == 0 || X >= 168 || Y+Height <= 16 || Y >= 160);
			}

			if(X + Width <= 0 || X > 255) {
				return false;
			}

			int endY = (Y + Height) & 0xFF;
			if(Y > 239 && endY > Y) {
				return false;
			}

			return true;
		}

		public static SpriteInfo GetGbSpriteInfo(byte[] oamRam, int spriteIndex, bool largeSprite, bool cgbEnabled)
		{
			SpriteInfo sprite = new SpriteInfo();
			int addr = spriteIndex << 2;
			sprite.Index = spriteIndex;
			sprite.Y = oamRam[addr];

			sprite.LargeSprite = largeSprite;
			sprite.Height = largeSprite ? 16 : 8;

			sprite.Width = 8;
			sprite.X = oamRam[addr + 1];
			sprite.TileIndex = oamRam[addr + 2];

			byte attributes = oamRam[addr + 3];
			sprite.Flags = attributes;
			sprite.UseSecondTable = cgbEnabled ? ((attributes & 0x08) != 0) : false;
			sprite.Palette = cgbEnabled ? (attributes & 0x07) : ((attributes & 0x10) >> 4);
			sprite.Priority = (attributes & 0x80) != 0 ? 1 : 0;
			sprite.HorizontalMirror = (attributes & 0x20) != 0;
			sprite.VerticalMirror = (attributes & 0x40) != 0;
			
			sprite.GameboyMode = true;

			return sprite;
		}

		public static SpriteInfo GetSpriteInfo(byte[] oamRam, int oamMode, int spriteIndex)
		{
			SpriteInfo sprite = new SpriteInfo();
			int addr = spriteIndex << 2;
			sprite.Index = spriteIndex;
			sprite.Y = oamRam[addr + 1];

			int highTableOffset = addr >> 4;
			int shift = ((addr >> 2) & 0x03) << 1;
			byte highTableValue = (byte)(oamRam[0x200 | highTableOffset] >> shift);
			sprite.LargeSprite = ((highTableValue & 0x02) >> 1) != 0;
			sprite.Height = _oamSizes[oamMode, sprite.LargeSprite ? 1 : 0, 1] << 3;

			sprite.Width = _oamSizes[oamMode, sprite.LargeSprite ? 1 : 0, 0] << 3;
			int sign = (highTableValue & 0x01) << 8;
			sprite.X = ((sign | oamRam[addr]) << 23) >> 23;
			sprite.TileIndex = oamRam[addr + 2];

			byte flags = oamRam[addr + 3];
			sprite.Flags = flags;
			sprite.UseSecondTable = (flags & 0x01) != 0;
			sprite.Palette = (flags >> 1) & 0x07;
			sprite.Priority = (flags >> 4) & 0x03;
			sprite.HorizontalMirror = (flags & 0x40) != 0;
			sprite.VerticalMirror = (flags & 0x80) != 0;

			return sprite;
		}

		private static byte[,,] _oamSizes = new byte[8, 2, 2] {
			{ { 1, 1 }, { 2, 2 } }, //8x8 + 16x16
			{ { 1, 1 }, { 4, 4 } }, //8x8 + 32x32
			{ { 1, 1 }, { 8, 8 } }, //8x8 + 64x64
			{ { 2, 2 }, { 4, 4 } }, //16x16 + 32x32
			{ { 2, 2 }, { 8, 8 } }, //16x16 + 64x64
			{ { 4, 4 }, { 8, 8 } }, //32x32 + 64x64
			{ { 2, 4 }, { 4, 8 } }, //16x32 + 32x64
			{ { 2, 4 }, { 4, 4 } }  //16x32 + 32x32
		};
	}
}
