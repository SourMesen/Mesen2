#include "pch.h"
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
#include "Shared/EventType.h"

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
	_state.Mode = PpuMode::HBlank;
	_state.CgbEnabled = _gameboy->IsCgb();
	_lastFrameTime = 0;

	_gameboy->InitializeRam(_state.CgbBgPalettes, 4 * 8 * sizeof(uint16_t));
	_gameboy->InitializeRam(_state.CgbObjPalettes, 4 * 8 * sizeof(uint16_t));

	UpdatePalette();

	Write(0xFF48, 0xFF);
	Write(0xFF49, 0xFF);

	//Reset state to ensure powering off and then back on works properly for SGB
	ResetRenderer();
	_wyEnableFlag = false;
	_wxEnableFlag = false;	
	_lcdDisabled = true;
	_stopOamBlocked = false;
	_stopVramBlocked = false;
	_stopPaletteBlocked = false;
	_oamReadBlocked = false;
	_oamWriteBlocked = false;
	_vramReadBlocked = false;
	_vramWriteBlocked = false;
	_isFirstFrame = true;
	_forceBlankFrame = true;
	_rendererIdle = false;
}

GbPpu::~GbPpu()
{
	delete[] _outputBuffers[0];
	delete[] _outputBuffers[1];
	delete[] _eventViewerBuffers[0];
	delete[] _eventViewerBuffers[1];
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

void GbPpu::SetCpuStopState(bool stopped)
{
	if(!_gameboy->IsCgb()) {
		if(stopped) {
			_lcdDisabled = true;
		} else if(_state.LcdEnabled) {
			_lcdDisabled = !_state.LcdEnabled;
		}
	} else {
		if(stopped) {
			//When the CPU is stopped on the CGB, the LCD is still active, but its oam/vram/palette
			//access is frozen to what it was when the CPU stopped. So if stopped during mode 3, the
			//LCD essentially continues rendering the last image that was on screen.
			//If this is done outside of mode 3, oam/vram/pal access can be disabled, causing the
			//screen to turn black, etc.
			_stopOamBlocked = !_oamReadBlocked;
			_stopVramBlocked = !_vramReadBlocked;
			_stopPaletteBlocked = _state.Mode <= PpuMode::OamEvaluation;
		} else {
			_stopOamBlocked = false;
			_stopVramBlocked = false;
			_stopPaletteBlocked = false;
		}
	}
}

template<bool singleStep>
void GbPpu::Exec()
{
	if(_lcdDisabled) {
		//LCD is disabled, prevent IRQs, etc.
		//Not quite correct in terms of frame pacing
		if(_gameboy->GetApuCycleCount() - _lastFrameTime > 70224) {
			//More than a full frame's worth of time has passed since the last frame, send another blank frame
			_forceBlankFrame = true;
			SendFrame();
			_forceBlankFrame = true;
			_lastFrameTime = _gameboy->GetApuCycleCount();
		}
		return;
	}

	uint8_t cyclesToRun = (_memoryManager->IsHighSpeed() || singleStep) ? 1 : 2;
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
			//Mode turns to hblank on the same cycle as the last pixel is output
			_state.Mode = PpuMode::HBlank;
			_state.IrqMode = PpuMode::HBlank;
			_state.IdleCycles = 456 - _state.Cycle - 1;
			
			_oamReadBlocked = false;
			_oamWriteBlocked = false;
			_vramReadBlocked = false;
			_vramWriteBlocked = false;

			if(_state.Scanline < 143) {
				//"This mode will transfer one block (16 bytes) during each H-Blank. No data is transferred during VBlank (LY = 143 - 153)"
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
			_drawnPixels = 0;

			if(_state.Scanline == 154) {
				_state.Scanline = 0;
				_state.Ly = 0;
				_state.LyForCompare = 0;
				_wyEnableFlag = _state.Scanline == _state.WindowY && _state.WindowEnabled;

				if(!_gameboy->IsCgb()) {
					//On scanline 0, hblank gets set here (not on CGB)
					_state.Mode = PpuMode::HBlank;
				}

				if(_emu->IsDebugging()) {
					_emu->ProcessEvent(EventType::StartFrame, CpuType::Gameboy);
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
	switch(_state.Cycle) {
		case 87:
			_oamReadBlocked = true;
			_oamWriteBlocked = true;
			_vramReadBlocked = true;
			_vramWriteBlocked = true;

			_rendererIdle = true;
			_wxEnableFlag = false;
			_state.Mode = PpuMode::Drawing;
			_state.IrqMode = PpuMode::Drawing;
			ResetRenderer();
			_rendererIdle = true;
			break;
		
		case 92:
			_rendererIdle = false;
			break;

		case 456:
			_state.Cycle = 0;
			_state.Scanline++;
			_wyEnableFlag |= _state.Scanline == _state.WindowY && _state.WindowEnabled;
			_drawnPixels = 0;
			_state.Ly = _state.Scanline;
			_drawnPixels = 0;
			break;
	}
}

void GbPpu::ProcessVisibleScanline()
{
	switch(_state.Cycle) {
		case 2:
			if(_state.Scanline > 0) {
				//On scanlines 1-143, the OAM IRQ fires 1 cycle early
				_state.IrqMode = PpuMode::OamEvaluation;
				_state.LyForCompare = -1;
			}
			break;

		case 3:
			_oamReadBlocked = true;
			break;

		case 4:
			_spriteCount = 0;
			_state.LyForCompare = _state.Scanline;
			_oamWriteBlocked = true;
			_state.Mode = PpuMode::OamEvaluation;
			_state.IrqMode = PpuMode::OamEvaluation;
			break;

		case 5:
			//Turning on OAM IRQs in the middle of evaluation has no effect?
			//Or is this a patch to get the proper behavior for the STAT write bug?
			_state.IrqMode = PpuMode::NoIrq;
			break;

		case 80:
			_oamWriteBlocked = false;
			_oamReadBlocked = true;
			_vramReadBlocked = true;
			break;

		case 84:
			_wxEnableFlag = false;
			_state.Mode = PpuMode::Drawing;
			_state.IrqMode = PpuMode::Drawing;
			_oamWriteBlocked = true;
			_vramWriteBlocked = true;
			_rendererIdle = true;
			break;

		case 89:
			ResetRenderer();
			_rendererIdle = false;
			break;

		case 456:
			_state.Cycle = 0;
			_state.Scanline++;
			_state.Ly = _state.Scanline;
			_wyEnableFlag |= _state.Scanline == _state.WindowY && _state.WindowEnabled;
			_drawnPixels = 0;
			if(_state.Scanline == 144) {
				_state.LyForCompare = -1;
			}
			break;
	}
}

void GbPpu::ProcessPpuCycle()
{
	_gbcTileGlitch = false;

	if(_emu->IsDebugging()) {
		_emu->ProcessPpuCycle<CpuType::Gameboy>();
		if(_state.Mode != PpuMode::Drawing) {
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

	//TODO fix/check behavior for WX=0 and WX=166
	if(!_wxEnableFlag) {
		_wxEnableFlag |= _drawnPixels == _state.WindowX - 7;

		bool fetchWindow = (
			_state.WindowEnabled && //"Window enable bit in LCDC is set"
			_wxEnableFlag && //"the current X coordinate being rendered + 7 was equal to WX"
			_wyEnableFlag //"at some point in this frame the value of WY was equal to LY (checked at the start of Mode 2 only)"
		);
		
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

		if(!_gameboy->IsCgb() && !_state.WindowEnabled && _windowCounter > 0 && _wxEnableFlag && ((int)_state.WindowX & 0x07) == 7 - ((int)_state.ScrollX & 0x07)) {
			//Emulate a DMG-only glitch that occurs in the following conditions (from the Windesync-validate test rom readme):
			//After the window is active at some point in the frame, an extra pixel is added to the BG FIFO on all other lines when:
			//  -The window is disabled AND
			//  -A window X hit occurs AND
			//  -(WindowX & 7) == 7 - (ScrollX & 7)
			//This glitch also occurs in Star Trek 25th anniversary's intro - not emulating it causes the graphics to be offset by a pixel
			_insertGlitchBgPixel = true;
		}
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
			GameboyConfig& cfg = _emu->GetSettings()->GetGameboyConfig();

			GbFifoEntry entry = _insertGlitchBgPixel ? GbFifoEntry{} : _bgFifo.Content[_bgFifo.Position];
			GbFifoEntry sprite = _oamFifo.Content[_oamFifo.Position];
			if(!cfg.DisableSprites && sprite.Color != 0 && (entry.Color == 0 || (!(sprite.Attributes & 0x80) && !(entry.Attributes & 0x80)) || (_state.CgbEnabled && !_state.BgEnabled))) {
				//Use sprite pixel if:
				//  -BG color is 0, OR
				//  -Sprite is background priority AND BG does not have its priority bit set, OR
				//  -On CGB, the "bg enabled" flag is cleared, causing all sprites to appear above BG tiles
				if(_state.CgbEnabled) {
					WriteObjPixel(sprite.Color | ((sprite.Attributes & 0x07) << 2));
				} else {
					uint8_t colorIndex = (((sprite.Attributes & 0x10) ? _state.ObjPalette1 : _state.ObjPalette0) >> (sprite.Color * 2)) & 0x03;
					WriteObjPixel(((sprite.Attributes & 0x10) ? 4 : 0) | colorIndex);
				}
				_lastPixelType = GbPixelType::Object;
			} else {
				if(!cfg.DisableBackground) {
					if(_state.CgbEnabled) {
						WriteBgPixel(entry.Color | ((entry.Attributes & 0x07) << 2));
					} else {
						WriteBgPixel(_state.BgEnabled ? ((_state.BgPalette >> (entry.Color * 2)) & 0x03) : (_state.BgPalette & 0x03));
					}
				} else {
					WriteBgPixel(_state.BgPalette & 0x03);
				}
				_lastPixelType = GbPixelType::Background;
				_lastBgColor = entry.Color;
			}
		}

		if(!_insertGlitchBgPixel) {
			_bgFifo.Pop();
		}
		_insertGlitchBgPixel = false;
		_drawnPixels++;

		if(_oamFifo.Size > 0) {
			_oamFifo.Pop();
		}
	}

	ClockTileFetcher();
}

void GbPpu::WriteBgPixel(uint8_t colorIndex)
{
	uint16_t outOffset = _state.Scanline * GbConstants::ScreenWidth + _drawnPixels;
	_currentBuffer[outOffset] = LcdReadBgPalette(colorIndex) & 0x7FFF;
	if(_gameboy->IsSgb()) {
		_gameboy->GetSgb()->WriteLcdColor(_state.Scanline, (uint8_t)_drawnPixels, colorIndex & 0x03);
	}
}

void GbPpu::WriteObjPixel(uint8_t colorIndex)
{
	uint16_t outOffset = _state.Scanline * GbConstants::ScreenWidth + _drawnPixels;
	_currentBuffer[outOffset] = LcdReadObjPalette(colorIndex) & 0x7FFF;
	if(_gameboy->IsSgb()) {
		_gameboy->GetSgb()->WriteLcdColor(_state.Scanline, (uint8_t)_drawnPixels, colorIndex & 0x03);
	}
}

void GbPpu::RunSpriteEvaluation()
{
	if(_state.Cycle & 0x01) {
		if(_spriteCount < 10) {
			uint8_t spriteIndex = ((_state.Cycle - 4) >> 1) * 4;
			
			if(!_dmaController->IsOamDmaRunning()) {
				//When DMA is running, the LCD appears to re-use the last values read from OAM
				//These are shared with the sprite fetcher, so they can contain tile index/attributes
				//instead of Y+X coordinates, depending on when the OAM DMA was started.
				//This is needed to pass the "strikethrough" test rom
				_oamReadBuffer[0] = _oam[spriteIndex]; //Y
				_oamReadBuffer[1] = _oam[spriteIndex + 1]; //X
			}

			int16_t sprY = ((int16_t)_oamReadBuffer[0] - 16);
			if(_state.Scanline >= sprY && _state.Scanline < sprY + (_state.LargeSprites ? 16 : 8)) {
				_spriteY[_spriteCount] = _oamReadBuffer[0];
				_spriteX[_spriteCount] = _oamReadBuffer[1];
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
	_fetchColumn = 0;

	_insertGlitchBgPixel = false;
}

void GbPpu::ClockSpriteFetcher()
{
	switch(_oamFetcher.Step++) {
		case 1: {
			//Fetch tile index
			uint8_t sprAddr = _spriteIndexes[_fetchSprite];
			int16_t sprY = (int16_t)_spriteY[_fetchSprite] - 16;

			if(!_dmaController->IsOamDmaRunning()) {
				_oamReadBuffer[0] = LcdReadOam(sprAddr + 2); //Tile Index
				_oamReadBuffer[1] = LcdReadOam(sprAddr + 3); //Attributes
			} else {
				//When DMA is running, this data is replaced by the content
				//at the address that the DMA unit was reading/writing at the time.
				uint16_t addr = _dmaController->GetLastWriteAddress() & 0xFE;
				_oamReadBuffer[0] = _oam[addr];
				_oamReadBuffer[1] = _oam[addr+1];
			}

			uint8_t sprTile = _oamReadBuffer[0];
			uint8_t sprAttr = _oamReadBuffer[1];
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

		case 3: _oamFetcher.LowByte = LcdReadVram(_oamFetcher.Addr); break;

		case 5: {
			//Fetch sprite data (high byte)
			_oamFetcher.HighByte = LcdReadVram(_oamFetcher.Addr + 1);
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
				_fetchSprite = i;
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
			uint8_t scrollPos;
			if(_fetchWindow) {
				tilemapAddr = _state.WindowTilemapSelect ? 0x1C00 : 0x1800;
				yOffset = (uint8_t)_windowCounter;
				scrollPos = _fetchColumn;
			} else {
				tilemapAddr = _state.BgTilemapSelect ? 0x1C00 : 0x1800;
				yOffset = _state.ScrollY + _state.Scanline;
				scrollPos = (_fetchColumn + (_state.ScrollX / 8)) & 0x1F;
			}

			uint8_t row = yOffset >> 3;
			uint16_t tileAddr = tilemapAddr + scrollPos + row * 32;
			_tileIndex = LcdReadVram(tileAddr);

			uint8_t attributes = _state.CgbEnabled ? _vram[tileAddr | 0x2000] : 0;
			bool vMirror = (attributes & 0x40) != 0;
			uint16_t tileBank = (attributes & 0x08) ? 0x2000 : 0x0000;

			uint16_t baseTile = _state.BgTileSelect ? 0 : 0x1000;
			uint8_t tileY = vMirror ? (7 - (yOffset & 0x07)) : (yOffset & 0x07);
			uint16_t tileRowAddr = baseTile + (baseTile ? (int8_t)_tileIndex * 16 : _tileIndex * 16) + tileY * 2;
			tileRowAddr |= tileBank;
			_bgFetcher.Addr = tileRowAddr;
			_bgFetcher.Attributes = (attributes & 0xBF);
			break;
		}

		case 3: {
			//Fetch tile data (low byte)
			if(_gbcTileGlitch) {
				_bgFetcher.LowByte = _tileIndex;
			} else {
				_bgFetcher.LowByte = LcdReadVram(_bgFetcher.Addr);
			}
			break;
		}

		case 5: {
			//Fetch tile data (high byte)
			if(_gbcTileGlitch) {
				_bgFetcher.HighByte = _tileIndex;
			} else {
				_bgFetcher.HighByte = LcdReadVram(_bgFetcher.Addr + 1);
			}
			
			[[fallthrough]];
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
	uint8_t spriteIndex = _fetchSprite;

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

		if(bits > 0 && (_oamFifo.Content[pos].Color == 0 || (IsCgbEnabled() && spriteIndex <= _oamFifo.Content[pos].Index))) {
			_oamFifo.Content[pos].Color = bits;
			_oamFifo.Content[pos].Attributes = _oamFetcher.Attributes;
			_oamFifo.Content[pos].Index = spriteIndex;
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

		_bgFifo.Content[i].Color = bits;
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

uint8_t GbPpu::LcdReadOam(uint8_t addr)
{
	return _stopOamBlocked ? 0xFF : _oam[addr];
}

uint8_t GbPpu::LcdReadVram(uint16_t addr)
{
	return _stopVramBlocked ? 0xFF : _vram[addr];
}

uint16_t GbPpu::LcdReadBgPalette(uint8_t addr)
{
	return _stopPaletteBlocked ? 0 : _state.CgbBgPalettes[addr];
}

uint16_t GbPpu::LcdReadObjPalette(uint8_t addr)
{
	return _stopPaletteBlocked ? 0 : _state.CgbObjPalettes[addr];
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
	_emu->ProcessEvent(EventType::EndFrame, CpuType::Gameboy);
	_state.FrameCount++;

	if(_gameboy->IsSgb()) {
		_isFirstFrame = false;
		_forceBlankFrame = false;
		return;
	}

	UpdatePalette();

	_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);

	if(_forceBlankFrame) {
		//Send blank frame on the first frame after enabling LCD
		//On CGB some games flicker if this is done when the LCD is only off for a short time (e.g Men in Black - The Series)
		//So for CGB, the screen is only cleared if the LCD has been turned off for a while
		std::fill(_outputBuffers[0], _outputBuffers[0] + GbConstants::PixelCount, 0x7FFF);
		std::fill(_outputBuffers[1], _outputBuffers[1] + GbConstants::PixelCount, 0x7FFF);
	}
	_forceBlankFrame = false;
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
					
					_oamReadBlocked = false;
					_oamWriteBlocked = false;
					_vramReadBlocked = false;
					_vramWriteBlocked = false;

					_state.Mode = PpuMode::HBlank;
					_state.IrqMode = PpuMode::NoIrq;
					_wyEnableFlag = false;
					_lcdDisabled = true;

					_lastFrameTime = _gameboy->GetApuCycleCount();
					
					//"If the HDMA started when the screen was on, when the screen is switched off it will copy one block after the switch."
					_dmaController->ProcessHdma();
				} else {
					_lcdDisabled = false;
					_isFirstFrame = true;
					_forceBlankFrame = !_gameboy->IsCgb() || (_gameboy->GetApuCycleCount() - _lastFrameTime) > 5000;
					_state.Cycle = 7;
					_state.IdleCycles = 0;
					_state.IrqMode = PpuMode::NoIrq;
					ResetRenderer();
					_state.LyCoincidenceFlag = _state.LyCompare == _state.LyForCompare;
					UpdateStatIrq();
					
					if(_emu->IsDebugging()) {
						_emu->ProcessEvent(EventType::StartFrame, CpuType::Gameboy);

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

			_wyEnableFlag |= _state.Scanline == _state.WindowY && _state.WindowEnabled;
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

		case 0xFF47:
			if(!_state.CgbEnabled && _state.Mode == PpuMode::Drawing && _drawnPixels > 0 && _lastPixelType == GbPixelType::Background) {
				//When BGP is changed during rendering, the current pixel is affected.
				//Re-draw the last pixel with the correct color.
				
				//On CGB, the pixel uses the new BGP value only
				uint8_t bgpValue = value;
				if(!_gameboy->IsCgb()) {
					//On DMG, the pixel uses the new BGP value ORed with the old value
					bgpValue |= _state.BgPalette;
				}

				_drawnPixels--;
				WriteBgPixel((bgpValue >> (_lastBgColor * 2)) & 0x03);
				if(_emu->IsDebugging()) {
					//Update the event viewer data, if the debugger is running
					_currentEventViewerBuffer[456 * _state.Scanline + _state.Cycle] = _currentBuffer[_state.Scanline * GbConstants::ScreenWidth + _drawnPixels];
				}
				_drawnPixels++;
			}
			_state.BgPalette = value;
			break;

		case 0xFF48: _state.ObjPalette0 = value; break;
		case 0xFF49: _state.ObjPalette1 = value; break;
		case 0xFF4A: _state.WindowY = value; break;
		case 0xFF4B: _state.WindowX = value; break;

		default:
			LogDebug("[Debug] GB - Missing write handler: $" + HexUtilities::ToHex(addr));
			break;
	}
}

void GbPpu::SetTileFetchGlitchState()
{
	_gbcTileGlitch = true;
}

bool GbPpu::IsVramReadAllowed()
{
	return !_vramReadBlocked;
}

bool GbPpu::IsVramWriteAllowed()
{
	return !_vramWriteBlocked;
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
	return !_oamWriteBlocked && !_memoryManager->IsOamDmaRunning();
}

bool GbPpu::IsOamReadAllowed()
{
	return !_oamReadBlocked && !_memoryManager->IsOamDmaRunning();
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
	
	//"This area returns $FF when OAM is blocked, and otherwise the behavior depends on the hardware revision."
	//TODOGB CGB behavior
	return _memoryManager->IsOamDmaRunning() ? 0xFF : 0;
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

template<GbOamCorruptionType oamCorruptionType>
void GbPpu::ProcessOamCorruption(uint16_t addr)
{
	if(_gameboy->IsCgb() || _state.Cycle >= 83 || _state.Mode != PpuMode::OamEvaluation || (addr & 0xFF00) != 0xFE00) {
		return;
	}

	int row = std::min(19, (_state.Cycle - 2) >> 2);

	//Quotes from: https://gbdev.io/pandocs/OAM_Corruption_Bug.html
	//"Sprites 1 & 2 ($FE00 & $FE04) are not affected by this bug"
	if(row == 0) {
		return;
	}

	//"If a register is increased or decreased in the same M cycle of a write, this will effectively trigger both
	//a read and a write in a single M-cycle, resulting in a more complex corruption pattern:"
	if constexpr(oamCorruptionType == GbOamCorruptionType::ReadIncDec) {
		ProcessOamIncDecCorruption(row);
		//"Regardless of whether the previous corruption occurred or not, a normal read corruption is then applied."
	}

	int prevRow = row - 1;
	uint16_t a = _oam[row * 8] | (_oam[row * 8 + 1] << 8);
	uint16_t b = _oam[prevRow * 8] | (_oam[prevRow * 8 + 1] << 8);
	uint16_t c = _oam[prevRow * 8 + 4] | (_oam[prevRow * 8 + 5] << 8);

	uint16_t result;
	if constexpr(oamCorruptionType == GbOamCorruptionType::Write) {
		//"The first word in the row is replaced with this bitwise expression: ((a ^ c) & (b ^ c)) ^ c,
		//where a is the original value of that word, b is the first word in the preceding row, and c 
		//is the third word in the preceding row.
		result = ((a ^ c) & (b ^ c)) ^ c;
	} else {
		//"A "read corruption" works similarly to a write corruption, except the bitwise expression is b | (a & c)."
		result = b | (a & c);
	}

	_oam[row * 8] = (uint8_t)result;
	_oam[row * 8 + 1] = result >> 8;

	//"The last three words are copied from the last three words in the preceding row."
	memcpy(_oam + row * 8 + 2, _oam + prevRow * 8 + 2, 3 * sizeof(uint16_t));
}

void GbPpu::ProcessOamIncDecCorruption(int row)
{
	//"This corruption will not happen if the accessed row is one of the first four, as well as if it's the last row"
	if(row >= 4 && row < 19) {
		int prevRow = row - 1;
		//"The first word in the row preceding the currently accessed row is replaced with the following bitwise expression:
		//(b & (a | c | d)) | (a & c & d) where a is the first word two rows before the currently accessed row, b is the first
		//word in the preceding row (the word being corrupted), c is the first word in the currently accessed row, and d is 
		//the third word in the preceding row."
		uint16_t a = _oam[(row - 2) * 8] | (_oam[(row - 2) * 8 + 1] << 8);
		uint16_t b = _oam[prevRow * 8] | (_oam[prevRow * 8 + 1] << 8);
		uint16_t c = _oam[row * 8] | (_oam[row * 8 + 1] << 8);
		uint16_t d = _oam[prevRow * 8 + 4] | (_oam[prevRow * 8 + 5] << 8);

		uint16_t result = (b & (a | c | d)) | (a & c & d);
		_oam[prevRow * 8] = (uint8_t)result;
		_oam[prevRow * 8 + 1] = result >> 8;

		//"The contents of the preceding row is copied (after the corruption of the first word in it) both to the
		//currently accessed row and to two rows before the currently accessed row"
		memcpy(_oam + row * 8, _oam + prevRow * 8, 4 * sizeof(uint16_t));
		memcpy(_oam + (row - 2) * 8, _oam + prevRow * 8, 4 * sizeof(uint16_t));
	}
}

uint8_t GbPpu::ReadCgbRegister(uint16_t addr)
{
	//TODOGBC restrict read/write access to GBC palette during rendering
	if(!_state.CgbEnabled) {
		return 0xFF;
	}

	switch(addr) {
		case 0xFF4F: return _state.CgbVramBank | 0xFE;
		case 0xFF68: return _state.CgbBgPalPosition | (_state.CgbBgPalAutoInc ? 0x80 : 0) | 0x40;
		case 0xFF69: return ReadCgbPalette(_state.CgbBgPalPosition, _state.CgbBgPalettes);
		case 0xFF6A: return _state.CgbObjPalPosition | (_state.CgbObjPalAutoInc ? 0x80 : 0) | 0x40;
		case 0xFF6B: return ReadCgbPalette(_state.CgbObjPalPosition, _state.CgbObjPalettes);
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

uint8_t GbPpu::ReadCgbPalette(uint8_t& pos, uint16_t* pal)
{
	if(_state.Mode <= PpuMode::OamEvaluation) {
		return (pal[pos >> 1] >> ((pos & 0x01) ? 8 : 0)) & 0xFF;
	}
	return 0xFF;
}

void GbPpu::WriteCgbPalette(uint8_t& pos, uint16_t* pal, bool autoInc, uint8_t value)
{
	if(_state.Mode <= PpuMode::OamEvaluation) {
		if(pos & 0x01) {
			pal[pos >> 1] = (pal[pos >> 1] & 0xFF) | (value << 8);
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
	SV(_state.Scanline); SV(_state.Cycle); SV(_state.Mode); SV(_state.LyCompare); SV(_state.BgPalette); SV(_state.ObjPalette0); SV(_state.ObjPalette1);
	SV(_state.ScrollX); SV(_state.ScrollY); SV(_state.WindowX); SV(_state.WindowY); SV(_state.Control); SV(_state.LcdEnabled); SV(_state.WindowTilemapSelect);
	SV(_state.WindowEnabled); SV(_state.BgTileSelect); SV(_state.BgTilemapSelect); SV(_state.LargeSprites); SV(_state.SpritesEnabled); SV(_state.BgEnabled);
	SV(_state.Status); SV(_state.FrameCount); SV(_state.LyCoincidenceFlag);
	SV(_state.CgbBgPalAutoInc); SV(_state.CgbBgPalPosition);
	SV(_state.CgbObjPalAutoInc); SV(_state.CgbObjPalPosition); SV(_state.CgbVramBank); SV(_state.CgbEnabled);
	SV(_state.Ly);
	SV(_state.StatIrqFlag);

	if(_gameboy->IsCgb()) {
		//Only save the palettes for GBC states
		//This makes the process of loading a GB state onto a GBC 
		//instance better since the GBC colors will be preserved
		SVArray(_state.CgbBgPalettes, 4 * 8);
		SVArray(_state.CgbObjPalettes, 4 * 8);
	}

	if(s.GetFormat() != SerializeFormat::Map) {
		//Hide these entries from the Lua API
		SV(_windowCounter); SV(_isFirstFrame); SV(_rendererIdle); SV(_forceBlankFrame);
		SV(_wyEnableFlag); SV(_wxEnableFlag);
		SV(_state.IdleCycles);
		SV(_lastFrameTime);

		SV(_state.LyForCompare);
		SV(_state.IrqMode);

		SV(_oamReadBlocked);
		SV(_oamWriteBlocked);
		SV(_vramReadBlocked);
		SV(_vramWriteBlocked);

		SV(_lastPixelType);
		SV(_lastBgColor);
		SV(_insertGlitchBgPixel);

		SV(_gbcTileGlitch);
		SV(_tileIndex);

		SV(_stopOamBlocked);
		SV(_stopVramBlocked);
		SV(_stopPaletteBlocked);
		SV(_lcdDisabled);
		
		SV(_bgFetcher.Attributes); SV(_bgFetcher.Step); SV(_bgFetcher.Addr); SV(_bgFetcher.LowByte); SV(_bgFetcher.HighByte);
		SV(_oamFetcher.Attributes); SV(_oamFetcher.Step); SV(_oamFetcher.Addr); SV(_oamFetcher.LowByte); SV(_oamFetcher.HighByte);
		SV(_drawnPixels); SV(_fetchColumn); SV(_fetchWindow); SV(_fetchSprite); SV(_spriteCount);
		SV(_bgFifo.Position); SV(_bgFifo.Size); SV(_oamFifo.Position); SV(_oamFifo.Size);

		for(int i = 0; i < 8; i++) {
			SVI(_bgFifo.Content[i].Color); SVI(_bgFifo.Content[i].Attributes);
			SVI(_oamFifo.Content[i].Color); SVI(_oamFifo.Content[i].Attributes);
		}

		SVArray(_oamReadBuffer, 2);
		SVArray(_spriteX, 10);
		SVArray(_spriteY, 10);
		SVArray(_spriteIndexes, 10);
	}

	if(!s.IsSaving()) {
		//For save state compatibility
		if(!_state.LcdEnabled) {
			_lcdDisabled = true;
		}
	}
}

template void GbPpu::ProcessOamCorruption<GbOamCorruptionType::Read>(uint16_t addr);
template void GbPpu::ProcessOamCorruption<GbOamCorruptionType::ReadIncDec>(uint16_t addr);
template void GbPpu::ProcessOamCorruption<GbOamCorruptionType::Write>(uint16_t addr);

template void GbPpu::Exec<true>();
template void GbPpu::Exec<false>();
