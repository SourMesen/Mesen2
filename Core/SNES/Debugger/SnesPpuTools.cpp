#include "stdafx.h"
#include "SNES/Debugger/SnesPpuTools.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"
#include "SNES/SnesDefaultVideoFilter.h"
#include "SNES/Ppu.h"

SnesPpuTools::SnesPpuTools(Debugger* debugger, Emulator *emu) : PpuTools(debugger, emu)
{
}

void SnesPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	PpuState& state = (PpuState&)baseState;
	FrameInfo outputSize = GetTilemapSize(options, state);

	static constexpr uint8_t layerBpp[8][4] = {
		{ 2,2,2,2 }, { 4,4,2,0 }, { 4,4,0,0 }, { 8,4,0,0 }, { 8,2,0,0 }, { 4,2,0,0 }, { 4,0,0,0 }, { 8,0,0,0 }
	};

	bool directColor = state.DirectColorMode && (state.BgMode == 3 || state.BgMode == 4 || state.BgMode == 7);

	uint16_t basePaletteOffset = 0;
	if(state.BgMode == 0) {
		basePaletteOffset = options.Layer * 64;
	}

	LayerConfig layer = state.Layers[options.Layer];

	std::fill(outBuffer, outBuffer + outputSize.Width*outputSize.Height, palette[0]);

	uint8_t bpp = layerBpp[state.BgMode][options.Layer];
	if(bpp == 0) {
		return;
	}

	bool largeTileWidth = layer.LargeTiles || state.BgMode == 5 || state.BgMode == 6;
	bool largeTileHeight = layer.LargeTiles;

	if(state.BgMode == 7) {
		for(int row = 0; row < 128; row++) {
			for(int column = 0; column < 128; column++) {
				uint32_t tileIndex = vram[row * 256 + column * 2];
				uint32_t tileAddr = tileIndex * 128;

				for(int y = 0; y < 8; y++) {
					uint32_t pixelStart = tileAddr + y * 16;

					for(int x = 0; x < 8; x++) {
						uint8_t color = vram[pixelStart + x * 2 + 1];

						if(color != 0) {
							uint32_t rgbColor;
							if(directColor) {
								rgbColor = SnesDefaultVideoFilter::ToArgb(((color & 0x07) << 2) | ((color & 0x38) << 4) | ((color & 0xC0) << 7));
							} else {
								rgbColor = GetRgbPixelColor(palette, color, 0, 8, false, 0);
							}
							outBuffer[((row * 8) + y) * outputSize.Width + column * 8 + x] = rgbColor;
						}
					}
				}
			}
		}
	} else {
		int tileHeight = largeTileHeight ? 16 : 8;
		int tileWidth = largeTileWidth ? 16 : 8;
		for(int row = 0; row < (layer.DoubleHeight ? 64 : 32); row++) {
			uint16_t addrVerticalScrollingOffset = layer.DoubleHeight ? ((row & 0x20) << (layer.DoubleWidth ? 6 : 5)) : 0;
			uint16_t baseOffset = layer.TilemapAddress + addrVerticalScrollingOffset + ((row & 0x1F) << 5);

			for(int column = 0; column < (layer.DoubleWidth ? 64 : 32); column++) {
				uint16_t addr = (baseOffset + (column & 0x1F) + (layer.DoubleWidth ? ((column & 0x20) << 5) : 0)) << 1;

				bool vMirror = (vram[addr + 1] & 0x80) != 0;
				bool hMirror = (vram[addr + 1] & 0x40) != 0;
				uint16_t tileIndex = ((vram[addr + 1] & 0x03) << 8) | vram[addr];

				for(int y = 0; y < tileHeight; y++) {
					uint8_t yOffset = vMirror ? (7 - (y & 0x07)) : (y & 0x07);

					for(int x = 0; x < tileWidth; x++) {
						uint16_t tileOffset = (
							(largeTileHeight ? ((y & 0x08) ? (vMirror ? 0 : 16) : (vMirror ? 16 : 0)) : 0) +
							(largeTileWidth ? ((x & 0x08) ? (hMirror ? 0 : 1) : (hMirror ? 1 : 0)) : 0)
						);

						uint16_t tileStart = (layer.ChrAddress << 1) + ((tileIndex + tileOffset) & 0x3FF) * 8 * bpp;
						uint16_t pixelStart = tileStart + yOffset * 2;

						uint8_t shift = hMirror ? (x & 0x07) : (7 - (x & 0x07));
						uint8_t color = GetTilePixelColor(vram, Ppu::VideoRamSize - 1, bpp, pixelStart, shift, 1);
						if(color != 0) {
							uint8_t paletteIndex = bpp == 8 ? 0 : (vram[addr + 1] >> 2) & 0x07;
							outBuffer[((row * tileHeight) + y) * outputSize.Width + column * tileWidth + x] = GetRgbPixelColor(palette, color, paletteIndex, bpp, directColor, basePaletteOffset);
						}
					}
				}
			}
		}
	}
}

static constexpr uint8_t _oamSizes[8][2][2] = {
	{ { 1, 1 }, { 2, 2 } }, //8x8 + 16x16
	{ { 1, 1 }, { 4, 4 } }, //8x8 + 32x32
	{ { 1, 1 }, { 8, 8 } }, //8x8 + 64x64
	{ { 2, 2 }, { 4, 4 } }, //16x16 + 32x32
	{ { 2, 2 }, { 8, 8 } }, //16x16 + 64x64
	{ { 4, 4 }, { 8, 8 } }, //32x32 + 64x64
	{ { 2, 4 }, { 4, 8 } }, //16x32 + 32x64
	{ { 2, 4 }, { 4, 4 } }  //16x32 + 32x32
};

void SnesPpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, uint8_t *vram, uint8_t *oamRam, uint32_t* palette, uint32_t* outBuffer)
{
	PpuState& state = (PpuState&)baseState;
	FrameInfo size = GetSpritePreviewSize(options, state);
	//TODO
	//uint16_t baseAddr = state.EnableOamPriority ? (_internalOamAddress & 0x1FC) : 0;
	uint16_t baseAddr = 0;

	bool filled[512*256] = {};
	std::fill(outBuffer, outBuffer + size.Width * size.Height, 0xFF333333);
	for(int i = 0; i < (state.OverscanMode ? 239 : 224); i++) {
		std::fill(outBuffer + size.Width * i + 256, outBuffer + size.Width * i + 512, 0xFF888888);
	}

	for(int scanline = 0; scanline < (int)size.Height; scanline++) {
		for(int i = 508; i >= 0; i -= 4) {
			DebugSpriteInfo sprite = GetSpriteInfo(i / 4, options, state, oamRam);
			
			uint8_t endY = (sprite.Y + (state.ObjInterlace ? (sprite.Height >> 1): sprite.Height)) & 0xFF;
			bool visible = (scanline >= sprite.Y && scanline < endY) || (endY < sprite.Y && scanline < endY);
			if(!visible) {
				//Not visible on this scanline
				continue;
			}

			int tileRow = (sprite.TileIndex & 0xF0) >> 4;
			int tileColumn = sprite.TileIndex & 0x0F;
		
			uint8_t yOffset;
			int rowOffset;

			int yGap = (scanline - sprite.Y);
			if(state.ObjInterlace) {
				yGap <<= 1;
				yGap |= (state.FrameCount & 0x01);
			}

			if(sprite.VerticalMirror) {
				yOffset = (sprite.Height - 1 - yGap) & 0x07;
				rowOffset = (sprite.Height - 1 - yGap) >> 3;
			} else {
				yOffset = yGap & 0x07;
				rowOffset = yGap >> 3;
			}

			uint8_t row = (tileRow + rowOffset) & 0x0F;

			for(int j = sprite.X; j < sprite.X + sprite.Width && j < 256; j++) {
				uint32_t outOffset = scanline * size.Width + (256 + j);
				if(filled[outOffset]) {
					continue;
				}

				uint8_t xOffset;
				int columnOffset;
				if(sprite.HorizontalMirror) {
					xOffset = (sprite.Width - (j - sprite.X) - 1) & 0x07;
					columnOffset = (sprite.Width - (j - sprite.X) - 1) >> 3;
				} else {
					xOffset = (j - sprite.X) & 0x07;
					columnOffset = (j - sprite.X) >> 3;
				}

				uint8_t column = (tileColumn + columnOffset) & 0x0F;
				uint8_t tileIndex = (row << 4) | column;
				uint16_t tileStart = ((state.OamBaseAddress + (tileIndex << 4) + (sprite.UseSecondTable ? state.OamAddressOffset : 0)) & 0x7FFF) << 1;

				uint8_t color = GetTilePixelColor(vram, Ppu::VideoRamSize - 1, 4, tileStart + yOffset * 2, 7 - xOffset, 1);
				if(color != 0) {
					if(options.SelectedSprite == i / 4) {
						filled[outOffset] = true;
					}
					outBuffer[outOffset] = GetRgbPixelColor(palette, color, sprite.Palette + 8, 4, false, 0);
				}
			}
		}
	}
}

DebugSpriteInfo SnesPpuTools::GetSpriteInfo(uint16_t spriteIndex, GetSpritePreviewOptions& options, PpuState& state, uint8_t* oamRam)
{
	uint16_t addr = (spriteIndex * 4) & 0x1FF;

	uint8_t highTableOffset = addr >> 4;
	uint8_t shift = ((addr >> 2) & 0x03) << 1;
	uint8_t highTableValue = oamRam[0x200 | highTableOffset] >> shift;
	uint8_t largeSprite = (highTableValue & 0x02) >> 1;
	uint8_t height = _oamSizes[state.OamMode][largeSprite][1] << 3;

	uint8_t width = _oamSizes[state.OamMode][largeSprite][0] << 3;
	uint16_t sign = (highTableValue & 0x01) << 8;
	int16_t x = (int16_t)((sign | oamRam[addr]) << 7) >> 7;
	uint8_t y = oamRam[addr + 1];
	uint8_t flags = oamRam[addr + 3];

	bool visible = true;
	if(x + width <= 0 || x > 255) {
		visible = false;
	} else {
		uint16_t scanlineCount = state.OverscanMode ? 239 : 224;
		uint8_t endY = (y + (state.ObjInterlace ? (height >> 1) : height)) & 0xFF;
		if(endY >= scanlineCount && y >= scanlineCount) {
			visible = false;
		}
	}

	DebugSpriteInfo sprite = {};
	sprite.SpriteIndex = spriteIndex;
	sprite.X = x;
	sprite.Y = y;
	sprite.Height = height;
	sprite.Width = width;
	sprite.TileIndex = oamRam[addr + 2];
	sprite.Palette = ((flags >> 1) & 0x07);
	sprite.Priority = (flags >> 4) & 0x03;
	sprite.HorizontalMirror = (flags & 0x40) != 0;
	sprite.VerticalMirror = (flags & 0x80) != 0;
	sprite.UseSecondTable = (flags & 0x01) != 0;
	sprite.Visible = visible;
	return sprite;	
}

uint32_t SnesPpuTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* oamRam, DebugSpriteInfo outBuffer[])
{
	PpuState& state = (PpuState&)baseState;
	for(int i = 0; i < 128; i++) {
		outBuffer[i] = GetSpriteInfo(i, options, state, oamRam);
	}
	return 128;
}

FrameInfo SnesPpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& baseState)
{
	FrameInfo size = { 256, 256 };

	PpuState& state = (PpuState&)baseState;
	if(state.BgMode == 7) {
		return { 1024, 1024 };
	}

	LayerConfig layer = state.Layers[options.Layer];
	bool largeTileWidth = layer.LargeTiles || state.BgMode == 5 || state.BgMode == 6;
	bool largeTileHeight = layer.LargeTiles;

	if(largeTileHeight) {
		size.Height *= 2;
	}
	if(layer.DoubleHeight) {
		size.Height *= 2;
	}

	if(largeTileWidth) {
		size.Width *= 2;
	}
	if(layer.DoubleWidth) {
		size.Width *= 2;
	}

	return size;
}

FrameInfo SnesPpuTools::GetSpritePreviewSize(GetSpritePreviewOptions options, BaseState& state)
{
	return { 512, 256 };
}
