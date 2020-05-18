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
	_drawModeLength = (_state.ScrollX & 0x07) + 160 + 8 * 5;
	_lastFrameTime = 0;

	_outputBuffers[0] = new uint16_t[256 * 240];
	_outputBuffers[1] = new uint16_t[256 * 240];
	memset(_outputBuffers[0], 0, 256 * 240 * sizeof(uint16_t));
	memset(_outputBuffers[1], 0, 256 * 240 * sizeof(uint16_t));
	_currentBuffer = _outputBuffers[0];

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

void GbPpu::Exec()
{
	if(!_state.LcdEnabled) {
		//LCD is disabled, prevent IRQs, etc.
		//Not quite correct in terms of frame pacing
		if(_gameboy->GetCycleCount() - _lastFrameTime > 70224) {
			//More than a full frame's worth of time has passed since the last frame, send another blank frame
			_lastFrameTime = _gameboy->GetCycleCount();
			SendFrame();
		}
		return;
	}

	ExecCycle();
	ExecCycle();
	ExecCycle();
	ExecCycle();
}

void GbPpu::ExecCycle()
{
	_state.Cycle++;
	if(_state.Cycle == 456) {
		_state.Cycle = 0;

		_state.Scanline++;

		if(_state.Scanline == 144) {
			_state.Mode = PpuMode::VBlank;
			_memoryManager->RequestIrq(GbIrqSource::VerticalBlank);

			if(_state.Status & GbPpuStatusFlags::VBlankIrq) {
				_memoryManager->RequestIrq(GbIrqSource::LcdStat);
			}

			SendFrame();
		} else if(_state.Scanline == 154) {
			_console->ProcessEvent(EventType::StartFrame);
			_state.Scanline = 0;
		}

		if(_state.Scanline < 144) {
			_state.Mode = PpuMode::OamEvaluation;
			_drawModeLength = (_state.ScrollX & 0x07) + 160 + 8 * 5;

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
			if(_state.Cycle == 79) {
				_state.Mode = PpuMode::Drawing;
			}
		} else if(_state.Mode == PpuMode::Drawing) {
			_drawModeLength--;
			if(_drawModeLength == 0) {
				_state.Mode = PpuMode::HBlank;
				if(_state.Status & GbPpuStatusFlags::HBlankIrq) {
					_memoryManager->RequestIrq(GbIrqSource::LcdStat);
				}
				RenderScanline();
			}
		}
	}
}

void GbPpu::GetPalette(uint16_t out[4], uint8_t palCfg)
{
	constexpr uint16_t rgbPalette[4] = { 0x7FFF, 0x6318, 0x318C, 0x0000 };
	out[0] = rgbPalette[palCfg & 0x03];
	out[1] = rgbPalette[(palCfg >> 2) & 0x03];
	out[2] = rgbPalette[(palCfg >> 4) & 0x03];
	out[3] = rgbPalette[(palCfg >> 6) & 0x03];
}

void GbPpu::RenderScanline()
{
	uint16_t bgColors[4];
	uint16_t oamColors[2][4];
	GetPalette(bgColors, _state.BgPalette);
	GetPalette(oamColors[0], _state.ObjPalette0);
	GetPalette(oamColors[1], _state.ObjPalette1);

	uint8_t visibleSprites[10] = {};
	uint8_t spriteCount = 0;
	for(uint8_t i = 0; i < 0xA0; i += 4) {
		int16_t sprY = (int16_t)_oam[i] - 16;
		if(_state.Scanline >= sprY && _state.Scanline < sprY + (_state.LargeSprites ? 16 : 8)) {
			visibleSprites[spriteCount] = i;
			spriteCount++;
			if(spriteCount == 10) {
				break;
			}
		}
	}

	if(spriteCount > 1) {
		//Sort sprites by their X position first, and then by their index when X values are equal
		std::sort(visibleSprites, visibleSprites + spriteCount, [=](uint8_t a, uint8_t b) {
			if(_oam[a + 1] == _oam[b + 1]) {
				return a < b;
			} else {
				return _oam[a + 1] < _oam[b + 1];
			}
		});
	}

	uint8_t xOffset;
	uint8_t yOffset;
	uint16_t tilemapAddr;
	uint16_t baseTile = _state.BgTileSelect ? 0 : 0x1000;

	for(int x = 0; x < 160; x++) {
		uint8_t bgColor = 0;
		uint16_t outOffset = _state.Scanline * 256 + x;
		if(_state.BgEnabled) {

			if(_state.WindowEnabled && x >= _state.WindowX - 7 && _state.Scanline >= _state.WindowY) {
				//Draw window content instead
				tilemapAddr = _state.WindowTilemapSelect ? 0x1C00 : 0x1800;
				xOffset = x - (_state.WindowX - 7);
				yOffset = _state.Scanline - _state.WindowY;
			} else {
				//Draw regular tilemap
				tilemapAddr = _state.BgTilemapSelect ? 0x1C00 : 0x1800;
				xOffset = _state.ScrollX + x;
				yOffset = _state.ScrollY + _state.Scanline;
			}

			uint8_t row = yOffset >> 3;
			uint8_t tileY = yOffset & 0x07;
			uint8_t column = xOffset >> 3;
			uint8_t tileIndex = _vram[tilemapAddr + column + row * 32];

			uint16_t tileRowAddr = baseTile + (baseTile ? (int8_t)tileIndex * 16 : tileIndex * 16) + tileY * 2;
			uint8_t shift = 7 - (xOffset & 0x07);
			bgColor = ((_vram[tileRowAddr] >> shift) & 0x01) | (((_vram[tileRowAddr + 1] >> shift) & 0x01) << 1);
		}
		_currentBuffer[outOffset] = bgColors[bgColor];

		if(_state.SpritesEnabled && spriteCount) {
			for(int i = 0; i < spriteCount; i++) {
				uint8_t sprIndex = visibleSprites[i];
				int16_t sprX = (int16_t)_oam[sprIndex + 1] - 8;
				if(x >= sprX && x < sprX + 8) {
					int16_t sprY = (int16_t)_oam[sprIndex] - 16;
					uint8_t sprTile = _oam[sprIndex + 2];
					bool bgPriority = (_oam[sprIndex + 3] & 0x80);
					bool vMirror = (_oam[sprIndex + 3] & 0x40);
					bool hMirror = (_oam[sprIndex + 3] & 0x20);
					bool palette = (_oam[sprIndex + 3] & 0x10);

					uint8_t sprOffsetY = vMirror ? (_state.LargeSprites ? 15 : 7) - (_state.Scanline - sprY) : (_state.Scanline - sprY);
					if(_state.LargeSprites) {
						sprTile &= 0xFE;
					}
					uint8_t sprShiftX = hMirror ? (x - sprX) : 7 - (x - sprX);

					uint16_t sprTileAddr = sprTile * 16 + sprOffsetY * 2;
					uint8_t sprColor = ((_vram[sprTileAddr] >> sprShiftX) & 0x01) | (((_vram[sprTileAddr + 1] >> sprShiftX) & 0x01) << 1);
					if(sprColor > 0 && (bgColor == 0 || !bgPriority)) {
						_currentBuffer[outOffset] = oamColors[(int)palette][sprColor];
						break;
					}
				}
			}
		}
	}
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
					std::fill(_currentBuffer, _currentBuffer + 256 * 239, 0x7FFF);
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
			//OAM DMA - TODO, restrict CPU accesses to high ram during this?
			for(int i = 0; i < 0xA0; i++) {
				WriteOam(i, _memoryManager->Read((value << 8) | i, MemoryOperationType::DmaRead));
			}
			break;

		case 0xFF47: _state.BgPalette = value; break;
		case 0xFF48: _state.ObjPalette0 = value; break;
		case 0xFF49: _state.ObjPalette1 = value; break;
		case 0xFF4A: _state.WindowY = value; break;
		case 0xFF4B: _state.WindowX = value; break;
	}
}

uint8_t GbPpu::ReadVram(uint16_t addr)
{
	if((int)_state.Mode <= (int)PpuMode::OamEvaluation) {
		return _vram[addr & 0x1FFF];
	} else {
		return 0xFF;
	}
}

void GbPpu::WriteVram(uint16_t addr, uint8_t value)
{
	if((int)_state.Mode <= (int)PpuMode::OamEvaluation) {
		_vram[addr & 0x1FFF] = value;
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

void GbPpu::Serialize(Serializer& s)
{
	s.Stream(
		_state.Scanline, _state.Cycle, _state.Mode, _state.LyCompare, _state.BgPalette, _state.ObjPalette0, _state.ObjPalette1,
		_state.ScrollX, _state.ScrollY, _state.WindowX, _state.WindowY, _state.Control, _state.LcdEnabled, _state.WindowTilemapSelect,
		_state.WindowEnabled, _state.BgTileSelect, _state.BgTilemapSelect, _state.LargeSprites, _state.SpritesEnabled, _state.BgEnabled,
		_state.Status, _state.FrameCount, _lastFrameTime
	);
}
