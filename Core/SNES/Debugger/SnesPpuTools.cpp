#include "stdafx.h"
#include "SNES/Debugger/SnesPpuTools.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/MemoryDumper.h"
#include "Shared/SettingTypes.h"
#include "SNES/SnesDefaultVideoFilter.h"
#include "SNES/SnesPpu.h"

static constexpr uint8_t layerBpp[8][4] = {
	{ 2,2,2,2 }, { 4,4,2,0 }, { 4,4,0,0 }, { 8,4,0,0 }, { 8,2,0,0 }, { 4,2,0,0 }, { 4,0,0,0 }, { 8,0,0,0 }
};

SnesPpuTools::SnesPpuTools(Debugger* debugger, Emulator *emu) : PpuTools(debugger, emu)
{
}

DebugTilemapInfo SnesPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	SnesPpuState& state = (SnesPpuState&)baseState;
	FrameInfo outputSize = GetTilemapSize(options, state);

	bool directColor = state.DirectColorMode && (state.BgMode == 3 || state.BgMode == 4 || state.BgMode == 7);

	uint16_t basePaletteOffset = 0;
	if(state.BgMode == 0) {
		basePaletteOffset = options.Layer * 64;
	}

	LayerConfig layer = state.Layers[options.Layer];

	std::fill(outBuffer, outBuffer + outputSize.Width*outputSize.Height, palette[0]);

	uint8_t bpp = layerBpp[state.BgMode][options.Layer];
	if(bpp == 0) {
		return {};
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
						uint8_t color = GetTilePixelColor(vram, SnesPpu::VideoRamSize - 1, bpp, pixelStart, shift, 1);
						if(color != 0) {
							uint8_t paletteIndex = bpp == 8 ? 0 : (vram[addr + 1] >> 2) & 0x07;
							outBuffer[((row * tileHeight) + y) * outputSize.Width + column * tileWidth + x] = GetRgbPixelColor(palette, color, paletteIndex, bpp, directColor, basePaletteOffset);
						}
					}
				}
			}
		}
	}

	int hScroll = state.BgMode == 7 ? state.Mode7.HScroll : layer.HScroll;
	int vScroll = state.BgMode == 7 ? state.Mode7.VScroll : layer.VScroll;
	int height = state.OverscanMode ? 239 : 224;

	bool isDoubleWidthScreen = state.HiResMode || state.BgMode == 5 || state.BgMode == 6;
	bool isDoubleHeightScreen = state.ScreenInterlace || state.BgMode == 5 || state.BgMode == 6;
	
	DebugTilemapInfo result = {};
	result.Bpp = bpp;
	result.ScrollX = hScroll;
	result.ScrollY = vScroll;
	result.ScrollWidth = isDoubleWidthScreen ? 512 : 256;
	result.ScrollHeight = isDoubleHeightScreen ? height * 2 : height;
	return result;
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
	SnesPpuState& state = (SnesPpuState&)baseState;
	DebugSpritePreviewInfo size = GetSpritePreviewInfo(options, state);
	//TODO
	//uint16_t baseAddr = state.EnableOamPriority ? (_internalOamAddress & 0x1FC) : 0;
	uint16_t baseAddr = 0;

	bool filled[512*256] = {};
	std::fill(outBuffer, outBuffer + size.Width * size.Height, 0xFF333333);
	for(int i = 0; i < (state.OverscanMode ? 239 : 224); i++) {
		std::fill(outBuffer + size.Width * i + 256, outBuffer + size.Width * i + 512, 0xFF888888);
	}
	
	DebugSpriteInfo sprite;
	for(int i = 508; i >= 0; i -= 4) {
		GetSpriteInfo(sprite, i / 4, options, state, vram, oamRam, palette);

		for(int y = 0; y < sprite.Height; y++) {
			int yPos = sprite.Y + y;
			if(yPos >= (int)size.Height) {
				break;
			}

			/*int yGap = (scanline - sprite.Y);
			if(state.ObjInterlace) {
				yGap <<= 1;
				yGap |= (state.FrameCount & 0x01);
			}*/

			for(int x = 0; x < sprite.Width; x++) {
				int xPos = 256 + sprite.X + x;
				if(xPos >= (int)size.Width) {
					break;
				}

				uint32_t color = sprite.SpritePreview[y * sprite.Width + x];
				if(color != 0) {
					uint32_t outOffset = yPos * size.Width + xPos;
					outBuffer[outOffset] = color;
				}
			}
		}
	}
}

void SnesPpuTools::GetSpriteInfo(DebugSpriteInfo& sprite, uint16_t spriteIndex, GetSpritePreviewOptions& options, SnesPpuState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette)
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

	bool useSecondTable = (flags & 0x01) != 0;

	sprite.Bpp = 4;
	sprite.SpriteIndex = spriteIndex;
	sprite.X = x;
	sprite.Y = y;
	sprite.Height = height;
	sprite.Width = width;
	sprite.TileIndex = oamRam[addr + 2];
	sprite.TileAddress = ((state.OamBaseAddress + (sprite.TileIndex << 4) + (useSecondTable ? state.OamAddressOffset : 0)) & 0x7FFF) << 1;
	sprite.Palette = ((flags >> 1) & 0x07);
	sprite.PaletteAddress = (sprite.Palette + 8) * 16;
	sprite.Priority = (DebugSpritePriority)((flags >> 4) & 0x03);
	sprite.HorizontalMirror = (flags & 0x40) != 0;
	sprite.VerticalMirror = (flags & 0x80) != 0;
	sprite.UseSecondTable = useSecondTable ? NullableBoolean::True : NullableBoolean::False;
	sprite.Visible = visible;

	int tileRow = (sprite.TileIndex & 0xF0) >> 4;
	int tileColumn = sprite.TileIndex & 0x0F;
	uint8_t yOffset;
	int rowOffset;

	for(int y = 0; y < sprite.Height; y++) {
		if(sprite.VerticalMirror) {
			yOffset = (sprite.Height - y - 1) & 0x07;
			rowOffset = (sprite.Height - y - 1) >> 3;
		} else {
			yOffset = y & 0x07;
			rowOffset = y >> 3;
		}

		uint8_t row = (tileRow + rowOffset) & 0x0F;

		for(int x = 0; x < sprite.Width; x++) {
			uint32_t outOffset = y * sprite.Width + x;

			uint8_t xOffset;
			int columnOffset;
			if(sprite.HorizontalMirror) {
				xOffset = (sprite.Width - x - 1) & 0x07;
				columnOffset = (sprite.Width - x - 1) >> 3;
			} else {
				xOffset = x & 0x07;
				columnOffset = x >> 3;
			}

			uint8_t column = (tileColumn + columnOffset) & 0x0F;
			uint8_t tileIndex = (row << 4) | column;
			uint16_t tileStart = ((state.OamBaseAddress + (tileIndex << 4) + (useSecondTable ? state.OamAddressOffset : 0)) & 0x7FFF) << 1;

			uint8_t color = GetTilePixelColor(vram, SnesPpu::VideoRamSize - 1, 4, tileStart + yOffset * 2, 7 - xOffset, 1);
			if(color != 0) {
				sprite.SpritePreview[outOffset] = GetRgbPixelColor(palette, color, sprite.Palette + 8, 4, false, 0);
			} else {
				sprite.SpritePreview[outOffset] = 0;
			}
		}
	}
}

void SnesPpuTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[])
{
	SnesPpuState& state = (SnesPpuState&)baseState;
	for(int i = 0; i < 128; i++) {
		outBuffer[i].Init();
		GetSpriteInfo(outBuffer[i], i, options, state, vram, oamRam, palette);
	}
}

FrameInfo SnesPpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& baseState)
{
	FrameInfo size = { 256, 256 };

	SnesPpuState& state = (SnesPpuState&)baseState;
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

DebugTilemapTileInfo SnesPpuTools::GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState)
{
	DebugTilemapTileInfo result;

	FrameInfo size = GetTilemapSize(options, baseState);
	if(x >= size.Width || y >= size.Height) {
		return result;
	}

	SnesPpuState& state = (SnesPpuState&)baseState;
	LayerConfig layer = state.Layers[options.Layer];

	uint8_t bpp = layerBpp[state.BgMode][options.Layer];
	if(bpp == 0) {
		return result;
	}

	uint16_t basePaletteOffset = 0;
	if(state.BgMode == 0) {
		basePaletteOffset = options.Layer * 64;
	}

	uint32_t row = y / 8;
	uint32_t column = x / 8;

	result.Row = row;
	result.Column = column;

	if(state.BgMode == 7) {
		result.TileMapAddress = row * 256 + column * 2;
		result.TileIndex = vram[result.TileMapAddress];
		result.TileAddress = result.TileIndex * 128;
		result.Height = 8;
		result.Width = 8;
	} else {
		bool largeTileWidth = layer.LargeTiles || state.BgMode == 5 || state.BgMode == 6;
		bool largeTileHeight = layer.LargeTiles;

		uint16_t addrVerticalScrollingOffset = layer.DoubleHeight ? ((row & 0x20) << (layer.DoubleWidth ? 6 : 5)) : 0;
		uint16_t baseOffset = layer.TilemapAddress + addrVerticalScrollingOffset + ((row & 0x1F) << 5);

		uint16_t addr = (baseOffset + (column & 0x1F) + (layer.DoubleWidth ? ((column & 0x20) << 5) : 0)) << 1;

		result.Height = largeTileHeight ? 16 : 8;
		result.Width = largeTileWidth ? 16 : 8;
		result.VerticalMirroring = (NullableBoolean)((vram[addr + 1] & 0x80) != 0);
		result.HorizontalMirroring = (NullableBoolean)((vram[addr + 1] & 0x40) != 0);
		result.HighPriority = (NullableBoolean)((vram[addr + 1] & 0x20) != 0);
		result.PaletteIndex = bpp == 8 ? 0 : (vram[addr + 1] >> 2) & 0x07;
		result.TileIndex = ((vram[addr + 1] & 0x03) << 8) | vram[addr];
		result.TileMapAddress = addr;
		
		uint16_t tileStart = (layer.ChrAddress << 1) + result.TileIndex * 8 * bpp;
		result.TileAddress = tileStart;

		result.PaletteAddress = basePaletteOffset + (result.PaletteIndex * (1 << bpp));
	}

	return result;
}

DebugSpritePreviewInfo SnesPpuTools::GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state)
{
	DebugSpritePreviewInfo info = {};
	info.Height = 256;
	info.Width = 512;
	info.SpriteCount = 128;
	info.CoordOffsetX = 256;
	info.CoordOffsetY = 0;
	return info;
}

DebugPaletteInfo SnesPpuTools::GetPaletteInfo()
{
	DebugPaletteInfo info = {};
	info.BgColorCount = 16 * 8;
	info.SpriteColorCount = 16 * 8;
	info.ColorCount = info.BgColorCount + info.SpriteColorCount;

	uint8_t* cgram= _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::SnesCgRam);
	for(int i = 0; i < 256; i++) {
		info.RawPalette[i] = cgram[i*2] | (cgram[i*2+1] << 8);
		info.RgbPalette[i] = SnesDefaultVideoFilter::ToArgb(info.RawPalette[i]);
	}

	return info;
}
