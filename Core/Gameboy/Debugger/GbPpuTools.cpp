#include "pch.h"
#include "Gameboy/Debugger/GbPpuTools.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"
#include "Gameboy/GbTypes.h"
#include "Gameboy/GbConstants.h"
#include "Shared/ColorUtilities.h"

GbPpuTools::GbPpuTools(Debugger* debugger, Emulator *emu) : PpuTools(debugger, emu)
{
}

DebugTilemapInfo GbPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	GbPpuState& state = (GbPpuState&)baseState;
	uint32_t offset = options.Layer == 1 ? 0x1C00 : 0x1800;
	bool isCgb = state.CgbEnabled;

	uint16_t baseTile = state.BgTileSelect ? 0 : 0x1000;

	std::fill(outBuffer, outBuffer + 256 * 256, 0xFFFFFFFF);

	uint16_t vramMask = isCgb ? 0x3FFF : 0x1FFF;

	uint8_t colorMask = 0xFF;
	if(options.DisplayMode == TilemapDisplayMode::Grayscale) {
		palette = (uint32_t*)_grayscaleColorsBpp2;
		colorMask = 0x03;
	}

	for(int row = 0; row < 32; row++) {
		uint16_t baseOffset = offset + ((row & 0x1F) << 5);

		for(int column = 0; column < 32; column++) {
			uint16_t addr = (baseOffset + column);
			uint8_t tileIndex = vram[addr];

			uint8_t attributes = isCgb ? vram[addr | 0x2000] : 0;

			uint8_t bgPalette = (attributes & 0x07) << 2;
			uint16_t tileBank = (attributes & 0x08) ? 0x2000 : 0x0000;
			bool hMirror = (attributes & 0x20) != 0;
			bool vMirror = (attributes & 0x40) != 0;
			//bool bgPriority = (attributes & 0x80) != 0;

			uint16_t tileStart = baseTile + (baseTile ? (int8_t)tileIndex * 16 : tileIndex * 16);
			tileStart |= tileBank;

			for(int y = 0; y < 8; y++) {
				uint16_t pixelStart = tileStart + (vMirror ? (7 - y) : y) * 2;
				for(int x = 0; x < 8; x++) {
					uint8_t pixelIndex = hMirror ? (7 - x) : x;
					uint8_t color = GetTilePixelColor<TileFormat::Bpp2>(vram, vramMask, pixelStart, pixelIndex);

					outBuffer[((row * 8) + y) * 256 + column * 8 + x] = palette[(bgPalette + color) & colorMask];
				}
			}
		}
	}

	DebugTilemapInfo result = {};
	result.Bpp = 2;
	result.Format = TileFormat::Bpp2;
	result.TileWidth = 8;
	result.TileHeight = 8;
	result.ColumnCount = 32;
	result.RowCount = 32;
	result.TilemapAddress = offset;
	result.TilesetAddress = baseTile;
	result.ScrollX = state.ScrollX;
	result.ScrollY = state.ScrollY;
	result.ScrollWidth = GbConstants::ScreenWidth;
	result.ScrollHeight = GbConstants::ScreenHeight;
	return result;
}

void GbPpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, DebugSpriteInfo* sprites, uint32_t* spritePreviews, uint32_t* palette, uint32_t* outBuffer)
{
	uint32_t bgColor = GetSpriteBackgroundColor(options.Background, palette, false);

	std::fill(outBuffer, outBuffer + 256 * 256, GetSpriteBackgroundColor(options.Background, palette, true));
	for(int i = 16; i < 16 + 144; i++) {
		std::fill(outBuffer + i * 256 + 8, outBuffer + i * 256 + 168, bgColor);
	}

	for(int i = 0; i < 40; i++) {
		DebugSpriteInfo& sprite = sprites[i];
		uint32_t* spritePreview = spritePreviews + i * _spritePreviewSize;

		for(int y = 0; y < sprite.Height; y++) {
			for(int x = 0; x < sprite.Width; x++) {
				uint32_t color = spritePreview[y * sprite.Width + x];
				if(color != 0) {
					if(sprite.Y + y >= 256 || sprite.X + x >= 256) {
						continue;
					}
					outBuffer[((sprite.Y + y) * 256) + sprite.X + x] = color;
				} else {
					spritePreview[y * sprite.Width + x] = bgColor;
				}
			}
		}
	}
}

FrameInfo GbPpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& state)
{
	return { 256, 256 };
}

DebugTilemapTileInfo GbPpuTools::GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState)
{
	DebugTilemapTileInfo result = {};

	FrameInfo size = GetTilemapSize(options, baseState);

	if(x >= size.Width || y >= size.Height) {
		return result;
	}

	GbPpuState& state = (GbPpuState&)baseState;
	bool isCgb = state.CgbEnabled;

	int row = y / 8;
	int column = x / 8;

	int offset = options.Layer == 1 ? 0x1C00 : 0x1800;
	uint16_t baseOffset = offset + ((row & 0x1F) << 5);
	uint16_t addr = (baseOffset + column);
	uint8_t tileIndex = vram[addr];

	uint8_t attributes = isCgb ? vram[addr | 0x2000] : 0;

	uint16_t baseTile = state.BgTileSelect ? 0 : 0x1000;
	uint16_t tileStart = baseTile + (baseTile ? (int8_t)tileIndex * 16 : tileIndex * 16);
	
	uint16_t tileBank = (attributes & 0x08) ? 0x2000 : 0x0000;
	tileStart |= tileBank;

	result.Column = column;
	result.Row = row;
	result.Height = 8;
	result.Width = 8;
	result.TileMapAddress = addr;
	result.TileIndex = tileIndex;
	result.TileAddress = tileStart;

	if(isCgb) {
		result.PaletteIndex = (attributes & 0x07);
		result.PaletteAddress = (result.PaletteIndex << 2);
		result.AttributeAddress = addr | 0x2000;
		result.AttributeData = vram[result.AttributeAddress];
		result.HorizontalMirroring = (NullableBoolean)((attributes & 0x20) != 0);
		result.VerticalMirroring = (NullableBoolean)((attributes & 0x40) != 0);
		result.HighPriority = (NullableBoolean)((attributes & 0x80) != 0);
	}

	return result;
}

DebugSpritePreviewInfo GbPpuTools::GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state, BaseState& ppuToolsState)
{
	DebugSpritePreviewInfo info = {};
	info.Height = 256;
	info.Width = 256;
	info.SpriteCount = 40;
	info.CoordOffsetX = 0;
	info.CoordOffsetY = 0;

	info.VisibleX = 8;
	info.VisibleY = 16;
	info.VisibleWidth = 160;
	info.VisibleHeight = 144;

	return info;
}

void GbPpuTools::GetSpriteInfo(DebugSpriteInfo& sprite, uint32_t* spritePreview, uint16_t i, GetSpritePreviewOptions& options, GbPpuState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette)
{
	sprite.Bpp = 2;
	sprite.Format = TileFormat::Bpp2;
	sprite.SpriteIndex = i;
	sprite.UseExtendedVram = false;
	
	sprite.Y = oamRam[i*4];
	sprite.X = oamRam[i * 4 + 1];
	sprite.RawY = sprite.Y;
	sprite.RawX = sprite.X;

	sprite.TileIndex = oamRam[i * 4 + 2];
	uint8_t attributes = oamRam[i * 4 + 3];

	bool useSecondTable = (state.CgbEnabled && (attributes & 0x08));
	sprite.UseSecondTable = useSecondTable ? NullableBoolean::True : NullableBoolean::False;
	sprite.Palette = state.CgbEnabled ? (attributes & 0x07) : ((attributes & 0x10) ? 1 : 0);
	bool horizontalMirror = (attributes & 0x20) != 0;
	bool verticalMirror = (attributes & 0x40) != 0;
	sprite.HorizontalMirror = horizontalMirror ? NullableBoolean::True : NullableBoolean::False;
	sprite.VerticalMirror = verticalMirror ? NullableBoolean::True : NullableBoolean::False;
	sprite.Visibility = sprite.X > 0 && sprite.Y > 0 && sprite.Y < 160 && sprite.X < 168 ? SpriteVisibility::Visible : SpriteVisibility::Offscreen;
	sprite.Width = 8;
	sprite.Height = state.LargeSprites ? 16 : 8;
	sprite.Priority = (attributes & 0x80) ? DebugSpritePriority::Background : DebugSpritePriority::Foreground;
	
	uint8_t tileIndex = (uint8_t)sprite.TileIndex;
	uint16_t tileBank = useSecondTable ? 0x2000 : 0x0000;
	uint16_t tileStart;
	if(state.LargeSprites) {
		tileStart = (tileIndex & 0xFE) * 16;
		sprite.TileAddresses[0] = tileStart;
		sprite.TileAddresses[1] = tileStart + 16;
		sprite.TileCount = 2;
	} else {
		tileStart = tileIndex * 16;
		sprite.TileAddresses[0] = tileStart;
		sprite.TileCount = 1;
	}

	tileStart |= tileBank;

	sprite.TileAddress = tileStart;

	for(int y = 0; y < sprite.Height; y++) {
		uint16_t pixelStart = tileStart + (verticalMirror ? (sprite.Height - 1 - y) : y) * 2;
		bool isCgb = state.CgbEnabled;
		for(int x = 0; x < 8; x++) {
			uint8_t shift = horizontalMirror ? (7 - x) : x;
			uint8_t color = GetTilePixelColor<TileFormat::Bpp2>(vram, 0x3FFF, pixelStart, shift);

			uint32_t outOffset = (y * 8) + x;
			if(color > 0) {
				if(!isCgb) {
					spritePreview[outOffset] = palette[4 + (sprite.Palette * 4) + color];
				} else {
					spritePreview[outOffset] = palette[32 + (sprite.Palette * 4) + color];
				}
			} else {
				spritePreview[outOffset] = 0;
			}
		}
	}
}

void GbPpuTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[], uint32_t* spritePreviews, uint32_t* screenPreview)
{
	GbPpuState& state = (GbPpuState&)baseState;
	for(int i = 0; i < 40; i++) {
		outBuffer[i].Init();
		GetSpriteInfo(outBuffer[i], spritePreviews + (i * _spritePreviewSize), i, options, state, vram, oamRam, palette);
	}

	GetSpritePreview(options, baseState, outBuffer, spritePreviews, palette, screenPreview);
}

DebugPaletteInfo GbPpuTools::GetPaletteInfo(GetPaletteInfoOptions options)
{
	DebugPaletteInfo info = {};
	GbPpuState state;
	_debugger->GetPpuState(state, CpuType::Gameboy);

	if(state.CgbEnabled) {
		info.RawFormat = RawPaletteFormat::Rgb555;
		info.ColorsPerPalette = 8;
		info.BgColorCount = 8 * 4;
		info.SpritePaletteOffset = info.BgColorCount;
		info.SpriteColorCount = 8 * 4;
		info.ColorCount = info.BgColorCount + info.SpriteColorCount;

		for(int i = 0; i < 8 * 4; i++) {
			info.RawPalette[i] = state.CgbBgPalettes[i];
			info.RgbPalette[i] = ColorUtilities::Rgb555ToArgb(state.CgbBgPalettes[i] & 0x7FFF);
		}
		for(int i = 0; i < 8 * 4; i++) {
			info.RawPalette[i+32] = state.CgbObjPalettes[i];
			info.RgbPalette[i+32] = ColorUtilities::Rgb555ToArgb(state.CgbObjPalettes[i] & 0x7FFF);
		}
	} else {
		info.RawFormat = RawPaletteFormat::Indexed;
		info.ColorsPerPalette = 4;
		info.BgColorCount = 4;
		info.SpritePaletteOffset = info.BgColorCount;
		info.SpriteColorCount = 2 * 4;
		info.ColorCount = info.BgColorCount + info.SpriteColorCount;

		for(int i = 0; i < 4; i++) {
			int bgColor = (state.BgPalette >> (i * 2)) & 0x03;
			info.RawPalette[i] = bgColor;
			info.RgbPalette[i] = ColorUtilities::Rgb555ToArgb(state.CgbBgPalettes[bgColor]);

			int objPal0Color = (state.ObjPalette0 >> (i * 2)) & 0x03;
			info.RawPalette[i + 4] = objPal0Color;
			info.RgbPalette[i + 4] = ColorUtilities::Rgb555ToArgb(state.CgbObjPalettes[objPal0Color]);

			int objPal1Color = (state.ObjPalette1 >> (i * 2)) & 0x03;
			info.RawPalette[i + 8] = objPal1Color;
			info.RgbPalette[i + 8] = ColorUtilities::Rgb555ToArgb(state.CgbObjPalettes[objPal1Color+4]);
		}
	}

	return info;
}

void GbPpuTools::SetPaletteColor(int32_t colorIndex, uint32_t color)
{
	GbPpuState state;
	_debugger->GetPpuState(state, CpuType::Gameboy);

	if(state.CgbEnabled) {
		uint8_t r = (color >> 19) & 0x1F;
		uint8_t g = (color >> 11) & 0x1F;
		uint8_t b = (color >> 3) & 0x1F;

		uint16_t rgb555 = (b << 10) | (g << 5) | r;

		if(colorIndex < 4 * 8) {
			state.CgbBgPalettes[colorIndex] = rgb555;
		} else if(colorIndex < 12*8) {
			state.CgbObjPalettes[colorIndex - 4*8] = rgb555;
		}
	} else {
		color &= 0x03;
		if(colorIndex < 4) {
			state.BgPalette &= ~(3 << (colorIndex * 2));
			state.BgPalette |= (color << (colorIndex * 2));
		} else if(colorIndex < 8) {
			state.ObjPalette0 &= ~(3 << (colorIndex * 2));
			state.ObjPalette0 |= (color << (colorIndex * 2));
		} else if(colorIndex < 12) {
			state.ObjPalette1 &= ~(3 << (colorIndex * 2));
			state.ObjPalette1 |= (color << (colorIndex * 2));
		}
	}
	_debugger->SetPpuState(state, CpuType::Gameboy);
}
