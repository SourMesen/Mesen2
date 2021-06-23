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

void NesPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
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
}

void NesPpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, uint32_t* outBuffer)
{
}

FrameInfo NesPpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& state)
{
	return { 512, 480 };
}

FrameInfo NesPpuTools::GetSpritePreviewSize(GetSpritePreviewOptions options, BaseState& state)
{
	return { 256, 240 };
}
