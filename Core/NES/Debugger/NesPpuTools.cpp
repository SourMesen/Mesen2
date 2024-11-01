#include "pch.h"
#include "NES/Debugger/NesPpuTools.h"
#include "NES/Debugger/IExtModeMapperDebug.h"
#include "NES/BaseMapper.h"
#include "NES/BaseNesPpu.h"
#include "NES/NesConsole.h"
#include "NES/NesConstants.h"
#include "NES/NesTypes.h"
#include "NES/NesDefaultVideoFilter.h"
#include "NES/Mappers/Homebrew/Rainbow.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Shared/SettingTypes.h"

static constexpr uint32_t grayscalePalette[4] = { 0xFF000000, 0xFF808080, 0xFFC0C0C0, 0xFFFFFFFF };

NesPpuTools::NesPpuTools(Debugger* debugger, Emulator *emu, NesConsole* console) : PpuTools(debugger, emu)
{
	_console = console;
	_mapper = console->GetMapper();
}

void NesPpuTools::GetPpuToolsState(BaseState& state)
{
	NesPpuToolsState nesState = {};
	
	IExtModeMapperDebug* exMode = dynamic_cast<IExtModeMapperDebug*>(_mapper);
	if(exMode) {
		nesState.ExtConfig = exMode->GetExModeConfig();
	}

	(NesPpuToolsState&)state = nesState;
}

void NesPpuTools::DrawNametable(uint8_t* ntSource, uint32_t ntBaseAddr, uint8_t ntIndex, GetTilemapOptions options, NesPpuState& state, IExtModeMapperDebug* exMode, ExtModeConfig& extCfg, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer, uint32_t bufferWidth)
{
	uint16_t baseAttributeAddr = ntBaseAddr + 960;

	for(uint8_t row = 0; row < 30; row++) {
		for(uint8_t column = 0; column < 32; column++) {
			uint16_t ntOffset = (row << 5) + column;
			uint16_t attributeAddress = baseAttributeAddr + ((row & 0xFC) << 1) + (column >> 2);
			uint8_t tileIndex = ntSource[ntBaseAddr + ntOffset];
			uint8_t attribute = ntSource[attributeAddress];
			uint8_t shift = (column & 0x02) | ((row & 0x02) << 1);

			uint8_t paletteBaseAddr;
			if(exMode && exMode->HasExtendedAttributes(extCfg, ntIndex)) {
				paletteBaseAddr = exMode->GetExAttributePalette(extCfg, ntIndex, ntOffset) << 2;
			} else {
				paletteBaseAddr = ((attribute >> shift) & 0x03) << 2;
			}

			uint16_t tileAddr = state.Control.BackgroundPatternAddr + (tileIndex << 4);
			if(options.DisplayMode == TilemapDisplayMode::AttributeView) {
				for(uint8_t y = 0; y < 8; y++) {
					for(uint8_t x = 0; x < 8; x++) {
						uint8_t color = ((x & 0x04) >> 2) + ((y & 0x04) >> 1);
						outBuffer[(row*bufferWidth*8) + (column << 3) + (y*bufferWidth) + x] = palette[paletteBaseAddr + color];
					}
				}
			} else {
				for(uint8_t y = 0; y < 8; y++) {
					uint8_t lowByte, highByte;
					if(exMode && exMode->HasExtendedBackground(extCfg, ntIndex)) {
						lowByte = exMode->GetExBackgroundChrData(extCfg, ntIndex, ntOffset, tileAddr + y);
						highByte = exMode->GetExBackgroundChrData(extCfg, ntIndex, ntOffset, tileAddr + y + 8);
					} else {
						lowByte = vram[tileAddr + y];
						highByte = vram[tileAddr + y + 8];
					}

					uint32_t offset = (row*bufferWidth*8) + (column << 3) + (y*bufferWidth);
					for(uint8_t x = 0; x < 8; x++) {
						uint8_t color = ((lowByte >> (7 - x)) & 0x01) | (((highByte >> (7 - x)) & 0x01) << 1);
						if(options.DisplayMode == TilemapDisplayMode::Grayscale) {
							outBuffer[offset + x] = grayscalePalette[color];
						} else {
							outBuffer[offset + x] = palette[paletteBaseAddr + color];
						}
					}
				}
			}
		}
	}
}

DebugTilemapInfo NesPpuTools::GetWindowTilemap(GetTilemapOptions options, NesPpuState& state, IExtModeMapperDebug* exMode, ExtModeConfig& extCfg, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	uint16_t baseAddr = extCfg.WindowBank * 0x400;	
	
	DrawNametable(extCfg.ExtRam, baseAddr, 4, options, state, exMode, extCfg, vram, palette, outBuffer, 256);

	DebugTilemapInfo result = {};
	result.Bpp = 2;
	result.Format = TileFormat::NesBpp2;
	result.TileWidth = 8;
	result.TileHeight = 8;
	result.ColumnCount = 32;
	result.RowCount = 30;
	result.TilemapAddress = extCfg.WindowBank * 0x400;
	result.TilesetAddress = state.Control.BackgroundPatternAddr;
	result.ScrollWidth = NesConstants::ScreenWidth;
	result.ScrollHeight = NesConstants::ScreenHeight;
	result.ScrollX = extCfg.WindowScrollX;
	result.ScrollY = extCfg.WindowScrollY;
	return result;
}

DebugTilemapInfo NesPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
	IExtModeMapperDebug* exMode = dynamic_cast<IExtModeMapperDebug*>(_mapper);
	NesPpuState& state = (NesPpuState&)baseState;
	ExtModeConfig& extCfg = ((NesPpuToolsState&)ppuToolsState).ExtConfig;

	for(int i = 0; i < 32; i+=4) {
		palette[i] = palette[0];
	}

	if(options.Layer == 1 && exMode) {
		return GetWindowTilemap(options, state, exMode, extCfg, vram, palette, outBuffer);
	}

	for(int nametableIndex = 0; nametableIndex < 4; nametableIndex++) {
		uint16_t baseAddr = 0x2000 + nametableIndex * 0x400;
		uint32_t bufferOffset = ((nametableIndex & 0x01) ? 256 : 0) + ((nametableIndex & 0x02) ? 512 * 240 : 0);

		DrawNametable(vram, baseAddr, nametableIndex, options, state, exMode, extCfg, vram, palette, outBuffer+bufferOffset, 512);
		
		if(options.DisplayMode != TilemapDisplayMode::AttributeView) {
			if(options.TileHighlightMode != TilemapHighlightMode::None || options.AttributeHighlightMode != TilemapHighlightMode::None) {
				ApplyHighlights(options, nametableIndex, vram, outBuffer);
			}
		}
	}

	DebugTilemapInfo result = {};
	result.Bpp = 2;
	result.Format = TileFormat::NesBpp2;
	result.TileWidth = 8;
	result.TileHeight = 8;
	result.ColumnCount = 64;
	result.RowCount = 60;
	result.TilemapAddress = 0x2000;
	result.TilesetAddress = state.Control.BackgroundPatternAddr;
	result.ScrollWidth = NesConstants::ScreenWidth;
	result.ScrollHeight = NesConstants::ScreenHeight;

	switch(_mapper->GetState().Mirroring) {
		case MirroringType::Horizontal: result.Mirroring = TilemapMirroring::Horizontal; break;
		case MirroringType::Vertical: result.Mirroring = TilemapMirroring::Vertical; break;
		case MirroringType::ScreenAOnly: result.Mirroring = TilemapMirroring::SingleScreenA; break;
		case MirroringType::ScreenBOnly: result.Mirroring = TilemapMirroring::SingleScreenB; break;
		case MirroringType::FourScreens: result.Mirroring = TilemapMirroring::FourScreens; break;
	}

	if(state.Scanline >= 240 || (state.Scanline == 239 && state.Cycle >= 256) || (state.Scanline == -1 && state.Cycle < 328)) {
		//During vblank, use T instead of V
		uint16_t t = state.TmpVideoRamAddr;
		result.ScrollX = ((t & 0x1F) << 3) | ((t & 0x400) ? 256 : 0) | state.ScrollX;
		result.ScrollY = (((t & 0x3E0) >> 2) | ((t & 0x7000) >> 12)) + ((t & 0x800) ? 240 : 0);
	} else {
		//During rendering, use V and subtract according to the current scanline/cycle
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

void NesPpuTools::ApplyHighlights(GetTilemapOptions options, uint8_t nametableIndex, uint8_t* vram, uint32_t* outBuffer)
{
	uint16_t baseAddr = 0x2000 + nametableIndex * 0x400;
	uint16_t baseAttributeAddr = baseAddr + 960;
	uint32_t bufferOffset = ((nametableIndex & 0x01) ? 256 : 0) + ((nametableIndex & 0x02) ? 512 * 240 : 0);

	AddressCounters* accessCounters = options.AccessCounters;
	uint8_t* prevVram = options.CompareVram != nullptr ? options.CompareVram : vram;

	uint64_t masterClock = options.MasterClock;
	uint32_t clockRate = _console->GetMasterClockRate() / _console->GetFps();

	auto isHighlighted = [&](uint16_t addr, TilemapHighlightMode mode) -> bool {
		switch(mode) {
			default:
			case TilemapHighlightMode::None: return false;

				//Highlight if modified since the last update
			case TilemapHighlightMode::Changes: return prevVram[addr] != vram[addr];

				//Highlight if modified in the last frame
			case TilemapHighlightMode::Writes: return accessCounters && masterClock - accessCounters[addr].WriteStamp < clockRate;
		}
	};

	for(uint8_t row = 0; row < 30; row++) {
		for(uint8_t column = 0; column < 32; column++) {
			uint16_t ntOffset = (row << 5) + column;
			uint16_t attributeAddress = baseAttributeAddr + ((row & 0xFC) << 1) + (column >> 2);
			bool tileHighlighted = isHighlighted(baseAddr + ntOffset, options.TileHighlightMode);
			bool attrHighlighted = isHighlighted(attributeAddress, options.AttributeHighlightMode);

			if(!tileHighlighted && !attrHighlighted) {
				continue;
			}

			for(uint8_t y = 0; y < 8; y++) {
				uint32_t offset = bufferOffset + (row << 12) + (column << 3) + (y << 9);
				for(uint8_t x = 0; x < 8; x++) {
					if(tileHighlighted) {
						static constexpr uint32_t tileChangedColor = 0x80FF0000;
						if(x == 0 || y == 0 || x == 7 || y == 7) {
							outBuffer[offset + x] = 0xFF000000 | tileChangedColor;
						} else {
							BlendColors((uint8_t*)&outBuffer[offset + x], (uint8_t*)&tileChangedColor);
						}
					}

					if(attrHighlighted) {
						static constexpr uint32_t attrChangedColor = 0x80FFFF00;
						bool isEdge = (
							((column & 3) == 0 && x == 0) ||
							((row & 3) == 0 && y == 0) ||
							((column & 3) == 3 && x == 7) ||
							((row & 3) == 3 && y == 7)
						);
						if(isEdge) {
							outBuffer[offset + x] = 0xFF000000 | attrChangedColor;
						} else {
							BlendColors((uint8_t*)&outBuffer[offset + x], (uint8_t*)&attrChangedColor);
						}
					}
				}
			}
		}
	}
}

void NesPpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, DebugSpriteInfo* sprites, uint32_t* spritePreviews, uint32_t* palette, uint32_t* outBuffer)
{
	uint32_t bgColor = GetSpriteBackgroundColor(options.Background, palette, false);

	std::fill(outBuffer, outBuffer + 256 * 240, bgColor);
	std::fill(outBuffer + 256 * 240, outBuffer + 256 * 256, GetSpriteBackgroundColor(options.Background, palette, true));

	for(int i = 63; i >= 0; i--) {
		DebugSpriteInfo& sprite = sprites[i];
		uint32_t* spritePreview = spritePreviews + i * _spritePreviewSize;

		int spritePosY = sprite.Y + 1;

		for(int y = 0; y < sprite.Height; y++) {
			for(int x = 0; x < sprite.Width; x++) {
				uint32_t color = spritePreview[y * sprite.Width + x];
				if(color != 0) {
					if(spritePosY + y >= 256 || sprite.X + x >= 256) {
						continue;
					}

					outBuffer[((spritePosY + y) * 256) + sprite.X + x] = color;
				} else {
					spritePreview[y * sprite.Width + x] = bgColor;
				}
			}
		}
	}
}

FrameInfo NesPpuTools::GetTilemapSize(GetTilemapOptions options, BaseState& state)
{
	if(options.Layer == 0) {
		return { 512, 480 };
	} else if(options.Layer == 1 && dynamic_cast<IExtModeMapperDebug*>(_mapper)) {
		return { 256, 240 };
	}
	return { 0,0 };
}

DebugTilemapTileInfo NesPpuTools::GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState)
{
	DebugTilemapTileInfo result = {};

	FrameInfo size = GetTilemapSize(options, baseState);
	if(x >= size.Width || y >= size.Height) {
		return result;
	}

	uint8_t row = y / 8;
	uint8_t column = x / 8;
	uint8_t nametableIndex = (column >= 32 ? 1 : 0) | (row >= 30 ? 2 : 0);

	column &= 0x1F;
	if(row >= 30) {
		row -= 30;
	}

	IExtModeMapperDebug* exMode = dynamic_cast<IExtModeMapperDebug*>(_mapper);
	ExtModeConfig& extCfg = ((NesPpuToolsState&)ppuToolsState).ExtConfig;
	NesPpuState& state = (NesPpuState&)baseState;

	uint16_t bgAddr = state.Control.BackgroundPatternAddr;
	uint16_t baseAddr = 0x2000 + nametableIndex * 0x400;
	uint16_t baseAttributeAddr = baseAddr + 960;
	uint16_t ntOffset = (row << 5) + column;
	uint16_t attributeAddress = baseAttributeAddr + ((row & 0xFC) << 1) + (column >> 2);
	uint8_t attribute = vram[attributeAddress];
	uint8_t shift = (column & 0x02) | ((row & 0x02) << 1);

	uint8_t paletteBaseAddr;
	if(exMode && exMode->HasExtendedAttributes(extCfg, nametableIndex)) {
		paletteBaseAddr = exMode->GetExAttributePalette(extCfg, nametableIndex, ntOffset) << 2;
	} else {
		paletteBaseAddr = ((attribute >> shift) & 0x03) << 2;
	}
	
	result.Row = row;
	result.Column = column;
	result.Width = 8;
	result.Height = 8;
	result.TileMapAddress = baseAddr + ntOffset;
	result.TileIndex = vram[result.TileMapAddress];
	result.TileAddress = bgAddr + (result.TileIndex << 4);
	result.PaletteIndex = paletteBaseAddr >> 2;
	result.PaletteAddress = 0x3F00 | paletteBaseAddr;
	result.AttributeAddress = attributeAddress;
	result.AttributeData = vram[attributeAddress];

	return result;
}

void NesPpuTools::GetSpriteInfo(DebugSpriteInfo& sprite, uint32_t* spritePreview, uint32_t i, GetSpritePreviewOptions& options, NesPpuState& state, NesPpuToolsState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette)
{
	IExtModeMapperDebug* exMode = dynamic_cast<IExtModeMapperDebug*>(_mapper);
	ExtModeConfig& extCfg = ppuToolsState.ExtConfig;

	sprite.Bpp = 2;
	sprite.Format = TileFormat::NesBpp2;
	sprite.SpriteIndex = i;
	sprite.UseExtendedVram = false;
	sprite.Y = oamRam[i * 4];
	sprite.X = oamRam[i * 4 + 3];
	sprite.RawY = sprite.Y;
	sprite.RawX = sprite.X;
	sprite.TileIndex = oamRam[i * 4 + 1];
	sprite.UseSecondTable = NullableBoolean::Undefined;

	uint8_t attributes = oamRam[i * 4 + 2];
	sprite.Palette = (attributes & 0x03);
	sprite.PaletteAddress = 0x3F00 | ((attributes & 0x03) << 2);
	bool horizontalMirror = (attributes & 0x40) != 0;
	bool verticalMirror = (attributes & 0x80) != 0;
	sprite.HorizontalMirror = horizontalMirror ? NullableBoolean::True : NullableBoolean::False;
	sprite.VerticalMirror = verticalMirror ? NullableBoolean::True : NullableBoolean::False;
	sprite.Priority = (attributes & 0x20) ? DebugSpritePriority::Background : DebugSpritePriority::Foreground;
	sprite.Visibility = sprite.Y < 239 ? SpriteVisibility::Visible : SpriteVisibility::Offscreen;
	sprite.Width = 8;

	bool largeSprites = state.Control.LargeSprites;
	sprite.Height = largeSprites ? 16 : 8;

	uint16_t sprAddr = state.Control.SpritePatternAddr;
	uint16_t tileStart;
	if(largeSprites) {
		if(sprite.TileIndex & 0x01) {
			tileStart = 0x1000 | ((sprite.TileIndex & 0xFE) * 16);
		} else {
			tileStart = 0x0000 | (sprite.TileIndex * 16);
		}
		sprite.TileAddresses[0] = tileStart;
		sprite.TileAddresses[1] = tileStart + 16;
		sprite.TileCount = 2;
	} else {
		tileStart = (sprite.TileIndex * 16) | sprAddr;
		sprite.TileAddresses[0] = tileStart;
		sprite.TileCount = 1;
	}
	sprite.TileAddress = tileStart;

	for(int y = 0; y < sprite.Height; y++) {
		uint8_t lineOffset = verticalMirror ? (sprite.Height - 1 - y) : y;
		uint16_t pixelStart = tileStart + lineOffset;
		if(largeSprites && lineOffset >= 8) {
			pixelStart += 8;
		}

		for(int x = 0; x < 8; x++) {
			uint8_t lowByte, highByte;
			if(exMode && exMode->HasExtendedSprites(extCfg)) {
				lowByte = exMode->GetExSpriteChrData(extCfg, i, pixelStart);
				highByte = exMode->GetExSpriteChrData(extCfg, i, pixelStart + 8);
			} else {
				lowByte = vram[pixelStart];
				highByte = vram[pixelStart + 8];
			}

			uint8_t shift = horizontalMirror ? x : (7 - x);
			uint8_t color = ((lowByte >> shift) & 0x01) | (((highByte >> shift) & 0x01) << 1);

			uint32_t outOffset = (y * 8) + x;
			if(color > 0) {
				spritePreview[outOffset] = palette[16 + (sprite.Palette * 4) + color];
			} else {
				spritePreview[outOffset] = 0;
			}
		}
	}
}

void NesPpuTools::GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[], uint32_t* spritePreviews, uint32_t* screenPreview)
{
	NesPpuState& state = (NesPpuState&)baseState;
	NesPpuToolsState& nesToolsState = (NesPpuToolsState&)ppuToolsState;
	for(int i = 0; i < 64; i++) {
		outBuffer[i].Init();
		GetSpriteInfo(outBuffer[i], spritePreviews+i*_spritePreviewSize, i, options, state, nesToolsState, vram, oamRam, palette);
	}

	GetSpritePreview(options, baseState, outBuffer, spritePreviews, palette, screenPreview);
}

DebugSpritePreviewInfo NesPpuTools::GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state, BaseState& ppuToolsState)
{
	DebugSpritePreviewInfo info = {};
	info.Height = 256;
	info.Width = 256;
	info.SpriteCount = 64;
	info.CoordOffsetX = 0;
	info.CoordOffsetY = 1;

	info.VisibleX = 0;
	info.VisibleY = 0;
	info.VisibleWidth = 256;
	info.VisibleHeight = 240;

	return info;
}

DebugPaletteInfo NesPpuTools::GetPaletteInfo(GetPaletteInfoOptions options)
{
	DebugPaletteInfo info = {};
	info.PaletteMemType = MemoryType::NesPaletteRam;
	info.HasMemType = true;

	info.RawFormat = RawPaletteFormat::Indexed;
	info.ColorsPerPalette = 4;
	info.BgColorCount = 4 * 4;
	info.SpritePaletteOffset = info.BgColorCount;
	info.SpriteColorCount = 4 * 4;
	info.ColorCount = info.BgColorCount + info.SpriteColorCount;

	uint32_t rgbPalette[512];
	NesDefaultVideoFilter::GetFullPalette(rgbPalette, _console->GetNesConfig(), _console->GetPpu()->GetPpuModel());

	uint8_t* paletteRam = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::NesPaletteRam);
	for(int i = 0; i < 32; i++) {
		info.RawPalette[i] = paletteRam[i];
		info.RgbPalette[i] = rgbPalette[paletteRam[i]];
	}

	return info;
}

void NesPpuTools::SetPaletteColor(int32_t colorIndex, uint32_t color)
{
	if(color < 0x3F) {
		_debugger->GetMemoryDumper()->SetMemoryValue(MemoryType::NesPaletteRam, colorIndex, (uint8_t)color);
	}
}
