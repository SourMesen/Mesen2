#pragma once
#include "pch.h"
#include "NES/HdPacks/HdNesPpu.h"
#include "NES/HdPacks/HdNesPack.h"
#include "NES/HdPacks/HdPackBuilder.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/RewindManager.h"
#include "Utilities/Serializer.h"

class HdBuilderPpu final : public NesPpu<HdBuilderPpu>
{
private:
	HdPackBuilder* _hdPackBuilder = nullptr;
	bool _needChrHash = false;
	uint32_t _chrRamBankSize = 0;
	uint32_t _chrRamIndexMask = 0;
	vector<uint32_t> _bankHashes;

	NesSpriteInfoEx _exSpriteInfo[64] = {};
	NesTileInfoEx _previousTileEx = {};
	NesTileInfoEx _currentTileEx = {};
	NesTileInfoEx _nextTileEx = {};

public:
	__forceinline bool RemoveSpriteLimit() { return _console->GetNesConfig().RemoveSpriteLimit; }
	__forceinline bool UseAdaptiveSpriteLimit() { return _console->GetNesConfig().AdaptiveSpriteLimit; }
	void* OnBeforeSendFrame() { return nullptr; }

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

		uint8_t tileIndex = ReadVram(GetNameTableAddr());
		uint16_t tileAddr = (tileIndex << 4) | (_videoRamAddr >> 12) | _control.BackgroundPatternAddr;

		_nextTileEx.OffsetY = _videoRamAddr >> 12;
		_nextTileEx.TileAddr = tileAddr;
		_nextTileEx.AbsoluteTileAddr = _mapper->GetPpuAbsoluteAddress(tileAddr).Address;
	}

	__forceinline void ProcessScanline()
	{
		ProcessScanlineImpl();
	}

	void DrawPixel()
	{
		if(IsRenderingEnabled() || ((_videoRamAddr & 0x3F00) != 0x3F00)) {
			BaseMapper* mapper = _console->GetMapper();
			bool isChrRam = !mapper->HasChrRom();

			_lastSprite = nullptr;
			uint32_t color = GetPixelColor();
			_currentOutputBuffer[(_scanline << 8) + _cycle - 1] = _paletteRam[color & 0x03 ? color : 0];
			uint32_t backgroundColor = 0;
			if(_mask.BackgroundEnabled && _cycle > _minimumDrawBgCycle) {
				backgroundColor = (((_lowBitShift << _xScroll) & 0x8000) >> 15) | (((_highBitShift << _xScroll) & 0x8000) >> 14);
			}

			if(_needChrHash ) {
				uint16_t addr = 0;
				_bankHashes.clear();
				while(addr < 0x2000) {
					uint32_t hash = 0;
					for(uint16_t i = 0; i < _chrRamBankSize; i++) {
						hash += mapper->DebugReadVram(i + addr);
						hash = (hash << 1) | (hash >> 31);
					}
					_bankHashes.push_back(hash);
					addr += _chrRamBankSize;
				}
				_needChrHash = false;
			}

			bool hasBgSprite = false;
			if(_lastSprite && _mask.SpritesEnabled) {
				uint8_t spriteIndex = (uint8_t)(_lastSprite - _spriteTiles);
				NesSpriteInfoEx& spriteInfoEx = _exSpriteInfo[spriteIndex];

				if(backgroundColor == 0) {
					for(uint8_t i = 0; i < _spriteCount; i++) {
						if(_spriteTiles[i].BackgroundPriority) {
							hasBgSprite = true;
							break;
						}
					}
				}

				if(spriteInfoEx.AbsoluteTileAddr >= 0) {
					HdPpuTileInfo sprite = {};
					sprite.TileIndex = (isChrRam ? (spriteInfoEx.TileAddr & _chrRamIndexMask) : spriteInfoEx.AbsoluteTileAddr) / 16;
					sprite.PaletteColors = ReadPaletteRam(_lastSprite->PaletteOffset + 3) | (ReadPaletteRam(_lastSprite->PaletteOffset + 2) << 8) | (ReadPaletteRam(_lastSprite->PaletteOffset + 1) << 16) | 0xFF000000;
					sprite.IsChrRamTile = isChrRam;
					mapper->CopyChrTile(spriteInfoEx.AbsoluteTileAddr & 0xFFFFFFF0, sprite.TileData);

					_hdPackBuilder->ProcessTile(_cycle - 1, _scanline, spriteInfoEx.AbsoluteTileAddr, sprite, mapper, false, _bankHashes[spriteInfoEx.TileAddr / _chrRamBankSize], false);
				}
			}

			if(_mask.BackgroundEnabled) {
				bool usePrev = (_xScroll + ((_cycle - 1) & 0x07) < 8);
				uint8_t tilePalette = usePrev ? _previousTilePalette : _currentTilePalette;
				NesTileInfoEx& lastTileEx = usePrev ? _previousTileEx : _currentTileEx;
				//TileInfo* lastTile = &((_xScroll + ((_cycle - 1) & 0x07) < 8) ? _previousTile : _currentTile);
				if(lastTileEx.AbsoluteTileAddr >= 0) {
					HdPpuTileInfo tile = {};
					tile.TileIndex = (isChrRam ? (lastTileEx.TileAddr & _chrRamIndexMask) : lastTileEx.AbsoluteTileAddr) / 16;
					tile.PaletteColors = ReadPaletteRam(tilePalette + 3) | (ReadPaletteRam(tilePalette + 2) << 8) | (ReadPaletteRam(tilePalette + 1) << 16) | (ReadPaletteRam(0) << 24);
					tile.IsChrRamTile = isChrRam;
					mapper->CopyChrTile(lastTileEx.AbsoluteTileAddr & 0xFFFFFFF0, tile.TileData);

					_hdPackBuilder->ProcessTile(_cycle - 1, _scanline, lastTileEx.AbsoluteTileAddr, tile, mapper, false, _bankHashes[lastTileEx.TileAddr / _chrRamBankSize], hasBgSprite);
				}
			}
		} else {
			//"If the current VRAM address points in the range $3F00-$3FFF during forced blanking, the color indicated by this palette location will be shown on screen instead of the backdrop color."
			_currentOutputBuffer[(_scanline << 8) + _cycle - 1] = _paletteRam[_videoRamAddr & 0x1F];
		}
	}

	void WriteRAM(uint16_t addr, uint8_t value)
	{
		if(GetRegisterID(addr) == PpuRegisters::VideoMemoryData) {
			if(_videoRamAddr < 0x2000) {
				_needChrHash = true;
			}
		}
		NesPpu::WriteRam(addr, value);
	}

	void Serialize(Serializer& s)
	{
		NesPpu::Serialize(s);
		if(!s.IsSaving()) {
			_needChrHash = true;
		}
	}

public:
	HdBuilderPpu(NesConsole* console, HdPackBuilder* hdPackBuilder, uint32_t chrRamBankSize) : NesPpu(console)
	{
		_hdPackBuilder = hdPackBuilder;
		_chrRamBankSize = chrRamBankSize;
		_chrRamIndexMask = chrRamBankSize - 1;
		_needChrHash = true;
	}
};