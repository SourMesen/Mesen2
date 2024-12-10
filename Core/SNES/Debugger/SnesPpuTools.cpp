#include "pch.h"
#include "SNES/Debugger/SnesPpuTools.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/MemoryDumper.h"
#include "Shared/SettingTypes.h"
#include "SNES/SnesPpu.h"
#include "Shared/ColorUtilities.h"
#include "Shared/MessageManager.h"

static constexpr uint8_t layerBpp[8][4] = {
	{ 2,2,2,2 }, { 4,4,2,0 }, { 4,4,0,0 }, { 8,4,0,0 }, { 8,2,0,0 }, { 4,2,0,0 }, { 4,0,0,0 }, { 8,8,0,0 }
};

SnesPpuTools::SnesPpuTools(Debugger* debugger, Emulator *emu) : PpuTools(debugger, emu)
{
}

void SnesPpuTools::GetPpuToolsState(BaseState& state)
{
	(SnesPpuToolsState&)state = _state;
}

void SnesPpuTools::SetPpuScanlineState(uint16_t scanline, uint8_t mode, int32_t mode7startX, int32_t mode7startY, int32_t mode7endX, int32_t mode7endY)
{
	_state.ScanlineBgMode[scanline] = mode;
	_state.Mode7StartX[scanline] = mode7startX;
	_state.Mode7StartY[scanline] = mode7startY;
	_state.Mode7EndX[scanline] = mode7endX;
	_state.Mode7EndY[scanline] = mode7endY;
}

void SnesPpuTools::SetPpuRowBuffers(uint16_t scanline, uint16_t xStart, uint16_t xEnd, uint16_t mainScreenRowBuffer[256], uint16_t subScreenRowBuffer[256])
{
	uint16_t len = xEnd - xStart + 1;
	if(mainScreenRowBuffer && subScreenRowBuffer) {
		memcpy(_mainScreenBuffer + scanline * 256 + xStart, mainScreenRowBuffer + xStart, len * sizeof(uint16_t));
		memcpy(_subScreenBuffer + scanline * 256 + xStart, subScreenRowBuffer + xStart, len * sizeof(uint16_t));
	} else {
		memset(_mainScreenBuffer + scanline * 256 + xStart, 0, len * sizeof(uint16_t));
		memset(_subScreenBuffer + scanline * 256 + xStart, 0, len * sizeof(uint16_t));
	}
}

DebugTilemapInfo SnesPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	SnesPpuState& state = (SnesPpuState&)baseState;
	FrameInfo outputSize = GetTilemapSize(options, state);

	bool directColor = state.DirectColorMode && (state.BgMode == 3 || state.BgMode == 4 || state.BgMode == 7);

	uint16_t basePaletteOffset = 0;
	if(state.BgMode == 0) {
		basePaletteOffset = options.Layer * 32;
	}

	if(options.Layer == SnesPpuTools::MainScreenViewLayer || options.Layer == SnesPpuTools::SubScreenViewLayer) {
		return RenderScreenView(options.Layer, outBuffer);
	}

	LayerConfig layer = state.Layers[options.Layer];

	std::fill(outBuffer, outBuffer + outputSize.Width*outputSize.Height, palette[0]);

	uint8_t bpp = layerBpp[state.BgMode][options.Layer];
	if(bpp == 0) {
		return {};
	}

	bool largeTileWidth = layer.LargeTiles || state.BgMode == 5 || state.BgMode == 6;
	bool largeTileHeight = layer.LargeTiles;
	int tileHeight = largeTileHeight ? 16 : 8;
	int tileWidth = largeTileWidth ? 16 : 8;
	int columnCount = layer.DoubleWidth ? 64 : 32;
	int rowCount = layer.DoubleHeight ? 64 : 32;
	TileFormat format = TileFormat::Bpp2;

	if(state.BgMode == 7) {
		tileHeight = 8;
		tileWidth = 8;
		columnCount = 128;
		rowCount = 128;
		if(options.Layer == 1) {
			format = TileFormat::Mode7ExtBg;
			RenderMode7Tilemap<TileFormat::Mode7ExtBg>(options, vram, outBuffer, palette);
		} else if(directColor) {
			format = TileFormat::Mode7DirectColor;
			RenderMode7Tilemap<TileFormat::Mode7DirectColor>(options, vram, outBuffer, palette);
		} else {
			format = TileFormat::Mode7;
			RenderMode7Tilemap<TileFormat::Mode7>(options, vram, outBuffer, palette);
		}
	} else {
		if(directColor) {
			format = TileFormat::DirectColor;
			RenderTilemap<TileFormat::DirectColor>(options, rowCount, layer, columnCount, vram, tileHeight, tileWidth, largeTileHeight, largeTileWidth, bpp, outBuffer, outputSize, palette, basePaletteOffset);
		} else {
			switch(bpp) {
				default:
				case 2:
					format = TileFormat::Bpp2;
					RenderTilemap<TileFormat::Bpp2>(options, rowCount, layer, columnCount, vram, tileHeight, tileWidth, largeTileHeight, largeTileWidth, bpp, outBuffer, outputSize, palette, basePaletteOffset);
					break;

				case 4:
					format = TileFormat::Bpp4;
					RenderTilemap<TileFormat::Bpp4>(options, rowCount, layer, columnCount, vram, tileHeight, tileWidth, largeTileHeight, largeTileWidth, bpp, outBuffer, outputSize, palette, basePaletteOffset);
					break;

				case 8:
					format = TileFormat::Bpp8;
					RenderTilemap<TileFormat::Bpp8>(options, rowCount, layer, columnCount, vram, tileHeight, tileWidth, largeTileHeight, largeTileWidth, bpp, outBuffer, outputSize, palette, basePaletteOffset);
					break;
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
	result.Format = format;
	result.TileWidth = tileWidth;
	result.TileHeight = tileHeight;
	result.ColumnCount = columnCount;
	result.RowCount = rowCount;
	result.TilemapAddress = state.BgMode == 7 ? 0 : (layer.TilemapAddress << 1);
	result.TilesetAddress = state.BgMode == 7 ? 0 : (layer.ChrAddress << 1);
	result.ScrollX = hScroll;
	result.ScrollY = vScroll;
	result.ScrollWidth = isDoubleWidthScreen ? 512 : 256;
	result.ScrollHeight = isDoubleHeightScreen ? height * 2 : height;
	return result;
}

template<TileFormat format>
void SnesPpuTools::RenderTilemap(GetTilemapOptions& options, int rowCount, LayerConfig& layer, int columnCount, uint8_t* vram, int tileHeight, int tileWidth, bool largeTileHeight, bool largeTileWidth, uint8_t bpp, uint32_t* outBuffer, FrameInfo outputSize, const uint32_t* palette, uint16_t basePaletteOffset)
{
	if(largeTileHeight) {
		if(largeTileWidth) {
			RenderTilemap<format, true, true>(options, rowCount, layer, columnCount, vram, tileHeight, tileWidth, bpp, outBuffer, outputSize, palette, basePaletteOffset);
		} else {
			RenderTilemap<format, true, false>(options, rowCount, layer, columnCount, vram, tileHeight, tileWidth, bpp, outBuffer, outputSize, palette, basePaletteOffset);
		}
	} else {
		if(largeTileWidth) {
			RenderTilemap<format, false, true>(options, rowCount, layer, columnCount, vram, tileHeight, tileWidth, bpp, outBuffer, outputSize, palette, basePaletteOffset);
		} else {
			RenderTilemap<format, false, false>(options, rowCount, layer, columnCount, vram, tileHeight, tileWidth, bpp, outBuffer, outputSize, palette, basePaletteOffset);
		}
	}
}

template<TileFormat format, bool largeTileHeight, bool largeTileWidth>
void SnesPpuTools::RenderTilemap(GetTilemapOptions& options, int rowCount, LayerConfig& layer, int columnCount, uint8_t* vram, int tileHeight, int tileWidth, uint8_t bpp, uint32_t* outBuffer, FrameInfo outputSize, const uint32_t* palette, uint16_t basePaletteOffset)
{
	uint8_t colorMask = 0xFF;
	bool grayscale = options.DisplayMode == TilemapDisplayMode::Grayscale;
	if(grayscale) {
		palette = bpp == 2 ? _grayscaleColorsBpp2 : _grayscaleColorsBpp4;
		colorMask = bpp == 2 ? 0x03 : 0x0F;
	}

	for(int row = 0; row < rowCount; row++) {
		uint16_t addrVerticalScrollingOffset = layer.DoubleHeight ? ((row & 0x20) << (layer.DoubleWidth ? 6 : 5)) : 0;
		uint16_t baseOffset = layer.TilemapAddress + addrVerticalScrollingOffset + ((row & 0x1F) << 5);

		for(int column = 0; column < columnCount; column++) {
			uint16_t addr = (baseOffset + (column & 0x1F) + (layer.DoubleWidth ? ((column & 0x20) << 5) : 0)) << 1;

			bool vMirror = (vram[addr + 1] & 0x80) != 0;
			bool hMirror = (vram[addr + 1] & 0x40) != 0;
			uint16_t tileIndex = ((vram[addr + 1] & 0x03) << 8) | vram[addr];

			for(int y = 0; y < tileHeight; y++) {
				uint8_t yOffset = vMirror ? (7 - (y & 0x07)) : (y & 0x07);
				uint8_t yTileOffset = (largeTileHeight ? ((y & 0x08) ? (vMirror ? 0 : 16) : (vMirror ? 16 : 0)) : 0);

				for(int x = 0; x < tileWidth; x++) {
					uint16_t tileOffset = (
						yTileOffset +
						(largeTileWidth ? ((x & 0x08) ? (hMirror ? 0 : 1) : (hMirror ? 1 : 0)) : 0)
					);

					uint16_t tileStart = (layer.ChrAddress << 1) + ((tileIndex + tileOffset) & 0x3FF) * 8 * bpp;
					uint16_t pixelStart = tileStart + yOffset * 2;

					uint8_t pixelIndex = hMirror ? (7 - (x & 0x07)) : (x & 0x07);
					uint8_t color = GetTilePixelColor<format>(vram, SnesPpu::VideoRamSize - 1, pixelStart, pixelIndex);
					if(color != 0) {
						uint8_t paletteIndex = bpp == 8 ? 0 : (vram[addr + 1] >> 2) & 0x07;
						int pos = ((row * tileHeight) + y) * outputSize.Width + column * tileWidth + x;
						outBuffer[pos] = grayscale ? palette[color & colorMask] : GetRgbPixelColor<format>(palette + basePaletteOffset, color, paletteIndex);
					}
				}
			}
		}
	}
}

template<TileFormat format>
void SnesPpuTools::RenderMode7Tilemap(GetTilemapOptions& options, uint8_t* vram, uint32_t* outBuffer, const uint32_t* palette)
{
	uint8_t colorMask = 0xFF;
	bool grayscale = options.DisplayMode == TilemapDisplayMode::Grayscale;
	if(grayscale) {
		palette = _grayscaleColorsBpp4;
		colorMask = 0x0F;
	}

	for(int row = 0; row < 1024; row++) {
		for(int column = 0; column < 128; column++) {
			uint32_t tileIndex = vram[(row>>3) * 256 + column * 2];
			uint32_t tileAddr = tileIndex * 128;
			uint32_t pixelStart = tileAddr + (row & 0x7) * 16;

			for(int x = 0; x < 8; x++) {
				uint8_t color = GetTilePixelColor<format>(vram, SnesPpu::VideoRamSize - 1, pixelStart, x);

				if(color != 0) {
					outBuffer[row * 1024 + column * 8 + x] = grayscale ? palette[color & colorMask] : GetRgbPixelColor<format>(palette, color, 0);
				}
			}
		}
	}
}

DebugTilemapInfo SnesPpuTools::RenderScreenView(uint8_t layer, uint32_t* outBuffer)
{
	uint16_t* src = layer == SnesPpuTools::MainScreenViewLayer ? _mainScreenBuffer : _subScreenBuffer;
	for(int i = 0; i < 256 * 239; i++) {
		outBuffer[i] = ColorUtilities::Rgb555ToArgb(src[i]);
	}
	return {};
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

void SnesPpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, DebugSpriteInfo* sprites, uint32_t* spritePreviews, uint32_t* palette, uint32_t* outBuffer)
{
	SnesPpuState& state = (SnesPpuState&)baseState;
	DebugSpritePreviewInfo size = GetSpritePreviewInfo(options, state, ppuToolsState);
	uint32_t bgColor = GetSpriteBackgroundColor(options.Background, palette, false);

	std::fill(outBuffer, outBuffer + size.Width * size.Height, GetSpriteBackgroundColor(options.Background, palette, true));
	for(int i = 0; i < (state.OverscanMode ? 239 : 224); i++) {
		std::fill(outBuffer + size.Width * i + 256, outBuffer + size.Width * i + 512, bgColor);
	}

	int startIndex = state.EnableOamPriority ? (state.OamRamAddress >> 1) : 0;
	for(int i = 0; i < 128; i++) {
		int spriteIndex = startIndex - i - 1;
		if(spriteIndex < 0) {
			spriteIndex += 128;
		}

		DebugSpriteInfo& sprite = sprites[spriteIndex];
		uint32_t* spritePreview = spritePreviews + spriteIndex * _spritePreviewSize;

		for(int y = 0; y < sprite.Height; y++) {
			int yPos = sprite.Y + y;
			if(yPos >= (int)size.Height) {
				yPos -= size.Height;
			}

			/*int yGap = (scanline - sprite.Y);
			if(state.ObjInterlace) {
				yGap <<= 1;
				yGap |= (state.FrameCount & 0x01);
			}*/

			for(int x = 0; x < sprite.Width; x++) {
				int xPos = 256 + sprite.X + x;
				
				uint32_t color = spritePreview[y * sprite.Width + x];
				if(color != 0) {
					if(xPos < (int)size.Width) {
						uint32_t outOffset = yPos * size.Width + xPos;
						outBuffer[outOffset] = color;
					}
				} else {
					spritePreview[y * sprite.Width + x] = bgColor;
				}
			}
		}
	}
}

void SnesPpuTools::GetSpriteInfo(DebugSpriteInfo& sprite, uint32_t* spritePreview, uint16_t spriteIndex, GetSpritePreviewOptions& options, SnesPpuState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette)
{
	uint16_t addr = (spriteIndex * 4) & 0x1FF;

	uint8_t highTableOffset = addr >> 4;
	uint8_t shift = ((addr >> 2) & 0x03) << 1;
	uint8_t highTableValue = oamRam[0x200 | highTableOffset] >> shift;
	uint8_t largeSprite = (highTableValue & 0x02) >> 1;
	uint8_t height = _oamSizes[state.OamMode][largeSprite][1] << 3;

	uint8_t width = _oamSizes[state.OamMode][largeSprite][0] << 3;
	uint16_t sign = (highTableValue & 0x01) << 8;
	int16_t spriteX = (int16_t)((sign | oamRam[addr]) << 7) >> 7;
	uint8_t spriteY = oamRam[addr + 1];
	uint8_t flags = oamRam[addr + 3];

	bool visible = true;
	if(spriteX + width <= 0 || spriteX > 255) {
		visible = false;
	} else {
		uint16_t scanlineCount = state.OverscanMode ? 239 : 224;
		uint16_t endY = spriteY + (state.ObjInterlace ? (height >> 1) : height);
		if((endY >= scanlineCount || endY == 256) && spriteY >= scanlineCount) {
			visible = false;
		}
	}

	bool useSecondTable = (flags & 0x01) != 0;

	sprite.Bpp = 4;
	sprite.Format = TileFormat::Bpp4;
	sprite.SpriteIndex = spriteIndex;
	sprite.UseExtendedVram = false;
	sprite.X = spriteX;
	sprite.Y = spriteY;
	sprite.RawX = sign | oamRam[addr];
	sprite.RawY = spriteY;
	sprite.Height = height;
	sprite.Width = width;
	sprite.TileIndex = oamRam[addr + 2];
	sprite.TileAddress = ((state.OamBaseAddress + (sprite.TileIndex << 4) + (useSecondTable ? state.OamAddressOffset : 0)) & 0x7FFF) << 1;
	sprite.Palette = ((flags >> 1) & 0x07);
	sprite.PaletteAddress = (sprite.Palette + 8) * 16;
	sprite.Priority = (DebugSpritePriority)((flags >> 4) & 0x03);
	bool horizontalMirror = (flags & 0x40) != 0;
	bool verticalMirror = (flags & 0x80) != 0;
	sprite.HorizontalMirror = horizontalMirror ? NullableBoolean::True : NullableBoolean::False;
	sprite.VerticalMirror = verticalMirror ? NullableBoolean::True : NullableBoolean::False;
	sprite.UseSecondTable = useSecondTable ? NullableBoolean::True : NullableBoolean::False;
	sprite.Visibility = visible ? SpriteVisibility::Visible : SpriteVisibility::Offscreen;

	int tileRow = (sprite.TileIndex & 0xF0) >> 4;
	int tileColumn = sprite.TileIndex & 0x0F;
	
	sprite.TileCount = 0;
	for(int i = 0, rowCount = height / 8; i < rowCount; i++) {
		int row = (i + tileRow) & 0x0F;
		for(int j = 0, columnCount = width / 8; j < columnCount; j++) {
			int col = (j + tileColumn) & 0x0F;
			sprite.TileAddresses[sprite.TileCount] = ((state.OamBaseAddress + (row * 16 + col) * 16 + (useSecondTable ? state.OamAddressOffset : 0)) & 0x7FFF) << 1;
			sprite.TileCount++;
		}
	}

	uint8_t yOffset;
	int rowOffset;

	for(int y = 0; y < sprite.Height; y++) {
		if(verticalMirror) {
			int pos;
			if(y < sprite.Width) {
				//Square sprites
				pos = sprite.Width - 1 - y;
			} else {
				//When using rectangular sprites (undocumented), vertical mirroring doesn't work properly
				//The top and bottom halves are mirrored separately and don't swap positions
				pos = sprite.Width * 3 - 1 - y;
			}
			yOffset = pos & 0x07;
			rowOffset = pos >> 3;
		} else {
			yOffset = y & 0x07;
			rowOffset = y >> 3;
		}

		uint8_t row = (tileRow + rowOffset) & 0x0F;

		for(int x = 0; x < sprite.Width; x++) {
			uint32_t outOffset = y * sprite.Width + x;

			uint8_t xOffset;
			int columnOffset;
			if(horizontalMirror) {
				xOffset = (sprite.Width - x - 1) & 0x07;
				columnOffset = (sprite.Width - x - 1) >> 3;
			} else {
				xOffset = x & 0x07;
				columnOffset = x >> 3;
			}

			uint8_t column = (tileColumn + columnOffset) & 0x0F;
			uint8_t tileIndex = (row << 4) | column;
			uint16_t tileStart = ((state.OamBaseAddress + (tileIndex << 4) + (useSecondTable ? state.OamAddressOffset : 0)) & 0x7FFF) << 1;

			uint8_t color = GetTilePixelColor<TileFormat::Bpp4>(vram, SnesPpu::VideoRamSize - 1, tileStart + yOffset * 2, xOffset);
			if(color != 0) {
				spritePreview[outOffset] = GetRgbPixelColor<TileFormat::Bpp4>(palette, color, sprite.Palette + 8);
			} else {
				spritePreview[outOffset] = 0;
			}
		}
	}
}

void SnesPpuTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[], uint32_t* spritePreviews, uint32_t* screenPreview)
{
	SnesPpuState& state = (SnesPpuState&)baseState;
	for(int i = 0; i < 128; i++) {
		outBuffer[i].Init();
		GetSpriteInfo(outBuffer[i], spritePreviews + (i * _spritePreviewSize), i, options, state, vram, oamRam, palette);
	}
	
	GetSpritePreview(options, baseState, ppuToolsState, outBuffer, spritePreviews, palette, screenPreview);
}

FrameInfo SnesPpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& baseState)
{
	SnesPpuState& state = (SnesPpuState&)baseState;

	if(options.Layer == SnesPpuTools::MainScreenViewLayer || options.Layer == SnesPpuTools::SubScreenViewLayer) {
		return { 256, 239 };
	}

	uint8_t bpp = layerBpp[state.BgMode][options.Layer];
	if(bpp == 0) {
		return {};
	}

	FrameInfo size = { 256, 256 };

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

DebugTilemapTileInfo SnesPpuTools::GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState)
{
	DebugTilemapTileInfo result = {};

	FrameInfo size = GetTilemapSize(options, baseState);
	if(x >= size.Width || y >= size.Height) {
		return result;
	} else if(options.Layer == SnesPpuTools::MainScreenViewLayer || options.Layer == SnesPpuTools::SubScreenViewLayer) {
		//No actual tiles to display for these views
		return result;
	}

	SnesPpuState& state = (SnesPpuState&)baseState;
	LayerConfig layer = state.Layers[options.Layer];

	uint8_t bpp = layerBpp[state.BgMode][options.Layer];
	if(bpp == 0) {
		return result;
	}

	uint32_t row;
	uint32_t column;

	if(state.BgMode == 7) {
		row = y / 8;
		column = x / 8;

		result.TileMapAddress = row * 256 + column * 2;
		result.TileIndex = vram[result.TileMapAddress];
		result.TileAddress = result.TileIndex * 128;
		result.Height = 8;
		result.Width = 8;
	} else {
		bool largeTileWidth = layer.LargeTiles || state.BgMode == 5 || state.BgMode == 6;
		bool largeTileHeight = layer.LargeTiles;

		row = y / (largeTileHeight ? 16 : 8);
		column = x / (largeTileWidth ? 16 : 8);

		uint16_t addrVerticalScrollingOffset = layer.DoubleHeight ? ((row & 0x20) << (layer.DoubleWidth ? 6 : 5)) : 0;
		uint16_t baseOffset = layer.TilemapAddress + addrVerticalScrollingOffset + ((row & 0x1F) << 5);

		uint16_t addr = (baseOffset + (column & 0x1F) + (layer.DoubleWidth ? ((column & 0x20) << 5) : 0)) << 1;

		result.Height = largeTileHeight ? 16 : 8;
		result.Width = largeTileWidth ? 16 : 8;
		result.VerticalMirroring = (NullableBoolean)((vram[addr + 1] & 0x80) != 0);
		result.HorizontalMirroring = (NullableBoolean)((vram[addr + 1] & 0x40) != 0);
		result.HighPriority = (NullableBoolean)((vram[addr + 1] & 0x20) != 0);
		result.PaletteIndex = bpp == 8 ? 0 : (vram[addr + 1] >> 2) & 0x07;
		if(state.BgMode == 0) {
			result.BasePaletteIndex = result.PaletteIndex;
			result.PaletteIndex += options.Layer * 8;
		}
		result.PaletteAddress = result.PaletteIndex * (1 << bpp);

		result.TileIndex = ((vram[addr + 1] & 0x03) << 8) | vram[addr];
		result.TileMapAddress = addr;
		
		uint16_t tileStart = (layer.ChrAddress << 1) + result.TileIndex * 8 * bpp;
		result.TileAddress = tileStart;

	}

	result.Row = row;
	result.Column = column;

	return result;
}

DebugSpritePreviewInfo SnesPpuTools::GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState)
{
	SnesPpuState& state = (SnesPpuState&)baseState;

	DebugSpritePreviewInfo info = {};
	info.Height = 256;
	info.Width = 512;
	info.SpriteCount = 128;
	info.CoordOffsetX = 256;
	info.CoordOffsetY = 0;
	
	info.VisibleX = 256;
	info.VisibleY = 0;
	info.VisibleWidth = 256;
	info.VisibleHeight = state.OverscanMode ? 239 : 224;

	info.WrapBottomToTop = true;

	return info;
}

DebugPaletteInfo SnesPpuTools::GetPaletteInfo(GetPaletteInfoOptions options)
{
	DebugPaletteInfo info = {};
	info.PaletteMemType = MemoryType::SnesCgRam;
	info.HasMemType = true;

	info.RawFormat = RawPaletteFormat::Rgb555;
	info.ColorsPerPalette = 16;
	info.BgColorCount = 16 * 8;
	info.SpritePaletteOffset = info.BgColorCount;
	info.SpriteColorCount = 16 * 8;
	info.ColorCount = info.BgColorCount + info.SpriteColorCount;

	switch(options.Format) {
		case TileFormat::DirectColor:
		case TileFormat::Mode7DirectColor:
			for(int i = 0; i < 256; i++) {
				info.RawPalette[i] = ((i & 0x07) << 2) | ((i & 0x38) << 4) | ((i & 0xC0) << 7);
				info.RgbPalette[i] = ColorUtilities::Rgb555ToArgb(info.RawPalette[i]);
			}
			break;

		default:
			uint8_t mask = options.Format == TileFormat::Mode7ExtBg ? 0x7F : 0xFF;
			uint8_t* cgram = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::SnesCgRam);
			for(int i = 0; i < 256; i++) {
				info.RawPalette[i] = cgram[(i & mask) * 2] | (cgram[(i & mask) * 2 + 1] << 8);
				info.RgbPalette[i] = ColorUtilities::Rgb555ToArgb(info.RawPalette[i]);
			}
			break;
	}
	return info;
}

void SnesPpuTools::SetPaletteColor(int32_t colorIndex, uint32_t color)
{
	uint8_t r = (color >> 19) & 0x1F;
	uint8_t g = (color >> 11) & 0x1F;
	uint8_t b = (color >> 3) & 0x1F;

	uint16_t rgb555 = (b << 10) | (g << 5) | r;
	_debugger->GetMemoryDumper()->SetMemoryValue(MemoryType::SnesCgRam, colorIndex * 2, rgb555 & 0xFF);
	_debugger->GetMemoryDumper()->SetMemoryValue(MemoryType::SnesCgRam, colorIndex * 2 + 1, (rgb555 >> 8));
}
