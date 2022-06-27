#include "stdafx.h"
#include "Gameboy/GbPpu.h"
#include "Gameboy/GbTypes.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbMemoryManager.h"
#include "Gameboy/GbDmaController.h"
#include "Gameboy/GbConstants.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/RewindManager.h"
#include "Shared/BaseControlManager.h"
#include "Shared/RenderedFrame.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/NotificationManager.h"
#include "Shared/MessageManager.h"
#include "SNES/Coprocessors/SGB/SuperGameboy.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"
#include "EventType.h"

constexpr uint16_t evtColors[6] = { 0x18C6, 0x294A, 0x108C, 0x4210, 0x3084, 0x1184 };

void GbPpu::Init(Emulator* emu, Gameboy* gameboy, GbMemoryManager* memoryManager, GbDmaController* dmaController, uint8_t* vram, uint8_t* oam)
{
	_emu = emu;
	_gameboy = gameboy;
	_memoryManager = memoryManager;
	_dmaController = dmaController;
	_vram = vram;
	_oam = oam;

	_outputBuffers[0] = new uint16_t[GbConstants::PixelCount];
	_outputBuffers[1] = new uint16_t[GbConstants::PixelCount];
	memset(_outputBuffers[0], 0, GbConstants::PixelCount * sizeof(uint16_t));
	memset(_outputBuffers[1], 0, GbConstants::PixelCount * sizeof(uint16_t));
	_currentBuffer = _outputBuffers[0];

	_eventViewerBuffers[0] = new uint16_t[456 * 154];
	_eventViewerBuffers[1] = new uint16_t[456 * 154];
	memset(_eventViewerBuffers[0], 0, 456 * 154 * sizeof(uint16_t));
	memset(_eventViewerBuffers[1], 0, 456 * 154 * sizeof(uint16_t));
	_currentEventViewerBuffer = _eventViewerBuffers[0];

	_state = {};
	_state.Cycle = -1;
	_state.Mode = PpuMode::HBlank;
	_state.CgbEnabled = _gameboy->IsCgb();
	_lastFrameTime = 0;

	UpdatePalette();

	Write(0xFF48, 0xFF);
	Write(0xFF49, 0xFF);
}

GbPpu::~GbPpu()
{
}

GbPpuState GbPpu::GetState()
{
	return _state;
}

GbPpuState& GbPpu::GetStateRef()
{
	return _state;
}

uint16_t* GbPpu::GetOutputBuffer()
{
	return _currentBuffer;
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

	uint8_t cyclesToRun = _memoryManager->IsHighSpeed() ? 1 : 2;
	for(int i = 0; i < cyclesToRun; i++) {
		_state.Cycle++;
		if(_state.IdleCycles > 0) {
			_state.IdleCycles--;
			ProcessPpuCycle();
			continue;
		}

		ExecCycle();
	}
}

void GbPpu::ExecCycle()
{
	PpuMode oldMode = _state.IrqMode;

	if(_state.Scanline < 144) {
		if(_state.Scanline == 0 && _isFirstFrame) {
			ProcessFirstScanlineAfterPowerOn();
		} else {
			ProcessVisibleScanline();
		}
	} else {
		ProcessVblankScanline();
	}

	if(_state.Mode == PpuMode::Drawing) {
		RunDrawCycle();
		if(_drawnPixels == 160) {
			//Mode turns to hblank on the same cycle as the last pixel is output (IRQ is on next cycle)
			_state.Mode = PpuMode::HBlank;
			if(_state.Scanline < 143) {
				//"This mode will transfer one block (16 bytes) during each H-Blank. No data is transferred during VBlank (LY = 143 – 153)"
				_dmaController->ProcessHdma();
			}
		}
	} else if(_state.Mode == PpuMode::OamEvaluation) {
		RunSpriteEvaluation();
	}

	bool coincidenceFlag = (_state.LyCompare == _state.LyForCompare);
	if(_state.IrqMode != oldMode || _state.LyCoincidenceFlag != coincidenceFlag) {
		_state.LyCoincidenceFlag = coincidenceFlag;
		UpdateStatIrq();
	}

	ProcessPpuCycle();
}

void GbPpu::ProcessVblankScanline()
{
	switch(_state.Cycle) {
		case 2:
			if(_state.Scanline == 144) {
				_state.IrqMode = PpuMode::OamEvaluation;
			}
			break;

		case 4:
			if(_state.Scanline < 153) {
				_state.LyForCompare = _state.Scanline;
				if(_state.Scanline == 144) {
					_state.Mode = PpuMode::VBlank;
					_state.IrqMode = PpuMode::VBlank;
					_windowCounter = -1;
					_memoryManager->RequestIrq(GbIrqSource::VerticalBlank);
					SendFrame();
				}
			}
			break;

		case 6:
			if(_state.Scanline == 153) {
				_state.Ly = 0;
				_state.LyForCompare = _state.Scanline;
			}
			break;

		case 8:
			if(_state.Scanline == 153) {
				_state.LyForCompare = -1;
			}
			break;

		case 12:
			if(_state.Scanline == 153) {
				_state.LyForCompare = 0;
			}
			_state.IdleCycles = 456 - 12 - 1;
			break;

		case 456:
			_state.Cycle = 0;
			_state.Scanline++;

			if(_state.Scanline == 154) {
				_state.Scanline = 0;
				_state.Ly = 0;
				_state.LyForCompare = 0;

				if(_emu->IsDebugging()) {
					_emu->ProcessEvent(EventType::GbStartFrame);
					_currentEventViewerBuffer = _currentEventViewerBuffer == _eventViewerBuffers[0] ? _eventViewerBuffers[1] : _eventViewerBuffers[0];
				}
			} else {
				_state.Ly = _state.Scanline;
				_state.LyForCompare = -1;
			}
			break;
	}
}

void GbPpu::ProcessFirstScanlineAfterPowerOn()
{
	if(_drawnPixels == 160) {
		//IRQ flag for Hblank is 1 cycle late compared to the mode register
		_state.IrqMode = PpuMode::HBlank;
		_drawnPixels = 0;
		_state.IdleCycles = 448 - _state.Cycle - 1;
	}

	switch(_state.Cycle) {
		case 1:
			_state.IrqMode = PpuMode::NoIrq;
			break;

		case 79:
			_latchWindowX = _state.WindowX;
			_latchWindowY = _state.WindowY;
			_latchWindowEnabled = _state.WindowEnabled;
			_state.Mode = PpuMode::Drawing;
			_state.IrqMode = PpuMode::Drawing;
			ResetRenderer();
			_rendererIdle = true;
			break;

		case 84:
			_rendererIdle = false;
			break;

		case 448:
			_state.Cycle = 0;
			_state.Scanline++;
			_drawnPixels = 0;
			_state.Mode = PpuMode::HBlank;
			_state.IrqMode = PpuMode::HBlank;
			break;
	}
}

void GbPpu::ProcessVisibleScanline()
{
	if(_drawnPixels == 160) {
		//IRQ flag for Hblank is 1 cycle late compared to the mode register
		_state.IrqMode = PpuMode::HBlank;
		_drawnPixels = 0;
		_state.IdleCycles = 456 - _state.Cycle - 1;
	}

	switch(_state.Cycle) {
		case 3:
			_state.Ly = _state.Scanline;

			if(_state.Scanline > 0) {
				//On scanlines 1-143, the OAM IRQ fires 1 cycle early
				_state.IrqMode = PpuMode::OamEvaluation;
				_state.LyForCompare = -1;
			} else {
				//On scanline 0, hblank gets set for 1 cycle here
				_state.Mode = PpuMode::HBlank;
			}
			break;

		case 4:
			_spriteCount = 0;
			_state.LyForCompare = _state.Scanline;
			_state.Mode = PpuMode::OamEvaluation;
			_state.IrqMode = PpuMode::OamEvaluation;
			break;

		case 5:
			//Turning on OAM IRQs in the middle of evaluation has no effect?
			//Or is this a patch to get the proper behavior for the STAT write bug?
			_state.IrqMode = PpuMode::NoIrq;
			break;

		case 84:
			_latchWindowX = _state.WindowX;
			_latchWindowY = _state.WindowY;
			_latchWindowEnabled = _state.WindowEnabled;
			_state.Mode = PpuMode::Drawing;
			_state.IrqMode = PpuMode::Drawing;
			_rendererIdle = true;
			ResetRenderer();
			break;

		case 89:
			_rendererIdle = false;
			break;

		case 456:
			_state.Cycle = 0;
			_state.Scanline++;
			if(_state.Scanline == 144) {
				_state.Ly = 144;
				_state.LyForCompare = -1;
			}
			break;
	}
}

void GbPpu::ProcessPpuCycle()
{
	if(_emu->IsDebugging()) {
		_emu->ProcessPpuCycle<CpuType::Gameboy>();
		if(_state.Mode <= PpuMode::OamEvaluation) {
			_currentEventViewerBuffer[456 * _state.Scanline + _state.Cycle] = evtColors[(int)_state.Mode];
		} else if(_prevDrawnPixels != _drawnPixels && _drawnPixels > 0) {
			uint16_t color = _currentBuffer[_state.Scanline * GbConstants::ScreenWidth + (_drawnPixels - 1)];
			_currentEventViewerBuffer[456 * _state.Scanline + _state.Cycle] = color;
		} else {
			_currentEventViewerBuffer[456 * _state.Scanline + _state.Cycle] = evtColors[(int)_evtColor];
		}
		_prevDrawnPixels = _drawnPixels;
	}
}

void GbPpu::RunDrawCycle()
{
	if(_rendererIdle) {
		//Idle cycles
		_evtColor = EvtColor::RenderingIdle;
		return;
	}

	bool fetchWindow = _latchWindowEnabled && _drawnPixels >= _latchWindowX - 7 && _state.Scanline >= _latchWindowY;
	if(_fetchWindow != fetchWindow) {
		//Switched between window & background, reset fetcher & pixel FIFO
		_fetchWindow = fetchWindow;
		_fetchColumn = 0;
		_windowCounter++;

		_bgFetcher.Step = 0;
		_bgFifo.Reset();

		//Idle cycle when switching to window
		_evtColor = EvtColor::RenderingIdle;
		return;
	}

	FindNextSprite();
	if(_fetchSprite >= 0 && _bgFetcher.Step >= 5 && _bgFifo.Size > 0) {
		_evtColor = EvtColor::RenderingOamLoad;
		ClockSpriteFetcher();
		FindNextSprite();
		return;
	}

	if(_fetchSprite == -1 && _bgFifo.Size > 0) {
		if(_drawnPixels >= 0) {
			uint16_t outOffset = _state.Scanline * GbConstants::ScreenWidth + _drawnPixels;

			GbFifoEntry entry = _bgFifo.Content[_bgFifo.Position];
			GbFifoEntry sprite = _oamFifo.Content[_oamFifo.Position];
			if(sprite.Color != 0 && (entry.Color == 0 || (!(sprite.Attributes & 0x80) && !(entry.Attributes & 0x80)) || (_state.CgbEnabled && !_state.BgEnabled))) {
				//Use sprite pixel if:
				//  -BG color is 0, OR
				//  -Sprite is background priority AND BG does not have its priority bit set, OR
				//  -On CGB, the "bg enabled" flag is cleared, causing all sprites to appear above BG tiles
				if(_state.CgbEnabled) {
					_currentBuffer[outOffset] = _state.CgbObjPalettes[sprite.Color | ((sprite.Attributes & 0x07) << 2)];
				} else {
					uint8_t colorIndex = (((sprite.Attributes & 0x10) ? _state.ObjPalette1 : _state.ObjPalette0) >> (sprite.Color * 2)) & 0x03;
					if(_gameboy->IsSgb()) {
						_gameboy->GetSgb()->WriteLcdColor(_state.Scanline, (uint8_t)_drawnPixels, colorIndex);
					} 
					_currentBuffer[outOffset] = _state.CgbObjPalettes[((sprite.Attributes & 0x10) ? 4 : 0) | colorIndex];
				}
			} else {
				if(_state.CgbEnabled) {
					_currentBuffer[outOffset] = _state.CgbBgPalettes[entry.Color | ((entry.Attributes & 0x07) << 2)];
				} else {
					uint8_t colorIndex = (_state.BgPalette >> (entry.Color * 2)) & 0x03;
					if(_gameboy->IsSgb()) {
						_gameboy->GetSgb()->WriteLcdColor(_state.Scanline, (uint8_t)_drawnPixels, colorIndex);
					}
					_currentBuffer[outOffset] = _state.CgbBgPalettes[colorIndex];
				}
			}
		}

		_bgFifo.Pop();
		_drawnPixels++;

		if(_oamFifo.Size > 0) {
			_oamFifo.Pop();
		}
	}

	ClockTileFetcher();
}

void GbPpu::RunSpriteEvaluation()
{
	if(_state.Cycle & 0x01) {
		if(_spriteCount < 10) {
			uint8_t spriteIndex = ((_state.Cycle - 4) >> 1) * 4;
			int16_t sprY = _dmaController->IsOamDmaRunning() ? 0xFF : ((int16_t)_oam[spriteIndex] - 16);
			if(_state.Scanline >= sprY && _state.Scanline < sprY + (_state.LargeSprites ? 16 : 8)) {
				_spriteX[_spriteCount] = _oam[spriteIndex + 1];
				_spriteIndexes[_spriteCount] = spriteIndex;
				_spriteCount++;
			}
		}
	} else {
		//TODO check proper timing for even&odd cycles
	}
}

void GbPpu::ResetRenderer()
{	
	//Reset fetcher & pixel FIFO
	_oamFifo.Reset();
	_oamFetcher.Step = 0;

	_bgFifo.Reset();
	_bgFifo.Size = 8;
	_bgFetcher.Step = 0;

	_drawnPixels = -8 - (_state.ScrollX & 0x07);
	_fetchSprite = -1;
	_fetchWindow = false;
	_fetchColumn = _state.ScrollX / 8;
}

void GbPpu::ClockSpriteFetcher()
{
	switch(_oamFetcher.Step++) {
		case 1: {
			//Fetch tile index
			int16_t sprY = (int16_t)_oam[_fetchSprite] - 16;
			uint8_t sprTile = _oam[_fetchSprite + 2];
			uint8_t sprAttr = _oam[_fetchSprite + 3];
			bool vMirror = (sprAttr & 0x40) != 0;
			uint16_t tileBank = _state.CgbEnabled ? ((sprAttr & 0x08) ? 0x2000 : 0x0000) : 0;

			uint8_t sprOffsetY = vMirror ? (_state.LargeSprites ? 15 : 7) - (_state.Scanline - sprY) : (_state.Scanline - sprY);
			if(_state.LargeSprites) {
				sprTile &= 0xFE;
			}

			uint16_t sprTileAddr = (sprTile * 16 + sprOffsetY * 2) | tileBank;
			_oamFetcher.Addr = sprTileAddr;
			_oamFetcher.Attributes = sprAttr;
			break;
		}

		case 3: _oamFetcher.LowByte = _vram[_oamFetcher.Addr]; break;

		case 5: {
			//Fetch sprite data (high byte)
			_oamFetcher.HighByte = _vram[_oamFetcher.Addr + 1];
			PushSpriteToPixelFifo();
			break;
		}
	}
}

void GbPpu::FindNextSprite()
{
	if(_fetchSprite < 0 && (_state.SpritesEnabled || _state.CgbEnabled)) {
		for(int i = 0; i < _spriteCount; i++) {
			if((int)_spriteX[i] - 8 == _drawnPixels) {
				_fetchSprite = _spriteIndexes[i];
				_spriteX[i] = 0xFF; //Prevent processing the same sprite again
				_oamFetcher.Step = 0;
				break;
			}
		}
	}
}

void GbPpu::ClockTileFetcher()
{
	_evtColor = EvtColor::RenderingBgLoad;

	switch(_bgFetcher.Step++) {
		case 1: {
			//Fetch tile index
			uint16_t tilemapAddr;
			uint8_t yOffset;
			if(_fetchWindow) {
				tilemapAddr = _state.WindowTilemapSelect ? 0x1C00 : 0x1800;
				yOffset = (uint8_t)_windowCounter;
			} else {
				tilemapAddr = _state.BgTilemapSelect ? 0x1C00 : 0x1800;
				yOffset = _state.ScrollY + _state.Scanline;
			}

			uint8_t row = yOffset >> 3;
			uint16_t tileAddr = tilemapAddr + _fetchColumn + row * 32;
			uint8_t tileIndex = _vram[tileAddr];

			uint8_t attributes = _state.CgbEnabled ? _vram[tileAddr | 0x2000] : 0;
			bool vMirror = (attributes & 0x40) != 0;
			uint16_t tileBank = (attributes & 0x08) ? 0x2000 : 0x0000;

			uint16_t baseTile = _state.BgTileSelect ? 0 : 0x1000;
			uint8_t tileY = vMirror ? (7 - (yOffset & 0x07)) : (yOffset & 0x07);
			uint16_t tileRowAddr = baseTile + (baseTile ? (int8_t)tileIndex * 16 : tileIndex * 16) + tileY * 2;
			tileRowAddr |= tileBank;
			_bgFetcher.Addr = tileRowAddr;
			_bgFetcher.Attributes = (attributes & 0xBF);
			break;
		}

		case 3: {
			//Fetch tile data (low byte)
			_bgFetcher.LowByte = _vram[_bgFetcher.Addr];
			break;
		}

		case 5: {
			//Fetch tile data (high byte)
			_bgFetcher.HighByte = _vram[_bgFetcher.Addr + 1];
			
			//Fallthrough
		}

		case 6:
		case 7:
			if(_bgFifo.Size == 0) {
				PushTileToPixelFifo();
			} else if(_bgFetcher.Step == 8) {
				//Wait until fifo is empty to push pixels
				_bgFetcher.Step = 7;
			}
			break;
	}
}

void GbPpu::PushSpriteToPixelFifo()
{
	_fetchSprite = -1;
	_oamFetcher.Step = 0;

	if(!_state.SpritesEnabled) {
		return;
	}

	uint8_t pos = _oamFifo.Position;

	//Overlap sprite
	for(int i = 0; i < 8; i++) {
		uint8_t shift = (_oamFetcher.Attributes & 0x20) ? i : (7 - i);
		uint8_t bits = ((_oamFetcher.LowByte >> shift) & 0x01);
		bits |= ((_oamFetcher.HighByte >> shift) & 0x01) << 1;

		if(bits > 0 && _oamFifo.Content[pos].Color == 0) {
			_oamFifo.Content[pos].Color = bits;
			_oamFifo.Content[pos].Attributes = _oamFetcher.Attributes;
		}
		pos = (pos + 1) & 0x07;
	}
	_oamFifo.Size = 8;
}

void GbPpu::PushTileToPixelFifo()
{
	//Add new tile to fifo
	for(int i = 0; i < 8; i++) {
		uint8_t shift = (_bgFetcher.Attributes & 0x20) ? i : (7 - i);
		uint8_t bits = ((_bgFetcher.LowByte >> shift) & 0x01);
		bits |= ((_bgFetcher.HighByte >> shift) & 0x01) << 1;

		_bgFifo.Content[i].Color = (_state.CgbEnabled || _state.BgEnabled) ? bits : 0;
		_bgFifo.Content[i].Attributes = _bgFetcher.Attributes;
	}

	_fetchColumn = (_fetchColumn + 1) & 0x1F;
	_bgFifo.Position = 0;
	_bgFifo.Size = 8;
	_bgFetcher.Step = 0;
}

void GbPpu::UpdateStatIrq()
{
	bool irqFlag = (
		_state.LcdEnabled &&
		((_state.LyCoincidenceFlag && (_state.Status & GbPpuStatusFlags::CoincidenceIrq)) ||
		(_state.IrqMode == PpuMode::HBlank && (_state.Status & GbPpuStatusFlags::HBlankIrq)) ||
		(_state.IrqMode == PpuMode::OamEvaluation && (_state.Status & GbPpuStatusFlags::OamIrq)) ||
		(_state.IrqMode == PpuMode::VBlank && (_state.Status & GbPpuStatusFlags::VBlankIrq)))
	);

	if(irqFlag && !_state.StatIrqFlag) {
		_memoryManager->RequestIrq(GbIrqSource::LcdStat);
	}
	_state.StatIrqFlag = irqFlag;
}

uint32_t GbPpu::GetFrameCount()
{
	return _state.FrameCount;
}

uint8_t GbPpu::GetScanline()
{
	return _state.Scanline;
}

uint16_t GbPpu::GetCycle()
{
	return _state.Cycle;
}

bool GbPpu::IsLcdEnabled()
{
	return _state.LcdEnabled;
}

bool GbPpu::IsCgbEnabled()
{
	return _state.CgbEnabled;
}

PpuMode GbPpu::GetMode()
{
	return _state.Mode;
}

void GbPpu::SendFrame()
{
	_emu->ProcessEvent(EventType::GbEndFrame);
	_state.FrameCount++;

	if(_gameboy->IsSgb()) {
		return;
	}

	UpdatePalette();

	_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);

	if(_isFirstFrame) {
		if(!_state.CgbEnabled) {
			//Send blank frame on the first frame after enabling LCD (DMG only)
			std::fill(_currentBuffer, _currentBuffer + GbConstants::PixelCount, 0x7FFF);
		} else {
			//CGB repeats the previous frame?
			uint16_t* src = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
			std::copy(src, src + GbConstants::PixelCount, _currentBuffer);
		}
	}
	_isFirstFrame = false;

	RenderedFrame frame(_currentBuffer, GbConstants::ScreenWidth, GbConstants::ScreenHeight, 1.0, _state.FrameCount, _gameboy->GetControlManager()->GetPortStates());
	bool rewinding = _emu->GetRewindManager()->IsRewinding();
	_emu->GetVideoDecoder()->UpdateFrame(frame, rewinding, rewinding);

	_emu->ProcessEndOfFrame();
	_gameboy->ProcessEndOfFrame();

	_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
}

void GbPpu::DebugSendFrame()
{
	if(_gameboy->IsSgb()) {
		return;
	}

	int lastPixel = std::max(0, _state.IrqMode == PpuMode::HBlank ? 160 : _drawnPixels);
	int offset = std::max(0, (int)(lastPixel + 1 + _state.Scanline * GbConstants::ScreenWidth));
	int pixelsToClear = GbConstants::PixelCount - offset;
	if(pixelsToClear > 0) {
		memset(_currentBuffer + offset, 0, pixelsToClear * sizeof(uint16_t));
	}

	RenderedFrame frame(_currentBuffer, GbConstants::ScreenWidth, GbConstants::ScreenHeight, 1.0, _state.FrameCount);
	
	_emu->GetVideoDecoder()->UpdateFrame(frame, false, false);
	//Send twice to prevent LCD blending behavior
	_emu->GetVideoDecoder()->UpdateFrame(frame, false, false);
}

void GbPpu::UpdatePalette()
{
	if(!_gameboy->IsCgb()) {
		GameboyConfig cfg = _emu->GetSettings()->GetGameboyConfig();
		for(int i = 0; i < 4; i++) {
			//Set palette based on settings (DMG)
			uint16_t bgColor = ((cfg.BgColors[i] & 0xF8) << 7) | ((cfg.BgColors[i] & 0xF800) >> 6) | ((cfg.BgColors[i] & 0xF80000) >> 19);
			_state.CgbBgPalettes[i] = bgColor;

			uint16_t obj0Color = ((cfg.Obj0Colors[i] & 0xF8) << 7) | ((cfg.Obj0Colors[i] & 0xF800) >> 6) | ((cfg.Obj0Colors[i] & 0xF80000) >> 19);
			_state.CgbObjPalettes[i] = obj0Color;
			
			uint16_t obj1Color = ((cfg.Obj1Colors[i] & 0xF8) << 7) | ((cfg.Obj1Colors[i] & 0xF800) >> 6) | ((cfg.Obj1Colors[i] & 0xF80000) >> 19);
			_state.CgbObjPalettes[i + 4] = obj1Color;
		}
	}
}

uint8_t GbPpu::Read(uint16_t addr)
{
	switch(addr) {
		case 0xFF40: return _state.Control;
		case 0xFF41:
			//FF41 - STAT - LCDC Status (R/W)
			return (
				0x80 | 
				(_state.Status & 0x78) |
				(_state.LyCoincidenceFlag ? 0x04 : 0x00) |
				(int)_state.Mode
			);

		case 0xFF42: return _state.ScrollY; //FF42 - SCY - Scroll Y (R/W)
		case 0xFF43: return _state.ScrollX; //FF43 - SCX - Scroll X (R/W)
		case 0xFF44: return _state.Ly; //FF44 - LY - LCDC Y-Coordinate (R)
		case 0xFF45: return _state.LyCompare; //FF45 - LYC - LY Compare (R/W)
		case 0xFF47: return _state.BgPalette; //FF47 - BGP - BG Palette Data (R/W) - Non CGB Mode Only
		case 0xFF48: return _state.ObjPalette0; //FF48 - OBP0 - Object Palette 0 Data (R/W) - Non CGB Mode Only
		case 0xFF49: return _state.ObjPalette1; //FF49 - OBP1 - Object Palette 1 Data (R/W) - Non CGB Mode Only
		case 0xFF4A: return _state.WindowY; //FF4A - WY - Window Y Position (R/W)
		case 0xFF4B: return _state.WindowX; //FF4B - WX - Window X Position minus 7 (R/W)
	}
	
	LogDebug("[Debug] GB - Missing read handler: $" + HexUtilities::ToHex(addr));
	return 0xFF;
}

void GbPpu::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0xFF40: 
			_state.Control = value; 
			if(_state.LcdEnabled != ((value & 0x80) != 0)) {
				_state.LcdEnabled = (value & 0x80) != 0;
				
				if(!_state.LcdEnabled) {
					//Reset LCD to top of screen when it gets turned off
					if(_state.Mode != PpuMode::VBlank) {
						_emu->BreakIfDebugging(CpuType::Gameboy, BreakSource::GbDisableLcdOutsideVblank);
						SendFrame();
					}

					_state.Cycle = 0;
					_state.Scanline = 0;
					_state.Ly = 0;
					_state.LyForCompare = 0;
					_state.Mode = PpuMode::HBlank;

					_lastFrameTime = _gameboy->GetApuCycleCount();
					
					//"If the HDMA started when the screen was on, when the screen is switched off it will copy one block after the switch."
					_dmaController->ProcessHdma();
				} else {
					_isFirstFrame = true;
					_state.Cycle = -1;
					_state.IdleCycles = 0;
					ResetRenderer();
					_state.LyCoincidenceFlag = _state.LyCompare == _state.LyForCompare;
					UpdateStatIrq();
					
					if(_emu->IsDebugging()) {
						_emu->ProcessEvent(EventType::GbStartFrame);

						_currentEventViewerBuffer = _currentEventViewerBuffer == _eventViewerBuffers[0] ? _eventViewerBuffers[1] : _eventViewerBuffers[0];
						for(int i = 0; i < 456 * 154; i++) {
							_currentEventViewerBuffer[i] = 0x18C6;
						}
					}
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

		case 0xFF41:
			if(!_gameboy->IsCgb()) {
				//STAT write bug (DMG ONLY)
				//Writing to STAT causes all IRQ types to be turned on for a single cycle
				_state.Status = 0xF8 | (_state.Status & 0x07);
				UpdateStatIrq();
			}

			_state.Status = value & 0xF8;
			UpdateStatIrq();
			break;

		case 0xFF42: _state.ScrollY = value; break;
		case 0xFF43: _state.ScrollX = value; break;
		case 0xFF45: 
			_state.LyCompare = value;
			if(_state.LcdEnabled) {
				_state.IdleCycles = 0;
				_state.LyCoincidenceFlag = (_state.LyCompare == _state.LyForCompare);
				UpdateStatIrq();
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

bool GbPpu::IsVramReadAllowed()
{
	return _state.Mode <= PpuMode::VBlank || (_state.Mode == PpuMode::OamEvaluation && _state.Cycle < 80);
}

bool GbPpu::IsVramWriteAllowed()
{
	return _state.Mode <= PpuMode::OamEvaluation;
}

uint8_t GbPpu::ReadVram(uint16_t addr)
{
	if(IsVramReadAllowed()) {
		uint16_t vramAddr = (_state.CgbVramBank << 13) | (addr & 0x1FFF);
		_emu->ProcessPpuRead<CpuType::Gameboy>(vramAddr, _vram[vramAddr], MemoryType::GbVideoRam);
		return _vram[vramAddr];
	} else {
		_emu->BreakIfDebugging(CpuType::Gameboy, BreakSource::GbInvalidVramAccess);
		return 0xFF;
	}
}

uint8_t GbPpu::PeekVram(uint16_t addr)
{
	return IsVramReadAllowed() ? _vram[(_state.CgbVramBank << 13) | (addr & 0x1FFF)] : 0xFF;
}

void GbPpu::WriteVram(uint16_t addr, uint8_t value)
{
	if(IsVramWriteAllowed()) {
		uint16_t vramAddr = (_state.CgbVramBank << 13) | (addr & 0x1FFF);
		_emu->ProcessPpuWrite<CpuType::Gameboy>(vramAddr, value, MemoryType::GbVideoRam);
		_vram[vramAddr] = value;
	} else {
		_emu->BreakIfDebugging(CpuType::Gameboy, BreakSource::GbInvalidVramAccess);
	}
}

bool GbPpu::IsOamWriteAllowed()
{
	if(_memoryManager->IsOamDmaRunning()) {
		return false;
	}

	if(_state.Scanline == 0 && _isFirstFrame) {
		return _state.Mode == PpuMode::HBlank && _state.Cycle != 77 && _state.Cycle != 78;
	} else {
		return _state.Mode <= PpuMode::VBlank || (_state.Cycle >= 80 && _state.Cycle < 84);
	}
}

bool GbPpu::IsOamReadAllowed()
{
	if(_memoryManager->IsOamDmaRunning()) {
		return false;
	}

	if(_state.Scanline == 0 && _isFirstFrame) {
		return _state.Mode == PpuMode::HBlank;
	} else {
		return _state.Mode == PpuMode::VBlank || (_state.Mode == PpuMode::HBlank && _state.Cycle != 3);
	}
}

uint8_t GbPpu::PeekOam(uint8_t addr)
{
	if(addr < 0xA0) {
		return IsOamReadAllowed() ? _oam[addr] : 0xFF;
	}
	return 0;
}

uint8_t GbPpu::ReadOam(uint8_t addr)
{
	if(addr < 0xA0) {
		if(IsOamReadAllowed()) {
			_emu->ProcessPpuRead<CpuType::Gameboy>(addr, _oam[addr], MemoryType::GbSpriteRam);
			return _oam[addr];
		} else {
			_emu->BreakIfDebugging(CpuType::Gameboy, BreakSource::GbInvalidOamAccess);
			return 0xFF;
		}
	}
	return 0;
}

void GbPpu::WriteOam(uint8_t addr, uint8_t value, bool forDma)
{
	//During DMA or rendering/oam evaluation, ignore writes to OAM
	//The DMA controller is always allowed to write to OAM (presumably the PPU can't read OAM during that time? TODO implement)
	//On the DMG, there is a 4 clock gap (80 to 83) between OAM evaluation & rendering where writing is allowed
	if(addr < 0xA0) {
		if(forDma) {
			_oam[addr] = value;
			_emu->ProcessPpuWrite<CpuType::Gameboy>(addr, value, MemoryType::GbSpriteRam);
		} else if(IsOamWriteAllowed()) {
			_oam[addr] = value;
			_emu->ProcessPpuWrite<CpuType::Gameboy>(addr, value, MemoryType::GbSpriteRam);
		} else {
			_emu->BreakIfDebugging(CpuType::Gameboy, BreakSource::GbInvalidOamAccess);
		}
	}
}

uint8_t GbPpu::ReadCgbRegister(uint16_t addr)
{
	if(!_state.CgbEnabled) {
		return 0xFF;
	}

	switch(addr) {
		case 0xFF4F: return _state.CgbVramBank | 0xFE;
		case 0xFF68: return _state.CgbBgPalPosition | (_state.CgbBgPalAutoInc ? 0x80 : 0) | 0x40;
		case 0xFF69: return (_state.CgbBgPalettes[_state.CgbBgPalPosition >> 1] >> ((_state.CgbBgPalPosition & 0x01) ? 8 : 0) & 0xFF);
		case 0xFF6A: return _state.CgbObjPalPosition | (_state.CgbObjPalAutoInc ? 0x80 : 0) | 0x40;
		case 0xFF6B: return (_state.CgbObjPalettes[_state.CgbObjPalPosition >> 1] >> ((_state.CgbObjPalPosition & 0x01) ? 8 : 0) & 0xFF);
	}
	LogDebug("[Debug] GBC - Missing read handler: $" + HexUtilities::ToHex(addr));
	return 0xFF;
}

void GbPpu::WriteCgbRegister(uint16_t addr, uint8_t value)
{
	if(!_state.CgbEnabled && _memoryManager->IsBootRomDisabled()) {
		return;
	}

	switch(addr) {
		case 0xFF4C: _state.CgbEnabled = (value & 0x0C) == 0; break;
		case 0xFF4F: _state.CgbVramBank = value & 0x01; break;
		
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
	if(_state.Mode <= PpuMode::OamEvaluation) {
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
		_state.Status, _state.FrameCount, _lastFrameTime, _state.LyCoincidenceFlag,
		_state.CgbBgPalAutoInc, _state.CgbBgPalPosition,
		_state.CgbObjPalAutoInc, _state.CgbObjPalPosition, _state.CgbVramBank, _state.CgbEnabled,
		_latchWindowX, _latchWindowY, _latchWindowEnabled, _windowCounter, _isFirstFrame, _rendererIdle,
		_state.IdleCycles, _state.Ly, _state.LyForCompare, _state.IrqMode
	);

	s.StreamArray(_state.CgbBgPalettes, 4 * 8);
	s.StreamArray(_state.CgbObjPalettes, 4 * 8);

	s.Stream(
		_bgFetcher.Attributes, _bgFetcher.Step, _bgFetcher.Addr, _bgFetcher.LowByte, _bgFetcher.HighByte,
		_oamFetcher.Attributes, _oamFetcher.Step, _oamFetcher.Addr, _oamFetcher.LowByte, _oamFetcher.HighByte,
		_drawnPixels, _fetchColumn, _fetchWindow, _fetchSprite, _spriteCount,
		_bgFifo.Position, _bgFifo.Size, _oamFifo.Position, _oamFifo.Size
	);

	for(int i = 0; i < 8; i++) {
		s.Stream(_bgFifo.Content[i].Color, _bgFifo.Content[i].Attributes);
		s.Stream(_oamFifo.Content[i].Color, _oamFifo.Content[i].Attributes);
	}

	s.StreamArray(_spriteX, 10);
	s.StreamArray(_spriteIndexes, 10);
}
