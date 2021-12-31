#include "stdafx.h"
#include "NES/Debugger/NesPpuTools.h"
#include "NES/Mappers/MMC5.h"
#include "NES/NesConsole.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"

NesPpuTools::NesPpuTools(Debugger* debugger, Emulator *emu, NesConsole* console) : PpuTools(debugger, emu)
{
	_mapper = console->GetMapper();
}

DebugTilemapInfo NesPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	MMC5* mmc5 = dynamic_cast<MMC5*>(_mapper);
	NesPpuState& state = (NesPpuState&)baseState;
	uint16_t bgAddr = (state.ControlReg & 0x10) ? 0x1000 : 0;

	uint8_t* prevVram = options.CompareVram != nullptr ? options.CompareVram : vram;
	constexpr uint32_t grayscalePalette[4] = { 0xFF000000, 0xFF808080, 0xFFC0C0C0, 0xFFFFFFFF };
	for(int nametableIndex = 0; nametableIndex < 4; nametableIndex++) {
		uint16_t baseAddr = 0x2000 + nametableIndex * 0x400;
		uint16_t baseAttributeAddr = baseAddr + 960;
		uint32_t bufferOffset = ((nametableIndex & 0x01) ? 256 : 0) + ((nametableIndex & 0x02) ? 512 * 240 : 0);
		for(uint8_t row = 0; row < 30; row++) {
			for(uint8_t column = 0; column < 32; column++) {
				uint16_t ntIndex = (row << 5) + column;
				uint16_t attributeAddress = baseAttributeAddr + ((row & 0xFC) << 1) + (column >> 2);
				uint8_t tileIndex = vram[baseAddr + ntIndex];
				uint8_t attribute = vram[attributeAddress];
				bool tileChanged = options.HighlightTileChanges && prevVram[baseAddr + ntIndex] != tileIndex;
				bool attrChanged = options.HighlightAttributeChanges && prevVram[attributeAddress] != attribute;
				uint8_t shift = (column & 0x02) | ((row & 0x02) << 1);

				uint8_t paletteBaseAddr;
				if(mmc5 && mmc5->IsExtendedAttributes()) {
					paletteBaseAddr = mmc5->GetExAttributeNtPalette(ntIndex) << 2;
				} else {
					paletteBaseAddr = ((attribute >> shift) & 0x03) << 2;
				}

				uint16_t tileAddr = bgAddr + (tileIndex << 4);
				if(options.DisplayMode == TilemapDisplayMode::AttributeView) {
					for(uint8_t y = 0; y < 8; y++) {
						for(uint8_t x = 0; x < 8; x++) {
							uint8_t color = ((x & 0x04) >> 2) + ((y & 0x04) >> 1);
							outBuffer[bufferOffset + (row << 12) + (column << 3) + (y << 9) + x] = palette[color == 0 ? 0 : (paletteBaseAddr + color)];
						}
					}
				} else {
					for(uint8_t y = 0; y < 8; y++) {
						uint8_t lowByte, highByte;
						if(mmc5 && mmc5->IsExtendedAttributes()) {
							lowByte = mmc5->GetExAttributeTileData(ntIndex, tileAddr + y);
							highByte = mmc5->GetExAttributeTileData(ntIndex, tileAddr + y + 8);
						} else {
							lowByte = vram[tileAddr + y];
							highByte = vram[tileAddr + y + 8];
						}

						for(uint8_t x = 0; x < 8; x++) {
							uint8_t color = ((lowByte >> (7 - x)) & 0x01) | (((highByte >> (7 - x)) & 0x01) << 1);
							uint32_t offset = bufferOffset + (row << 12) + (column << 3) + (y << 9) + x;
							if(options.DisplayMode == TilemapDisplayMode::Grayscale) {
								outBuffer[offset] = grayscalePalette[color];
							} else {
								outBuffer[offset] = palette[color == 0 ? 0 : (paletteBaseAddr + color)];
								if(tileChanged) {
									uint32_t tileChangedColor = 0x80FF0000;
									BlendColors((uint8_t*)&outBuffer[offset], (uint8_t*)&tileChangedColor);
								} else if(attrChanged) {
									uint32_t attrChangedColor = 0x80FFFF00;
									BlendColors((uint8_t*)&outBuffer[offset], (uint8_t*)&attrChangedColor);
								}
							}
						}
					}
				}
			}
		}
	}

	DebugTilemapInfo result = {};
	result.Bpp = 2;
	result.ScrollWidth = NesConstants::ScreenWidth;
	result.ScrollHeight = NesConstants::ScreenHeight;

	if(state.Scanline >= 240 || (state.Scanline == 239 && state.Cycle >= 256) || (state.Scanline == -1 && state.Cycle < 328)) {
		//During vblank, use T instead of V
		uint16_t t = state.TmpVideoRamAddr;
		result.ScrollX = ((t & 0x1F) << 3) | ((t & 0x400) ? 256 : 0) | state.ScrollX;
		result.ScrollY = (((t & 0x3E0) >> 2) | ((t & 0x7000) >> 12)) + ((t & 0x800) ? 240 : 0);
	} else {
		//During rendering, use V and substract according to the current scanline/cycle
		uint16_t v = state.VideoRamAddr;
		int32_t scrollX = ((v & 0x1F) << 3) | ((v & 0x400) ? 256 : 0);
		if(state.Cycle <= 256) {
			if(state.Cycle >= 8) {
				scrollX -= (state.Cycle & ~0x07);
			}
			//Adjust for the 2 x increments at the end of the previous scanline
			scrollX -= 16;
		} else if(state.Cycle >= 328) {
			scrollX -= 8;
			if(state.Cycle >= 336) {
				scrollX -= 8;
			}
		}

		if(scrollX < 0) {
			scrollX += 512;
		}
		scrollX += state.ScrollX;

		int32_t scrollY = (((v & 0x3E0) >> 2) | ((v & 0x7000) >> 12)) + ((v & 0x800) ? 240 : 0);
		if(state.Scanline >= 0) {
			if(state.Cycle < 256) {
				scrollY -= state.Scanline;
			} else {
				scrollY -= state.Scanline + 1;
			}
		}
		if(scrollY < 0) {
			scrollY += 480;
		}

		result.ScrollX = (uint32_t)scrollX;
		result.ScrollY = (uint32_t)scrollY;
	}

	return result;
}

void NesPpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, uint32_t* outBuffer)
{
	NesPpuState& state = (NesPpuState&)baseState;
	
	bool largeSprites = (state.ControlReg & 0x20) ? true : false;
	uint16_t sprAddr = (state.ControlReg & 0x08) ? 0x1000 : 0x0000;

	std::fill(outBuffer, outBuffer + 256 * 240, 0xFF666666);
	std::fill(outBuffer + 256 * 240, outBuffer + 256 * 256, 0xFF333333);

	DebugSpriteInfo sprite;
	for(int i = 0x100 - 4; i >= 0; i -= 4) {
		GetSpriteInfo(sprite, i / 4, options, state, vram, oamRam, palette);
		int spritePosY = sprite.Y + 1;

		for(int y = 0; y < sprite.Height; y++) {
			if(spritePosY + y >= 256) {
				break;
			}

			for(int x = 0; x < sprite.Width; x++) {
				if(sprite.X + x >= 256) {
					break;
				}
				
				uint32_t color = sprite.SpritePreview[y * sprite.Width + x];
				if(color != 0) {
					outBuffer[((spritePosY + y) * 256) + sprite.X + x] = color;
				}
			}
		}
	}
}

FrameInfo NesPpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& state)
{
	return { 512, 480 };
}

DebugTilemapTileInfo NesPpuTools::GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState)
{
	DebugTilemapTileInfo result;

	FrameInfo size = GetTilemapSize(options, baseState);
	if(x >= size.Width || y >= size.Height) {
		return result;
	}

	uint8_t row = y / 8;
	uint8_t column = x / 8;
	uint8_t nametableIndex = (column >= 32 ? 1 : 0) | (row >= 30 ? 2 : 0);

	column &= 0x1F;
	if(row > 30) {
		row -= 30;
	}

	MMC5* mmc5 = dynamic_cast<MMC5*>(_mapper);
	NesPpuState& state = (NesPpuState&)baseState;

	uint16_t bgAddr = (state.ControlReg & 0x10) ? 0x1000 : 0;
	uint16_t baseAddr = 0x2000 + nametableIndex * 0x400;
	uint16_t baseAttributeAddr = baseAddr + 960;
	uint16_t ntIndex = (row << 5) + column;
	uint16_t attributeAddress = baseAttributeAddr + ((row & 0xFC) << 1) + (column >> 2);
	uint8_t attribute = vram[attributeAddress];
	uint8_t shift = (column & 0x02) | ((row & 0x02) << 1);

	uint8_t paletteBaseAddr;
	if(mmc5 && mmc5->IsExtendedAttributes()) {
		paletteBaseAddr = mmc5->GetExAttributeNtPalette(ntIndex) << 2;
	} else {
		paletteBaseAddr = ((attribute >> shift) & 0x03) << 2;
	}
	
	result.Row = row;
	result.Column = column;
	result.Width = 8;
	result.Height = 8;
	result.TileMapAddress = baseAddr + ntIndex;
	result.TileIndex = vram[result.TileMapAddress];
	result.TileAddress = bgAddr + (result.TileIndex << 4);
	result.PaletteIndex = paletteBaseAddr >> 2;
	result.PaletteAddress = 0x3F00 | paletteBaseAddr;

	return result;
}

void NesPpuTools::GetSpriteInfo(DebugSpriteInfo& sprite, uint32_t i, GetSpritePreviewOptions& options, NesPpuState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette)
{
	sprite.SpriteIndex = i;
	sprite.Y = oamRam[i * 4];
	sprite.X = oamRam[i * 4 + 3];
	sprite.TileIndex = oamRam[i * 4 + 1];
	sprite.TileAddress = ((state.ControlReg & 0x08) ? 0x1000 : 0x0000) | (sprite.TileIndex * 16);

	uint8_t attributes = oamRam[i * 4 + 2];
	sprite.Palette = (attributes & 0x03);
	sprite.PaletteAddress = 0x3F00 | ((attributes & 0x03) << 2);
	sprite.HorizontalMirror = (attributes & 0x40) != 0;
	sprite.VerticalMirror = (attributes & 0x80) != 0;
	sprite.Priority = (attributes & 0x20) ? DebugSpritePriority::Background : DebugSpritePriority::Foreground;
	sprite.Visible = sprite.Y < 239;
	sprite.Width = 8;

	bool largeSprites = (state.ControlReg & 0x20) ? true : false;
	sprite.Height = largeSprites ? 16 : 8;

	uint16_t sprAddr = (state.ControlReg & 0x08) ? 0x1000 : 0x0000;
	uint16_t tileStart;
	if(largeSprites) {
		if(sprite.TileIndex & 0x01) {
			tileStart = 0x1000 | ((sprite.TileIndex & 0xFE) * 16);
		} else {
			tileStart = 0x0000 | (sprite.TileIndex * 16);
		}
	} else {
		tileStart = (sprite.TileIndex * 16) | sprAddr;
	}

	for(int y = 0; y < sprite.Height; y++) {
		uint8_t lineOffset = sprite.VerticalMirror ? (sprite.Height - 1 - y) : y;
		uint16_t pixelStart = tileStart + lineOffset;
		if(largeSprites && lineOffset >= 8) {
			pixelStart += 8;
		}

		for(int x = 0; x < sprite.Width; x++) {
			uint8_t shift = sprite.HorizontalMirror ? (x & 0x07) : (7 - (x & 0x07));
			uint8_t color = GetTilePixelColor(vram, 0x3FFF, 2, pixelStart, shift, 8);

			uint32_t outOffset = (y * sprite.Width) + x;
			if(color > 0) {
				sprite.SpritePreview[outOffset] = palette[16 + (sprite.Palette * 4) + color];
			} else {
				sprite.SpritePreview[outOffset] = 0;
			}
		}
	}
}

void NesPpuTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[])
{
	NesPpuState& state = (NesPpuState&)baseState;
	for(int i = 0; i < 64; i++) {
		outBuffer[i].Init();
		GetSpriteInfo(outBuffer[i], i, options, state, vram, oamRam, palette);
	}
}

DebugSpritePreviewInfo NesPpuTools::GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state)
{
	DebugSpritePreviewInfo info = {};
	info.Height = 256;
	info.Width = 256;
	info.SpriteCount = 64;
	info.CoordOffsetX = 0;
	info.CoordOffsetY = 1;
	return info;
}
