#include "pch.h"
#include "SNES/SnesPpu.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesCpu.h"
#include "SNES/Spc.h"
#include "SNES/InternalRegisters.h"
#include "SNES/SnesControlManager.h"
#include "SNES/InternalRegisters.h"
#include "SNES/SnesDmaController.h"
#include "SNES/Debugger/SnesPpuTools.h"
#include "Debugger/Debugger.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/Video/VideoRenderer.h"
#include "Shared/NotificationManager.h"
#include "Shared/RenderedFrame.h"
#include "Shared/MessageManager.h"
#include "Shared/EventType.h"
#include "Shared/RewindManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"

SnesPpu::SnesPpu(Emulator* emu, SnesConsole* console)
{
	_emu = emu;
	_console = console;

	_vram = new uint16_t[SnesPpu::VideoRamSize >> 1];
	_emu->RegisterMemory(MemoryType::SnesVideoRam, _vram, SnesPpu::VideoRamSize);

	_emu->RegisterMemory(MemoryType::SnesSpriteRam, _oamRam, SnesPpu::SpriteRamSize);
	_emu->RegisterMemory(MemoryType::SnesCgRam, _cgram, SnesPpu::CgRamSize);

	_outputBuffers[0] = new uint16_t[512 * 478];
	_outputBuffers[1] = new uint16_t[512 * 478];
	memset(_outputBuffers[0], 0, 512 * 478 * sizeof(uint16_t));
	memset(_outputBuffers[1], 0, 512 * 478 * sizeof(uint16_t));
}

SnesPpu::~SnesPpu()
{
	delete[] _vram;
	delete[] _outputBuffers[0];
	delete[] _outputBuffers[1];
}

void SnesPpu::PowerOn()
{
	_skipRender = false;
	_regs = _console->GetInternalRegisters();
	_settings = _emu->GetSettings();
	_spc = _console->GetSpc();
	_memoryManager = _console->GetMemoryManager();

	_currentBuffer = _outputBuffers[0];
	
	_state = {};
	_state.ForcedBlank = true;
	_state.VramIncrementValue = 1;
	if(_settings->GetSnesConfig().EnableRandomPowerOnState) {
		RandomizeState();
	}

	_console->InitializeRam(_vram, SnesPpu::VideoRamSize);
	_console->InitializeRam(_cgram, SnesPpu::CgRamSize);
	for(int i = 0; i < SnesPpu::CgRamSize / 2; i++) {
		_cgram[i] &= 0x7FFF;
	}

	_console->InitializeRam(_oamRam, SnesPpu::SpriteRamSize);

	memset(_spriteIndexes, 0xFF, sizeof(_spriteIndexes));
	
	UpdateNmiScanline();
}

void SnesPpu::Reset()
{
	_scanline = 0;
	_state.ForcedBlank = true;
	_oddFrame = 0;
}

uint32_t SnesPpu::GetFrameCount()
{
	return _frameCount;
}

uint16_t SnesPpu::GetScanline()
{
	return _scanline;
}

uint16_t SnesPpu::GetCycle()
{
	//"normally dots 323 and 327 are 6 master cycles instead of 4."
	uint16_t hClock = _memoryManager->GetHClock();
	if(hClock <= 1292) {
		return hClock >> 2;
	} else if(hClock <= 1310) {
		return (hClock - 2) >> 2;
	} else {
		return (hClock - 4) >> 2;
	}
}

uint16_t SnesPpu::GetNmiScanline()
{
	return _nmiScanline;
}

uint16_t SnesPpu::GetVblankStart()
{
	return _vblankStartScanline;
}

SnesPpuState SnesPpu::GetState()
{
	SnesPpuState state;
	GetState(state, false);
	return state;
}

SnesPpuState& SnesPpu::GetStateRef()
{
	_state.Cycle = GetCycle();
	_state.Scanline = _scanline;
	_state.HClock = _memoryManager->GetHClock();
	_state.FrameCount = _frameCount;
	return _state;
}

void SnesPpu::GetState(SnesPpuState &state, bool returnPartialState)
{
	if(!returnPartialState) {
		state = _state;
	}
	state.Cycle = GetCycle();
	state.Scanline = _scanline;
	state.HClock = _memoryManager->GetHClock();
	state.FrameCount = _frameCount;
}

template<bool hiResMode>
void SnesPpu::GetTilemapData(uint8_t layerIndex, uint8_t columnIndex)
{
	/* The current layer's options */
	LayerConfig &config = _state.Layers[layerIndex];

	uint16_t vScroll = config.VScroll;
	uint16_t hScroll = hiResMode ? (config.HScroll << 1) : config.HScroll;	
	if(_hOffset || _vOffset) {
		uint16_t enableBit = layerIndex == 0 ? 0x2000 : 0x4000;
		if(_state.BgMode == 4) {
			if((_hOffset & 0x8000) == 0 && (_hOffset & enableBit)) {
				hScroll = (hScroll & 0x07) | (_hOffset & 0x3F8);
			}
			if((_hOffset & 0x8000) != 0 && (_hOffset & enableBit)) {
				vScroll = (_hOffset & 0x3FF);
			}
		} else {
			if(_hOffset & enableBit) {
				hScroll = (hScroll & 0x07) | (_hOffset & 0x3F8);
			}
			if(_vOffset & enableBit) {
				vScroll = (_vOffset & 0x3FF);
			}
		}
	}
	if(hiResMode) {
		hScroll >>= 1;
	}

	uint16_t realY = IsDoubleHeight() ? (_oddFrame ? ((_scanline << 1) + 1) : (_scanline << 1)) : _scanline;

	if(_state.MosaicEnabled && (_state.MosaicEnabled & (1 << layerIndex))) {
		//Keep the "scanline" to what it was at the start of this mosaic block
		realY -= _state.MosaicSize - _mosaicScanlineCounter;
		if(IsDoubleHeight()) {
			realY -= _state.MosaicSize - _mosaicScanlineCounter;
		}
	}

	/* The current row of tiles (e.g scanlines 16-23 is row 2) */
	uint16_t row = (realY + vScroll) >> (config.LargeTiles ? 4 : 3);

	/* Tilemap offset based on the current row & tilemap size options */
	uint16_t addrVerticalScrollingOffset = config.DoubleHeight ? ((row & 0x20) << (config.DoubleWidth ? 6 : 5)) : 0;

	/* The start address for tiles on this row */
	uint16_t baseOffset = config.TilemapAddress + addrVerticalScrollingOffset + ((row & 0x1F) << 5);

	/* The current column index (in terms of 8x8 or 16x16 tiles) */
	uint16_t column = columnIndex + (hScroll >> 3);
	if constexpr(!hiResMode) {
		if(config.LargeTiles) {
			//For 16x16 tiles, need to return the same tile for 2 columns 8 pixel columns in a row
			column >>= 1;
		}
	}

	/* The tilemap address to read the tile data from */
	uint16_t addr = baseOffset + (column & 0x1F) + (config.DoubleWidth ? (column & 0x20) << 5 : 0);
	_layerData[layerIndex].Tiles[columnIndex].TilemapData = _vram[addr & 0x7FFF];
	_layerData[layerIndex].Tiles[columnIndex].VScroll = vScroll;
}

template<bool hiResMode, uint8_t bpp, bool secondTile>
void SnesPpu::GetChrData(uint8_t layerIndex, uint8_t column, uint8_t plane)
{
	LayerConfig &config = _state.Layers[layerIndex];
	TileData &tileData = _layerData[layerIndex].Tiles[column];
	uint16_t tilemapData = tileData.TilemapData;

	bool largeTileWidth = hiResMode || config.LargeTiles;

	bool vMirror = (tilemapData & 0x8000) != 0;
	bool hMirror = (tilemapData & 0x4000) != 0;

	uint16_t realY = IsDoubleHeight() ? (_oddFrame ? ((_scanline << 1) + 1) : (_scanline << 1)) : _scanline;

	if(_state.MosaicEnabled && (_state.MosaicEnabled & (1 << layerIndex))) {
		//Keep the "scanline" to what it was at the start of this mosaic block
		realY -= _state.MosaicSize - _mosaicScanlineCounter;
		if(IsDoubleHeight()) {
			realY -= _state.MosaicSize - _mosaicScanlineCounter + (_oddFrame ? 1 : 0);
		}
	}

	bool useSecondTile = secondTile;
	if constexpr(!hiResMode) {
		if(config.LargeTiles) {
			//For 16x16 tiles, need to return the 2nd part of the tile every other column
			useSecondTile = (((column << 3) + config.HScroll) & 0x08) == 0x08;
		}
	}

	uint16_t tileIndex = tilemapData & 0x3FF;
	if(largeTileWidth) {
		tileIndex = (
			tileIndex +
			(config.LargeTiles ? (((realY + tileData.VScroll) & 0x08) ? (vMirror ? 0 : 16) : (vMirror ? 16 : 0)) : 0) +
			(largeTileWidth ? (useSecondTile ? (hMirror ? 0 : 1) : (hMirror ? 1 : 0)) : 0)
		) & 0x3FF;
	}

	uint16_t tileStart = config.ChrAddress + tileIndex * 4 * bpp;
	
	uint8_t baseYOffset = (realY + tileData.VScroll) & 0x07;

	uint8_t yOffset = vMirror ? (7 - baseYOffset) : baseYOffset;
	uint16_t pixelStart = tileStart + yOffset + (plane << 3);
	tileData.ChrData[plane + (secondTile ? bpp / 2 : 0)] = _vram[pixelStart & 0x7FFF];
}

void SnesPpu::GetHorizontalOffsetByte(uint8_t columnIndex)
{
	uint16_t columnOffset = (((columnIndex << 3) + (_state.Layers[2].HScroll & ~0x07)) >> 3) & (_state.Layers[2].DoubleWidth ? 0x3F : 0x1F);
	uint16_t rowOffset = (_state.Layers[2].VScroll >> 3) & (_state.Layers[2].DoubleHeight ? 0x3F : 0x1F);

	_hOffset = _vram[(_state.Layers[2].TilemapAddress + columnOffset + (rowOffset << 5)) & 0x7FFF];
}

void SnesPpu::GetVerticalOffsetByte(uint8_t columnIndex)
{
	uint16_t columnOffset = (((columnIndex << 3) + (_state.Layers[2].HScroll & ~0x07)) >> 3) & (_state.Layers[2].DoubleWidth ? 0x3F : 0x1F);
	uint16_t rowOffset = (_state.Layers[2].VScroll >> 3) & (_state.Layers[2].DoubleHeight ? 0x3F : 0x1F);

	uint16_t tileOffset = columnOffset + (rowOffset << 5);

	//The vertical offset is 0x40 bytes later - but wraps around within the tilemap based on the tilemap size (0x800 or 0x1000 bytes)
	uint16_t vOffsetAddr = _state.Layers[2].TilemapAddress + ((tileOffset + 0x20) & (_state.Layers[2].DoubleHeight ? 0x7FF : 0x3FF));

	_vOffset = _vram[vOffsetAddr & 0x7FFF];
}

void SnesPpu::FetchTileData()
{
	if(_state.ForcedBlank) {
		return;
	}

	if(_fetchBgStart == 0) {
		_hOffset = 0;
		_vOffset = 0;
	}

	if(_state.BgMode == 0) {
		for(int x = _fetchBgStart; x <= _fetchBgEnd; x++) {
			switch(x & 0x07) {
				case 0: GetTilemapData<false>(3, x >> 3); break;
				case 1: GetTilemapData<false>(2, x >> 3); break;
				case 2: GetTilemapData<false>(1, x >> 3); break;
				case 3: GetTilemapData<false>(0, x >> 3); break;

				case 4: GetChrData<false, 2>(3, x >> 3, 0); break;
				case 5: GetChrData<false, 2>(2, x >> 3, 0); break;
				case 6: GetChrData<false, 2>(1, x >> 3, 0); break;
				case 7: GetChrData<false, 2>(0, x >> 3, 0); break;
			}
		}
	} else if(_state.BgMode == 1) {
		for(int x = _fetchBgStart; x <= _fetchBgEnd; x++) {
			switch(x & 0x07) {
				case 0: GetTilemapData<false>(2, x >> 3); break;
				case 1: GetTilemapData<false>(1, x >> 3); break;
				case 2: GetTilemapData<false>(0, x >> 3); break;
				case 3: GetChrData<false, 2>(2, x >> 3, 0); break;
				case 4: GetChrData<false, 4>(1, x >> 3, 0); break;
				case 5: GetChrData<false, 4>(1, x >> 3, 1); break;
				case 6: GetChrData<false, 4>(0, x >> 3, 0); break;
				case 7: GetChrData<false, 4>(0, x >> 3, 1); break;
			}
		}
	} else if(_state.BgMode == 2) {
		for(int x = _fetchBgStart; x <= _fetchBgEnd; x++) {
			switch(x & 0x07) {
				case 0: GetTilemapData<false>(1, x >> 3); break;
				case 1: GetTilemapData<false>(0, x >> 3); break;

				case 2: GetHorizontalOffsetByte(x >> 3); break;
				case 3: GetVerticalOffsetByte(x >> 3); break;

				case 4: GetChrData<false, 4>(1, x >> 3, 0); break;
				case 5: GetChrData<false, 4>(1, x >> 3, 1); break;
				case 6: GetChrData<false, 4>(0, x >> 3, 0); break;
				case 7: GetChrData<false, 4>(0, x >> 3, 1); break;
			}
		}
	} else if(_state.BgMode == 3) {
		for(int x = _fetchBgStart; x <= _fetchBgEnd; x++) {
			switch(x & 0x07) {
				case 0: GetTilemapData<false>(1, x >> 3); break;
				case 1: GetTilemapData<false>(0, x >> 3); break;

				case 2: GetChrData<false, 4>(1, x >> 3, 0); break;
				case 3: GetChrData<false, 4>(1, x >> 3, 1); break;

				case 4: GetChrData<false, 8>(0, x >> 3, 0); break;
				case 5: GetChrData<false, 8>(0, x >> 3, 1); break;
				case 6: GetChrData<false, 8>(0, x >> 3, 2); break;
				case 7: GetChrData<false, 8>(0, x >> 3, 3); break;
			}
		}
	} else if(_state.BgMode == 4) {
		for(int x = _fetchBgStart; x <= _fetchBgEnd; x++) {
			switch(x & 0x07) {
				case 0: GetTilemapData<false>(1, x >> 3); break;
				case 1: GetTilemapData<false>(0, x >> 3); break;

				case 2: GetHorizontalOffsetByte(x >> 3); break;

				case 3: GetChrData<false, 2>(1, x >> 3, 0); break;

				case 4: GetChrData<false, 8>(0, x >> 3, 0); break;
				case 5: GetChrData<false, 8>(0, x >> 3, 1); break;
				case 6: GetChrData<false, 8>(0, x >> 3, 2); break;
				case 7: GetChrData<false, 8>(0, x >> 3, 3); break;
			}
		}
	} else if(_state.BgMode == 5) {
		for(int x = _fetchBgStart; x <= _fetchBgEnd; x++) {
			switch(x & 0x07) {
				case 0: GetTilemapData<true>(1, x >> 3); break;
				case 1: GetTilemapData<true>(0, x >> 3); break;

				case 2: GetChrData<true, 2>(1, x >> 3, 0); break;
				case 3: GetChrData<true, 2, true>(1, x >> 3, 0); break;

				case 4: GetChrData<true, 4>(0, x >> 3, 0); break;
				case 5: GetChrData<true, 4>(0, x >> 3, 1); break;
				case 6: GetChrData<true, 4, true>(0, x >> 3, 0); break;
				case 7: GetChrData<true, 4, true>(0, x >> 3, 1); break;
			}
		}
	} else if(_state.BgMode == 6) {
		for(int x = _fetchBgStart; x <= _fetchBgEnd; x++) {
			switch(x & 0x07) {
				case 0: GetTilemapData<true>(1, x >> 3); break;
				case 1: GetTilemapData<true>(0, x >> 3); break;

				case 2: GetHorizontalOffsetByte(x >> 3); break;
				case 3: GetVerticalOffsetByte(x >> 3); break;
					
				case 4: GetChrData<true, 4>(0, x >> 3, 0); break;
				case 5: GetChrData<true, 4>(0, x >> 3, 1); break;
				case 6: GetChrData<true, 4, true>(0, x >> 3, 0); break;
				case 7: GetChrData<true, 4, true>(0, x >> 3, 1); break;
			}
		}
	}
}

bool SnesPpu::ProcessEndOfScanline(uint16_t& hClock)
{
	if(hClock >= 1364 || (hClock == 1360 && _scanline == 240 && _oddFrame && !_state.ScreenInterlace)) {
		//"In non-interlace mode scanline 240 of every other frame (those with $213f.7=1) is only 1360 cycles."
		if(_scanline < _vblankStartScanline) {
			RenderScanline();

			if(_scanline == 0) {
				_overscanFrame = _state.OverscanMode;
				_mosaicScanlineCounter = _state.MosaicEnabled ? _state.MosaicSize + 1 : 0;

				//Update overclock timings once per frame
				UpdateNmiScanline();

				if(!_skipRender) {
					_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
					if(_interlacedFrame) {
						memcpy(_currentBuffer, GetPreviousScreenBuffer(), 512 * 478 * sizeof(uint16_t));
					}
					
					//If we're not skipping this frame, reset the high resolution/interlace flags
					_useHighResOutput = IsDoubleWidth() || _state.ScreenInterlace;
					_interlacedFrame = _state.ScreenInterlace;
				}
			}
			
			if(_mosaicScanlineCounter) {
				_mosaicScanlineCounter--;
				if(_state.MosaicEnabled && !_mosaicScanlineCounter) {
					_mosaicScanlineCounter = _state.MosaicSize;
				}
			}

			_drawStartX = 0;
			_drawEndX = 0;
			_fetchBgStart = 0;
			_fetchBgEnd = 0;
			_fetchSpriteStart = 0;
			_fetchSpriteEnd = 0;
			_spriteEvalStart = 0;
			_spriteEvalEnd = 0;
			_spriteFetchingDone = false;

			memset(_hasSpritePriority, 0, sizeof(_hasSpritePriority));
			memcpy(_spritePriority, _spritePriorityCopy, sizeof(_spritePriority));
			for(int i = 0; i < 255; i++) {
				if(_spritePriority[i] < 4) {
					_hasSpritePriority[_spritePriority[i]] = true;
				}
			}

			memcpy(_spritePalette, _spritePaletteCopy, sizeof(_spritePalette));
			memcpy(_spriteColors, _spriteColorsCopy, sizeof(_spriteColors));

			memset(_spriteIndexes, 0xFF, sizeof(_spriteIndexes));

			memset(_mainScreenFlags, 0, sizeof(_mainScreenFlags));
			memset(_subScreenPriority, 0, sizeof(_subScreenPriority));

			if(!_skipRender && _emu->IsDebugging()) {
				DebugProcessMode7Overlay();
			}
		}

		_scanline++;
		hClock = 0;

		_console->GetInternalRegisters()->ProcessAutoJoypad();

		if(_scanline == _nmiScanline) {
			ProcessLocationLatchRequest();
			_latchRequest = false;

			//Reset OAM address at the start of vblank?
			if(!_state.ForcedBlank) {
				//TODO, the timing of this may be slightly off? should happen at H=10 based on anomie's docs
				_state.InternalOamAddress = (_state.OamRamAddress << 1);
			}

			SnesConfig& cfg = _settings->GetSnesConfig();
			_configVisibleLayers = (cfg.HideBgLayer1 ? 0 : 1) | (cfg.HideBgLayer2 ? 0 : 2) | (cfg.HideBgLayer3 ? 0 : 4) | (cfg.HideBgLayer4 ? 0 : 8) | (cfg.HideSprites ? 0 : 16);

			_emu->ProcessEvent(EventType::EndFrame);

			_frameCount++;
			SendFrame();

			_console->ProcessEndOfFrame();
		} else if(_scanline >= _vblankEndScanline + 1) {
			//"Frames are 262 scanlines in non-interlace mode, while in interlace mode frames with $213f.7=0 are 263 scanlines"
			_oddFrame ^= 1;
			_scanline = 0;
			_rangeOver = false;
			_timeOver = false;
			_emu->ProcessEvent(EventType::StartFrame);

			_skipRender = (
				!_settings->GetSnesConfig().DisableFrameSkipping &&
				(!_interlacedFrame || (_frameCount & 0x02)) &&
				!_emu->GetRewindManager()->IsRewinding() &&
				!_emu->GetVideoRenderer()->IsRecording() &&
				(_settings->GetEmulationSpeed() == 0 || _settings->GetEmulationSpeed() > 150) &&
				_frameSkipTimer.GetElapsedMS() < 10
			);
			
			if(_emu->IsRunAheadFrame()) {
				_skipRender = true;
			}

			//Ensure the SPC is re-enabled for the next frame
			_spc->SetSpcState(true);
		}

		UpdateSpcState();
		return true;
	}
	return false;
}

bool SnesPpu::IsInOverclockedScanline()
{
	return _inOverclockedScanline;
}

void SnesPpu::UpdateSpcState()
{
	//When using overclocking, turn off the SPC during the extra scanlines
	if(_overclockEnabled && _scanline > _vblankStartScanline) {
		if(_scanline > _adjustedVblankEndScanline) {
			//Disable APU for extra lines after NMI
			_spc->SetSpcState(false);
			_inOverclockedScanline = true;
		} else if(_scanline >= _vblankStartScanline && _scanline < _nmiScanline) {
			//Disable APU for extra lines before NMI
			_spc->SetSpcState(false);
			_inOverclockedScanline = true;
		} else {
			_spc->SetSpcState(true);
		}
	}
	_inOverclockedScanline = false;
}

void SnesPpu::UpdateNmiScanline()
{
	if(_console->GetRegion() == ConsoleRegion::Ntsc) {
		if(!_state.ScreenInterlace || _oddFrame) {
			_baseVblankEndScanline = 261;
		} else {
			_baseVblankEndScanline = 262;
		}
	} else {
		if(!_state.ScreenInterlace || _oddFrame) {
			_baseVblankEndScanline = 311;
		} else {
			_baseVblankEndScanline = 312;
		}
	}

	SnesConfig snesCfg = _settings->GetSnesConfig();
	_overclockEnabled = snesCfg.PpuExtraScanlinesBeforeNmi > 0 || snesCfg.PpuExtraScanlinesAfterNmi > 0;

	_adjustedVblankEndScanline = _baseVblankEndScanline + snesCfg.PpuExtraScanlinesBeforeNmi;
	_vblankEndScanline = _baseVblankEndScanline + snesCfg.PpuExtraScanlinesAfterNmi + snesCfg.PpuExtraScanlinesBeforeNmi;
	_vblankStartScanline = _state.OverscanMode ? 240 : 225;
	_nmiScanline = _vblankStartScanline + snesCfg.PpuExtraScanlinesBeforeNmi;
}

uint16_t SnesPpu::GetRealScanline()
{
	if(!_overclockEnabled) {
		return _scanline;
	}

	if(_scanline > _vblankStartScanline && _scanline <= _nmiScanline) {
		//Pretend to be just before vblank until extra scanlines are over
		return _vblankStartScanline - 1;
	} else if(_scanline > _nmiScanline) {
		if(_scanline > _adjustedVblankEndScanline) {
			//Pretend to be at the end of vblank until extra scanlines are over
			return _baseVblankEndScanline;
		} else {
			//Number the regular scanlines as they would normally be
			return _scanline - _nmiScanline + _vblankStartScanline;
		}
	}

	return _scanline;
}

uint16_t SnesPpu::GetVblankEndScanline()
{
	return _vblankEndScanline;
}

uint16_t SnesPpu::GetLastScanline()
{
	return _baseVblankEndScanline;
}

void SnesPpu::EvaluateNextLineSprites()
{
	if(_spriteEvalStart == 0) {
		_spriteCount = 0;
		_oamEvaluationIndex = _state.EnableOamPriority ? ((_state.InternalOamAddress & 0x1FC) >> 2) : 0;
	}

	if(_state.ForcedBlank) {
		return;
	}

	for(int i = _spriteEvalStart; i <= _spriteEvalEnd; i++) {
		if(i & 0x01) {
			//Second cycle: Check if sprite is in range, if so, keep its index
			if(_currentSprite.IsVisible(_scanline, _state.ObjInterlace)) {
				if(_spriteCount < 32) {
					_spriteIndexes[_spriteCount] = _oamEvaluationIndex;
					_spriteCount++;
				} else {
					_rangeOver = true;
				}
			}
			_oamEvaluationIndex = (_oamEvaluationIndex + 1) & 0x7F;
		} else {
			//First cycle, read X & Y and high oam byte
			FetchSpritePosition(_oamEvaluationIndex);
		}
	}
}

void SnesPpu::FetchSpriteData()
{
	//From H=272 to 339, fetch a single word of CHR data on every cycle (for up to 34 sprites)
	if(_fetchSpriteStart == 0) {
		memset(_spritePriorityCopy, 0xFF, sizeof(_spritePriorityCopy));

		_spriteTileCount = 0;
		_currentSprite.Index = 0xFF;

		if(_spriteCount == 0) {
			_spriteFetchingDone = true;
			return;
		}

		_oamTimeIndex = _spriteIndexes[_spriteCount - 1];
	}

	for(int x = _fetchSpriteStart; x <= _fetchSpriteEnd; x++) {
		if(x >= 2) {
			//Fetch the tile using the OAM data loaded on the past 2 cycles, before overwriting it in FetchSpriteAttributes below
			if(!_state.ForcedBlank) {
				FetchSpriteTile(x & 0x01);
			}

			if((x & 1) && _spriteCount == 0 && _currentSprite.ColumnOffset == 0) {
				//End this step
				_spriteFetchingDone = true;
				break;
			}
		}

		if(_spriteCount > 0) {
			if(x & 1) {
				FetchSpriteAttributes((_oamTimeIndex << 2) | 0x02);
				if(_spriteCount > 0) {
					_oamTimeIndex = _spriteIndexes[_spriteCount - 1];
				}
			} else {
				FetchSpritePosition(_oamTimeIndex);
			}
		}
	}
}

void SnesPpu::FetchSpritePosition(uint8_t spriteIndex)
{
	static constexpr uint8_t oamWidth[16] = { 8,8,8,16,16,32,16,16, 16,32,64,32,64,64,32,32 };
	static constexpr uint8_t oamHeight[16] = { 8,8,8,16,16,32,32,32, 16,32,64,32,64,64,64,32 };
	static constexpr uint16_t sign[2] = { 0x0000, 0xFF00 };

	uint8_t highTableValue = _oamRam[0x200 | (spriteIndex >> 2)] >> ((spriteIndex << 1) & 0x06);
	_currentSprite.X = (int16_t)(sign[highTableValue & 0x01] | _oamRam[(spriteIndex << 2)]);
	_currentSprite.Y = _oamRam[(spriteIndex << 2) + 1];

	uint8_t mode = _state.OamMode | ((highTableValue & 0x02) << 2);
	_currentSprite.Width = oamWidth[mode];
	_currentSprite.Height = oamHeight[mode];

	if(spriteIndex != _currentSprite.Index) {
		_currentSprite.Index = spriteIndex;
		_currentSprite.ColumnOffset = (_currentSprite.Width / 8);
		if(_currentSprite.X <= -8 && _currentSprite.X != -256) {
			//Skip the first tiles of the sprite (because the tiles are hidden to the left of the screen)
			_currentSprite.ColumnOffset += _currentSprite.X / 8;
		}
	}
}

void SnesPpu::FetchSpriteAttributes(uint16_t oamAddress)
{
	_spriteTileCount++;
	if(_spriteTileCount > 34) {
		_timeOver = true;
	}

	uint8_t flags = _oamRam[oamAddress + 1];
	bool useSecondTable = (flags & 0x01) != 0;
	_currentSprite.Palette = (flags >> 1) & 0x07;
	_currentSprite.Priority = (flags >> 4) & 0x03;
	_currentSprite.HorizontalMirror = (flags & 0x40) != 0;

	_currentSprite.ColumnOffset--;
	
	uint8_t yOffset;
	int rowOffset;
	int yGap = (_scanline - _currentSprite.Y);
	if(_state.ObjInterlace) {
		yGap <<= 1;
		yGap |= _oddFrame;
	}

	bool verticalMirror = (flags & 0x80) != 0;
	if(verticalMirror) {
		int pos;
		if(yGap < _currentSprite.Width) {
			//Square sprites
			pos = _currentSprite.Width - 1 - yGap;
		} else {
			//When using rectangular sprites (undocumented), vertical mirroring doesn't work properly
			//The top and bottom halves are mirrored separately and don't swap positions
			pos = _currentSprite.Width * 3 - 1 - yGap;
		}
		yOffset = pos & 0x07;
		rowOffset = pos >> 3;
	} else {
		yOffset = yGap & 0x07;
		rowOffset = yGap >> 3;
	}

	uint8_t columnCount = (_currentSprite.Width / 8);
	uint8_t tileRow = (_oamRam[oamAddress] & 0xF0) >> 4;
	uint8_t tileColumn = _oamRam[oamAddress] & 0x0F;
	uint8_t row = (tileRow + rowOffset) & 0x0F;
	uint8_t columnOffset = _currentSprite.HorizontalMirror ? _currentSprite.ColumnOffset : (columnCount - _currentSprite.ColumnOffset - 1);
	uint8_t tileIndex = (row << 4) | ((tileColumn + columnOffset) & 0x0F);
	uint16_t tileStart = (_state.OamBaseAddress + (tileIndex << 4) + (useSecondTable ? _state.OamAddressOffset : 0));
	_currentSprite.FetchAddress = (tileStart + yOffset) & 0x7FFF;

	int16_t x = _currentSprite.X == -256 ? 0 : _currentSprite.X;
	int16_t endTileX = x + ((columnCount - _currentSprite.ColumnOffset - 1) << 3) + 8;
	_currentSprite.DrawX = _currentSprite.X + ((columnCount - _currentSprite.ColumnOffset - 1) << 3);

	if(_currentSprite.ColumnOffset == 0 || endTileX >= 256) {
		//Last tile of the sprite, or skip the remaining tiles (because the tiles are hidden to the right of the screen)
		_spriteCount--;
		_currentSprite.ColumnOffset = 0;
	}
}

void SnesPpu::FetchSpriteTile(bool secondCycle)
{
	//The timing for the fetches should be (mostly) accurate (H=272 to 339)
	uint16_t chrData = _vram[_currentSprite.FetchAddress];
	_currentSprite.ChrData[secondCycle] = chrData;

	if(!secondCycle) {
		_currentSprite.FetchAddress = (_currentSprite.FetchAddress + 8) & 0x7FFF;
	} else {
		int16_t xPos = _currentSprite.DrawX;
		for(int x = 0; x < 8; x++) {
			if(xPos + x < 0 || xPos + x > 255) {
				continue;
			}

			uint8_t xOffset = _currentSprite.HorizontalMirror ? ((7 - x) & 0x07) : x;
			uint8_t color = GetTilePixelColor<4>(_currentSprite.ChrData, 7 - xOffset);

			if(color != 0) {
				_spriteColorsCopy[xPos + x] = color;
				_spritePriorityCopy[xPos + x] = _currentSprite.Priority;
				_spritePaletteCopy[xPos + x] = _currentSprite.Palette;
			}
		}
	}
}

void SnesPpu::RenderMode0()
{
	constexpr uint8_t spritePriorities[4] = { 3, 6, 9, 12 };
	RenderSprites(spritePriorities);

	RenderTilemap<0, 2, 8, 11, 0>();
	RenderTilemap<1, 2, 7, 10, 32>();
	RenderTilemap<2, 2, 2, 5, 64>();
	RenderTilemap<3, 2, 1, 4, 96>();
}

void SnesPpu::RenderMode1()
{
	constexpr uint8_t spritePriorities[4] = { 2, 4, 7, 10 };
	RenderSprites(spritePriorities);

	RenderTilemap<0, 4, 6, 9>();
	RenderTilemap<1, 4, 5, 8>();
	if(!_state.Mode1Bg3Priority) {
		RenderTilemap<2, 2, 1, 3>();
	} else {
		RenderTilemap<2, 2, 1, 11>();
	}
}

void SnesPpu::RenderMode2()
{
	constexpr uint8_t spritePriorities[4] = { 2, 4, 6, 8 };
	RenderSprites(spritePriorities);

	RenderTilemap<0, 4, 3, 7>();
	RenderTilemap<1, 4, 1, 5>();
}

void SnesPpu::RenderMode3()
{
	constexpr uint8_t spritePriorities[4] = { 2, 4, 6, 8 };
	RenderSprites(spritePriorities);

	RenderTilemap<0, 8, 3, 7>();
	RenderTilemap<1, 4, 1, 5>();
}

void SnesPpu::RenderMode4()
{
	constexpr uint8_t spritePriorities[4] = { 2, 4, 6, 8 };
	RenderSprites(spritePriorities);

	RenderTilemap<0, 8, 3, 7>();
	RenderTilemap<1, 2, 1, 5>();
}

void SnesPpu::RenderMode5()
{
	constexpr uint8_t spritePriorities[4] = { 2, 4, 6, 8 };
	RenderSprites(spritePriorities);

	RenderTilemap<0, 4, 3, 7>();
	RenderTilemap<1, 2, 1, 5>();
}

void SnesPpu::RenderMode6()
{
	constexpr uint8_t spritePriorities[4] = { 2, 3, 4, 6 };
	RenderSprites(spritePriorities);

	RenderTilemap<0, 4, 1, 5>();
}

void SnesPpu::RenderMode7()
{
	constexpr uint8_t spritePriorities[4] = { 2, 4, 6, 7 };
	RenderSprites(spritePriorities);

	RenderTilemapMode7<0, 3, 3>();
	if(_state.ExtBgEnabled) {
		RenderTilemapMode7<1, 1, 5>();
	}
}

void SnesPpu::RenderScanline()
{
	int32_t hPos = GetCycle();

	if(hPos <= 255 || _spriteEvalEnd < 255) {
		_spriteEvalEnd = std::min(hPos, 255);
		if(_spriteEvalStart <= _spriteEvalEnd) {
			EvaluateNextLineSprites();
		}
		_spriteEvalStart = _spriteEvalEnd + 1;
	}

	if(!_skipRender && (hPos <= 263 || _fetchBgEnd < 263)) {
		//Fetch tilemap and tile CHR data, as needed, between H=0 and H=263
		_fetchBgEnd = std::min(hPos, 263);
		if(_fetchBgStart <= _fetchBgEnd) {
			FetchTileData();
		}
		_fetchBgStart = _fetchBgEnd + 1;
	} 

	//Render the scanline
	if(!_skipRender && _drawStartX <= 255 && hPos > 22 && _scanline > 0) {
		_drawEndX = std::min(hPos - 22, 255);

		if(_state.ForcedBlank) {
			//Forced blank, output black
			memset(_mainScreenBuffer + _drawStartX, 0, (_drawEndX - _drawStartX + 1) * 2);
			memset(_subScreenBuffer + _drawStartX, 0, (_drawEndX - _drawStartX + 1) * 2);
		} else {
			switch(_state.BgMode) {
				case 0: RenderMode0(); break;
				case 1: RenderMode1(); break;
				case 2: RenderMode2(); break;
				case 3: RenderMode3(); break;
				case 4: RenderMode4(); break;
				case 5: RenderMode5(); break;
				case 6: RenderMode6(); break;
				case 7: RenderMode7(); break;
			}
			RenderBgColor();
		}

		ApplyColorMath();
		ApplyBrightness<true>();
		ApplyHiResMode();

		_drawStartX = _drawEndX + 1;
	}
	
	if(hPos >= 270 && !_spriteFetchingDone) {
		//Fetch sprite data from OAM and calculated which CHR data needs to be loaded (between H=270 and H=337)
		//Fetch sprite CHR data, as needed, between H=272 and H=339
		_fetchSpriteEnd = std::min(hPos - 270, 69);
		if(_fetchSpriteStart <= _fetchSpriteEnd) {
			FetchSpriteData();
		}
		_fetchSpriteStart = _fetchSpriteEnd + 1;
	}
}

void SnesPpu::RenderBgColor()
{
	uint8_t pixelFlags = (_state.ColorMathEnabled & 0x20) ? PixelFlags::AllowColorMath : 0;
	for(int x = _drawStartX; x <= _drawEndX; x++) {
		if((_mainScreenFlags[x] & 0x0F) == 0) {
			_state.InternalCgramAddress = 0;
			_mainScreenBuffer[x] = _cgram[0];
			_mainScreenFlags[x] = pixelFlags;
		}
		if(_subScreenPriority[x] == 0) {
			_state.InternalCgramAddress = 0;
			_subScreenBuffer[x] = _cgram[0];
		}
	}
}

void SnesPpu::RenderSprites(const uint8_t priority[4])
{
	if(!IsRenderRequired(SnesPpu::SpriteLayerIndex)) {
		return;
	}

	bool drawMain = (bool)(((_state.MainScreenLayers & _configVisibleLayers) >> SnesPpu::SpriteLayerIndex) & 0x01);
	bool drawSub = (bool)(((_state.SubScreenLayers & _configVisibleLayers) >> SnesPpu::SpriteLayerIndex) & 0x01);

	uint8_t mainWindowCount = 0;
	uint8_t subWindowCount = 0;
	if(_state.WindowMaskMain[SnesPpu::SpriteLayerIndex]) {
		mainWindowCount = (uint8_t)_state.Window[0].ActiveLayers[SnesPpu::SpriteLayerIndex] + (uint8_t)_state.Window[1].ActiveLayers[SnesPpu::SpriteLayerIndex];
	}
	if(_state.WindowMaskSub[SnesPpu::SpriteLayerIndex]) {
		subWindowCount = (uint8_t)_state.Window[0].ActiveLayers[SnesPpu::SpriteLayerIndex] + (uint8_t)_state.Window[1].ActiveLayers[SnesPpu::SpriteLayerIndex];
	}

	for(int x = _drawStartX; x <= _drawEndX; x++) {
		if(_spritePriority[x] <= 3) {
			uint8_t spritePrio = priority[_spritePriority[x]];
			if(drawMain && ((_mainScreenFlags[x] & 0x0F) < spritePrio) && !ProcessMaskWindow<SnesPpu::SpriteLayerIndex>(mainWindowCount, x)) {
				uint16_t paletteRamOffset = 128 + (_spritePalette[x] << 4) + _spriteColors[x];
				_mainScreenBuffer[x] = _cgram[paletteRamOffset];
				_mainScreenFlags[x] = spritePrio | (((_state.ColorMathEnabled & 0x10) && _spritePalette[x] > 3) ? PixelFlags::AllowColorMath : 0);
			}

			if(drawSub && (_subScreenPriority[x] < spritePrio) && !ProcessMaskWindow<SnesPpu::SpriteLayerIndex>(subWindowCount, x)) {
				uint16_t paletteRamOffset = 128 + (_spritePalette[x] << 4) + _spriteColors[x];
				_subScreenBuffer[x] = _cgram[paletteRamOffset];
				_subScreenPriority[x] = spritePrio;
			}
		}
	}
}

template<uint8_t layerIndex, uint8_t bpp, uint8_t normalPriority, uint8_t highPriority, uint16_t basePaletteOffset, bool hiResMode, bool applyMosaic, bool directColorMode>
void SnesPpu::RenderTilemap()
{
	bool drawMain = (bool)(((_state.MainScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01);
	bool drawSub = (bool)(((_state.SubScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01);

	uint8_t mainWindowCount = _state.WindowMaskMain[layerIndex] ? (uint8_t)_state.Window[0].ActiveLayers[layerIndex] + (uint8_t)_state.Window[1].ActiveLayers[layerIndex] : 0;
	uint8_t subWindowCount = _state.WindowMaskSub[layerIndex] ? (uint8_t)_state.Window[0].ActiveLayers[layerIndex] + (uint8_t)_state.Window[1].ActiveLayers[layerIndex] : 0;

	uint16_t hScrollOriginal = _state.Layers[layerIndex].HScroll;
	uint16_t hScroll = hiResMode ? (hScrollOriginal << 1) : hScrollOriginal;

	TileData* tileData  = _layerData[layerIndex].Tiles;

	uint8_t mosaicCounter = applyMosaic ? (_drawStartX % _state.MosaicSize) : 0;

	uint8_t lookupIndex;
	uint8_t chrDataOffset;
	uint8_t hiresSubColor;
	uint8_t pixelFlags = (((_state.ColorMathEnabled >> layerIndex) & 0x01) ? PixelFlags::AllowColorMath : 0);

	for(int x = _drawStartX; x <= _drawEndX; x++) {
		if constexpr(hiResMode) {
			lookupIndex = (x + (hScrollOriginal & 0x07)) >> 2;
			chrDataOffset = (lookupIndex & 0x01) * bpp / 2;
			lookupIndex >>= 1;
		} else {
			lookupIndex = (x + (hScrollOriginal & 0x07)) >> 3;
		}

		uint16_t tilemapData = tileData[lookupIndex].TilemapData;
		uint16_t* chrData = tileData[lookupIndex].ChrData;
		bool hMirror = (tilemapData & 0x4000) != 0;

		uint8_t color;
		if constexpr(hiResMode) {
			uint8_t xOffset = ((x << 1) + 1 + hScroll) & 0x07;
			uint8_t shift = hMirror ? xOffset : (7 - xOffset);
			color = GetTilePixelColor<bpp>(chrData + chrDataOffset, shift);
			
			xOffset = ((x << 1) + hScroll) & 0x07;
			shift = hMirror ? xOffset : (7 - xOffset);
			hiresSubColor = GetTilePixelColor<bpp>(chrData + chrDataOffset, shift);
		} else {
			uint8_t xOffset = (x + hScroll) & 0x07;
			uint8_t shift = hMirror ? xOffset : (7 - xOffset);
			color = GetTilePixelColor<bpp>(chrData, shift);
		}

		uint8_t paletteIndex = (tilemapData >> 10) & 0x07;
		uint8_t priority = (tilemapData & 0x2000) ? highPriority : normalPriority;

		if constexpr(applyMosaic) {
			if(mosaicCounter == 0) {
				if constexpr(hiResMode) {
					color = hiresSubColor;
				}
				_mosaicColor[layerIndex] = (paletteIndex << 8) | color;
				_mosaicPriority[layerIndex] = priority;
			} else {
				color = _mosaicColor[layerIndex] & 0xFF;
				paletteIndex = _mosaicColor[layerIndex] >> 8;
				priority = _mosaicPriority[layerIndex];
				if constexpr(hiResMode) {
					hiresSubColor = color;
				}
			}

			if(++mosaicCounter == _state.MosaicSize) {
				mosaicCounter = 0;
			}
		}

		if(color > 0) {
			uint16_t rgbColor = GetRgbColor<bpp, directColorMode, basePaletteOffset>(paletteIndex, color);
			if(drawMain && (_mainScreenFlags[x] & 0x0F) < priority && !ProcessMaskWindow<layerIndex>(mainWindowCount, x)) {
				DrawMainPixel(x, rgbColor, priority | pixelFlags);
			}
			if constexpr(!hiResMode) {
				if(drawSub && _subScreenPriority[x] < priority && !ProcessMaskWindow<layerIndex>(subWindowCount, x)) {
					DrawSubPixel(x, rgbColor, priority);
				}
			}
		}

		if constexpr(hiResMode) {
			if(hiresSubColor > 0 && drawSub && _subScreenPriority[x] < priority && !ProcessMaskWindow<layerIndex>(subWindowCount, x)) {
				uint16_t hiresSubRgbColor = GetRgbColor<bpp, directColorMode, basePaletteOffset>(paletteIndex, hiresSubColor);
				DrawSubPixel(x, hiresSubRgbColor, priority);
			}
		}
	}
}

template<uint8_t bpp, bool directColorMode, uint8_t basePaletteOffset>
uint16_t SnesPpu::GetRgbColor(uint8_t paletteIndex, uint8_t colorIndex)
{
	if constexpr(bpp == 8 && directColorMode) {
		return (
			((((colorIndex & 0x07) << 1) | (paletteIndex & 0x01)) << 1) |
			(((colorIndex & 0x38) | ((paletteIndex & 0x02) << 1)) << 4) |
			(((colorIndex & 0xC0) | ((paletteIndex & 0x04) << 3)) << 7)
		);
	} else if constexpr(bpp == 8) {
		//Ignore palette bits for 256-color layers
		_state.InternalCgramAddress = basePaletteOffset + colorIndex;
		return _cgram[_state.InternalCgramAddress];
	} else {
		_state.InternalCgramAddress = basePaletteOffset + paletteIndex * (1 << bpp) + colorIndex;
		return _cgram[_state.InternalCgramAddress];
	}
}

bool SnesPpu::IsRenderRequired(uint8_t layerIndex)
{
	if(((_state.MainScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01) {
		return true;
	}
	if(((_state.SubScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01) {
		return true;
	}

	return false;
}

template<uint8_t bpp>
uint8_t SnesPpu::GetTilePixelColor(const uint16_t chrData[4], const uint8_t shift)
{
	uint8_t color;
	if constexpr(bpp == 2) {
		color = (chrData[0] >> shift) & 0x01;
		color |= (chrData[0] >> (7 + shift)) & 0x02;
	} else if constexpr(bpp == 4) {
		color = (chrData[0] >> shift) & 0x01;
		color |= (chrData[0] >> (7 + shift)) & 0x02;
		color |= ((chrData[1] >> shift) & 0x01) << 2;
		color |= ((chrData[1] >> (7 + shift)) & 0x02) << 2;
	} else if constexpr(bpp == 8) {
		color = (chrData[0] >> shift) & 0x01;
		color |= (chrData[0] >> (7 + shift)) & 0x02;
		color |= ((chrData[1] >> shift) & 0x01) << 2;
		color |= ((chrData[1] >> (7 + shift)) & 0x02) << 2;
		color |= ((chrData[2] >> shift) & 0x01) << 4;
		color |= ((chrData[2] >> (7 + shift)) & 0x02) << 4;
		color |= ((chrData[3] >> shift) & 0x01) << 6;
		color |= ((chrData[3] >> (7 + shift)) & 0x02) << 6;
	} else {
		throw std::runtime_error("unsupported bpp");
	}
	return color;
}

template<uint8_t layerIndex, uint8_t normalPriority, uint8_t highPriority, bool applyMosaic, bool directColorMode>
void SnesPpu::RenderTilemapMode7()
{
	uint8_t mainWindowCount = _state.WindowMaskMain[layerIndex] ? (uint8_t)_state.Window[0].ActiveLayers[layerIndex] + (uint8_t)_state.Window[1].ActiveLayers[layerIndex] : 0;
	uint8_t subWindowCount = _state.WindowMaskSub[layerIndex] ? (uint8_t)_state.Window[0].ActiveLayers[layerIndex] + (uint8_t)_state.Window[1].ActiveLayers[layerIndex] : 0;
	
	bool drawMain = (bool)(((_state.MainScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01);
	bool drawSub = (bool)(((_state.SubScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01);

	auto clip = [](int32_t val) { return (val & 0x2000) ? (val | ~0x3ff) : (val & 0x3ff); };

	if(_drawStartX == 0) {
		//Keep the same scroll offsets for the entire scanline
		_state.Mode7.HScrollLatch = _state.Mode7.HScroll;
		_state.Mode7.VScrollLatch = _state.Mode7.VScroll;
	}

	int32_t hScroll = ((int32_t)_state.Mode7.HScrollLatch << 19) >> 19;
	int32_t vScroll = ((int32_t)_state.Mode7.VScrollLatch << 19) >> 19;
	int32_t centerX = ((int32_t)_state.Mode7.CenterX << 19) >> 19;
	int32_t centerY = ((int32_t)_state.Mode7.CenterY << 19) >> 19;
	uint16_t realY = _state.Mode7.VerticalMirroring ? (255 - _scanline) : _scanline;

	if(applyMosaic) {
		//Keep the "scanline" to what it was at the start of this mosaic block
		realY -= _state.MosaicSize - _mosaicScanlineCounter;
	}
	uint8_t mosaicCounter = applyMosaic ? (_drawStartX % _state.MosaicSize) : 0;

	int32_t xValue = (
		((_state.Mode7.Matrix[0] * clip(hScroll - centerX)) & ~63) +
		((_state.Mode7.Matrix[1] * realY) & ~63) +
		((_state.Mode7.Matrix[1] * clip(vScroll - centerY)) & ~63) +
		(centerX << 8)
	);

	int32_t yValue = (
		((_state.Mode7.Matrix[2] * clip(hScroll - centerX)) & ~63) +
		((_state.Mode7.Matrix[3] * realY) & ~63) +
		((_state.Mode7.Matrix[3] * clip(vScroll - centerY)) & ~63) +
		(centerY << 8)
	);

	int16_t xStep = _state.Mode7.Matrix[0];
	int16_t yStep = _state.Mode7.Matrix[2];
	if(_state.Mode7.HorizontalMirroring) {
		//Calculate the value at the end of the scanline, and then start going backwards
		xValue += xStep * _drawEndX;
		yValue += yStep * _drawEndX;
		xStep = -xStep;
		yStep = -yStep;
	}
	
	if(_drawStartX == 0) {
		//Keep start/end values - used by tilemap viewer
		_debugMode7StartX = xValue;
		_debugMode7StartY = yValue;
		_debugMode7EndX = xValue + xStep * 256;
		_debugMode7EndY = yValue + yStep * 256;
	}

	xValue += xStep * _drawStartX;
	yValue += yStep * _drawStartX;
	
	uint8_t pixelFlags = ((_state.ColorMathEnabled >> layerIndex) & 0x01) ? PixelFlags::AllowColorMath : 0;

	for(int x = _drawStartX; x <= _drawEndX; x++) {
		int32_t xOffset = xValue >> 8;
		int32_t yOffset = yValue >> 8;
		xValue += xStep;
		yValue += yStep;

		uint8_t tileIndex;
		if(!_state.Mode7.LargeMap) {
			yOffset &= 0x3FF;
			xOffset &= 0x3FF;
			tileIndex = (uint8_t)_vram[((yOffset & ~0x07) << 4) | (xOffset >> 3)];
		} else {
			if(yOffset < 0 || yOffset > 0x3FF || xOffset < 0 || xOffset > 0x3FF) {
				if(_state.Mode7.FillWithTile0) {
					tileIndex = 0;
				} else {
					//Draw nothing for this pixel, we're outside the map
					continue;
				}
			} else {
				tileIndex = (uint8_t)_vram[((yOffset & ~0x07) << 4) | (xOffset >> 3)];
			}
		}

		uint16_t colorIndex;
		uint8_t priority;
		if constexpr(layerIndex == 1) {
			uint8_t color = _vram[((tileIndex << 6) + ((yOffset & 0x07) << 3) + (xOffset & 0x07))] >> 8;
			priority = (color & 0x80) ? highPriority : normalPriority;
			colorIndex = (color & 0x7F);
		} else {
			priority = normalPriority;
			colorIndex = _vram[((tileIndex << 6) + ((yOffset & 0x07) << 3) + (xOffset & 0x07))] >> 8;
		}

		if(applyMosaic) {
			if(mosaicCounter == 0) {
				_mosaicColor[layerIndex] = colorIndex;
				_mosaicPriority[layerIndex] = priority;
			} else {
				colorIndex = _mosaicColor[layerIndex];
				priority = _mosaicPriority[layerIndex];
			}

			if(++mosaicCounter == _state.MosaicSize) {
				mosaicCounter = 0;
			}
		}

		if(colorIndex > 0) {
			uint16_t paletteColor;
			if(directColorMode) {
				paletteColor = ((colorIndex & 0x07) << 2) | ((colorIndex & 0x38) << 4) | ((colorIndex & 0xC0) << 7);
			} else {
				paletteColor = _cgram[colorIndex];
			}
			
			if(drawMain && (_mainScreenFlags[x] & 0x0F) < priority && !ProcessMaskWindow<layerIndex>(mainWindowCount, x)) {
				DrawMainPixel(x, paletteColor, priority | pixelFlags);
			} 

			if(drawSub && _subScreenPriority[x] < priority && !ProcessMaskWindow<layerIndex>(subWindowCount, x)) {
				DrawSubPixel(x, paletteColor, priority);
			}
		}
	}
}

void SnesPpu::DrawMainPixel(uint8_t x, uint16_t color, uint8_t flags)
{
	_mainScreenBuffer[x] = color;
	_mainScreenFlags[x] = flags;
}

void SnesPpu::DrawSubPixel(uint8_t x, uint16_t color, uint8_t priority)
{
	_subScreenBuffer[x] = color;
	_subScreenPriority[x] = priority;
}

void SnesPpu::ApplyColorMath()
{
	if(!_skipRender && _emu->IsDebugging()) {
		DebugProcessMainSubScreenViews();
	}

	uint8_t activeWindowCount = (uint8_t)_state.Window[0].ActiveLayers[SnesPpu::ColorWindowIndex] + (uint8_t)_state.Window[1].ActiveLayers[SnesPpu::ColorWindowIndex];
	bool hiResMode = _state.HiResMode || _state.BgMode == 5 || _state.BgMode == 6;

	if(hiResMode) {
		for(int x = _drawStartX; x <= _drawEndX; x++) {
			bool isInsideWindow = ProcessMaskWindow<SnesPpu::ColorWindowIndex>(activeWindowCount, x);

			//Keep original subscreen color, which is used to apply color math to the main screen after
			uint16_t subPixel = _subScreenBuffer[x];
			//Apply the color math based on the previous main pixel
			uint16_t prevMainPixel = x > 0 ? _mainScreenBuffer[x - 1] : 0;
			int prevX = x > 0 ? x - 1 : 0;
			ApplyColorMathToPixel(_subScreenBuffer[x], prevMainPixel, prevX, isInsideWindow);

			ApplyColorMathToPixel(_mainScreenBuffer[x], subPixel, x, isInsideWindow);
		}
	} else {
		for(int x = _drawStartX; x <= _drawEndX; x++) {
			bool isInsideWindow = ProcessMaskWindow<SnesPpu::ColorWindowIndex>(activeWindowCount, x);
			ApplyColorMathToPixel(_mainScreenBuffer[x], _subScreenBuffer[x], x, isInsideWindow);
		}
	}
}

void SnesPpu::ApplyColorMathToPixel(uint16_t &pixelA, uint16_t pixelB, int x, bool isInsideWindow)
{
	uint8_t halfShift = (uint8_t)_state.ColorMathHalveResult;

	//Set color to black as needed based on clip mode
	switch(_state.ColorMathClipMode) {
		default:
		case ColorWindowMode::Never: break;

		case ColorWindowMode::OutsideWindow:
			if(!isInsideWindow) {
				pixelA = 0;
				halfShift = 0;
			}
			break;

		case ColorWindowMode::InsideWindow:
			if(isInsideWindow) {
				pixelA = 0;
				halfShift = 0;
			}
			break;

		case ColorWindowMode::Always: pixelA = 0; break;
	}

	if(!(_mainScreenFlags[x] & PixelFlags::AllowColorMath)) {
		//Color math doesn't apply to this pixel
		return;
	}

	//Prevent color math as needed based on mode
	switch(_state.ColorMathPreventMode) {
		default:
		case ColorWindowMode::Never: break;

		case ColorWindowMode::OutsideWindow:
			if(!isInsideWindow) {
				return;
			}
			break;

		case ColorWindowMode::InsideWindow:
			if(isInsideWindow) {
				return;
			}
			break;

		case ColorWindowMode::Always: return;
	}

	uint16_t otherPixel;
	if(_state.ColorMathAddSubscreen) {
		if(_subScreenPriority[x] > 0) {
			otherPixel = pixelB;
		} else {
			//there's nothing in the subscreen at this pixel, use the fixed color and disable halve operation
			otherPixel = _state.FixedColor;
			halfShift = 0;
		}
	} else {
		otherPixel = _state.FixedColor;
	}

	constexpr unsigned int mask = 0x1F;
	if(_state.ColorMathSubtractMode) {
		uint16_t r = std::max((int)((pixelA & mask) - (otherPixel & mask)), 0) >> halfShift;
		uint16_t g = std::max((int)(((pixelA >> 5U) & mask) - ((otherPixel >> 5U) & mask)), 0) >> halfShift;
		uint16_t b = std::max((int)(((pixelA >> 10U) & mask) - ((otherPixel >> 10U) & mask)), 0) >> halfShift;

		pixelA = r | (g << 5U) | (b << 10U);
	} else {
		uint16_t r = std::min(((pixelA & mask) + (otherPixel & mask)) >> halfShift, mask);
		uint16_t g = std::min((((pixelA >> 5U) & mask) + ((otherPixel >> 5U) & mask)) >> halfShift, mask);
		uint16_t b = std::min((((pixelA >> 10U) & mask) + ((otherPixel >> 10U) & mask)) >> halfShift, mask);

		pixelA = r | (g << 5U) | (b << 10U);
	}
}

template<bool forMainScreen>
void SnesPpu::ApplyBrightness()
{
	if(_state.ScreenBrightness != 15) {
		for(int x = _drawStartX; x <= _drawEndX; x++) {
			uint16_t &pixel = (forMainScreen ? _mainScreenBuffer : _subScreenBuffer)[x];
			uint16_t r = (pixel & 0x1F) * _state.ScreenBrightness / 15;
			uint16_t g = ((pixel >> 5) & 0x1F) * _state.ScreenBrightness / 15;
			uint16_t b = ((pixel >> 10) & 0x1F) * _state.ScreenBrightness / 15;
			pixel = r | (g << 5) | (b << 10);
		}
	}
}

void SnesPpu::ConvertToHiRes()
{
	if(_skipRender) {
		return;
	}

	bool useHighResOutput = _useHighResOutput || IsDoubleWidth() || _state.ScreenInterlace;
	if(!useHighResOutput || _useHighResOutput == useHighResOutput || _scanline >= _vblankStartScanline || _scanline == 0) {
		return;
	}

	//Convert standard res picture to high resolution when the PPU starts drawing in high res mid frame
	_useHighResOutput = useHighResOutput;

	uint16_t scanline = _overscanFrame ? (_scanline - 1) : (_scanline + 6);

	if(_drawStartX > 0) {
		for(int x = 0; x < _drawStartX; x++) {
			_currentBuffer[(scanline << 10) + (x << 1)] = _currentBuffer[(scanline << 8) + x];
			_currentBuffer[(scanline << 10) + (x << 1) + 1] = _currentBuffer[(scanline << 8) + x];
		}
		memcpy(_currentBuffer + (scanline << 10) + 512, _currentBuffer + (scanline << 10), 512 * sizeof(uint16_t));
	}

	for(int i = scanline - 1; i >= 0; i--) {
		for(int x = 0; x < 256; x++) {
			_currentBuffer[(i << 10) + (x << 1)] = _currentBuffer[(i << 8) + x];
			_currentBuffer[(i << 10) + (x << 1) + 1] = _currentBuffer[(i << 8) + x];
		}
		memcpy(_currentBuffer + (i << 10) + 512, _currentBuffer + (i << 10), 512 * sizeof(uint16_t));
	}
}

void SnesPpu::ApplyHiResMode()
{
	//When overscan mode is off, center the 224-line picture in the center of the 239-line output buffer
	uint16_t scanline = _overscanFrame ? (_scanline - 1) : (_scanline + 6);

	if(!_useHighResOutput) {
		memcpy(_currentBuffer + (scanline << 8) + _drawStartX, _mainScreenBuffer + _drawStartX, (_drawEndX - _drawStartX + 1) << 1);
	} else {
		_interlacedFrame |= _state.ScreenInterlace;
		uint32_t screenY = _state.ScreenInterlace ? (_oddFrame ? ((scanline << 1) + 1) : (scanline << 1)) : (scanline << 1);
		uint32_t baseAddr = (screenY << 9);

		if(IsDoubleWidth()) {
			ApplyBrightness<false>();
			for(int x = _drawStartX; x <= _drawEndX; x++) {
				_currentBuffer[baseAddr + (x << 1)] = _subScreenBuffer[x];
				_currentBuffer[baseAddr + (x << 1) + 1] = _mainScreenBuffer[x];
			}
		} else {
			for(int x = _drawStartX; x <= _drawEndX; x++) {
				_currentBuffer[baseAddr + (x << 1)] = _mainScreenBuffer[x];
				_currentBuffer[baseAddr + (x << 1) + 1] = _mainScreenBuffer[x];
			}
		}

		if(!_state.ScreenInterlace) {
			//Copy this line's content to the next line (between the current start & end bounds)
			memcpy(
				_currentBuffer + baseAddr + 512 + (_drawStartX << 1),
				_currentBuffer + baseAddr + (_drawStartX << 1),
				(_drawEndX - _drawStartX + 1) << 2
			);
		}
	}
}

template<uint8_t layerIndex>
bool SnesPpu::ProcessMaskWindow(uint8_t activeWindowCount, int x)
{
	switch(activeWindowCount) {
		case 1: 
			if(_state.Window[0].ActiveLayers[layerIndex]) {
				return _state.Window[0].PixelNeedsMasking<layerIndex>(x);
			}
			return _state.Window[1].PixelNeedsMasking<layerIndex>(x);

		case 2:
			switch(_state.MaskLogic[layerIndex]) {
				default:
				case WindowMaskLogic::Or: return _state.Window[0].PixelNeedsMasking<layerIndex>(x) | _state.Window[1].PixelNeedsMasking<layerIndex>(x);
				case WindowMaskLogic::And: return _state.Window[0].PixelNeedsMasking<layerIndex>(x) & _state.Window[1].PixelNeedsMasking<layerIndex>(x);
				case WindowMaskLogic::Xor: return _state.Window[0].PixelNeedsMasking<layerIndex>(x) ^ _state.Window[1].PixelNeedsMasking<layerIndex>(x);
				case WindowMaskLogic::Xnor: return !(_state.Window[0].PixelNeedsMasking<layerIndex>(x) ^ _state.Window[1].PixelNeedsMasking<layerIndex>(x));
			}
	}
	return false;
}

void SnesPpu::ProcessWindowMaskSettings(uint8_t value, uint8_t offset)
{
	_state.Window[0].ActiveLayers[0 + offset] = (value & 0x02) != 0;
	_state.Window[0].ActiveLayers[1 + offset] = (value & 0x20) != 0;
	_state.Window[0].InvertedLayers[0 + offset] = (value & 0x01) != 0;
	_state.Window[0].InvertedLayers[1 + offset] = (value & 0x10) != 0;

	_state.Window[1].ActiveLayers[0 + offset] = (value & 0x08) != 0;
	_state.Window[1].ActiveLayers[1 + offset] = (value & 0x80) != 0;
	_state.Window[1].InvertedLayers[0 + offset] = (value & 0x04) != 0;
	_state.Window[1].InvertedLayers[1 + offset] = (value & 0x40) != 0;
}

void SnesPpu::SendFrame()
{
	uint16_t width = _useHighResOutput ? 512 : 256;
	uint16_t height = _useHighResOutput ? 478 : 239;

	if(!_overscanFrame) {
		//Clear the top 7 and bottom 8 rows
		int top = (_useHighResOutput ? 14 : 7);
		int bottom = (_useHighResOutput ? 16 : 8);
		memset(_currentBuffer, 0, width * top * sizeof(uint16_t));
		memset(_currentBuffer + width * (height - bottom), 0, width * bottom * sizeof(uint16_t));
	}

	_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);

	bool isRewinding = _emu->GetRewindManager()->IsRewinding();
	if(isRewinding && _needFullFrame) {
		FillInterlacedFrame();
	}
	_needFullFrame = false;

	RenderedFrame frame(_currentBuffer, width, height, _useHighResOutput ? 0.5 : 1.0, _frameCount, _console->GetControlManager()->GetPortStates());
	_emu->GetVideoDecoder()->UpdateFrame(frame, isRewinding, isRewinding);

	if(!_skipRender) {
		_frameSkipTimer.Reset();
	}
}

void SnesPpu::DebugSendFrame()
{
	if(_scanline < _vblankStartScanline) {
		RenderScanline();
	}

	uint16_t width = _useHighResOutput ? 512 : 256;
	uint16_t height = _useHighResOutput ? 478 : 239;

	int lastDrawnPixel = _drawEndX * (_useHighResOutput ? 2 : 1);
	int scanline = _overscanFrame ? ((int)_scanline - 1) : ((int)_scanline + 6);

	int offset = std::max(0, lastDrawnPixel + 1 + scanline * width);
	int pixelsToClear = width * height - offset;
	if(pixelsToClear > 0) {
		memset(_currentBuffer + offset, 0, pixelsToClear * sizeof(uint16_t));
	}

	RenderedFrame frame(_currentBuffer, width, height, _useHighResOutput ? 0.5 : 1.0, _frameCount);
	_emu->GetVideoDecoder()->UpdateFrame(frame, false, false);
}

void SnesPpu::DebugProcessMode7Overlay()
{
	//Store mode 7 related information in ppu tools to allow tilemap viewer to display mode 7 overlay
	SnesPpuTools* ppuTools = ((SnesPpuTools*)_emu->InternalGetDebugger()->GetPpuTools(CpuType::Snes));
	ppuTools->SetPpuScanlineState(_scanline, _state.ForcedBlank ? 0 : _state.BgMode, _debugMode7StartX, _debugMode7StartY, _debugMode7EndX, _debugMode7EndY);
	if(_scanline == _vblankStartScanline - 1) {
		for(int i = _scanline + 1; i < 239; i++) {
			ppuTools->SetPpuScanlineState(i, 0, 0, 0, 0, 0);
		}
	}
	_debugMode7StartX = _debugMode7StartY = _debugMode7EndX = _debugMode7EndY = 0;
}

void SnesPpu::DebugProcessMainSubScreenViews()
{
	//Store main/sub screen buffers in ppu tools to allow display main/sub screen view in tilemap viewer
	SnesPpuTools* ppuTools = ((SnesPpuTools*)_emu->InternalGetDebugger()->GetPpuTools(CpuType::Snes));
	ppuTools->SetPpuRowBuffers(_scanline - 1, _drawStartX, _drawEndX, _mainScreenBuffer, _subScreenBuffer);
	if(_scanline == _vblankStartScanline - 1) {
		for(int i = _scanline; i < 239; i++) {
			ppuTools->SetPpuRowBuffers(i, _drawStartX, _drawEndX, nullptr, nullptr);
		}
	}
}

void SnesPpu::FillInterlacedFrame()
{
	//Patch to make rewinding interlaced games look less glitchy (otherwise a half a frame's rows are wrong every 30 frames)
	for(int i = 0; i < 478 / 2; i++) {
		memcpy(_currentBuffer+(i*2+(_oddFrame^1))*512, _currentBuffer+(i*2+(_oddFrame))*512, 512*sizeof(uint16_t));
	}
}

bool SnesPpu::IsHighResOutput()
{
	return _useHighResOutput;
}

uint16_t* SnesPpu::GetScreenBuffer()
{
	return _currentBuffer;
}

uint16_t* SnesPpu::GetPreviousScreenBuffer()
{
	return _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
}

uint8_t* SnesPpu::GetVideoRam()
{
	return (uint8_t*)_vram;
}

uint8_t* SnesPpu::GetCgRam()
{
	return (uint8_t*)_cgram;
}

uint8_t* SnesPpu::GetSpriteRam()
{
	return (uint8_t*)_oamRam;
}

bool SnesPpu::IsDoubleHeight()
{
	return _state.ScreenInterlace && (_state.BgMode == 5 || _state.BgMode == 6);
}

bool SnesPpu::IsDoubleWidth()
{
	return _state.HiResMode || _state.BgMode == 5 || _state.BgMode == 6;
}

bool SnesPpu::CanAccessCgram()
{
	return _scanline >= _nmiScanline || _scanline == 0 || _state.ForcedBlank || _memoryManager->GetHClock() < 88 || _memoryManager->GetHClock() >= 1096;
}

bool SnesPpu::CanAccessVram()
{
	return _scanline >= _nmiScanline || _state.ForcedBlank;
}

void SnesPpu::SetLocationLatchRequest(uint16_t x, uint16_t y)
{
	//Used by super scope
	_latchRequest = true;
	_latchRequestX = x;
	_latchRequestY = y;
}

void SnesPpu::ProcessLocationLatchRequest()
{
	//Used by super scope
	if(_latchRequest) {
		uint16_t cycle = GetCycle();
		uint16_t scanline = GetRealScanline();
		if(scanline > _latchRequestY || (_latchRequestY == scanline && cycle >= _latchRequestX)) {
			_latchRequest = false;
			_horizontalLocation = _latchRequestX;
			_verticalLocation = _latchRequestY;
			_locationLatched = true;
		}
	}
}

void SnesPpu::LatchLocationValues()
{
	_horizontalLocation = GetCycle();
	_verticalLocation = GetRealScanline();
	_locationLatched = true;
}

void SnesPpu::UpdateOamAddress()
{
	_state.InternalOamAddress = (_state.OamRamAddress << 1);
}

uint16_t SnesPpu::GetOamAddress()
{
	if(_state.ForcedBlank || _scanline >= _vblankStartScanline) {
		return _state.InternalOamAddress;
	} else {
		if(_memoryManager->GetHClock() <= 255 * 4) {
			return _oamEvaluationIndex << 2;
		} else {
			return _oamTimeIndex << 2;
		}
	}
}

void SnesPpu::UpdateVramReadBuffer()
{
	//During rendering, this can't read the correct VRAM address
	//Unknown: does it read from the address the ppu is currently reading from (like oam/cgram)?
	_state.VramReadBuffer = CanAccessVram() ? _vram[GetVramAddress()] : 0;
}

uint16_t SnesPpu::GetVramAddress()
{
	uint16_t addr = _state.VramAddress;
	switch(_state.VramAddressRemapping) {
		default:
		case 0: return addr;
		case 1: return (addr & 0xFF00) | ((addr & 0xE0) >> 5) | ((addr & 0x1F) << 3);
		case 2: return (addr & 0xFE00) | ((addr & 0x1C0) >> 6) | ((addr & 0x3F) << 3);
		case 3: return (addr & 0xFC00) | ((addr & 0x380) >> 7) | ((addr & 0x7F) << 3);
	}
}

uint8_t SnesPpu::Read(uint16_t addr)
{
	if(_scanline < _vblankStartScanline) {
		RenderScanline();
	}

	switch(addr) {
		case 0x2134:
			_state.Ppu1OpenBus = ((int16_t)_state.Mode7.Matrix[0] * ((int16_t)_state.Mode7.Matrix[1] >> 8)) & 0xFF;
			return _state.Ppu1OpenBus;

		case 0x2135:
			_state.Ppu1OpenBus = (((int16_t)_state.Mode7.Matrix[0] * ((int16_t)_state.Mode7.Matrix[1] >> 8)) >> 8) & 0xFF;
			return _state.Ppu1OpenBus;

		case 0x2136:
			_state.Ppu1OpenBus = (((int16_t)_state.Mode7.Matrix[0] * ((int16_t)_state.Mode7.Matrix[1] >> 8)) >> 16) & 0xFF;
			return _state.Ppu1OpenBus;

		case 0x2137:
			//SLHV - Software Latch for H/V Counter
			//Latch values on read, and return open bus
			if(_regs->GetIoPortOutput() & 0x80) {
				//Only latch H/V counters if bit 7 of $4201 is set.
				LatchLocationValues();
			}
			break;
			
		case 0x2138: {
			//OAMDATAREAD - Data for OAM read
			//When trying to read/write during rendering, the internal address used by the PPU's sprite rendering is used
			uint16_t oamAddr = GetOamAddress();
			uint8_t value;
			if(oamAddr < 512) {
				value = _oamRam[oamAddr];
				_emu->ProcessPpuRead<CpuType::Snes>(oamAddr, value, MemoryType::SnesSpriteRam);
			} else {
				value = _oamRam[0x200 | (oamAddr & 0x1F)];
				_emu->ProcessPpuRead<CpuType::Snes>(0x200 | (oamAddr & 0x1F), value, MemoryType::SnesSpriteRam);
			}
			
			_state.InternalOamAddress = (_state.InternalOamAddress + 1) & 0x3FF;
			_state.Ppu1OpenBus = value;
			return value;
		}

		case 0x2139: {
			//VMDATALREAD - VRAM Data Read low byte
			uint8_t returnValue = (uint8_t)_state.VramReadBuffer;
			_emu->ProcessPpuRead<CpuType::Snes>(GetVramAddress(), returnValue, MemoryType::SnesVideoRam);
			if(!_state.VramAddrIncrementOnSecondReg) {
				UpdateVramReadBuffer();
				_state.VramAddress = (_state.VramAddress + _state.VramIncrementValue) & 0x7FFF;
			}
			_state.Ppu1OpenBus = returnValue;
			return returnValue;
		}

		case 0x213A: {
			//VMDATAHREAD - VRAM Data Read high byte
			uint8_t returnValue = (uint8_t)(_state.VramReadBuffer >> 8);
			_emu->ProcessPpuRead<CpuType::Snes>(GetVramAddress() + 1, returnValue, MemoryType::SnesVideoRam);
			if(_state.VramAddrIncrementOnSecondReg) {
				UpdateVramReadBuffer();
				_state.VramAddress = (_state.VramAddress + _state.VramIncrementValue) & 0x7FFF;
			}
			_state.Ppu1OpenBus = returnValue;
			return returnValue;
		}

		case 0x213B: {
			//CGDATAREAD - CGRAM Data read
			uint8_t value;
			
			//During rendering, reads to CGRAM end up returning the value a the address the PPU is currently reading
			uint16_t cgAddr = CanAccessCgram() ? _state.CgramAddress : _state.InternalCgramAddress;

			if(_state.CgramAddressLatch){
				value = ((_cgram[cgAddr] >> 8) & 0x7F) | (_state.Ppu2OpenBus & 0x80);
				_emu->ProcessPpuRead<CpuType::Snes>((cgAddr << 1) + 1, value, MemoryType::SnesCgRam);
				_state.CgramAddress++;
			} else {
				value = (uint8_t)_cgram[cgAddr];
				_emu->ProcessPpuRead<CpuType::Snes>(cgAddr << 1, value, MemoryType::SnesCgRam);
			}
			_state.CgramAddressLatch = !_state.CgramAddressLatch;
			
			_state.Ppu2OpenBus = value;
			return value;
		}

		case 0x213C: {
			//OPHCT - Horizontal Scanline Location
			ProcessLocationLatchRequest();

			uint8_t value;
			if(_horizontalLocToggle) {
				//"Note that the value read is only 9 bits: bits 1-7 of the high byte are PPU2 Open Bus."
				value = ((_horizontalLocation & 0x100) >> 8) | (_state.Ppu2OpenBus & 0xFE);
			} else {
				value = _horizontalLocation & 0xFF;
			}
			_state.Ppu2OpenBus = value;
			_horizontalLocToggle = !_horizontalLocToggle;
			return value;
		}

		case 0x213D: {
			//OPVCT - Vertical Scanline Location
			ProcessLocationLatchRequest();

			uint8_t value;
			if(_verticalLocationToggle) {
				//"Note that the value read is only 9 bits: bits 1-7 of the high byte are PPU2 Open Bus."
				value = ((_verticalLocation & 0x100) >> 8) | (_state.Ppu2OpenBus & 0xFE);
			} else {
				value = _verticalLocation & 0xFF;
			}
			_state.Ppu2OpenBus = value;
			_verticalLocationToggle = !_verticalLocationToggle;
			return value;
		}

		case 0x213E: {
			//STAT77 - PPU Status Flag and Version
			uint8_t value = (
				(_timeOver ? 0x80 : 0) |
				(_rangeOver ? 0x40 : 0) |
				(_state.Ppu1OpenBus & 0x10) |
				0x01 //PPU (5c77) chip version
			);
			_state.Ppu1OpenBus = value;
			return value;
		}

		case 0x213F: {
			//STAT78 - PPU Status Flag and Version
			ProcessLocationLatchRequest();

			uint8_t value = (
				(_oddFrame ? 0x80 : 0) |
				(_locationLatched ? 0x40 : 0) |
				(_state.Ppu2OpenBus & 0x20) |
				(_console->GetRegion() == ConsoleRegion::Pal ? 0x10 : 0) |
				0x03 //PPU (5c78) chip version
			);

			if(_regs->GetIoPortOutput() & 0x80) {
				_locationLatched = false;
			}

			//"The high/low selector is reset to elowf when $213F is read" (the selector is NOT reset when the counter is latched)
			_horizontalLocToggle = false;
			_verticalLocationToggle = false;

			_state.Ppu2OpenBus = value;
			return value;
		}

		default:
			LogDebug("[Debug] Unimplemented register read: " + HexUtilities::ToHex(addr));
			break;
	}
	
	uint16_t reg = addr & 0x210F;
	if((reg >= 0x2104 && reg <= 0x2106) || (reg >= 0x2108 && reg <= 0x210A)) {
		//Registers matching $21x4-6 or $21x8-A (where x is 0-2) return the last value read from any of the PPU1 registers $2134-6, $2138-A, or $213E.
		return _state.Ppu1OpenBus;
	}
	return _console->GetMemoryManager()->GetOpenBus();
}

void SnesPpu::Write(uint32_t addr, uint8_t value)
{
	if(_scanline < _vblankStartScanline) {
		RenderScanline();
	}

	switch(addr) {
		case 0x2100:
			if(_state.ForcedBlank && _scanline == _nmiScanline) {
				//"writing this register on the first line of V-Blank (225 or 240, depending on overscan) when force blank is currently active causes the OAM Address Reset to occur."
				UpdateOamAddress();
			}

			_state.ForcedBlank = (value & 0x80) != 0;
			_state.ScreenBrightness = value & 0x0F;
			break;

		case 0x2101:
			_state.OamMode = (value & 0xE0) >> 5;
			_state.OamBaseAddress = (value & 0x07) << 13;
			_state.OamAddressOffset = (((value & 0x18) >> 3) + 1) << 12;
			break;

		case 0x2102:
			_state.OamRamAddress = (_state.OamRamAddress & 0x100) | value;
			UpdateOamAddress();
			break;

		case 0x2103:
			_state.OamRamAddress = (_state.OamRamAddress & 0xFF) | ((value & 0x01) << 8);
			UpdateOamAddress();
			_state.EnableOamPriority = (value & 0x80) != 0;
			break;

		case 0x2104: {
			//When trying to read/write during rendering, the internal address used by the PPU's sprite rendering is used
			//This is approximated by _oamRenderAddress (but is not cycle accurate) - needed for Uniracers
			uint16_t oamAddr = GetOamAddress();
			
			if(oamAddr < 512) {
				if(oamAddr & 0x01) {
					_emu->ProcessPpuWrite<CpuType::Snes>(oamAddr - 1, _oamWriteBuffer, MemoryType::SnesSpriteRam);
					_oamRam[oamAddr - 1] = _oamWriteBuffer;
	
					_emu->ProcessPpuWrite<CpuType::Snes>(oamAddr, value, MemoryType::SnesSpriteRam);
					_oamRam[oamAddr] = value;
				} else {
					_oamWriteBuffer = value;
				}
			} 

			if(!_state.ForcedBlank && _scanline < _nmiScanline) {
				//During rendering the high table is also written to when writing to OAM
				oamAddr = 0x200 | ((oamAddr & 0x1F0) >> 4);
			}
			
			if(oamAddr >= 512) {
				uint16_t address = 0x200 | (oamAddr & 0x1F);
				if((oamAddr & 0x01) == 0) {
					_oamWriteBuffer = value;
				}
				_emu->ProcessPpuWrite<CpuType::Snes>(address, value, MemoryType::SnesSpriteRam);
				_oamRam[address] = value;
			}
			_state.InternalOamAddress = (_state.InternalOamAddress + 1) & 0x3FF;
			break;
		}

		case 0x2105:
			_state.BgMode = value & 0x07;
			ConvertToHiRes();

			_state.Mode1Bg3Priority = (value & 0x08) != 0;

			_state.Layers[0].LargeTiles = (value & 0x10) != 0;
			_state.Layers[1].LargeTiles = (value & 0x20) != 0;
			_state.Layers[2].LargeTiles = (value & 0x40) != 0;
			_state.Layers[3].LargeTiles = (value & 0x80) != 0;
			break;

		case 0x2106: {
			//MOSAIC - Screen Pixelation
			_state.MosaicSize = ((value & 0xF0) >> 4) + 1;
			uint8_t mosaicEnabled = value & 0x0F;
			if(!_state.MosaicEnabled && mosaicEnabled) {
				//"If this register is set during the frame, the starting scanline is the current scanline, otherwise it is the first visible scanline of the frame."
				//This is only done when mosaic is turned on from an off state (FF6 mosaic effect looks wrong otherwise)
				//FF6's mosaic effect is broken on some screens without this.
				_mosaicScanlineCounter = _state.MosaicSize + 1;
			}
			_state.MosaicEnabled = mosaicEnabled;
			break;
		}

		case 0x2107: case 0x2108: case 0x2109: case 0x210A:
			//BG 1-4 Tilemap Address and Size (BG1SC, BG2SC, BG3SC, BG4SC)
			_state.Layers[addr - 0x2107].TilemapAddress = (value & 0x7C) << 8;
			_state.Layers[addr - 0x2107].DoubleWidth = (value & 0x01) != 0;
			_state.Layers[addr - 0x2107].DoubleHeight = (value & 0x02) != 0;
			break;

		case 0x210B: case 0x210C:
			//BG1+2 / BG3+4 Chr Address (BG12NBA / BG34NBA)
			_state.Layers[(addr - 0x210B) * 2].ChrAddress = (value & 0x07) << 12;
			_state.Layers[(addr - 0x210B) * 2 + 1].ChrAddress = (value & 0x70) << 8;
			break;
		
		case 0x210D:
			//M7HOFS - Mode 7 BG Horizontal Scroll
			//BG1HOFS - BG1 Horizontal Scroll
			_state.Mode7.HScroll = ((value << 8) | (_state.Mode7.ValueLatch)) & 0x1FFF;
			_state.Mode7.ValueLatch = value;
			
			//no break, keep executing to set the matching BG1 HScroll register, too
			[[fallthrough]];

		case 0x210F: case 0x2111: case 0x2113:
			//BGXHOFS - BG1/2/3/4 Horizontal Scroll
			_state.Layers[(addr - 0x210D) >> 1].HScroll = ((value << 8) | (_hvScrollLatchValue & ~0x07) | (_hScrollLatchValue & 0x07)) & 0x3FF;
			_hvScrollLatchValue = value;
			_hScrollLatchValue = value;
			break;

		case 0x210E:
			//M7VOFS - Mode 7 BG Vertical Scroll
			//BG1VOFS - BG1 Vertical Scroll
			_state.Mode7.VScroll = ((value << 8) | (_state.Mode7.ValueLatch)) & 0x1FFF;
			_state.Mode7.ValueLatch = value;
			
			//no break, keep executing to set the matching BG1 HScroll register, too
			[[fallthrough]];

		case 0x2110: case 0x2112: case 0x2114:
			//BGXVOFS - BG1/2/3/4 Vertical Scroll
			_state.Layers[(addr - 0x210E) >> 1].VScroll = ((value << 8) | _hvScrollLatchValue) & 0x3FF;
			_hvScrollLatchValue = value;
			break;

		case 0x2115:
			//VMAIN - Video Port Control
			switch(value & 0x03) {
				case 0: _state.VramIncrementValue = 1; break;
				case 1: _state.VramIncrementValue = 32; break;
				
				case 2: 
				case 3: _state.VramIncrementValue = 128; break;
			}

			_state.VramAddressRemapping = (value & 0x0C) >> 2;
			_state.VramAddrIncrementOnSecondReg = (value & 0x80) != 0;
			break;

		case 0x2116:
			//VMADDL - VRAM Address low byte
			_state.VramAddress = (_state.VramAddress & 0x7F00) | value;
			UpdateVramReadBuffer();
			break;

		case 0x2117:
			//VMADDH - VRAM Address high byte
			_state.VramAddress = (_state.VramAddress & 0x00FF) | ((value & 0x7F) << 8);
			UpdateVramReadBuffer();
			break;

		case 0x2118:
			//VMDATAL - VRAM Data Write low byte
			if(CanAccessVram()) {
				//Only write the value if in vblank or forced blank (writes to VRAM outside vblank/forced blank are not allowed)
				_emu->ProcessPpuWrite<CpuType::Snes>(GetVramAddress() << 1, value, MemoryType::SnesVideoRam);
				_vram[GetVramAddress()] = value | (_vram[GetVramAddress()] & 0xFF00);
			}

			//The VRAM address is incremented even outside of vblank/forced blank
			if(!_state.VramAddrIncrementOnSecondReg) {
				_state.VramAddress = (_state.VramAddress + _state.VramIncrementValue) & 0x7FFF;
			}
			break;

		case 0x2119:
			//VMDATAH - VRAM Data Write high byte
			if(CanAccessVram()) {
				//Only write the value if in vblank or forced blank (writes to VRAM outside vblank/forced blank are not allowed)
				_emu->ProcessPpuWrite<CpuType::Snes>((GetVramAddress() << 1) + 1, value, MemoryType::SnesVideoRam);
				_vram[GetVramAddress()] = (value << 8) | (_vram[GetVramAddress()] & 0xFF); 
			}
			
			//The VRAM address is incremented even outside of vblank/forced blank
			if(_state.VramAddrIncrementOnSecondReg) {
				_state.VramAddress = (_state.VramAddress + _state.VramIncrementValue) & 0x7FFF;
			}
			break;

		case 0x211A:
			//M7SEL - Mode 7 Settings
			_state.Mode7.LargeMap = (value & 0x80) != 0;
			_state.Mode7.FillWithTile0 = (value & 0x40) != 0;
			_state.Mode7.HorizontalMirroring = (value & 0x01) != 0;
			_state.Mode7.VerticalMirroring = (value & 0x02) != 0;
			break;

		case 0x211B: case 0x211C: case 0x211D: case 0x211E:
			//M7A/B/C/D - Mode 7 Matrix A/B/C/D (A/B are also used with $2134/6)
			_state.Mode7.Matrix[addr - 0x211B] = (value << 8) | _state.Mode7.ValueLatch;
			_state.Mode7.ValueLatch = value;
			break;
		
		case 0x211F:
			//M7X - Mode 7 Center X
			_state.Mode7.CenterX = ((value << 8) | _state.Mode7.ValueLatch);
			_state.Mode7.ValueLatch = value;
			break;

		case 0x2120:
			//M7Y - Mode 7 Center Y
			_state.Mode7.CenterY = ((value << 8) | _state.Mode7.ValueLatch);
			_state.Mode7.ValueLatch = value;
			break;

		case 0x2121:
			//CGRAM Address(CGADD)
			_state.CgramAddress = value;
			_state.CgramAddressLatch = false;
			break;

		case 0x2122: 
			//CGRAM Data write (CGDATA)
			if(_state.CgramAddressLatch) {
				//MSB ignores the 7th bit (colors are 15-bit only)
				value &= 0x7F;

				//During rendering, writes to CGRAM end up writing to the address the PPU is currently reading
				uint16_t cgAddr = CanAccessCgram() ? _state.CgramAddress : _state.InternalCgramAddress;

				_emu->ProcessPpuWrite<CpuType::Snes>(cgAddr << 1, _state.CgramWriteBuffer, MemoryType::SnesCgRam);
				_emu->ProcessPpuWrite<CpuType::Snes>((cgAddr << 1) + 1, value, MemoryType::SnesCgRam);

				_cgram[cgAddr] = _state.CgramWriteBuffer | (value << 8);
				_state.CgramAddress++;
			} else {
				_state.CgramWriteBuffer = value;
			}
			_state.CgramAddressLatch = !_state.CgramAddressLatch;
			break;

		case 0x2123:
			//W12SEL - Window Mask Settings for BG1 and BG2
			ProcessWindowMaskSettings(value, 0);
			break;

		case 0x2124:
			//W34SEL - Window Mask Settings for BG3 and BG4
			ProcessWindowMaskSettings(value, 2);
			break;

		case 0x2125:
			//WOBJSEL - Window Mask Settings for OBJ and Color Window
			ProcessWindowMaskSettings(value, 4);
			break;

		case 0x2126:
			//WH0 - Window 1 Left Position
			_state.Window[0].Left = value;
			break;
		
		case 0x2127:
			//WH1 - Window 1 Right Position
			_state.Window[0].Right = value;
			break;

		case 0x2128:
			//WH2 - Window 2 Left Position
			_state.Window[1].Left = value;
			break;

		case 0x2129:
			//WH3 - Window 2 Right Position
			_state.Window[1].Right = value;
			break;

		case 0x212A:
			//WBGLOG - Window mask logic for BG
			_state.MaskLogic[0] = (WindowMaskLogic)(value & 0x03);
			_state.MaskLogic[1] = (WindowMaskLogic)((value >> 2) & 0x03);
			_state.MaskLogic[2] = (WindowMaskLogic)((value >> 4) & 0x03);
			_state.MaskLogic[3] = (WindowMaskLogic)((value >> 6) & 0x03);
			break;

		case 0x212B:
			//WOBJLOG - Window mask logic for OBJs and Color Window
			_state.MaskLogic[4] = (WindowMaskLogic)((value >> 0) & 0x03);
			_state.MaskLogic[5] = (WindowMaskLogic)((value >> 2) & 0x03);
			break;

		case 0x212C:
			//TM - Main Screen Designation
			_state.MainScreenLayers = value & 0x1F;
			break;

		case 0x212D:
			//TS - Subscreen Designation
			_state.SubScreenLayers = value & 0x1F;
			break;

		case 0x212E:
			//TMW - Window Mask Designation for the Main Screen
			for(int i = 0; i < 5; i++) {
				_state.WindowMaskMain[i] = ((value >> i) & 0x01) != 0;
			}
			break;

		case 0x212F:
			//TSW - Window Mask Designation for the Subscreen
			for(int i = 0; i < 5; i++) {
				_state.WindowMaskSub[i] = ((value >> i) & 0x01) != 0;
			}
			break;
		
		case 0x2130:
			//CGWSEL - Color Addition Select
			_state.ColorMathClipMode = (ColorWindowMode)((value >> 6) & 0x03);
			_state.ColorMathPreventMode = (ColorWindowMode)((value >> 4) & 0x03);
			_state.ColorMathAddSubscreen = (value & 0x02) != 0;
			_state.DirectColorMode = (value & 0x01) != 0;
			break;

		case 0x2131:
			//CGADSUB - Color math designation
			_state.ColorMathEnabled = value & 0x3F;
			_state.ColorMathSubtractMode = (value & 0x80) != 0;
			_state.ColorMathHalveResult = (value & 0x40) != 0;
			break;

		case 0x2132: 
			//COLDATA - Fixed Color Data
			if(value & 0x80) { //B
				_state.FixedColor = (_state.FixedColor & ~0x7C00) | ((value & 0x1F) << 10);
			}
			if(value & 0x40) { //G
				_state.FixedColor = (_state.FixedColor & ~0x3E0) | ((value & 0x1F) << 5);
			}
			if(value & 0x20) { //R
				_state.FixedColor = (_state.FixedColor & ~0x1F) | (value & 0x1F);
			}
			break;

		case 0x2133: {
			//SETINI - Screen Mode/Video Select
			//_externalSync = (value & 0x80) != 0;  //NOT USED
			_state.ExtBgEnabled = (value & 0x40) != 0;
			_state.HiResMode = (value & 0x08) != 0;
			_state.OverscanMode = (value & 0x04) != 0;
			_state.ObjInterlace = (value & 0x02) != 0;

			bool interlace = (value & 0x01) != 0;
			if(_state.ScreenInterlace != interlace) {
				_state.ScreenInterlace = interlace;
				if(_scanline >= _vblankStartScanline && interlace) {
					//Clear buffer when turning on interlace mode during vblank
					memset(GetPreviousScreenBuffer(), 0, 512 * 478 * sizeof(uint16_t));
				}
			}
			ConvertToHiRes();
			break;
		}

		default:
			LogDebug("[Debug] Unimplemented register write: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

void SnesPpu::Serialize(Serializer &s)
{
	SV(_state.ForcedBlank); SV(_state.ScreenBrightness); SV(_scanline); SV(_frameCount);  SV(_state.BgMode);
	SV(_state.Mode1Bg3Priority); SV(_state.MainScreenLayers); SV(_state.SubScreenLayers); SV(_state.VramAddress); SV(_state.VramIncrementValue); SV(_state.VramAddressRemapping);
	SV(_state.VramAddrIncrementOnSecondReg); SV(_state.VramReadBuffer); SV(_state.Ppu1OpenBus); SV(_state.Ppu2OpenBus); SV(_state.CgramAddress); SV(_state.MosaicSize); SV(_state.MosaicEnabled);
	SV(_state.OamMode); SV(_state.OamBaseAddress); SV(_state.OamAddressOffset); SV(_state.OamRamAddress); SV(_state.EnableOamPriority);
	SV(_oamWriteBuffer); SV(_timeOver); SV(_rangeOver); SV(_state.HiResMode); SV(_state.ScreenInterlace); SV(_state.ObjInterlace);
	SV(_state.OverscanMode); SV(_state.DirectColorMode); SV(_state.ColorMathClipMode); SV(_state.ColorMathPreventMode); SV(_state.ColorMathAddSubscreen); SV(_state.ColorMathEnabled);
	SV(_state.ColorMathSubtractMode); SV(_state.ColorMathHalveResult); SV(_state.FixedColor); SV(_hvScrollLatchValue); SV(_hScrollLatchValue); 
	SV(_state.MaskLogic[0]); SV(_state.MaskLogic[1]); SV(_state.MaskLogic[2]); SV(_state.MaskLogic[3]); SV(_state.MaskLogic[4]); SV(_state.MaskLogic[5]);
	SV(_state.WindowMaskMain[0]); SV(_state.WindowMaskMain[1]); SV(_state.WindowMaskMain[2]); SV(_state.WindowMaskMain[3]); SV(_state.WindowMaskMain[4]);
	SV(_state.WindowMaskSub[0]); SV(_state.WindowMaskSub[1]); SV(_state.WindowMaskSub[2]); SV(_state.WindowMaskSub[3]); SV(_state.WindowMaskSub[4]);
	SV(_state.Mode7.CenterX); SV(_state.Mode7.CenterY); SV(_state.ExtBgEnabled); SV(_state.Mode7.FillWithTile0); SV(_state.Mode7.HorizontalMirroring);
	SV(_state.Mode7.HScroll); SV(_state.Mode7.LargeMap); SV(_state.Mode7.Matrix[0]); SV(_state.Mode7.Matrix[1]); SV(_state.Mode7.Matrix[2]); SV(_state.Mode7.Matrix[3]);
	SV(_state.Mode7.ValueLatch); SV(_state.Mode7.VerticalMirroring); SV(_state.Mode7.VScroll);
	SV(_state.CgramAddressLatch); SV(_state.CgramWriteBuffer);
	SV(_state.InternalOamAddress);
	SV(_state.InternalCgramAddress);

	for(int i = 0; i < 4; i++) {
		SVI(_state.Layers[i].ChrAddress); SVI(_state.Layers[i].DoubleHeight); SVI(_state.Layers[i].DoubleWidth); SVI(_state.Layers[i].HScroll);
		SVI(_state.Layers[i].LargeTiles); SVI(_state.Layers[i].TilemapAddress); SVI(_state.Layers[i].VScroll);
	}

	for(int i = 0; i < 2; i++) {
		SVI(_state.Window[i].ActiveLayers[0]); SVI(_state.Window[i].ActiveLayers[1]); SVI(_state.Window[i].ActiveLayers[2]); SVI(_state.Window[i].ActiveLayers[3]); SVI(_state.Window[i].ActiveLayers[4]); SVI(_state.Window[i].ActiveLayers[5]);
		SVI(_state.Window[i].InvertedLayers[0]); SVI(_state.Window[i].InvertedLayers[1]); SVI(_state.Window[i].InvertedLayers[2]); SVI(_state.Window[i].InvertedLayers[3]); SVI(_state.Window[i].InvertedLayers[4]); SVI(_state.Window[i].InvertedLayers[5]);
		SVI(_state.Window[i].Left); SVI(_state.Window[i].Right);
	}

	SVArray(_vram, SnesPpu::VideoRamSize >> 1);
	SVArray(_oamRam, SnesPpu::SpriteRamSize);
	SVArray(_cgram, SnesPpu::CgRamSize >> 1);
	
	if(s.GetFormat() != SerializeFormat::Map) {
		//Hide these entries from the Lua API
		SV(_horizontalLocation); SV(_horizontalLocToggle); SV(_verticalLocation); SV(_verticalLocationToggle); SV(_locationLatched);
		SV(_oddFrame); SV(_vblankStartScanline);
		SV(_nmiScanline); SV(_vblankEndScanline); SV(_adjustedVblankEndScanline); SV(_baseVblankEndScanline);
		SV(_overclockEnabled);

		SV(_drawStartX); SV(_drawEndX);
		SV(_mosaicScanlineCounter);

		for(int i = 0; i < 33; i++) {
			SVI(_layerData[0].Tiles[i].ChrData[0]); SVI(_layerData[0].Tiles[i].ChrData[1]); SVI(_layerData[0].Tiles[i].ChrData[2]); SVI(_layerData[0].Tiles[i].ChrData[3]);
			SVI(_layerData[0].Tiles[i].TilemapData); SVI(_layerData[0].Tiles[i].VScroll);
			SVI(_layerData[1].Tiles[i].ChrData[0]); SVI(_layerData[1].Tiles[i].ChrData[1]); SVI(_layerData[1].Tiles[i].ChrData[2]); SVI(_layerData[1].Tiles[i].ChrData[3]);
			SVI(_layerData[1].Tiles[i].TilemapData); SVI(_layerData[1].Tiles[i].VScroll);
			SVI(_layerData[2].Tiles[i].ChrData[0]); SVI(_layerData[2].Tiles[i].ChrData[1]); SVI(_layerData[2].Tiles[i].ChrData[2]); SVI(_layerData[2].Tiles[i].ChrData[3]);
			SVI(_layerData[2].Tiles[i].TilemapData); SVI(_layerData[2].Tiles[i].VScroll);
			SVI(_layerData[3].Tiles[i].ChrData[0]); SVI(_layerData[3].Tiles[i].ChrData[1]); SVI(_layerData[3].Tiles[i].ChrData[2]); SVI(_layerData[3].Tiles[i].ChrData[3]);
			SVI(_layerData[3].Tiles[i].TilemapData); SVI(_layerData[3].Tiles[i].VScroll);
		}
		SV(_hOffset); SV(_vOffset); SV(_fetchBgStart); SV(_fetchBgEnd); SV(_fetchSpriteStart); SV(_fetchSpriteEnd);
	}

	if(!s.IsSaving() && _interlacedFrame && _emu->GetRewindManager()->IsRewinding()) {
		_needFullFrame = true;
	}
}

void SnesPpu::RandomizeState()
{
	_state.ScreenBrightness = _settings->GetRandomValue(0x0F);
	_state.Mode7.CenterX = _settings->GetRandomValue(0xFFFF);
	_state.Mode7.CenterY = _settings->GetRandomValue(0xFFFF);
	_state.Mode7.FillWithTile0 = _settings->GetRandomBool();
	_state.Mode7.HorizontalMirroring = _settings->GetRandomBool();
	_state.Mode7.HScroll = _settings->GetRandomValue(0x1FFF);
	_state.Mode7.HScrollLatch = _settings->GetRandomValue(0x1FFF);
	_state.Mode7.LargeMap = _settings->GetRandomBool();
	_state.Mode7.Matrix[0] = _settings->GetRandomValue(0xFFFF);
	_state.Mode7.Matrix[1] = _settings->GetRandomValue(0xFFFF);
	_state.Mode7.Matrix[2] = _settings->GetRandomValue(0xFFFF);
	_state.Mode7.Matrix[3] = _settings->GetRandomValue(0xFFFF);
	_state.Mode7.ValueLatch = _settings->GetRandomValue(0xFF);
	_state.Mode7.VerticalMirroring = _settings->GetRandomBool();
	_state.Mode7.VScroll = _settings->GetRandomValue(0x1FFF);
	_state.Mode7.VScrollLatch = _settings->GetRandomValue(0x1FFF);

	_state.BgMode = _settings->GetRandomValue(7);
	_state.Mode1Bg3Priority = _settings->GetRandomBool();
	_state.MainScreenLayers = _settings->GetRandomValue(0x1F);
	_state.SubScreenLayers = _settings->GetRandomValue(0x1F);

	for(int i = 0; i < 4; i++) {
		_state.Layers[i].TilemapAddress = _settings->GetRandomValue(0x1F) << 10;
		_state.Layers[i].ChrAddress = _settings->GetRandomValue(0x07) << 12;
		_state.Layers[i].HScroll = _settings->GetRandomValue(0x1FFF);
		_state.Layers[i].VScroll = _settings->GetRandomValue(0x1FFF);
		_state.Layers[i].DoubleWidth = _settings->GetRandomBool();
		_state.Layers[i].DoubleHeight = _settings->GetRandomBool();
		_state.Layers[i].LargeTiles = _settings->GetRandomBool();
	}

	for(int i = 0; i < 2; i++) {
		_state.Window[i].Left = _settings->GetRandomValue(0xFF);
		_state.Window[i].Right = _settings->GetRandomValue(0xFF);
		for(int j = 0; j < 6; j++) {
			_state.Window[i].ActiveLayers[j] = _settings->GetRandomBool();
			_state.Window[i].InvertedLayers[j] = _settings->GetRandomBool();
		}
	}

	for(int i = 0; i < 6; i++) {
		_state.MaskLogic[i] = (WindowMaskLogic)_settings->GetRandomValue(3);
	}

	for(int i = 0; i < 5; i++) {
		_state.WindowMaskMain[i] = _settings->GetRandomBool();
		_state.WindowMaskSub[i] = _settings->GetRandomBool();
	}

	_state.VramAddress = _settings->GetRandomValue(0x7FFF);
	switch(_settings->GetRandomValue(0x03)) {
		case 0: _state.VramIncrementValue = 1; break;
		case 1: _state.VramIncrementValue = 32; break;
		case 2: case 3: _state.VramIncrementValue = 128; break;
	}

	_state.VramAddressRemapping = _settings->GetRandomValue(0x03);
	_state.VramAddrIncrementOnSecondReg = _settings->GetRandomBool();
	_state.VramReadBuffer = _settings->GetRandomValue(0xFFFF);

	_state.Ppu1OpenBus = _settings->GetRandomValue(0xFF);
	_state.Ppu2OpenBus = _settings->GetRandomValue(0xFF);

	_state.CgramAddress = _settings->GetRandomValue(0xFF);
	_state.CgramWriteBuffer = _settings->GetRandomValue(0xFF);
	_state.CgramAddressLatch = _settings->GetRandomBool();

	_state.MosaicSize = _settings->GetRandomValue(0x0F) + 1;
	_state.MosaicEnabled = _settings->GetRandomValue(0x0F);

	_state.OamRamAddress = _settings->GetRandomValue(0x1FF);
	_state.OamMode = _settings->GetRandomValue(0x07);
	_state.OamBaseAddress = _settings->GetRandomValue(0x07) << 13;
	_state.OamAddressOffset = (_settings->GetRandomValue(0x03) + 1) << 12;
	_state.EnableOamPriority = _settings->GetRandomBool();

	_state.ExtBgEnabled = _settings->GetRandomBool();
	_state.HiResMode = _settings->GetRandomBool();
	_state.ScreenInterlace = _settings->GetRandomBool();
	_state.ObjInterlace = _settings->GetRandomBool();
	_state.OverscanMode = _settings->GetRandomBool();
	_state.DirectColorMode = _settings->GetRandomBool();

	_state.ColorMathClipMode = (ColorWindowMode)_settings->GetRandomValue(3);
	_state.ColorMathPreventMode = (ColorWindowMode)_settings->GetRandomValue(3);
	_state.ColorMathAddSubscreen = _settings->GetRandomBool();
	_state.ColorMathEnabled = _settings->GetRandomValue(0x3F);
	_state.ColorMathSubtractMode = _settings->GetRandomBool();
	_state.ColorMathHalveResult = _settings->GetRandomBool();
	_state.FixedColor = _settings->GetRandomValue(0x7FFF);
}

/* Everything below this point is used to select the proper arguments for templates */
template<uint8_t layerIndex, uint8_t bpp, uint8_t normalPriority, uint8_t highPriority, uint16_t basePaletteOffset, bool hiResMode, bool applyMosaic>
void SnesPpu::RenderTilemap()
{
	if(_state.DirectColorMode) {
		RenderTilemap<layerIndex, bpp, normalPriority, highPriority, basePaletteOffset, hiResMode, applyMosaic, true>();
	} else {
		RenderTilemap<layerIndex, bpp, normalPriority, highPriority, basePaletteOffset, hiResMode, applyMosaic, false>();
	}
}

template<uint8_t layerIndex, uint8_t bpp, uint8_t normalPriority, uint8_t highPriority, uint16_t basePaletteOffset, bool hiResMode>
void SnesPpu::RenderTilemap()
{
	bool applyMosaic = ((_state.MosaicEnabled >> layerIndex) & 0x01) != 0 && (_state.MosaicSize > 1 || _state.BgMode == 5 || _state.BgMode == 6);

	if(applyMosaic) {
		RenderTilemap<layerIndex, bpp, normalPriority, highPriority, basePaletteOffset, hiResMode, true>();
	} else {
		RenderTilemap<layerIndex, bpp, normalPriority, highPriority, basePaletteOffset, hiResMode, false>();
	}
}

template<uint8_t layerIndex, uint8_t bpp, uint8_t normalPriority, uint8_t highPriority, uint16_t basePaletteOffset>
void SnesPpu::RenderTilemap()
{
	if(!IsRenderRequired(layerIndex)) {
		return;
	}

	if(_state.BgMode == 5 || _state.BgMode == 6) {
		RenderTilemap<layerIndex, bpp, normalPriority, highPriority, basePaletteOffset, true>();
	} else {
		RenderTilemap<layerIndex, bpp, normalPriority, highPriority, basePaletteOffset, false>();
	}
}

template<uint8_t layerIndex, uint8_t normalPriority, uint8_t highPriority>
void SnesPpu::RenderTilemapMode7()
{
	if(!IsRenderRequired(layerIndex)) {
		return;
	}

	bool applyMosaic = ((_state.MosaicEnabled >> layerIndex) & 0x01) != 0 && _state.MosaicSize > 1;

	if(applyMosaic) {
		RenderTilemapMode7<layerIndex, normalPriority, highPriority, true>();
	} else {
		RenderTilemapMode7<layerIndex, normalPriority, highPriority, false>();
	}
}

template<uint8_t layerIndex, uint8_t normalPriority, uint8_t highPriority, bool applyMosaic>
void SnesPpu::RenderTilemapMode7()
{
	if(_state.DirectColorMode && layerIndex == 0) {
		RenderTilemapMode7<layerIndex, normalPriority, highPriority, applyMosaic, true>();
	} else {
		RenderTilemapMode7<layerIndex, normalPriority, highPriority, applyMosaic, false>();
	}
}
