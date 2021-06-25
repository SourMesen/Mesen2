#include "stdafx.h"
#include "Gameboy/Debugger/GbPpuTools.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"
#include "Gameboy/GbTypes.h"

GbPpuTools::GbPpuTools(Debugger* debugger, Emulator *emu) : PpuTools(debugger, emu)
{
}

void GbPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	GbPpuState& state = (GbPpuState&)baseState;
	int offset = options.Layer == 1 ? 0x1C00 : 0x1800;
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
}

void GbPpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, uint32_t* outBuffer)
{
	GbPpuState& state = (GbPpuState&)baseState;

	std::fill(outBuffer, outBuffer + 256 * 256, 0xFF333311);
	for(int i = 16; i < 16 + 144; i++) {
		std::fill(outBuffer + i * 256 + 8, outBuffer + i * 256 + 168, 0xFF888866);
	}

	bool isCgb = state.CgbEnabled;
	bool largeSprites = state.LargeSprites;
	uint8_t height = largeSprites ? 16 : 8;
	constexpr uint8_t width = 8;

	uint8_t filled[256];
	for(int row = 0; row < 256; row++) {
		std::fill(filled, filled + 256, 0xFF);

		for(int i = 0; i < 0xA0; i += 4) {
			DebugSpriteInfo sprite = GetSpriteInfo(i / 4, options, state, oamRam);
			if(sprite.Y > row || sprite.Y + height <= row) {
				continue;
			}

			int y = row - sprite.Y;
			uint8_t tileIndex = (uint8_t)sprite.TileIndex;
			uint16_t tileBank = sprite.UseSecondTable ? 0x2000 : 0x0000;
			if(largeSprites) {
				tileIndex &= 0xFE;
			}

			uint16_t tileStart = tileIndex * 16;
			tileStart |= tileBank;

			uint16_t pixelStart = tileStart + (sprite.VerticalMirror ? (height - 1 - y) : y) * 2;
			for(int x = 0; x < width; x++) {
				if(sprite.X + x >= 256) {
					break;
				} else if(filled[sprite.X + x] < sprite.X) {
					continue;
				}

				uint8_t shift = sprite.HorizontalMirror ? (x & 0x07) : (7 - (x & 0x07));
				uint8_t color = GetTilePixelColor(vram, 0x3FFF, 2, pixelStart, shift, 1);

				if(color > 0) {
					uint32_t outOffset = (row * 256) + sprite.X + x;
					if(!isCgb) {
						outBuffer[outOffset] = palette[4 + (sprite.Palette * 4) + color];
					} else {
						outBuffer[outOffset] = palette[32 + (sprite.Palette * 4) + color];
					}
					filled[sprite.X + x] = (uint8_t)sprite.X;
				}
			}
		}
	}
}

FrameInfo GbPpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& state)
{
	return { 256, 256 };
}

FrameInfo GbPpuTools::GetSpritePreviewSize(GetSpritePreviewOptions options, BaseState& state)
{
	return { 256, 256 };
}

DebugSpriteInfo GbPpuTools::GetSpriteInfo(uint16_t i, GetSpritePreviewOptions& options, GbPpuState& state, uint8_t* oamRam)
{
	DebugSpriteInfo sprite = {};
	
	sprite.SpriteIndex = i;
	sprite.Y = oamRam[i*4];

	sprite.X = oamRam[i * 4 + 1];
	sprite.TileIndex = oamRam[i * 4 + 2];
	uint8_t attributes = oamRam[i * 4 + 3];

	sprite.UseSecondTable = (state.CgbEnabled && (attributes & 0x08)) ? true : false;
	sprite.Palette = state.CgbEnabled ? (attributes & 0x07) : ((attributes & 0x10) ? 1 : 0);
	sprite.HorizontalMirror = (attributes & 0x20) != 0;
	sprite.VerticalMirror = (attributes & 0x40) != 0;
	sprite.Visible = sprite.X > 0 && sprite.Y > 0 && sprite.Y < 160 && sprite.X < 168;
	sprite.Width = 8;
	sprite.Height = state.LargeSprites ? 16 : 8;

	return sprite;
}

uint32_t GbPpuTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* oamRam, DebugSpriteInfo outBuffer[])
{
	GbPpuState& state = (GbPpuState&)baseState;
	for(int i = 0; i < 40; i++) {
		outBuffer[i] = GetSpriteInfo(i, options, state, oamRam);
	}
	return 40;
}
