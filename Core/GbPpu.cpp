#include "stdafx.h"
#include "GbPpu.h"
#include "GbTypes.h"
#include "EventType.h"
#include "Console.h"
#include "Gameboy.h"
#include "VideoDecoder.h"
#include "RewindManager.h"
#include "GbMemoryManager.h"
#include "NotificationManager.h"
#include "MessageManager.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/Serializer.h"

void GbPpu::Init(Console* console, Gameboy* gameboy, GbMemoryManager* memoryManager, uint8_t* vram, uint8_t* oam)
{
	_console = console;
	_gameboy = gameboy;
	_memoryManager = memoryManager;
	_vram = vram;
	_oam = oam;
	
	_state = {};
	_state.Mode = PpuMode::HBlank;
	_lastFrameTime = 0;

	_outputBuffers[0] = new uint16_t[256 * 240];
	_outputBuffers[1] = new uint16_t[256 * 240];
	memset(_outputBuffers[0], 0, 256 * 240 * sizeof(uint16_t));
	memset(_outputBuffers[1], 0, 256 * 240 * sizeof(uint16_t));
	_currentBuffer = _outputBuffers[0];

	_eventViewerBuffers[0] = new uint16_t[456 * 154];
	_eventViewerBuffers[1] = new uint16_t[456 * 154];
	memset(_eventViewerBuffers[0], 0, 456 * 154 * sizeof(uint16_t));
	memset(_eventViewerBuffers[1], 0, 456 * 154 * sizeof(uint16_t));
	_currentEventViewerBuffer = _eventViewerBuffers[0];

#ifndef USEBOOTROM
	Write(0xFF40, 0x91);
	Write(0xFF42, 0x00);
	Write(0xFF43, 0x00);
	Write(0xFF45, 0x00);
	Write(0xFF47, 0xFC);
	Write(0xFF48, 0xFF);
	Write(0xFF49, 0xFF);
	Write(0xFF4A, 0);
	Write(0xFF4B, 0);
#endif
}

GbPpu::~GbPpu()
{
}

GbPpuState GbPpu::GetState()
{
	return _state;
}

uint16_t* GbPpu::GetEventViewerBuffer()
{
	return _currentEventViewerBuffer;
}

uint16_t* GbPpu::GetPreviousEventViewerBuffer()
{
	return _currentEventViewerBuffer == _eventViewerBuffers[0] ? _eventViewerBuffers[1] : _eventViewerBuffers[0];
}

void GbPpu::Exec()
{
	if(!_state.LcdEnabled) {
		//LCD is disabled, prevent IRQs, etc.
		//Not quite correct in terms of frame pacing
		if(_gameboy->GetApuCycleCount() - _lastFrameTime > 70224) {
			//More than a full frame's worth of time has passed since the last frame, send another blank frame
			_lastFrameTime = _gameboy->GetApuCycleCount();
			SendFrame();
		}
		return;
	}

	ExecCycle();
	ExecCycle();
	if(!_memoryManager->IsHighSpeed()) {
		ExecCycle();
		ExecCycle();
	}
}

void GbPpu::ExecCycle()
{
	_state.Cycle++;
	if(_state.Cycle == 456) {
		_state.Cycle = 0;

		_state.Scanline++;
		_spriteCount = 0;

		if(_state.Scanline == 144) {
			_state.Mode = PpuMode::VBlank;
			_memoryManager->RequestIrq(GbIrqSource::VerticalBlank);

			if(_state.Status & GbPpuStatusFlags::VBlankIrq) {
				_memoryManager->RequestIrq(GbIrqSource::LcdStat);
			}

			SendFrame();
		} else if(_state.Scanline == 154) {
			_state.Scanline = 0;
			_console->ProcessEvent(EventType::StartFrame);
			if(_console->IsDebugging()) {
				_currentEventViewerBuffer = _currentEventViewerBuffer == _eventViewerBuffers[0] ? _eventViewerBuffers[1] : _eventViewerBuffers[0];
				for(int i = 0; i < 456 * 154; i++) {
					_currentEventViewerBuffer[i] = 0x18C6;
				}
			}
		}

		if(_state.Scanline < 144) {
			_state.Mode = PpuMode::OamEvaluation;

			if(_state.Status & GbPpuStatusFlags::OamIrq) {
				_memoryManager->RequestIrq(GbIrqSource::LcdStat);
			}
		}

		if(_state.LyCompare == _state.Scanline && (_state.Status & GbPpuStatusFlags::CoincidenceIrq)) {
			_memoryManager->RequestIrq(GbIrqSource::LcdStat);
		}
	}

	_console->ProcessPpuCycle(_state.Scanline, _state.Cycle);

	//TODO: Dot-based renderer, currently draws at the end of the scanline
	if(_state.Scanline < 144) {
		if(_state.Cycle < 80) {
			RunSpriteEvaluation();
		} else if(_state.Mode == PpuMode::Drawing) {
			bool fetchWindow = _state.WindowEnabled && _shiftedPixels >= _state.WindowX - 7 && _state.Scanline >= _state.WindowY;
			if(_fetchWindow != fetchWindow) {
				//Switched between window & background, reset fetcher & pixel FIFO
				_fetchWindow = fetchWindow;
				_fetchColumn = 0;
				_fetcherStep = 0;
				_fifoPosition = 0;
				_fifoSize = 0;
			}
			
			ClockTileFetcher();

			if(_fetchSprite == -1 && _fifoSize > 8) {
				if(!fetchWindow && _shiftedPixels < (_state.ScrollX & 0x07)) {
					//Throw away pixels that are outside the screen due to the ScrollX value
					_fifoPosition = (_fifoPosition + 1) & 0x0F;
				} else {
					uint16_t outOffset = _state.Scanline * 256 + _drawnPixels;
					
					FifoEntry& entry = _fifoContent[_fifoPosition];
					
					uint16_t rgbColor;
					if(_gameboy->IsCgb()) {
						if(entry.Attributes & 0x40) {
							rgbColor = _state.CgbObjPalettes[entry.Color | ((entry.Attributes & 0x07) << 2)];
						} else {
							rgbColor = _state.CgbBgPalettes[entry.Color | ((entry.Attributes & 0x07) << 2)];
						}
					} else {
						uint16_t palette[4];
						if(entry.Attributes & 0x40) {
							GetPalette(palette, (entry.Attributes & 0x10) ? _state.ObjPalette1 : _state.ObjPalette0);
						} else {
							GetPalette(palette, _state.BgPalette);
						}
						rgbColor = palette[entry.Color];
					}
						_currentBuffer[outOffset] = rgbColor;
					_fifoPosition = (_fifoPosition + 1) & 0x0F;

					if(_console->IsDebugging()) {
						_currentEventViewerBuffer[456 * _state.Scanline + _state.Cycle] = rgbColor;
					}

					_drawnPixels++;
				}
				_fifoSize--;
				_shiftedPixels++;
			}

			if(_drawnPixels >= 160) {
				_state.Mode = PpuMode::HBlank;
				if(_state.Status & GbPpuStatusFlags::HBlankIrq) {
					_memoryManager->RequestIrq(GbIrqSource::LcdStat);
				}
			}
		}
	}
}

void GbPpu::RunSpriteEvaluation()
{
	if(_state.Cycle & 0x01) {
		if(_spriteCount < 10) {
			uint8_t spriteIndex = (_state.Cycle >> 1) * 4;
			int16_t sprY = (int16_t)_oam[spriteIndex] - 16;
			if(_state.Scanline >= sprY && _state.Scanline < sprY + (_state.LargeSprites ? 16 : 8)) {
				_spriteX[_spriteCount] = _oam[spriteIndex + 1];
				_spriteIndexes[_spriteCount] = spriteIndex;
				_spriteCount++;
			}
		}

		if(_state.Cycle == 79) {
			_state.Mode = PpuMode::Drawing;

			//Reset fetcher & pixel FIFO
			_fetcherStep = 0;
			_fifoPosition = 0;
			_fifoSize = 0;
			_shiftedPixels = 0;
			_drawnPixels = 0;
			_fetchSprite = -1;
			_fetchWindow = false;
			_fetchColumn = _state.ScrollX / 8;
		}
	} else {
		//Hardware probably reads sprite Y and loads the X counter with the value on the next cycle
	}
}

void GbPpu::ResetTileFetcher()
{
	_fetcherStep = 0;
}

void GbPpu::ClockTileFetcher()
{
	if(_fetchSprite < 0 && _fifoSize >= 8) {
		for(int i = 0; i < _spriteCount; i++) {
			if((int)_spriteX[i] - 8 <= _drawnPixels) {
				_fetchSprite = _spriteIndexes[i];
				_fetchSpriteOffset = _spriteX[i] < 8 ? (8 - _spriteX[i]) : 0;
				_spriteX[i] = 0xFF; //prevent processing this sprite again
				ResetTileFetcher();
				break;
			}
		}
	}

	switch(_fetcherStep++) {
		case 0: {
			//Fetch tile index
			if(_fetchSprite >= 0) {
				int16_t sprY = (int16_t)_oam[_fetchSprite] - 16;
				uint8_t sprTile = _oam[_fetchSprite + 2];
				uint8_t sprAttr = _oam[_fetchSprite + 3];
				bool vMirror = (sprAttr & 0x40) != 0;
				uint16_t tileBank = (sprAttr & 0x08) ? 0x2000 : 0x0000;

				uint8_t sprOffsetY = vMirror ? (_state.LargeSprites ? 15 : 7) - (_state.Scanline - sprY) : (_state.Scanline - sprY);
				if(_state.LargeSprites) {
					sprTile &= 0xFE;
				}

				uint16_t sprTileAddr = (sprTile * 16 + sprOffsetY * 2) | tileBank;
				_fetcherTileAddr = sprTileAddr;
				_fetcherAttributes = (sprAttr & 0xBF) | 0x40; //Use 0x40 as a marker to designate this pixel as a sprite pixel
			} else {
				uint16_t tilemapAddr;
				uint8_t yOffset;
				if(_fetchWindow) {
					tilemapAddr = _state.WindowTilemapSelect ? 0x1C00 : 0x1800;
					yOffset = _state.Scanline - _state.WindowY;
				} else {
					tilemapAddr = _state.BgTilemapSelect ? 0x1C00 : 0x1800;
					yOffset = _state.ScrollY + _state.Scanline;
				}

				uint8_t row = yOffset >> 3;
				uint16_t tileAddr = tilemapAddr + _fetchColumn + row * 32;
				uint8_t tileIndex = _vram[tileAddr];

				uint8_t attributes = _gameboy->IsCgb() ? _vram[tileAddr | 0x2000] : 0;
				bool vMirror = (attributes & 0x40) != 0;
				uint16_t tileBank = (attributes & 0x08) ? 0x2000 : 0x0000;

				uint16_t baseTile = _state.BgTileSelect ? 0 : 0x1000;
				uint8_t tileY = vMirror ? (7 - (yOffset & 0x07)) : (yOffset & 0x07);
				uint16_t tileRowAddr = baseTile + (baseTile ? (int8_t)tileIndex * 16 : tileIndex * 16) + tileY * 2;
				tileRowAddr |= tileBank;
				_fetcherTileAddr = tileRowAddr;
				_fetcherAttributes = (attributes & 0xBF);
			}
			break;
		}

		case 2: {
			//Fetch tile data (low byte)
			_fetcherTileLowByte = _vram[_fetcherTileAddr];
			break;
		}

		case 4: {
			//Fetch tile data (high byte)
			_fetcherTileHighByte = _vram[_fetcherTileAddr + 1];
			break;
		}
	}

	if(_fetcherStep > 4) {
		if(_fetchSprite >= 0) {
			PushSpriteToPixelFifo();
		} else if(_fifoSize <= 8) {
			PushTileToPixelFifo();
		}
	}
}

void GbPpu::PushSpriteToPixelFifo()
{
	_fetchSprite = -1;
	ResetTileFetcher();

	if(!_state.SpritesEnabled) {
		return;
	}

	uint8_t pos = _fifoPosition;

	//Overlap sprite
	for(int i = _fetchSpriteOffset; i < 8; i++) {
		uint8_t shift = (_fetcherAttributes & 0x20) ? i : (7 - i);
		uint8_t bits = ((_fetcherTileLowByte >> shift) & 0x01);
		bits |= ((_fetcherTileHighByte >> shift) & 0x01) << 1;

		if(bits > 0) {
			if(!(_fifoContent[pos].Attributes & 0x40) && !(_fifoContent[pos].Attributes & 0x80) && (_fifoContent[pos].Color == 0 || !(_fetcherAttributes & 0x80))) {
				//Draw pixel if the current pixel:
				// -Is a BG pixel, and
				// -Does not have the BG priority flag turned on (CGB only)
				// -Is color 0, or the sprite is NOT background priority
				_fifoContent[pos].Color = bits;
				_fifoContent[pos].Attributes = _fetcherAttributes;
			}
		}
		pos = (pos + 1) & 0x0F;
	}
}

void GbPpu::PushTileToPixelFifo()
{
	//Add new tile to fifo
	for(int i = 0; i < 8; i++) {
		uint8_t shift = (_fetcherAttributes & 0x20) ? i : (7 - i);
		uint8_t bits = ((_fetcherTileLowByte >> shift) & 0x01);
		bits |= ((_fetcherTileHighByte >> shift) & 0x01) << 1;

		uint8_t pos = (_fifoPosition + _fifoSize + i) & 0x0F;
		_fifoContent[pos].Color = _state.BgEnabled ? bits : 0;
		_fifoContent[pos].Attributes = _fetcherAttributes;
	}

	_fetchColumn = (_fetchColumn + 1) & 0x1F;
	_fifoSize += 8;
	ResetTileFetcher();
}

void GbPpu::GetPalette(uint16_t out[4], uint8_t palCfg)
{
	constexpr uint16_t rgbPalette[4] = { 0x7FFF, 0x6318, 0x318C, 0x0000 };
	out[0] = rgbPalette[palCfg & 0x03];
	out[1] = rgbPalette[(palCfg >> 2) & 0x03];
	out[2] = rgbPalette[(palCfg >> 4) & 0x03];
	out[3] = rgbPalette[(palCfg >> 6) & 0x03];
}

void GbPpu::SendFrame()
{
	_console->ProcessEvent(EventType::EndFrame);
	_state.FrameCount++;
	_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);

#ifdef LIBRETRO
	_console->GetVideoDecoder()->UpdateFrameSync(_currentBuffer, 256, 239, _state.FrameCount, false);
#else
	if(_console->GetRewindManager()->IsRewinding()) {
		_console->GetVideoDecoder()->UpdateFrameSync(_currentBuffer, 256, 239, _state.FrameCount, true);
	} else {
		_console->GetVideoDecoder()->UpdateFrame(_currentBuffer, 256, 239, _state.FrameCount);
	}
#endif

	//TODO move this somewhere that makes more sense
	uint8_t prevInput = _memoryManager->ReadInputPort();
	_console->ProcessEndOfFrame();
	uint8_t newInput = _memoryManager->ReadInputPort();
	if(prevInput != newInput) {
		_memoryManager->RequestIrq(GbIrqSource::Joypad);
	}

	_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
}

uint8_t GbPpu::Read(uint16_t addr)
{
	switch(addr) {
		case 0xFF40: return _state.Control;
		case 0xFF41:
			//FF41 - STAT - LCDC Status (R/W)
			return (
				(_state.Status & 0xF8) |
				((_state.LyCompare == _state.Scanline) ? 0x04 : 0x00) |
				(int)_state.Mode
			);

		case 0xFF42: return _state.ScrollY; //FF42 - SCY - Scroll Y (R/W)
		case 0xFF43: return _state.ScrollX; //FF43 - SCX - Scroll X (R/W)
		case 0xFF44: return _state.Scanline; //FF44 - LY - LCDC Y-Coordinate (R)
		case 0xFF45: return _state.LyCompare; //FF45 - LYC - LY Compare (R/W)
		case 0xFF47: return _state.BgPalette; //FF47 - BGP - BG Palette Data (R/W) - Non CGB Mode Only
		case 0xFF48: return _state.ObjPalette0; //FF48 - OBP0 - Object Palette 0 Data (R/W) - Non CGB Mode Only
		case 0xFF49: return _state.ObjPalette1; //FF49 - OBP1 - Object Palette 1 Data (R/W) - Non CGB Mode Only
		case 0xFF4A: return _state.WindowY; //FF4A - WY - Window Y Position (R/W)
		case 0xFF4B: return _state.WindowX; //FF4B - WX - Window X Position minus 7 (R/W)
	}
	
	LogDebug("[Debug] GB - Missing read handler: $" + HexUtilities::ToHex(addr));
	return 0;
}

void GbPpu::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0xFF40: 
			_state.Control = value; 
			if(_state.LcdEnabled != ((value & 0x80) != 0)) {
				_state.LcdEnabled = (value & 0x80) != 0;
				
				if(!_state.LcdEnabled) {
					//Reset LCD to top of screen when it gets turned on
					_state.Cycle = 0;
					_state.Scanline = 0;
					_state.Mode = PpuMode::HBlank;

					//Send a blank (white) frame
					_lastFrameTime = _gameboy->GetCycleCount();
					std::fill(_outputBuffers[0], _outputBuffers[0] + 256 * 239, 0x7FFF);
					std::fill(_outputBuffers[1], _outputBuffers[1] + 256 * 239, 0x7FFF);
					SendFrame();
				}
			}
			_state.WindowTilemapSelect = (value & 0x40) != 0;
			_state.WindowEnabled = (value & 0x20) != 0;
			_state.BgTileSelect = (value & 0x10) != 0;
			_state.BgTilemapSelect = (value & 0x08) != 0;
			_state.LargeSprites = (value & 0x04) != 0;
			_state.SpritesEnabled = (value & 0x02) != 0;
			_state.BgEnabled = (value & 0x01) != 0;
			break;

		case 0xFF41: _state.Status = value & 0xF8; break;
		case 0xFF42: _state.ScrollY = value; break;
		case 0xFF43: _state.ScrollX = value; break;
		case 0xFF45: _state.LyCompare = value; break;
		
		case 0xFF46:
			//OAM DMA
			//TODO restrict CPU accesses to high ram during this
			//TODO timing
			for(int i = 0; i < 0xA0; i++) {
				_memoryManager->Write(0xFE00 | i, _memoryManager->Read((value << 8) | i, MemoryOperationType::DmaRead));
			}
			break;

		case 0xFF47: _state.BgPalette = value; break;
		case 0xFF48: _state.ObjPalette0 = value; break;
		case 0xFF49: _state.ObjPalette1 = value; break;
		case 0xFF4A: _state.WindowY = value; break;
		case 0xFF4B: _state.WindowX = value; break;

		default:
			LogDebug("[Debug] GB - Missing write handler: $" + HexUtilities::ToHex(addr));
			break;
	}
}

uint8_t GbPpu::ReadVram(uint16_t addr)
{
	if((int)_state.Mode <= (int)PpuMode::OamEvaluation) {
		return _vram[(_state.CgbVramBank << 13) | (addr & 0x1FFF)];
	} else {
		return 0xFF;
	}
}

void GbPpu::WriteVram(uint16_t addr, uint8_t value)
{
	if((int)_state.Mode <= (int)PpuMode::OamEvaluation) {
		_vram[(_state.CgbVramBank << 13) | (addr & 0x1FFF)] = value;
	}
}

uint8_t GbPpu::ReadOam(uint8_t addr)
{
	if(addr < 0xA0) {
		if((int)_state.Mode <= (int)PpuMode::VBlank) {
			return _oam[addr];
		} else {
			return 0xFF;
		}
	}
	return 0;
}

void GbPpu::WriteOam(uint8_t addr, uint8_t value)
{
	if(addr < 0xA0 && (int)_state.Mode <= (int)PpuMode::VBlank) {
		_oam[addr] = value;
	}
}

uint8_t GbPpu::ReadCgbRegister(uint16_t addr)
{
	switch(addr) {
		case 0xFF4F: return _state.CgbVramBank;
		case 0xFF51: return _state.CgbDmaSource >> 8;
		case 0xFF52: return _state.CgbDmaSource & 0xFF;
		case 0xFF53: return _state.CgbDmaDest >> 8;
		case 0xFF54: return _state.CgbDmaDest & 0xFF;
		case 0xFF55: return _state.CgbDmaLength | (_state.CgbHdmaMode ? 0x80 : 0);
		case 0xFF68: return _state.CgbBgPalPosition | (_state.CgbBgPalAutoInc ? 0x80 : 0);
		case 0xFF69: return (_state.CgbBgPalettes[_state.CgbBgPalPosition >> 1] >> ((_state.CgbBgPalPosition & 0x01) ? 8 : 0) & 0xFF);
		case 0xFF6A: return _state.CgbObjPalPosition | (_state.CgbObjPalAutoInc ? 0x80 : 0);
		case 0xFF6B: return (_state.CgbObjPalettes[_state.CgbObjPalPosition >> 1] >> ((_state.CgbObjPalPosition & 0x01) ? 8 : 0) & 0xFF);
	}
	LogDebug("[Debug] GBC - Missing read handler: $" + HexUtilities::ToHex(addr));
	return 0;
}

void GbPpu::WriteCgbRegister(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0xFF4F: _state.CgbVramBank = value & 0x01; break;
		case 0xFF51: _state.CgbDmaSource = (_state.CgbDmaSource & 0xFF) | (value << 8); break;
		case 0xFF52: _state.CgbDmaSource = (_state.CgbDmaSource & 0xFF00) | value; break;
		case 0xFF53: _state.CgbDmaDest = (_state.CgbDmaDest & 0xFF) | (value << 8); break;
		case 0xFF54: _state.CgbDmaDest = (_state.CgbDmaDest & 0xFF00) | value; break;
		case 0xFF55: 
			_state.CgbDmaLength = value & 0x7F;
			_state.CgbHdmaMode = (value & 0x80) != 0;

			if(!_state.CgbHdmaMode) {
				//TODO check invalid dma sources/etc.
				//TODO timing
				for(int i = 0; i < _state.CgbDmaLength * 16; i++) {
					uint16_t dst = 0x8000 | (((_state.CgbDmaDest & 0x1FF0) + i) & 0x1FFF);
					_memoryManager->Write(dst, _memoryManager->Read((_state.CgbDmaSource & 0xFFF0) + i, MemoryOperationType::DmaRead));
				}
				_state.CgbDmaLength = 0x7F;
			} else {
				MessageManager::Log("TODO HDMA");
			}
			break;

		case 0xFF68:
			//FF68 - BCPS/BGPI - CGB Mode Only - Background Palette Index
			_state.CgbBgPalPosition = value & 0x3F;
			_state.CgbBgPalAutoInc = (value & 0x80) != 0;
			break;

		case 0xFF69: {
			//FF69 - BCPD/BGPD - CGB Mode Only - Background Palette Data
			WriteCgbPalette(_state.CgbBgPalPosition, _state.CgbBgPalettes, _state.CgbBgPalAutoInc, value);
			break;
		}

		case 0xFF6A:
			//FF6A - OCPS/OBPI - CGB Mode Only - Sprite Palette Index
			_state.CgbObjPalPosition = value & 0x3F;
			_state.CgbObjPalAutoInc = (value & 0x80) != 0;
			break;

		case 0xFF6B:
			//FF6B - OCPD/OBPD - CGB Mode Only - Sprite Palette Data
			WriteCgbPalette(_state.CgbObjPalPosition, _state.CgbObjPalettes, _state.CgbObjPalAutoInc, value);
			break;

		default:
			LogDebug("[Debug] GBC - Missing write handler: $" + HexUtilities::ToHex(addr));
			break;
	}
}

void GbPpu::WriteCgbPalette(uint8_t& pos, uint16_t* pal, bool autoInc, uint8_t value)
{
	if((int)_state.Mode <= (int)PpuMode::OamEvaluation) {
		if(pos & 0x01) {
			pal[pos >> 1] = (pal[pos >> 1] & 0xFF) | ((value & 0x7F) << 8);
		} else {
			pal[pos >> 1] = (pal[pos >> 1] & 0xFF00) | value;
		}
	}

	if(autoInc) {
		pos = (pos + 1) & 0x3F;
	}
}

void GbPpu::Serialize(Serializer& s)
{
	s.Stream(
		_state.Scanline, _state.Cycle, _state.Mode, _state.LyCompare, _state.BgPalette, _state.ObjPalette0, _state.ObjPalette1,
		_state.ScrollX, _state.ScrollY, _state.WindowX, _state.WindowY, _state.Control, _state.LcdEnabled, _state.WindowTilemapSelect,
		_state.WindowEnabled, _state.BgTileSelect, _state.BgTilemapSelect, _state.LargeSprites, _state.SpritesEnabled, _state.BgEnabled,
		_state.Status, _state.FrameCount, _lastFrameTime,
		_state.CgbBgPalAutoInc, _state.CgbBgPalPosition, _state.CgbDmaDest, _state.CgbDmaLength, _state.CgbDmaSource, _state.CgbHdmaMode,
		_state.CgbObjPalAutoInc, _state.CgbObjPalPosition, _state.CgbVramBank
	);

	s.StreamArray(_state.CgbBgPalettes, 4 * 8);
	s.StreamArray(_state.CgbObjPalettes, 4 * 8);

	s.Stream(
		_fifoPosition, _fifoSize, _shiftedPixels, _drawnPixels,
		_fetcherAttributes, _fetcherStep, _fetchColumn, _fetcherTileAddr,
		_fetcherTileLowByte, _fetcherTileHighByte, _fetchWindow, _fetchSprite,
		_spriteCount, _fetchSpriteOffset
	);

	s.StreamArray(_spriteX, 10);
	s.StreamArray(_spriteIndexes, 10);

	for(int i = 0; i < 16; i++) {
		s.Stream(_fifoContent[i].Color, _fifoContent[i].Attributes);
	}
}
