#include "stdafx.h"
#include "SNES/SnesDefaultVideoFilter.h"
#include "Gameboy/Debugger/GbPpuTools.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"
#include "Gameboy/GbTypes.h"
#include "Gameboy/GbConstants.h"

GbPpuTools::GbPpuTools(Debugger* debugger, Emulator *emu) : PpuTools(debugger, emu)
{
}

DebugTilemapInfo GbPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	GbPpuState& state = (GbPpuState&)baseState;
	uint32_t offset = options.Layer == 1 ? 0x1C00 : 0x1800;
	bool isCgb = state.CgbEnabled;

	uint16_t baseTile = state.BgTileSelect ? 0 : 0x1000;

	std::fill(outBuffer, outBuffer + 256 * 256, 0xFFFFFFFF);

	uint16_t vramMask = isCgb ? 0x3FFF : 0x1FFF;

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
					uint8_t shift = hMirror ? (x & 0x07) : (7 - (x & 0x07));
					uint8_t color = GetTilePixelColor(vram, vramMask, 2, pixelStart, shift, 1);

					outBuffer[((row * 8) + y) * 256 + column * 8 + x] = palette[bgPalette + color];
				}
			}
		}
	}

	DebugTilemapInfo result = {};
	result.Bpp = 2;
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

void GbPpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, uint32_t* outBuffer)
{
	GbPpuState& state = (GbPpuState&)baseState;

	std::fill(outBuffer, outBuffer + 256 * 256, 0xFF333311);
	for(int i = 16; i < 16 + 144; i++) {
		std::fill(outBuffer + i * 256 + 8, outBuffer + i * 256 + 168, 0xFF888866);
	}

	DebugSpriteInfo sprite;
	for(int i = 0; i < 0xA0; i += 4) {
		GetSpriteInfo(sprite, i / 4, options, state, vram, oamRam, palette);

		for(int y = 0; y < sprite.Height; y++) {
			if(sprite.Y + y > 256) {
				break;
			}

			for(int x = 0; x < sprite.Width; x++) {
				if(sprite.X + x >= 256) {
					break;
				}

				uint32_t color = sprite.SpritePreview[y * sprite.Width + x];
				if(color != 0) {
					outBuffer[((sprite.Y + y) * 256) + sprite.X + x] = color;
				}
			}
		}
	}
}

FrameInfo GbPpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& state)
{
	return { 256, 256 };
}

DebugTilemapTileInfo GbPpuTools::GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState)
{
	DebugTilemapTileInfo result;

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
		result.HorizontalMirroring = (NullableBoolean)((attributes & 0x20) != 0);
		result.VerticalMirroring = (NullableBoolean)((attributes & 0x40) != 0);
		result.HighPriority = (NullableBoolean)((attributes & 0x80) != 0);
	}

	return result;
}

DebugSpritePreviewInfo GbPpuTools::GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state)
{
	DebugSpritePreviewInfo info = {};
	info.Height = 256;
	info.Width = 256;
	info.SpriteCount = 40;
	info.CoordOffsetX = 0;
	info.CoordOffsetY = 0;
	return info;
}

void GbPpuTools::GetSpriteInfo(DebugSpriteInfo& sprite, uint16_t i, GetSpritePreviewOptions& options, GbPpuState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette)
{
	sprite.Bpp = 2;
	sprite.SpriteIndex = i;
	
	sprite.Y = oamRam[i*4];
	sprite.X = oamRam[i * 4 + 1];
	sprite.RawY = sprite.Y;
	sprite.RawX = sprite.X;

	sprite.TileIndex = oamRam[i * 4 + 2];
	uint8_t attributes = oamRam[i * 4 + 3];

	bool useSecondTable = (state.CgbEnabled && (attributes & 0x08));
	sprite.UseSecondTable = useSecondTable ? NullableBoolean::True : NullableBoolean::False;
	sprite.Palette = state.CgbEnabled ? (attributes & 0x07) : ((attributes & 0x10) ? 1 : 0);
	sprite.HorizontalMirror = (attributes & 0x20) != 0;
	sprite.VerticalMirror = (attributes & 0x40) != 0;
	sprite.Visible = sprite.X > 0 && sprite.Y > 0 && sprite.Y < 160 && sprite.X < 168;
	sprite.Width = 8;
	sprite.Height = state.LargeSprites ? 16 : 8;
	sprite.Priority = (attributes & 0x80) ? DebugSpritePriority::Background : DebugSpritePriority::Foreground;
	
	uint8_t tileIndex = (uint8_t)sprite.TileIndex;
	uint16_t tileBank = useSecondTable ? 0x2000 : 0x0000;
	if(state.LargeSprites) {
		tileIndex &= 0xFE;
	}

	uint16_t tileStart = tileIndex * 16;
	tileStart |= tileBank;

	sprite.TileAddress = tileStart;

	for(int y = 0; y < sprite.Height; y++) {
		uint16_t pixelStart = tileStart + (sprite.VerticalMirror ? (sprite.Height - 1 - y) : y) * 2;
		bool isCgb = state.CgbEnabled;
		for(int x = 0; x < sprite.Width; x++) {
			uint8_t shift = sprite.HorizontalMirror ? (x & 0x07) : (7 - (x & 0x07));
			uint8_t color = GetTilePixelColor(vram, 0x3FFF, 2, pixelStart, shift, 1);

			uint32_t outOffset = (y * sprite.Width) + x;
			if(color > 0) {
				if(!isCgb) {
					sprite.SpritePreview[outOffset] = palette[4 + (sprite.Palette * 4) + color];
				} else {
					sprite.SpritePreview[outOffset] = palette[32 + (sprite.Palette * 4) + color];
				}
			} else {
				sprite.SpritePreview[outOffset] = 0;
			}
		}
	}
}

void GbPpuTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[])
{
	GbPpuState& state = (GbPpuState&)baseState;
	for(int i = 0; i < 40; i++) {
		outBuffer[i].Init();
		GetSpriteInfo(outBuffer[i], i, options, state, vram, oamRam, palette);
	}
}

DebugPaletteInfo GbPpuTools::GetPaletteInfo()
{
	DebugPaletteInfo info = {};
	GbPpuState state;
	_debugger->GetPpuState(state, CpuType::Gameboy);

	if(state.CgbEnabled) {
		info.RawFormat = RawPaletteFormat::Rgb555;
		info.ColorsPerPalette = 8;
		info.BgColorCount = 8 * 4;
		info.SpriteColorCount = 8 * 4;
		info.ColorCount = info.BgColorCount + info.SpriteColorCount;

		for(int i = 0; i < 8 * 4; i++) {
			info.RawPalette[i] = state.CgbBgPalettes[i];
			info.RgbPalette[i] = SnesDefaultVideoFilter::ToArgb(state.CgbBgPalettes[i]);
		}
		for(int i = 0; i < 8 * 4; i++) {
			info.RawPalette[i+32] = state.CgbObjPalettes[i];
			info.RgbPalette[i+32] = SnesDefaultVideoFilter::ToArgb(state.CgbObjPalettes[i]);
		}
	} else {
		info.RawFormat = RawPaletteFormat::Indexed;
		info.ColorsPerPalette = 4;
		info.BgColorCount = 4;
		info.SpriteColorCount = 2 * 4;
		info.ColorCount = info.BgColorCount + info.SpriteColorCount;

		for(int i = 0; i < 4; i++) {
			info.RawPalette[i] = (state.BgPalette >> (i * 2)) & 0x03;
			info.RgbPalette[i] = SnesDefaultVideoFilter::ToArgb(state.CgbBgPalettes[i]);

			int objPal0Color = (state.ObjPalette0 >> (i * 2)) & 0x03;
			info.RawPalette[i + 4] = objPal0Color;
			info.RgbPalette[i + 4] = SnesDefaultVideoFilter::ToArgb(state.CgbObjPalettes[objPal0Color]);

			int objPal1Color = (state.ObjPalette1 >> (i * 2)) & 0x03;
			info.RawPalette[i + 8] = objPal1Color;
			info.RgbPalette[i + 8] = SnesDefaultVideoFilter::ToArgb(state.CgbObjPalettes[objPal1Color+4]);
		}
	}

	return info;
}
