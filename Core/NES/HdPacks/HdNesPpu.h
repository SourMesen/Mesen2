#pragma once
#include "pch.h"
#include "NES/NesPpu.h"
#include "NES/NesConsole.h"
#include "NES/HdPacks/HdPackConditions.h"
#include "NES/NesMemoryManager.h"
#include "NES/BaseMapper.h"
#include "NES/HdPacks/HdData.h"

struct NesSpriteInfoEx
{
	uint32_t AbsoluteTileAddr;
	uint16_t TileAddr;
	bool VerticalMirror;
	uint8_t OffsetY;
};

struct NesTileInfoEx
{
	uint32_t AbsoluteTileAddr;
	uint16_t TileAddr;
	uint8_t OffsetY;
};

class HdNesPpu final : public NesPpu<HdNesPpu>
{
	HdScreenInfo* _screenInfo[2] = {};
	HdScreenInfo* _info = nullptr;
	uint32_t _version = 0;
	bool _isChrRam = false;
	bool _forceRemoveSpriteLimit = false;
	HdPackData* _hdData = nullptr;
	NesSpriteInfoEx _exSpriteInfo[64] = {};
	NesTileInfoEx _previousTileEx = {};
	NesTileInfoEx _currentTileEx = {};
	NesTileInfoEx _nextTileEx = {};

public:
	HdNesPpu(NesConsole* console, HdPackData* hdData);
	virtual ~HdNesPpu();
	
	void* OnBeforeSendFrame();

	__forceinline bool RemoveSpriteLimit() { return _forceRemoveSpriteLimit || _console->GetNesConfig().RemoveSpriteLimit; }
	__forceinline bool UseAdaptiveSpriteLimit() { return _forceRemoveSpriteLimit || _console->GetNesConfig().AdaptiveSpriteLimit; }

	__forceinline void StoreSpriteInformation(bool verticalMirror, uint16_t tileAddr, uint8_t lineOffset)
	{
		NesSpriteInfoEx& info = _exSpriteInfo[_spriteIndex];
		info.TileAddr = tileAddr;
		info.AbsoluteTileAddr = _mapper->GetPpuAbsoluteAddress(info.TileAddr).Address;
		info.VerticalMirror = verticalMirror;
		info.OffsetY = lineOffset;
	}

	__forceinline void StoreTileInformation()
	{
		_previousTileEx = _currentTileEx;
		_currentTileEx = _nextTileEx;

		uint8_t tileIndex = _mapper->DebugReadVram(GetNameTableAddr());
		uint16_t tileAddr = (tileIndex << 4) | (_videoRamAddr >> 12) | _control.BackgroundPatternAddr;

		_nextTileEx.OffsetY = _videoRamAddr >> 12;
		_nextTileEx.AbsoluteTileAddr = _mapper->GetPpuAbsoluteAddress(tileAddr).Address;
	}

	__forceinline void ProcessScanline()
	{
		ProcessScanlineImpl();
	}

	void DrawPixel()
	{
		uint16_t bufferOffset = (_scanline << 8) + _cycle - 1;
		uint16_t& pixel = _currentOutputBuffer[bufferOffset];
		_lastSprite = nullptr;

		if(IsRenderingEnabled() || ((_videoRamAddr & 0x3F00) != 0x3F00)) {
			uint32_t color = GetPixelColor();
			pixel = (_paletteRam[color & 0x03 ? color : 0] & _paletteRamMask) | _intensifyColorBits;

			bool usePrev = (_xScroll + ((_cycle - 1) & 0x07) < 8);
			uint8_t tilePalette = usePrev ? _previousTilePalette : _currentTilePalette;
			NesTileInfoEx& lastTileEx = usePrev ? _previousTileEx : _currentTileEx;
			uint32_t backgroundColor = 0;
			if(_mask.BackgroundEnabled && _cycle > _minimumDrawBgCycle) {
				backgroundColor = (((_lowBitShift << _xScroll) & 0x8000) >> 15) | (((_highBitShift << _xScroll) & 0x8000) >> 14);
			}

			HdPpuPixelInfo& tileInfo = _info->ScreenTiles[bufferOffset];

			tileInfo.Grayscale = _paletteRamMask == 0x30;
			tileInfo.EmphasisBits = _intensifyColorBits >> 6;
			tileInfo.Tile.PpuBackgroundColor = ReadPaletteRam(0);
			tileInfo.Tile.BgColorIndex = backgroundColor;
			tileInfo.Tile.PaletteOffset = tileInfo.Tile.PaletteOffset;
			if(backgroundColor == 0) {
				tileInfo.Tile.BgColor = tileInfo.Tile.PpuBackgroundColor;
			} else {
				tileInfo.Tile.BgColor = ReadPaletteRam(tilePalette + backgroundColor);
			}

			tileInfo.XScroll = _xScroll;
			tileInfo.TmpVideoRamAddr = _tmpVideoRamAddr;

			if(_lastSprite && _mask.SpritesEnabled) {
				int j = 0;
				for(uint8_t i = 0; i < _spriteCount; i++) {
					int32_t shift = (int32_t)_cycle - _spriteTiles[i].SpriteX - 1;
					NesSpriteInfo& sprite = _spriteTiles[i];
					NesSpriteInfoEx& spriteEx = _exSpriteInfo[i];
					if(shift >= 0 && shift < 8) {
						tileInfo.Sprite[j].TileIndex = spriteEx.AbsoluteTileAddr / 16;
						if(_isChrRam) {
							_console->GetMapper()->CopyChrTile(spriteEx.AbsoluteTileAddr & 0xFFFFFFF0, tileInfo.Sprite[j].TileData);
						}
						if(_version >= 100) {
							tileInfo.Sprite[j].PaletteColors = 0xFF000000 | _paletteRam[sprite.PaletteOffset + 3] | (_paletteRam[sprite.PaletteOffset + 2] << 8) | (_paletteRam[sprite.PaletteOffset + 1] << 16);
						} else {
							tileInfo.Sprite[j].PaletteColors = _paletteRam[sprite.PaletteOffset + 3] | (_paletteRam[sprite.PaletteOffset + 2] << 8) | (_paletteRam[sprite.PaletteOffset + 1] << 16);
						}
						if(spriteEx.OffsetY >= 8) {
							tileInfo.Sprite[j].OffsetY = spriteEx.OffsetY - 8;
						} else {
							tileInfo.Sprite[j].OffsetY = spriteEx.OffsetY;
						}

						tileInfo.Sprite[j].OffsetX = shift;
						tileInfo.Sprite[j].HorizontalMirroring = sprite.HorizontalMirror;
						tileInfo.Sprite[j].VerticalMirroring = spriteEx.VerticalMirror;
						tileInfo.Sprite[j].BackgroundPriority = sprite.BackgroundPriority;
						tileInfo.Sprite[j].PaletteOffset = sprite.PaletteOffset;

						if(sprite.HorizontalMirror) {
							tileInfo.Sprite[j].SpriteColorIndex = ((sprite.LowByte >> shift) & 0x01) | ((sprite.HighByte >> shift) & 0x01) << 1;
						} else {
							tileInfo.Sprite[j].SpriteColorIndex = ((sprite.LowByte << shift) & 0x80) >> 7 | ((sprite.HighByte << shift) & 0x80) >> 6;
						}

						if(tileInfo.Sprite[j].SpriteColorIndex == 0) {
							tileInfo.Sprite[j].SpriteColor = ReadPaletteRam(0);
						} else {
							tileInfo.Sprite[j].SpriteColor = ReadPaletteRam(sprite.PaletteOffset + tileInfo.Sprite[j].SpriteColorIndex);
						}

						tileInfo.Sprite[j].PpuBackgroundColor = tileInfo.Tile.PpuBackgroundColor;
						tileInfo.Sprite[j].BgColorIndex = tileInfo.Tile.BgColorIndex;

						j++;
						if(j >= 4) {
							break;
						}
					}
				}
				tileInfo.SpriteCount = j;
			} else {
				tileInfo.SpriteCount = 0;
			}

			if(_mask.BackgroundEnabled && _cycle > _minimumDrawBgCycle) {
				tileInfo.Tile.TileIndex = lastTileEx.AbsoluteTileAddr / 16;
				if(_isChrRam) {
					_console->GetMapper()->CopyChrTile(lastTileEx.AbsoluteTileAddr & 0xFFFFFFF0, tileInfo.Tile.TileData);
				}
				if(_version >= 100) {
					tileInfo.Tile.PaletteColors = _paletteRam[tilePalette + 3] | (_paletteRam[tilePalette + 2] << 8) | (_paletteRam[tilePalette + 1] << 16) | (_paletteRam[0] << 24);
				} else {
					tileInfo.Tile.PaletteColors = _paletteRam[tilePalette + 3] | (_paletteRam[tilePalette + 2] << 8) | (_paletteRam[tilePalette + 1] << 16);
				}
				tileInfo.Tile.OffsetY = lastTileEx.OffsetY;
				tileInfo.Tile.OffsetX = (_xScroll + ((_cycle - 1) & 0x07)) & 0x07;
			} else {
				tileInfo.Tile.TileIndex = HdPpuTileInfo::NoTile;
			}
		} else {
			//"If the current VRAM address points in the range $3F00-$3FFF during forced blanking, the color indicated by this palette location will be shown on screen instead of the backdrop color."
			pixel = ReadPaletteRam(_videoRamAddr) | _intensifyColorBits;
			_info->ScreenTiles[bufferOffset].Tile.TileIndex = HdPpuTileInfo::NoTile;
			_info->ScreenTiles[bufferOffset].SpriteCount = 0;
		}
	}
};