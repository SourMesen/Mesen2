#include "stdafx.h"
#include "PCE/Debugger/PcePpuTools.h"
#include "PCE/PceConsole.h"
#include "PCE/PceDefaultVideoFilter.h"
#include "PCE/PceTypes.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/MemoryDumper.h"
#include "Shared/SettingTypes.h"

PcePpuTools::PcePpuTools(Debugger* debugger, Emulator *emu, PceConsole* console) : PpuTools(debugger, emu)
{
}

DebugTilemapInfo PcePpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	DebugTilemapInfo result = {};
	result.Bpp = 2;
	result.Format = TileFormat::Bpp4;
	result.TileWidth = 8;
	result.TileHeight = 8;
	result.ColumnCount = 64;
	result.RowCount = 60;
	result.TilemapAddress = 0x2000;
	result.TilesetAddress = 0;
	result.ScrollWidth = 256;
	result.ScrollHeight = 240;

	return result;
}

void PcePpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, uint32_t* outBuffer)
{
	PcePpuState& state = (PcePpuState&)baseState;

	std::fill(outBuffer, outBuffer + 1024*1024, 0xFF666666);
	for(int i = 64; i < 240 + 64; i++) {
		std::fill(outBuffer + i * 1024 + 32, outBuffer + i * 1024 + 32 + 256, 0xFF666666);
	}

	DebugSpriteInfo sprite;
	for(int i = 0; i < 64; i++) {
		GetSpriteInfo(sprite, i, options, state, vram, oamRam, palette);

		for(int y = 0; y < sprite.Height; y++) {
			if(sprite.Y + y >= 1024) {
				break;
			}

			for(int x = 0; x < sprite.Width; x++) {
				if(sprite.X + x >= 1024) {
					break;
				}

				uint32_t color = sprite.SpritePreview[y * sprite.Width + x];
				if(color != 0) {
					outBuffer[((sprite.Y + y) * 1024) + sprite.X + x] = color;
				}
			}
		}
	}
}

FrameInfo PcePpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& state)
{
	return { 512, 480 };
}

DebugTilemapTileInfo PcePpuTools::GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState)
{
	return DebugTilemapTileInfo();
}

void PcePpuTools::GetSpriteInfo(DebugSpriteInfo& sprite, uint16_t spriteIndex, GetSpritePreviewOptions& options, PcePpuState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette)
{
	uint16_t addr = (spriteIndex * 8);

	uint16_t spriteY = (oamRam[addr] | (oamRam[addr+1] << 8)) & 0x3FF;
	uint16_t spriteX = (oamRam[addr + 2] | (oamRam[addr + 3] << 8)) & 0x3FF;
	uint16_t patternCode = (oamRam[addr + 4] | (oamRam[addr + 5] << 8)) & 0x7FF;
	uint16_t tileIndex = patternCode >> 1;
	uint16_t flags = (oamRam[addr + 6] | (oamRam[addr + 7] << 8));
	
	uint8_t width = (flags & 0x100) ? 32 : 16;
	uint8_t height;
	switch((flags >> 12) & 0x03) {
		case 0: height = 16; break;
		case 1: height = 32; break;
		
		case 2:
		case 3:
			height = 64;
			break;
	}

	bool visible = true;//TODO

	sprite.Bpp = 4;
	sprite.Format = TileFormat::Bpp4;
	sprite.SpriteIndex = spriteIndex;
	sprite.X = spriteX;
	sprite.Y = spriteY;
	sprite.RawX = spriteX;
	sprite.RawY = spriteY;
	sprite.Height = height;
	sprite.Width = width;
	sprite.TileIndex = tileIndex;
	sprite.TileAddress = tileIndex * 64;
	sprite.Palette = (flags & 0x0F);
	sprite.PaletteAddress = (sprite.Palette + 16) * 16;
	sprite.Priority = (flags & 0x80) ? DebugSpritePriority::Foreground : DebugSpritePriority::Background;
	sprite.HorizontalMirror = (flags & 0x800) != 0;
	sprite.VerticalMirror = (flags & 0x8000) != 0;
	sprite.Visible = visible;

	if(width == 32) {
		tileIndex &= ~0x01;
	}
	if(height == 32) {
		tileIndex &= ~0x02;
	} else if(height == 64) {
		tileIndex &= ~0x06;
	}

	uint8_t yOffset;
	int rowOffset;

	for(int y = 0; y < sprite.Height; y++) {
		if(sprite.VerticalMirror) {
			yOffset = (sprite.Height - y - 1) & 0x0F;
			rowOffset = (sprite.Height - y - 1) >> 4;
		} else {
			yOffset = y & 0x0F;
			rowOffset = y >> 4;
		}

		uint16_t spriteTileY = tileIndex | (rowOffset << 1);

		for(int x = 0; x < sprite.Width; x++) {
			uint32_t outOffset = y * sprite.Width + x;

			uint8_t xOffset;
			int columnOffset;
			if(sprite.HorizontalMirror) {
				xOffset = (sprite.Width - x - 1) & 0x0F;
				columnOffset = (sprite.Width - x - 1) >> 4;
			} else {
				xOffset = x & 0x0F;
				columnOffset = x >> 4;
			}

			uint16_t spriteTile = spriteTileY | columnOffset;
			uint16_t tileStart = spriteTile * 64;

			uint16_t pixelStart = tileStart + yOffset;
			uint8_t shift = 15 - xOffset;

			//TODO FIX
			uint16_t* ram = (uint16_t*)vram;

			uint8_t color = (((ram[(pixelStart + 0) & 0xFFFF] >> shift) & 0x01) << 0);
			color |= (((ram[(pixelStart + 16) & 0xFFFF] >> shift) & 0x01) << 1);
			color |= (((ram[(pixelStart + 32) & 0xFFFF] >> shift) & 0x01) << 2);
			color |= (((ram[(pixelStart + 48) & 0xFFFF] >> shift) & 0x01) << 3);

			if(color != 0) {
				sprite.SpritePreview[outOffset] = palette[color + sprite.Palette * 16 + 16*16];
			} else {
				sprite.SpritePreview[outOffset] = 0;
			}
		}
	}
}

void PcePpuTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[])
{
	PcePpuState& state = (PcePpuState&)baseState;
	for(int i = 0; i < 64; i++) {
		outBuffer[i].Init();
		GetSpriteInfo(outBuffer[i], i, options, state, vram, oamRam, palette);
	}
}

DebugSpritePreviewInfo PcePpuTools::GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state)
{
	DebugSpritePreviewInfo info = {};
	info.Height = 1024;
	info.Width = 1024;
	info.SpriteCount = 64;
	info.CoordOffsetX = 0;
	info.CoordOffsetY = 0;
	return info;
}

DebugPaletteInfo PcePpuTools::GetPaletteInfo()
{
	DebugPaletteInfo info = {};
	info.RawFormat = RawPaletteFormat::Rgb333;
	info.ColorsPerPalette = 16;
	info.BgColorCount = 16 * 16;
	info.SpriteColorCount = 16 * 16;
	info.ColorCount = info.BgColorCount + info.SpriteColorCount;

	uint8_t* pal = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::PcePaletteRam);
	for(int i = 0; i < 512; i++) {
		info.RawPalette[i] = pal[i * 2] | (pal[i * 2 + 1] << 8);
		info.RgbPalette[i] = PceDefaultVideoFilter::ToArgb(info.RawPalette[i]);
	}

	return info;
}
