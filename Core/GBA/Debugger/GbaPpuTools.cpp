#include "pch.h"
#include "GBA/Debugger/GbaPpuTools.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/MemoryDumper.h"
#include "Shared/SettingTypes.h"
#include "GBA/GbaTypes.h"
#include "Shared/ColorUtilities.h"

GbaPpuTools::GbaPpuTools(Debugger* debugger, Emulator *emu) : PpuTools(debugger, emu)
{
}

void GbaPpuTools::SetMemoryAccessData(uint16_t scanline, uint8_t* data)
{
	memcpy(_memoryAccess + scanline * 308 * 4, data, 308 * 4);
}

DebugTilemapInfo GbaPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	GbaPpuState& state = (GbaPpuState&)baseState;
	FrameInfo outputSize = GetTilemapSize(options, state);

	if(options.Layer == 4) {
		DebugTilemapInfo result = {};
		result.TileHeight = 1;
		result.TileWidth = 1;
		result.RowCount = 228;
		result.ColumnCount = 308*4;
		result.Bpp = 8;
		result.Format = TileFormat::DirectColor;
		for(int i = 0; i < sizeof(_memoryAccess); i++) {
			outBuffer[i] = (_memoryAccess[i] & GbaPpuMemAccess::Vram) ? 0xFFA00000 : 0xFF000000;
			outBuffer[i] |= (_memoryAccess[i] & GbaPpuMemAccess::VramObj) ? 0xFF5F0000 : 0xFF000000;
			outBuffer[i] |= (_memoryAccess[i] & GbaPpuMemAccess::Oam) ? 0xFF00FF00 : 0xFF000000;
			outBuffer[i] |= (_memoryAccess[i] & GbaPpuMemAccess::Palette) ? 0xFF0000FF : 0xFF000000;
		}
		return result;
	}

	uint8_t colorMask = 0xFF;
	bool grayscale = options.DisplayMode == TilemapDisplayMode::Grayscale;
	if(grayscale) {
		palette = (uint32_t*)_grayscaleColorsBpp4;
		colorMask = 0x0F;
	}
	
	std::fill(outBuffer, outBuffer + outputSize.Width * outputSize.Height, palette[0]);

	DebugTilemapInfo result = {};

	switch(state.BgMode) {
		case 0: case 1: case 2:
		{
			GbaBgConfig& layer = state.BgLayers[options.Layer];

			bool transformEnabled = (state.BgMode != 0 && options.Layer >= 2);
			bool bpp8Mode = layer.Bpp8Mode || transformEnabled;
			
			result.ScrollX = transformEnabled ? 0 : layer.ScrollX;
			result.ScrollY = transformEnabled ? 0 : layer.ScrollY;
			result.ScrollWidth = transformEnabled ? 0 : 240;
			result.ScrollHeight = transformEnabled ? 0 : 160;

			result.Bpp = bpp8Mode ? 8 : 4;
			result.Format = bpp8Mode ? TileFormat::GbaBpp8 : TileFormat::GbaBpp4;
			result.TileWidth = 8;
			result.TileHeight = 8;
			if(transformEnabled) {
				uint16_t screenSize = 128 << layer.ScreenSize;
				result.ColumnCount = screenSize / 8;
				result.RowCount = screenSize / 8;
			} else {
				result.ColumnCount = layer.DoubleWidth ? 64 : 32;
				result.RowCount = layer.DoubleHeight ? 64 : 32;
			}
			result.TilemapAddress = layer.TilemapAddr;
			result.TilesetAddress = layer.TilesetAddr;
			result.Priority = layer.Priority;

			for(uint32_t row = 0; row < result.RowCount; row++) {
				for(uint32_t column = 0; column < result.ColumnCount; column++) {
					uint16_t vramAddr;
					if(transformEnabled) {
						vramAddr = layer.TilemapAddr + row * result.ColumnCount + column;
					} else {
						vramAddr = layer.TilemapAddr + (((row & 0x1F) * 32 + (column & 0x1F)) * 2);
						if(column >= 32) {
							vramAddr += 0x800;
						}
						if(row >= 32) {
							vramAddr += layer.DoubleWidth ? 0x1000 : 0x800;
						}
					}

					uint16_t tileData;
					if(transformEnabled) {
						tileData = vram[vramAddr];
					} else {
						tileData = vram[vramAddr] | (vram[vramAddr + 1] << 8);
					}

					uint16_t tileIndex = tileData & 0x3FF;
					bool hMirror = tileData & (1 << 10);
					bool vMirror = tileData & (1 << 11);
					uint8_t paletteIndex = (tileData >> 12) & 0x0F;

					for(int y = 0; y < 8; y++) {
						uint8_t tileRow = vMirror ? 7 - y : y;
						for(int x = 0; x < 8; x++) {
							uint8_t tileColumn = hMirror ? 7 - x : x;

							uint8_t color = 0;
							if(bpp8Mode) {
								color = vram[(layer.TilesetAddr + tileIndex * 64 + tileRow * 8 + tileColumn) & 0xFFFF];
								paletteIndex = 0;
							} else {
								color = vram[(layer.TilesetAddr + tileIndex * 32 + tileRow * 4 + (tileColumn >> 1)) & 0xFFFF];
								if(tileColumn & 0x01) {
									color >>= 4;
								} else {
									color &= 0x0F;
								}
							}

							if(color) {
								int pos = ((row * 8) + y) * outputSize.Width + column * 8 + x;
								outBuffer[pos] = palette[((paletteIndex * 16) + color) & colorMask];
							}
						}
					}
				}
			}
			break;
		}

		case 3:
		{
			if(options.Layer != 2) {
				break;
			}

			result.ScrollX = 0;
			result.ScrollY = 0;
			result.Bpp = 16;
			result.Format = TileFormat::DirectColor;
			result.TileWidth = 1;
			result.TileHeight = 1;
			result.ColumnCount = 240;
			result.RowCount = 160 * 2;
			result.TilemapAddress = 0;
			result.TilesetAddress = 0;
			result.ScrollWidth = 0;
			result.ScrollHeight = 0;

			for(int i = 0; i < GbaConstants::PixelCount; i++) {
				outBuffer[i] = ColorUtilities::Rgb555ToArgb(vram[i * 2] | (vram[i * 2 + 1] << 8));
			}
			break;
		}

		case 4:
		{
			if(options.Layer != 2) {
				break;
			}

			result.ScrollX = 0;
			result.ScrollY = state.DisplayFrameSelect ? 160 : 0;
			result.Bpp = 8;
			result.Format = TileFormat::GbaBpp8;
			result.TileWidth = 1;
			result.TileHeight = 1;
			result.ColumnCount = 240;
			result.RowCount = 160*2;
			result.TilemapAddress = 0;
			result.TilesetAddress = 0;
			result.ScrollWidth = 240;
			result.ScrollHeight = 160;

			for(int i = 0; i < GbaConstants::PixelCount * 2; i++) {
				outBuffer[i] = palette[vram[i]];
			}
			break;
		}

		case 5:
		{
			if(options.Layer != 2) {
				break;
			}

			result.ScrollX = 0;
			result.ScrollY = state.DisplayFrameSelect ? 128 : 0;
			result.Bpp = 16;
			result.Format = TileFormat::DirectColor;
			result.TileWidth = 1;
			result.TileHeight = 1;
			result.ColumnCount = 160;
			result.RowCount = 128*2;
			result.TilemapAddress = 0;
			result.TilesetAddress = 0;
			result.ScrollWidth = 160;
			result.ScrollHeight = 128;

			for(int i = 0; i < 160*128*2; i++) {
				outBuffer[i] = ColorUtilities::Rgb555ToArgb(vram[i * 2] | (vram[i * 2 + 1] << 8));
			}
			break;
		}

		default:
			break;
	}

	return result;
}

FrameInfo GbaPpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& baseState)
{
	if(options.Layer == 4) {
		return { 308 * 4, 228 };
	}

	GbaPpuState& state = (GbaPpuState&)baseState;
	switch(state.BgMode) {
		case 0: case 1: case 2: {
			GbaBgConfig& layer = state.BgLayers[options.Layer];
			bool transformEnabled = (state.BgMode != 0 && options.Layer >= 2);
			if(transformEnabled) {
				uint32_t screenSize = 128 << layer.ScreenSize;
				return { screenSize, screenSize };
			} else {
				return { 256u * (layer.DoubleWidth ? 2 : 1), 256u * (layer.DoubleHeight ? 2 : 1) };
			}
		}

		case 3:
			if(options.Layer == 2) {
				return { 240, 160 };
			}
			break;

		case 4:
			if(options.Layer == 2) {
				return { 240, 160 * 2 };
			}
			break;

		case 5: 
			if(options.Layer == 2) {
				return { 160, 128 * 2 };
			}
			break;
	}

	return { 0, 0 };
}

DebugTilemapTileInfo GbaPpuTools::GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState)
{
	DebugTilemapTileInfo result = {};

	FrameInfo size = GetTilemapSize(options, baseState);

	if(x >= size.Width || y >= size.Height || options.Layer == 4) {
		return result;
	}

	GbaPpuState& state = (GbaPpuState&)baseState;
	bool transformEnabled = (state.BgMode != 0 && options.Layer >= 2);

	int32_t width = -1;
	int32_t height = -1;
	int32_t column = -1;
	int32_t row = -1;
	int32_t tilemapAddr = -1;
	int32_t pixelData = -1;
	int32_t tileIndex = -1;
	int32_t tileStart = -1;
	int32_t paletteIndex = -1;
	NullableBoolean hMirror = NullableBoolean::Undefined;
	NullableBoolean vMirror = NullableBoolean::Undefined;
	
	switch(state.BgMode) {
		case 0:
		case 1:
		case 2: {
			row = y / 8;
			column = x / 8;
			width = 8;
			height = 8;

			int offset = state.BgLayers[options.Layer].TilemapAddr;
			if(transformEnabled) {
				uint16_t screenSize = 128 << state.BgLayers[options.Layer].ScreenSize;
				uint16_t columnCount = screenSize >> 3;
				tilemapAddr = offset + row * columnCount + column;
			} else {
				tilemapAddr = offset + (((row & 0x1F) << 5) + column) * 2;
				if(column >= 32) {
					tilemapAddr += 0x800;
				}
				if(row >= 32) {
					tilemapAddr += size.Width > 256 ? 0x1000 : 0x800;
				}
			}

			uint16_t tileData;
			if(transformEnabled) {
				tileData = vram[tilemapAddr];
			} else {
				tileData = vram[tilemapAddr] | (vram[tilemapAddr + 1] << 8);
			}

			bool bpp8Mode = transformEnabled || state.BgLayers[options.Layer].Bpp8Mode;
			tileIndex = tileData & 0x3FF;
			if(!transformEnabled) {
				paletteIndex = (tileData >> 12) & 0x0F;
				hMirror = (NullableBoolean)(tileData & (1 << 10));
				vMirror = (NullableBoolean)(tileData & (1 << 11));
			}

			tileStart = state.BgLayers[options.Layer].TilesetAddr + (tileIndex * (bpp8Mode ? 64 : 32));
			break;
		}

		case 3:
		case 4:
		case 5: {
			uint16_t screenWidth = state.BgMode == 5 ? 160 : 240;

			column = x;
			row = y;
			width = 1;
			height = 1;
			
			if(state.BgMode == 3 || state.BgMode == 5) {
				tilemapAddr = (y * screenWidth + x) * 2;
				pixelData = vram[tilemapAddr] | (vram[tilemapAddr + 1] << 8);
			} else {
				tilemapAddr = (y * screenWidth + x);
				pixelData = vram[tilemapAddr];
			}
			break;
		}
	}
	
	result.Column = column;
	result.Row = row;
	result.Height = height;
	result.Width = width;
	result.TileMapAddress = tilemapAddr;
	result.TileIndex = tileIndex;
	result.TileAddress = tileStart;
	result.PixelData = pixelData;
	result.PaletteIndex = paletteIndex;
	result.PaletteAddress = paletteIndex << 5;
	result.HorizontalMirroring = hMirror;
	result.VerticalMirroring = vMirror;
	return result;
}


void GbaPpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, DebugSpriteInfo* sprites, uint32_t* spritePreviews, uint32_t* palette, uint32_t* outBuffer)
{
	uint32_t bgColor = GetSpriteBackgroundColor(options.Background, palette, false);

	vector<uint8_t> priority(512 * 256);
	std::fill(priority.begin(), priority.end(), 4);

	std::fill(outBuffer, outBuffer + 512 * 256, GetSpriteBackgroundColor(options.Background, palette, true));
	for(int i = 0; i < 160; i++) {
		std::fill(outBuffer + i * 512, outBuffer + i * 512 + 240, bgColor);
	}

	for(int i = 0; i < 128; i++) {
		DebugSpriteInfo& sprite = sprites[i];
		uint32_t* spritePreview = spritePreviews + i * _spritePreviewSize;

		for(int y = 0; y < sprite.Height; y++) {
			uint8_t outputRow = (uint8_t)(sprite.Y + y);
			for(int x = 0; x < sprite.Width; x++) {
				uint16_t drawPos = (sprite.X + x) & 0x1FF;
				uint32_t pos = (outputRow * 512) + drawPos;

				uint32_t color = spritePreview[y * sprite.Width + x];
				if(color != 0) {
					if((uint8_t)sprite.Priority < priority[pos]) {
						priority[pos] = (uint8_t)sprite.Priority;
						outBuffer[pos] = color;
					}
				} else {
					spritePreview[y * sprite.Width + x] = bgColor;
				}
			}
		}
	}
}

DebugSpritePreviewInfo GbaPpuTools::GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state, BaseState& ppuToolsState)
{
	DebugSpritePreviewInfo info = {};
	info.Height = 256;
	info.Width = 512;
	info.SpriteCount = 128;
	info.CoordOffsetX = 0;
	info.CoordOffsetY = 0;

	info.VisibleX = 0;
	info.VisibleY = 0;
	info.VisibleWidth = 240;
	info.VisibleHeight = 160;
	info.WrapRightToLeft = true;
	info.WrapBottomToTop = true;
	return info;
}

void GbaPpuTools::GetSpriteInfo(DebugSpriteInfo& sprite, uint32_t* spritePreview, uint16_t i, GetSpritePreviewOptions& options, GbaPpuState& state, uint8_t* vram, uint8_t* oam, uint32_t* palette)
{
	uint32_t addr = i * 8;
	sprite.Y = oam[addr];
	sprite.X = oam[addr + 2] | ((oam[addr + 3] & 0x01) << 8);

	sprite.RawY = sprite.Y;
	sprite.RawX = sprite.X;
	
	bool bpp8Mode = oam[addr + 1] & 0x20;
	sprite.Bpp = bpp8Mode ? 8 : 4;
	sprite.Format = bpp8Mode ? TileFormat::GbaBpp8 : TileFormat::GbaBpp4;

	bool hMirror = false;
	bool vMirror = false;

	uint8_t shape = (oam[addr + 1] >> 6) & 0x03;
	bool transformEnabled = oam[addr + 1] & 0x01;
	bool doubleSize = transformEnabled && (oam[addr + 1] & 0x02);
	bool spriteHidden = !transformEnabled && (oam[addr + 1] & 0x02);
	sprite.TransformEnabled = transformEnabled ? NullableBoolean::True : NullableBoolean::False;
	if(transformEnabled) {
		sprite.TransformParamIndex = (oam[addr + 3] >> 1) & 0x1F;
		sprite.DoubleSize = doubleSize ? NullableBoolean::True : NullableBoolean::False;

		//Clear all pixels first, since the drawing logic won't always draw on each pixel
		std::fill(spritePreview, spritePreview + _spritePreviewSize, 0);
	} else {
		hMirror = oam[addr + 3] & 0x10;
		vMirror = oam[addr + 3] & 0x20;
		sprite.HorizontalMirror = hMirror ? NullableBoolean::True : NullableBoolean::False;
		sprite.VerticalMirror = vMirror ? NullableBoolean::True : NullableBoolean::False;
	}

	sprite.MosaicEnabled = oam[addr + 1] & 0x10 ? NullableBoolean::True : NullableBoolean::False;

	GbaPpuObjMode mode = (GbaPpuObjMode)((oam[addr + 1] >> 2) & 0x03);
	sprite.BlendingEnabled = mode == GbaPpuObjMode::Blending ? NullableBoolean::True : NullableBoolean::False;
	sprite.WindowMode = mode == GbaPpuObjMode::Window ? NullableBoolean::True : NullableBoolean::False;

	uint8_t size = (oam[addr + 3] >> 6) & 0x03;

	sprite.TileIndex = oam[addr + 4] | ((oam[addr + 5] & 0x03) << 8);
	sprite.Priority = (DebugSpritePriority)((oam[addr + 5] >> 2) & 0x03);
	sprite.Palette = bpp8Mode ? 0 : (oam[addr + 5] >> 4) & 0x0F;
	sprite.PaletteAddress = bpp8Mode ? -1 : (sprite.Palette << 5);

	static constexpr uint8_t sprSize[4][4][2] = {
		{ { 8, 8 }, { 16, 8 }, { 8, 16 }, { 8, 8 } },
		{ { 16, 16 }, { 32, 8 }, { 8, 32 }, { 16, 16 } },
		{ { 32, 32 }, { 32, 16 }, { 16, 32 }, { 32, 32 } },
		{ { 64, 64 }, { 64, 32 }, { 32, 64 }, { 64, 64 } }
	};

	uint8_t width = sprSize[size][shape][0];
	uint8_t height = sprSize[size][shape][1];
	sprite.Width = width << (doubleSize ? 1 : 0);
	sprite.Height = height << (doubleSize ? 1 : 0);

	sprite.SpriteIndex = i;
	sprite.UseExtendedVram = false;
	
	if(spriteHidden || mode == GbaPpuObjMode::Invalid) {
		sprite.Visibility = SpriteVisibility::Disabled;
	} else {
		uint16_t x1 = sprite.X;
		uint16_t x2 = (sprite.X + sprite.Width - 1) & 0x1FF;
		uint8_t y1 = sprite.Y;
		uint8_t y2 = (sprite.Y + sprite.Height - 1) & 0xFF;
		sprite.Visibility = ((x1 < 240 || x2 < 240) && (y1 < 160 || y2 < 160)) ? SpriteVisibility::Visible : SpriteVisibility::Offscreen;
	}
	
	sprite.TileCount = (height / 8) * (width / 8);
	
	if(state.ObjVramMappingOneDimension) {
		for(uint32_t j = 0; j < sprite.TileCount; j++) {
			sprite.TileAddresses[j] = 0x10000 | (((sprite.TileIndex * 32) + (j * 32 * (bpp8Mode ? 2 : 1))) & 0x7FFF);
		}
	} else {
		int count = 0;
		for(int y = 0; y < height / 8; y++) {
			for(int x = 0; x < width / 8; x++) {
				uint32_t tileIndex = (sprite.TileIndex & ~0x1F) + y * 0x20 + (((sprite.TileIndex & 0x1F) + (bpp8Mode ? x * 2 : x)) & 0x1F);
				sprite.TileAddresses[count++] = 0x10000 | ((tileIndex * 32) & 0x7FFF);
			}
		}
	}

	sprite.TileAddress = sprite.TileAddresses[0];

	int16_t pa = 0;
	int16_t pc = 0;
	int32_t xValue = 0;
	int32_t yValue = 0;
	int16_t centerX = 0;
	int16_t centerY = 0;

	for(int y = 0; y < sprite.Height; y++) {
		uint8_t yOffset = vMirror ? (sprite.Height - y - 1) : y;

		if(transformEnabled) {
			uint16_t transformAddr = (sprite.TransformParamIndex << 5) + 6;
			pa = oam[transformAddr] | (oam[transformAddr + 1] << 8);
			int16_t pb = oam[transformAddr + 8] | (oam[transformAddr + 9] << 8);
			pc = oam[transformAddr + 16] | (oam[transformAddr + 17] << 8);
			int16_t pd = oam[transformAddr + 24] | (oam[transformAddr + 25] << 8);

			centerX = width / 2;
			centerY = height / 2;

			int32_t originX = -(centerX << (uint8_t)doubleSize);
			int32_t originY = -(centerY << (uint8_t)doubleSize);

			xValue = originX * pa + (originY + yOffset) * pb;
			yValue = originX * pc + (originY + yOffset) * pd;
		}

		uint8_t tilesPerRow = (width / 8) * (bpp8Mode ? 2 : 1);

		uint32_t xRender = 0;
		uint32_t yRender = 0;
		for(int x = 0; x < sprite.Width; x++) {
			if(transformEnabled) {
				xRender = (xValue >> 8) + centerX;
				yRender = (yValue >> 8) + centerY;
				xValue += pa;
				yValue += pc;
			} else {
				xRender = hMirror ? (width - x - 1) : x;
				yRender = vMirror ? (height - yOffset - 1) : yOffset;
			}

			if(xRender >= width || yRender >= height) {
				continue;
			}

			uint8_t tileColumn = xRender / 8;
			uint8_t tileRow = yRender / 8;

			uint16_t index;
			if(state.ObjVramMappingOneDimension) {
				index = sprite.TileIndex + tileRow * tilesPerRow + (bpp8Mode ? tileColumn * 2 : tileColumn);
			} else {
				index = (sprite.TileIndex & ~0x1F) + tileRow * 0x20 + (((sprite.TileIndex & 0x1F) + (bpp8Mode ? tileColumn * 2 : tileColumn)) & 0x1F);
			}

			uint8_t color = 0;
			if(bpp8Mode) {
				uint32_t tileAddr = (index * 32 + (yRender & 0x07) * 8 + (xRender & 0x07)) & 0x7FFF;
				color = vram[0x10000 + tileAddr];
			} else {
				uint32_t tileAddr = (index * 32 + (yRender & 0x07) * 4 + ((xRender & 0x07) >> 1)) & 0x7FFF;
				color = vram[0x10000 + tileAddr];
				if(xRender & 0x01) {
					color >>= 4;
				} else {
					color &= 0x0F;
				}
			}

			uint32_t outOffset = (y * sprite.Width) + x;
			
			if(color != 0) {
				spritePreview[outOffset] = palette[0x100 + (sprite.Palette * 16) + color];
			} else {
				spritePreview[outOffset] = 0;
			}
		}
	}
}

void GbaPpuTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[], uint32_t* spritePreviews, uint32_t* screenPreview)
{
	GbaPpuState& state = (GbaPpuState&)baseState;
	for(int i = 0; i < 128; i++) {
		outBuffer[i].Init();
		GetSpriteInfo(outBuffer[i], spritePreviews+i*_spritePreviewSize, i, options, state, vram, oamRam, palette);
	}

	GetSpritePreview(options, baseState, outBuffer, spritePreviews, palette, screenPreview);
}

DebugPaletteInfo GbaPpuTools::GetPaletteInfo(GetPaletteInfoOptions options)
{
	DebugPaletteInfo info = {};
	info.PaletteMemType = MemoryType::GbaPaletteRam;
	info.HasMemType = true;

	info.RawFormat = RawPaletteFormat::Rgb555;
	info.ColorsPerPalette = 16;
	info.BgColorCount = 256;
	info.SpritePaletteOffset = info.BgColorCount;
	info.SpriteColorCount = 256;
	info.ColorCount = info.BgColorCount + info.SpriteColorCount;

	uint8_t* palette = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::GbaPaletteRam);
	for(int i = 0; i < 512; i++) {
		info.RawPalette[i] = palette[i * 2] | (palette[i * 2 + 1] << 8);
		info.RgbPalette[i] = ColorUtilities::Rgb555ToArgb(info.RawPalette[i]);
	}
	return info;
}

void GbaPpuTools::SetPaletteColor(int32_t colorIndex, uint32_t color)
{
	uint8_t r = (color >> 19) & 0x1F;
	uint8_t g = (color >> 11) & 0x1F;
	uint8_t b = (color >> 3) & 0x1F;

	uint16_t rgb555 = (b << 10) | (g << 5) | r;
	_debugger->GetMemoryDumper()->SetMemoryValue(MemoryType::GbaPaletteRam, colorIndex * 2, rgb555 & 0xFF);
	_debugger->GetMemoryDumper()->SetMemoryValue(MemoryType::GbaPaletteRam, colorIndex * 2 + 1, (rgb555 >> 8));
}
