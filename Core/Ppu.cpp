#include "stdafx.h"
#include "Ppu.h"
#include "Console.h"
#include "MemoryManager.h"
#include "Cpu.h"
#include "Spc.h"
#include "InternalRegisters.h"
#include "EmuSettings.h"
#include "ControlManager.h"
#include "VideoDecoder.h"
#include "VideoRenderer.h"
#include "NotificationManager.h"
#include "DmaController.h"
#include "MessageManager.h"
#include "EventType.h"
#include "RewindManager.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/Serializer.h"

static constexpr uint8_t _oamSizes[8][2][2] = {
	{ { 1, 1 }, { 2, 2 } }, //8x8 + 16x16
	{ { 1, 1 }, { 4, 4 } }, //8x8 + 32x32
	{ { 1, 1 }, { 8, 8 } }, //8x8 + 64x64
	{ { 2, 2 }, { 4, 4 } }, //16x16 + 32x32
	{ { 2, 2 }, { 8, 8 } }, //16x16 + 64x64
	{ { 4, 4 }, { 8, 8 } }, //32x32 + 64x64
	{ { 2, 4 }, { 4, 8 } }, //16x32 + 32x64
	{ { 2, 4 }, { 4, 4 } }  //16x32 + 32x32
};

Ppu::Ppu(Console* console)
{
	_console = console;

	_vram = new uint16_t[Ppu::VideoRamSize >> 1];

	_outputBuffers[0] = new uint16_t[512 * 478];
	_outputBuffers[1] = new uint16_t[512 * 478];
	memset(_outputBuffers[0], 0, 512 * 478 * sizeof(uint16_t));
	memset(_outputBuffers[1], 0, 512 * 478 * sizeof(uint16_t));
}

Ppu::~Ppu()
{
	delete[] _vram;
	delete[] _outputBuffers[0];
	delete[] _outputBuffers[1];
}

void Ppu::PowerOn()
{
	//Boot up PPU in deterministic state (for now, need to randomize this)
	_state = {};
	_state.ForcedVblank = true;

	_skipRender = false;
	_regs = _console->GetInternalRegisters().get();
	_settings = _console->GetSettings().get();
	_spc = _console->GetSpc().get();
	_memoryManager = _console->GetMemoryManager().get();

	_currentBuffer = _outputBuffers[0];

	_state.Layers[0] = {};
	_state.Layers[1] = {};
	_state.Layers[2] = {};
	_state.Layers[3] = {};

	_state.CgramAddress = 0;

	_settings->InitializeRam(_vram, Ppu::VideoRamSize);
	_settings->InitializeRam(_cgram, Ppu::CgRamSize);
	_settings->InitializeRam(_oamRam, Ppu::SpriteRamSize);

	memset(_spriteIndexes, 0xFF, sizeof(_spriteIndexes));

	_state.VramAddress = 0;
	_state.VramIncrementValue = 1;
	_state.VramAddressRemapping = 0;
	_state.VramAddrIncrementOnSecondReg = false;

	UpdateNmiScanline();
}

void Ppu::Reset()
{
	_scanline = 0;
	_state.ForcedVblank = true;
	_oddFrame = 0;
}

uint32_t Ppu::GetFrameCount()
{
	return _frameCount;
}

uint16_t Ppu::GetScanline()
{
	return _scanline;
}

uint16_t Ppu::GetCycle()
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

uint16_t Ppu::GetNmiScanline()
{
	return _nmiScanline;
}

uint16_t Ppu::GetVblankStart()
{
	return _vblankStartScanline;
}

PpuState Ppu::GetState()
{
	PpuState state;
	GetState(state, false);
	return state;
}

void Ppu::GetState(PpuState &state, bool returnPartialState)
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
void Ppu::GetTilemapData(uint8_t layerIndex, uint8_t columnIndex)
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
	if(!hiResMode && config.LargeTiles) {
		//For 16x16 tiles, need to return the same tile for 2 columns 8 pixel columns in a row
		column >>= 1;
	}

	/* The tilemap address to read the tile data from */
	uint16_t addr = baseOffset + (column & 0x1F) + (config.DoubleWidth ? (column & 0x20) << 5 : 0);
	_layerData[layerIndex].Tiles[columnIndex].TilemapData = _vram[addr];
	_layerData[layerIndex].Tiles[columnIndex].VScroll = vScroll;
	_layerData[layerIndex].HasPriorityTiles |= (_vram[addr] & 0x2000) >> 13;
}

template<bool hiResMode, uint8_t bpp, bool secondTile>
void Ppu::GetChrData(uint8_t layerIndex, uint8_t column, uint8_t plane)
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
	if(!hiResMode && config.LargeTiles) {
		//For 16x16 tiles, need to return the 2nd part of the tile every other column
		useSecondTile = (((column << 3) + config.HScroll) & 0x08) == 0x08;
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

void Ppu::GetHorizontalOffsetByte(uint8_t columnIndex)
{
	uint16_t columnOffset = (((columnIndex << 3) + (_state.Layers[2].HScroll & ~0x07)) >> 3) & (_state.Layers[2].DoubleWidth ? 0x3F : 0x1F);
	uint16_t rowOffset = (_state.Layers[2].VScroll >> 3) & (_state.Layers[2].DoubleHeight ? 0x3F : 0x1F);

	_hOffset = _vram[_state.Layers[2].TilemapAddress + columnOffset + (rowOffset << 5)];
}

void Ppu::GetVerticalOffsetByte(uint8_t columnIndex)
{
	uint16_t columnOffset = (((columnIndex << 3) + (_state.Layers[2].HScroll & ~0x07)) >> 3) & (_state.Layers[2].DoubleWidth ? 0x3F : 0x1F);
	uint16_t rowOffset = (_state.Layers[2].VScroll >> 3) & (_state.Layers[2].DoubleHeight ? 0x3F : 0x1F);

	uint16_t tileOffset = columnOffset + (rowOffset << 5);

	//The vertical offset is 0x40 bytes later - but wraps around within the tilemap based on the tilemap size (0x800 or 0x1000 bytes)
	uint16_t vOffsetAddr = _state.Layers[2].TilemapAddress + ((tileOffset + 0x20) & (_state.Layers[2].DoubleHeight ? 0x7FF : 0x3FF));

	_vOffset = _vram[vOffsetAddr];
}

void Ppu::FetchTileData()
{
	if(_state.ForcedVblank) {
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

bool Ppu::ProcessEndOfScanline(uint16_t hClock)
{
	if(hClock >= 1364 || (hClock == 1360 && _scanline == 240 && _oddFrame && !_state.ScreenInterlace)) {
		//"In non-interlace mode scanline 240 of every other frame (those with $213f.7=1) is only 1360 cycles."
		if(_scanline < _vblankStartScanline) {
			RenderScanline();

			if(_scanline == 0) {
				_mosaicScanlineCounter = _state.MosaicEnabled ? _state.MosaicSize + 1 : 0;
				if(!_skipRender) {
					if(!_state.ScreenInterlace) {
						_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
					}
					
					//If we're not skipping this frame, reset the high resolution flag
					_useHighResOutput = false;
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
			for(int i = 0; i < 4; i++) {
				_layerData[i].HasPriorityTiles = false;
			}

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

			_pixelsDrawn = 0;
			_subPixelsDrawn = 0;
			memset(_rowPixelFlags, 0, sizeof(_rowPixelFlags));
			memset(_subScreenFilled, 0, sizeof(_subScreenFilled));
		}

		_scanline++;

		if(_scanline == _nmiScanline) {
			ProcessLocationLatchRequest();
			_latchRequest = false;

			//Reset OAM address at the start of vblank?
			if(!_state.ForcedVblank) {
				//TODO, the timing of this may be slightly off? should happen at H=10 based on anomie's docs
				_internalOamAddress = (_state.OamRamAddress << 1);
			}

			VideoConfig cfg = _settings->GetVideoConfig();
			_configVisibleLayers = (cfg.HideBgLayer0 ? 0 : 1) | (cfg.HideBgLayer1 ? 0 : 2) | (cfg.HideBgLayer2 ? 0 : 4) | (cfg.HideBgLayer3 ? 0 : 8) | (cfg.HideSprites ? 0 : 16);

			_console->ProcessEvent(EventType::EndFrame);

			_frameCount++;
			_spc->ProcessEndFrame();
			_regs->SetNmiFlag(true);
			SendFrame();

			if(_regs->IsNmiEnabled()) {
				_console->GetCpu()->SetNmiFlag();
			}
		} else if(_scanline >= _vblankEndScanline + 1) {
			//"Frames are 262 scanlines in non-interlace mode, while in interlace mode frames with $213f.7=0 are 263 scanlines"
			_oddFrame ^= 1;
			_regs->SetNmiFlag(false);
			_scanline = 0;
			_rangeOver = false;
			_timeOver = false;
			_console->ProcessEvent(EventType::StartFrame);

			_skipRender = (
				!_settings->GetVideoConfig().DisableFrameSkipping &&
				!_console->GetRewindManager()->IsRewinding() &&
				!_console->GetVideoRenderer()->IsRecording() &&
				(_settings->GetEmulationSpeed() == 0 || _settings->GetEmulationSpeed() > 150) &&
				_frameSkipTimer.GetElapsedMS() < 10
			);

			//Update overclock timings once per frame
			UpdateNmiScanline();

			//Ensure the SPC is re-enabled for the next frame
			_spc->SetSpcState(true);
		}

		UpdateSpcState();
		return true;
	}
	return false;
}

void Ppu::UpdateSpcState()
{
	//When using overclocking, turn off the SPC during the extra scanlines
	if(_overclockEnabled && _scanline > _vblankStartScanline) {
		if(_scanline > _adjustedVblankEndScanline) {
			//Disable APU for extra lines after NMI
			_spc->SetSpcState(false);
		} else if(_scanline >= _vblankStartScanline && _scanline < _nmiScanline) {
			//Disable APU for extra lines before NMI
			_spc->SetSpcState(false);
		} else {
			_spc->SetSpcState(true);
		}
	}
}

void Ppu::UpdateNmiScanline()
{
	EmulationConfig cfg = _settings->GetEmulationConfig();
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

	_overclockEnabled = cfg.PpuExtraScanlinesBeforeNmi > 0 || cfg.PpuExtraScanlinesAfterNmi > 0;

	_adjustedVblankEndScanline = _baseVblankEndScanline + cfg.PpuExtraScanlinesBeforeNmi;
	_vblankEndScanline = _baseVblankEndScanline + cfg.PpuExtraScanlinesAfterNmi + cfg.PpuExtraScanlinesBeforeNmi;
	_vblankStartScanline = _state.OverscanMode ? 240 : 225;
	_nmiScanline = _vblankStartScanline + cfg.PpuExtraScanlinesBeforeNmi;
}

uint16_t Ppu::GetRealScanline()
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

uint16_t Ppu::GetVblankEndScanline()
{
	return _vblankEndScanline;
}

uint16_t Ppu::GetLastScanline()
{
	return _baseVblankEndScanline;
}

void Ppu::EvaluateNextLineSprites()
{
	if(_spriteEvalStart == 0) {
		_spriteCount = 0;
		_oamEvaluationIndex = _state.EnableOamPriority ? ((_internalOamAddress & 0x1FC) >> 2) : 0;
	}

	if(_state.ForcedVblank) {
		return;
	}

	for(int i = _spriteEvalStart; i <= _spriteEvalEnd; i++) {
		if(!(i & 0x01)) {
			//First cycle, read X & Y and high oam byte
			FetchSpritePosition(_oamEvaluationIndex << 2);
		} else {
			//Second cycle: Check if sprite is in range, if so, keep its index
			if(_currentSprite.IsVisible(_scanline)) {
				if(_spriteCount < 32) {
					_spriteIndexes[_spriteCount] = _oamEvaluationIndex;
					_spriteCount++;
				} else {
					_rangeOver = true;
				}
			}
			_oamEvaluationIndex = (_oamEvaluationIndex + 1) & 0x7F;
		}
	}
}

void Ppu::FetchSpriteData()
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
			if(!_state.ForcedVblank) {
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
				FetchSpritePosition(_oamTimeIndex << 2);
			}
		}
	}
}

void Ppu::FetchSpritePosition(uint16_t oamAddress)
{
	uint8_t highTableOffset = oamAddress >> 4;
	uint8_t shift = ((oamAddress >> 1) & 0x06);
	uint8_t highTableValue = _oamRam[0x200 | highTableOffset] >> shift;
	uint8_t largeSprite = (highTableValue & 0x02) >> 1;

	uint16_t oamValue = _oamRam[oamAddress] | (_oamRam[oamAddress + 1] << 8);
	uint16_t sign = (highTableValue & 0x01) << 8;

	uint8_t spriteIndex = oamAddress >> 2;
	_currentSprite.X = (int16_t)((sign | (oamValue & 0xFF)) << 7) >> 7;
	_currentSprite.Y = (oamValue >> 8);
	_currentSprite.Width = _oamSizes[_state.OamMode][largeSprite][0] << 3;
	
	if(spriteIndex != _currentSprite.Index) {
		_currentSprite.Index = oamAddress >> 2;
		_currentSprite.ColumnOffset = (_currentSprite.Width / 8);
		if(_currentSprite.X <= -8 && _currentSprite.X != -256) {
			//Skip the first tiles of the sprite (because the tiles are hidden to the left of the screen)
			_currentSprite.ColumnOffset += _currentSprite.X / 8;
		}
	}

	uint8_t height = _oamSizes[_state.OamMode][largeSprite][1] << 3;
	if(_state.ObjInterlace) {
		height /= 2;
	}
	_currentSprite.Height = height;
}

void Ppu::FetchSpriteAttributes(uint16_t oamAddress)
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
		yOffset = (_currentSprite.Height - 1 - yGap) & 0x07;
		rowOffset = (_currentSprite.Height - 1 - yGap) >> 3;
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
	uint16_t tileStart = (_state.OamBaseAddress + (tileIndex << 4) + (useSecondTable ? _state.OamAddressOffset : 0)) & 0x7FFF;
	_currentSprite.FetchAddress = tileStart + yOffset;

	int16_t x = _currentSprite.X == -256 ? 0 : _currentSprite.X;
	int16_t endTileX = x + ((columnCount - _currentSprite.ColumnOffset - 1) << 3) + 8;
	_currentSprite.DrawX = _currentSprite.X + ((columnCount - _currentSprite.ColumnOffset - 1) << 3);

	if(_currentSprite.ColumnOffset == 0 || endTileX >= 256) {
		//Last tile of the sprite, or skip the remaining tiles (because the tiles are hidden to the right of the screen)
		_spriteCount--;
		_currentSprite.ColumnOffset = 0;
	}
}

void Ppu::FetchSpriteTile(bool secondCycle)
{
	//The timing for the fetches should be (mostly) accurate (H=272 to 339)
	uint16_t chrData = _vram[_currentSprite.FetchAddress];
	_currentSprite.ChrData[secondCycle] = chrData;

	if(!secondCycle) {
		_currentSprite.FetchAddress += 8;
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

void Ppu::RenderMode0()
{
	RenderSprites<3>();
	RenderTilemap<0, 2, true>();
	RenderTilemap<1, 2, true, 32>();
	RenderSprites<2>();
	RenderTilemap<0, 2, false>();
	RenderTilemap<1, 2, false, 32>();
	RenderSprites<1>();
	RenderTilemap<2, 2, true, 64>();
	RenderTilemap<3, 2, true, 96>();
	RenderSprites<0>();
	RenderTilemap<2, 2, false, 64>();
	RenderTilemap<3, 2, false, 96>();
	RenderBgColor();
}

void Ppu::RenderMode1()
{
	if(_state.Mode1Bg3Priority) {
		RenderTilemap<2, 2, true>();
	}
	RenderSprites<3>();
	RenderTilemap<0, 4, true>();
	RenderTilemap<1, 4, true>();
	RenderSprites<2>();
	RenderTilemap<0, 4, false>();
	RenderTilemap<1, 4, false>();
	RenderSprites<1>();
	if(!_state.Mode1Bg3Priority) {
		RenderTilemap<2, 2, true>();
	}
	RenderSprites<0>();
	RenderTilemap<2, 2, false>();
	RenderBgColor();
}

void Ppu::RenderMode2()
{
	RenderSprites<3>();
	RenderTilemap<0, 4, true>();
	RenderSprites<2>();
	RenderTilemap<1, 4, true>();
	RenderSprites<1>();
	RenderTilemap<0, 4, false>();
	RenderSprites<0>();
	RenderTilemap<1, 4, false>();
	RenderBgColor();
}

void Ppu::RenderMode3()
{
	RenderSprites<3>();
	RenderTilemap<0, 8, true>();
	RenderSprites<2>();
	RenderTilemap<1, 4, true>();
	RenderSprites<1>();
	RenderTilemap<0, 8, false>();
	RenderSprites<0>();
	RenderTilemap<1, 4, false>();
	RenderBgColor();
}

void Ppu::RenderMode4()
{
	RenderSprites<3>();
	RenderTilemap<0, 8, true>();
	RenderSprites<2>();
	RenderTilemap<1, 2, true>();
	RenderSprites<1>();
	RenderTilemap<0, 8, false>();
	RenderSprites<0>();
	RenderTilemap<1, 2, false>();
	RenderBgColor();
}

void Ppu::RenderMode5()
{
	RenderSprites<3>();
	RenderTilemap<0, 4, true>();
	RenderSprites<2>();
	RenderTilemap<1, 2, true>();
	RenderSprites<1>();
	RenderTilemap<0, 4, false>();
	RenderSprites<0>();
	RenderTilemap<1, 2, false>();
	RenderBgColor();
}

void Ppu::RenderMode6()
{
	RenderSprites<3>();
	RenderTilemap<0, 4, true>();
	RenderSprites<2>();
	RenderSprites<1>();
	RenderTilemap<0, 4, false>();
	RenderSprites<0>();
	RenderBgColor();
}

void Ppu::RenderMode7()
{
	RenderSprites<3>();
	RenderSprites<2>();
	if(_state.ExtBgEnabled) {
		RenderTilemapMode7<1, true>();
	}
	RenderSprites<1>();
	RenderTilemapMode7<0, false>();
	RenderSprites<0>();
	if(_state.ExtBgEnabled) {
		RenderTilemapMode7<1, false>();
	}
	RenderBgColor();
}

void Ppu::RenderScanline()
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

		uint8_t bgMode = _state.BgMode;
		if(_state.ForcedVblank) {
			bgMode = 8;
		}

		switch(bgMode) {
			case 0: RenderMode0(); break;
			case 1: RenderMode1(); break;
			case 2: RenderMode2(); break;
			case 3: RenderMode3(); break;
			case 4: RenderMode4(); break;
			case 5: RenderMode5(); break;
			case 6: RenderMode6(); break;
			case 7: RenderMode7(); break;

			case 8:
				//Forced blank, output black
				memset(_mainScreenBuffer + _drawStartX, 0, (_drawEndX - _drawStartX + 1) * 2);
				memset(_subScreenBuffer + _drawStartX, 0, (_drawEndX - _drawStartX + 1) * 2);
				break;
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

void Ppu::RenderBgColor()
{
	if(_pixelsDrawn == 256 && _subPixelsDrawn == 256) {
		return;
	}

	for(int x = _drawStartX; x <= _drawEndX; x++) {
		if(!_rowPixelFlags[x]) {
			uint8_t pixelFlags = PixelFlags::Filled | ((_state.ColorMathEnabled & 0x20) ? PixelFlags::AllowColorMath : 0);
			_mainScreenBuffer[x] = _cgram[0];
			_rowPixelFlags[x] = pixelFlags;
		}
		if(!_subScreenFilled[x]) {
			_subScreenBuffer[x] = _cgram[0];
		}
	}
}

template<uint8_t priority>
void Ppu::RenderSprites()
{
	if(!_hasSpritePriority[priority] || !IsRenderRequired(Ppu::SpriteLayerIndex)) {
		return;
	}

	bool drawMain = (bool)(((_state.MainScreenLayers & _configVisibleLayers) >> Ppu::SpriteLayerIndex) & 0x01);
	bool drawSub = (bool)(((_state.SubScreenLayers & _configVisibleLayers) >> Ppu::SpriteLayerIndex) & 0x01);

	uint8_t mainWindowCount = 0;
	uint8_t subWindowCount = 0;
	if(_state.WindowMaskMain[Ppu::SpriteLayerIndex]) {
		mainWindowCount = (uint8_t)_state.Window[0].ActiveLayers[Ppu::SpriteLayerIndex] + (uint8_t)_state.Window[1].ActiveLayers[Ppu::SpriteLayerIndex];
	}
	if(_state.WindowMaskSub[Ppu::SpriteLayerIndex]) {
		subWindowCount = (uint8_t)_state.Window[0].ActiveLayers[Ppu::SpriteLayerIndex] + (uint8_t)_state.Window[1].ActiveLayers[Ppu::SpriteLayerIndex];
	}

	for(int x = _drawStartX; x <= _drawEndX; x++) {
		if(_spritePriority[x] == priority) {
			if(drawMain && !_rowPixelFlags[x] && !ProcessMaskWindow<Ppu::SpriteLayerIndex>(mainWindowCount, x)) {
				uint16_t paletteRamOffset = 128 + (_spritePalette[x] << 4) + _spriteColors[x];
				_mainScreenBuffer[x] = _cgram[paletteRamOffset];
				_rowPixelFlags[x] = PixelFlags::Filled | (((_state.ColorMathEnabled & 0x10) && _spritePalette[x] > 3) ? PixelFlags::AllowColorMath : 0);
				_pixelsDrawn++;
			}

			if(drawSub && !_subScreenFilled[x] && !ProcessMaskWindow<Ppu::SpriteLayerIndex>(subWindowCount, x)) {
				uint16_t paletteRamOffset = 128 + (_spritePalette[x] << 4) + _spriteColors[x];
				_subScreenBuffer[x] = _cgram[paletteRamOffset];
				_subScreenFilled[x] = true;
				_subPixelsDrawn++;
			}
		}
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, uint16_t basePaletteOffset, bool hiResMode, bool applyMosaic, bool directColorMode>
void Ppu::RenderTilemap()
{
	bool drawMain = (bool)(((_state.MainScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01);
	bool drawSub = (bool)(((_state.SubScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01);

	uint8_t mainWindowCount = _state.WindowMaskMain[layerIndex] ? (uint8_t)_state.Window[0].ActiveLayers[layerIndex] + (uint8_t)_state.Window[1].ActiveLayers[layerIndex] : 0;
	uint8_t subWindowCount = _state.WindowMaskSub[layerIndex] ? (uint8_t)_state.Window[0].ActiveLayers[layerIndex] + (uint8_t)_state.Window[1].ActiveLayers[layerIndex] : 0;

	uint16_t hScrollOriginal = _state.Layers[layerIndex].HScroll;
	uint16_t hScroll = hiResMode ? (hScrollOriginal << 1) : hScrollOriginal;

	TileData* tileData  = _layerData[layerIndex].Tiles;

	uint8_t mosaicCounter = applyMosaic ? _state.MosaicSize - (_drawStartX % _state.MosaicSize) : 0;

	uint8_t lookupIndex;
	uint8_t chrDataOffset;
	uint8_t hiresSubColor;
	uint8_t pixelFlags = PixelFlags::Filled | (((_state.ColorMathEnabled >> layerIndex) & 0x01) ? PixelFlags::AllowColorMath : 0);

	for(int x = _drawStartX; x <= _drawEndX; x++) {
		if(hiResMode) {
			lookupIndex = (x + (hScrollOriginal & 0x07)) >> 2;
			chrDataOffset = (lookupIndex & 0x01) * bpp / 2;
			lookupIndex >>= 1;
		} else {
			lookupIndex = (x + (hScrollOriginal & 0x07)) >> 3;
		}

		uint16_t tilemapData = tileData[lookupIndex].TilemapData;
		if((uint8_t)processHighPriority != ((tilemapData & 0x2000) >> 13)) {
			continue;
		}

		uint16_t* chrData = tileData[lookupIndex].ChrData;
		bool hMirror = (tilemapData & 0x4000) != 0;

		uint8_t color;
		uint16_t rgbColor;

		if(hiResMode) {
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

		if(applyMosaic) {
			if(mosaicCounter == _state.MosaicSize) {
				mosaicCounter = 1;
				if(hiResMode) {
					color = hiresSubColor;
				}
				_mosaicColor[layerIndex] = (paletteIndex << 8) | color;
			} else {
				mosaicCounter++;
				color = _mosaicColor[layerIndex] & 0xFF;
				paletteIndex = _mosaicColor[layerIndex] >> 8;
				if(hiResMode) {
					hiresSubColor = color;
				}
			}			
		}

		if(color > 0) {
			rgbColor = GetRgbColor<bpp, directColorMode, basePaletteOffset>(paletteIndex, color);
			if(drawMain && !_rowPixelFlags[x] && !ProcessMaskWindow<layerIndex>(mainWindowCount, x)) {
				/* Keeps track of whether or not the pixel is allowed to participate in color math */
				DrawMainPixel(x, rgbColor, pixelFlags);
			}
			if(!hiResMode && drawSub && !_subScreenFilled[x] && !ProcessMaskWindow<layerIndex>(subWindowCount, x)) {
				DrawSubPixel(x, rgbColor);
			}
		}

		if(hiResMode) {
			if(hiresSubColor > 0 && drawSub && !_subScreenFilled[x] && !ProcessMaskWindow<layerIndex>(subWindowCount, x)) {
				uint16_t hiresSubRgbColor = GetRgbColor<bpp, directColorMode, basePaletteOffset>(paletteIndex, hiresSubColor);
				DrawSubPixel(x, hiresSubRgbColor);
			}
		}
	}
}

template<uint8_t bpp, bool directColorMode, uint8_t basePaletteOffset>
uint16_t Ppu::GetRgbColor(uint8_t paletteIndex, uint8_t colorIndex)
{
	if(bpp == 8 && directColorMode) {
		return (
			((((colorIndex & 0x07) << 1) | (paletteIndex & 0x01)) << 1) |
			(((colorIndex & 0x38) | ((paletteIndex & 0x02) << 1)) << 4) |
			(((colorIndex & 0xC0) | ((paletteIndex & 0x04) << 3)) << 7)
		);
	} else if(bpp == 8) {
		//Ignore palette bits for 256-color layers
		return _cgram[basePaletteOffset + colorIndex];
	} else {
		return _cgram[basePaletteOffset + paletteIndex * (1 << bpp) + colorIndex];
	}
}

bool Ppu::IsRenderRequired(uint8_t layerIndex)
{
	if(_pixelsDrawn == 256 && _subPixelsDrawn == 256) {
		return false;
	}

	if(((_state.MainScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01) {
		return true;
	}
	if(((_state.SubScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01) {
		return true;
	}

	return false;
}

template<uint8_t bpp>
uint8_t Ppu::GetTilePixelColor(const uint16_t chrData[4], const uint8_t shift)
{
	uint8_t color;
	if(bpp == 2) {
		color = (chrData[0] >> shift) & 0x01;
		color |= (chrData[0] >> (7 + shift)) & 0x02;
	} else if(bpp == 4) {
		color = (chrData[0] >> shift) & 0x01;
		color |= (chrData[0] >> (7 + shift)) & 0x02;
		color |= ((chrData[1] >> shift) & 0x01) << 2;
		color |= ((chrData[1] >> (7 + shift)) & 0x02) << 2;
	} else if(bpp == 8) {
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

template<uint8_t layerIndex, bool processHighPriority, bool applyMosaic, bool directColorMode>
void Ppu::RenderTilemapMode7()
{
	if(!IsRenderRequired(layerIndex)) {
		return;
	}

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
	uint8_t mosaicCounter = applyMosaic ? _state.MosaicSize - (_drawStartX % _state.MosaicSize) : 0;

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

	xValue += xStep * _drawStartX;
	yValue += yStep * _drawStartX;
	
	uint8_t pixelFlags = PixelFlags::Filled | (((_state.ColorMathEnabled >> layerIndex) & 0x01) ? PixelFlags::AllowColorMath : 0);

	for(int x = _drawStartX; x <= _drawEndX; x++) {
		int32_t xOffset = xValue >> 8;
		int32_t yOffset = yValue >> 8;
		xValue += xStep;
		yValue += yStep;
		
		if(_rowPixelFlags[x] && _subScreenFilled[x]) {
			continue;
		}
		
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
		if(layerIndex == 1) {
			uint8_t color = _vram[((tileIndex << 6) + ((yOffset & 0x07) << 3) + (xOffset & 0x07))] >> 8;
			if(((uint8_t)processHighPriority << 7) != (color & 0x80)) {
				//Wrong priority, skip this pixel
				continue;
			}
			colorIndex = (color & 0x7F);
		} else {
			colorIndex = _vram[((tileIndex << 6) + ((yOffset & 0x07) << 3) + (xOffset & 0x07))] >> 8;
		}

		if(applyMosaic) {
			if(mosaicCounter == _state.MosaicSize) {
				mosaicCounter = 1;
				_mosaicColor[layerIndex] = colorIndex;
			} else {
				mosaicCounter++;
				colorIndex = _mosaicColor[layerIndex];
			}
		}

		if(colorIndex > 0) {
			uint16_t paletteColor;
			if(directColorMode) {
				paletteColor = ((colorIndex & 0x07) << 2) | ((colorIndex & 0x38) << 4) | ((colorIndex & 0xC0) << 7);
			} else {
				paletteColor = _cgram[colorIndex];
			}
			
			if(drawMain && !_rowPixelFlags[x] && !ProcessMaskWindow<layerIndex>(mainWindowCount, x)) {
				DrawMainPixel(x, paletteColor, pixelFlags);
			} 

			if(drawSub && !_subScreenFilled[x] && !ProcessMaskWindow<layerIndex>(subWindowCount, x)) {
				DrawSubPixel(x, paletteColor);
			}
		}
	}
}

void Ppu::DrawMainPixel(uint8_t x, uint16_t color, uint8_t flags)
{
	_mainScreenBuffer[x] = color;
	_rowPixelFlags[x] = flags;
	_pixelsDrawn++;
}

void Ppu::DrawSubPixel(uint8_t x, uint16_t color)
{
	_subScreenBuffer[x] = color;
	_subScreenFilled[x] = true;
	_subPixelsDrawn++;
}

void Ppu::ApplyColorMath()
{
	uint8_t activeWindowCount = (uint8_t)_state.Window[0].ActiveLayers[Ppu::ColorWindowIndex] + (uint8_t)_state.Window[1].ActiveLayers[Ppu::ColorWindowIndex];
	bool hiResMode = _state.HiResMode || _state.BgMode == 5 || _state.BgMode == 6;
	uint16_t prevMainPixel = 0;
	int prevX = _drawStartX > 0 ? _drawStartX - 1 : 0;

	for(int x = _drawStartX; x <= _drawEndX; x++) {
		bool isInsideWindow = ProcessMaskWindow<Ppu::ColorWindowIndex>(activeWindowCount, x);

		uint16_t subPixel = _subScreenBuffer[x];
		if(hiResMode) {
			//Apply the color math based on the previous main pixel
			ApplyColorMathToPixel(_subScreenBuffer[x], prevMainPixel, prevX, isInsideWindow);
			prevMainPixel = _mainScreenBuffer[x];
			prevX = x;
		}
		ApplyColorMathToPixel(_mainScreenBuffer[x], subPixel, x, isInsideWindow);
	}
}

void Ppu::ApplyColorMathToPixel(uint16_t &pixelA, uint16_t pixelB, int x, bool isInsideWindow)
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

	if(!(_rowPixelFlags[x] & PixelFlags::AllowColorMath)) {
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
		if(_subScreenFilled[x]) {
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
	if(_state.ColorMathSubstractMode) {
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
void Ppu::ApplyBrightness()
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

void Ppu::ConvertToHiRes()
{
	uint16_t scanline = _state.OverscanMode ? (_scanline - 1) : (_scanline + 6);

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

void Ppu::ApplyHiResMode()
{
	//When overscan mode is off, center the 224-line picture in the center of the 239-line output buffer
	uint16_t scanline = _state.OverscanMode ? (_scanline - 1) : (_scanline + 6);

	bool useHighResOutput = _useHighResOutput || IsDoubleWidth() || _state.ScreenInterlace;
	if(_useHighResOutput != useHighResOutput) {
		//Convert standard res picture to high resolution when the PPU starts drawing in high res mid frame
		ConvertToHiRes();
		_useHighResOutput = useHighResOutput;
	}

	if(!_useHighResOutput) {
		memcpy(_currentBuffer + (scanline << 8) + _drawStartX, _mainScreenBuffer + _drawStartX, (_drawEndX - _drawStartX + 1) << 1);
	} else {
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
bool Ppu::ProcessMaskWindow(uint8_t activeWindowCount, int x)
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

void Ppu::ProcessWindowMaskSettings(uint8_t value, uint8_t offset)
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

void Ppu::SendFrame()
{
	uint16_t width = _useHighResOutput ? 512 : 256;
	uint16_t height = _useHighResOutput ? 478 : 239;

	if(!_state.OverscanMode) {
		//Clear the top 7 and bottom 8 rows
		int top = (_useHighResOutput ? 14 : 7);
		int bottom = (_useHighResOutput ? 16 : 8);
		memset(_currentBuffer, 0, width * top * sizeof(uint16_t));
		memset(_currentBuffer + width * (height - bottom), 0, width * bottom * sizeof(uint16_t));
	}

	_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);

	bool isRewinding = _console->GetRewindManager()->IsRewinding();
#ifdef LIBRETRO
	_console->GetVideoDecoder()->UpdateFrameSync(_currentBuffer, width, height, _frameCount, isRewinding);
#else
	if(isRewinding || _state.ScreenInterlace) {
		_console->GetVideoDecoder()->UpdateFrameSync(_currentBuffer, width, height, _frameCount, isRewinding);
	} else {
		_console->GetVideoDecoder()->UpdateFrame(_currentBuffer, width, height, _frameCount);
	}

	if(!_skipRender) {
		_frameSkipTimer.Reset();
	}
#endif
}


bool Ppu::IsHighResOutput()
{
	return _useHighResOutput;
}

uint16_t* Ppu::GetScreenBuffer()
{
	return _currentBuffer;
}

uint16_t* Ppu::GetPreviousScreenBuffer()
{
	return _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
}

uint8_t* Ppu::GetVideoRam()
{
	return (uint8_t*)_vram;
}

uint8_t* Ppu::GetCgRam()
{
	return (uint8_t*)_cgram;
}

uint8_t* Ppu::GetSpriteRam()
{
	return (uint8_t*)_oamRam;
}

bool Ppu::IsDoubleHeight()
{
	return _state.ScreenInterlace && (_state.BgMode == 5 || _state.BgMode == 6);
}

bool Ppu::IsDoubleWidth()
{
	return _state.HiResMode || _state.BgMode == 5 || _state.BgMode == 6;
}

void Ppu::SetLocationLatchRequest(uint16_t x, uint16_t y)
{
	//Used by super scope
	_latchRequest = true;
	_latchRequestX = x;
	_latchRequestY = y;
}

void Ppu::ProcessLocationLatchRequest()
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

void Ppu::LatchLocationValues()
{
	_horizontalLocation = GetCycle();
	_verticalLocation = GetRealScanline();
	_locationLatched = true;
}

void Ppu::UpdateOamAddress()
{
	_internalOamAddress = (_state.OamRamAddress << 1);
}

uint16_t Ppu::GetOamAddress()
{
	if(_state.ForcedVblank || _scanline >= _vblankStartScanline) {
		return _internalOamAddress;
	} else {
		if(_memoryManager->GetHClock() <= 255 * 4) {
			return _oamEvaluationIndex << 2;
		} else {
			return _oamTimeIndex << 2;
		}
	}
}

void Ppu::UpdateVramReadBuffer()
{
	uint16_t addr = GetVramAddress();
	_state.VramReadBuffer = _vram[addr];
}

uint16_t Ppu::GetVramAddress()
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

uint8_t Ppu::Read(uint16_t addr)
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
				_console->ProcessPpuRead(oamAddr, value, SnesMemoryType::SpriteRam);
			} else {
				value = _oamRam[0x200 | (oamAddr & 0x1F)];
				_console->ProcessPpuRead(0x200 | (oamAddr & 0x1F), value, SnesMemoryType::SpriteRam);
			}
			
			_internalOamAddress = (_internalOamAddress + 1) & 0x3FF;
			_state.Ppu1OpenBus = value;
			return value;
		}

		case 0x2139: {
			//VMDATALREAD - VRAM Data Read low byte
			uint8_t returnValue = (uint8_t)_state.VramReadBuffer;
			_console->ProcessPpuRead(GetVramAddress(), returnValue, SnesMemoryType::VideoRam);
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
			_console->ProcessPpuRead(GetVramAddress() + 1, returnValue, SnesMemoryType::VideoRam);
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
			if(_state.CgramAddressLatch){
				value = ((_cgram[_state.CgramAddress] >> 8) & 0x7F) | (_state.Ppu2OpenBus & 0x80);
				_state.CgramAddress++;
				
				_console->ProcessPpuRead((_state.CgramAddress >> 1) + 1, value, SnesMemoryType::CGRam);
			} else {
				value = (uint8_t)_cgram[_state.CgramAddress];
				_console->ProcessPpuRead(_state.CgramAddress >> 1, value, SnesMemoryType::CGRam);
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

				//"The high/low selector is reset to elowf when $213F is read" (the selector is NOT reset when the counter is latched)
				_horizontalLocToggle = false;
				_verticalLocationToggle = false;
			}
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

void Ppu::Write(uint32_t addr, uint8_t value)
{
	if(_scanline < _vblankStartScanline) {
		RenderScanline();
	}

	switch(addr) {
		case 0x2100:
			if(_state.ForcedVblank && _scanline == _nmiScanline) {
				//"writing this register on the first line of V-Blank (225 or 240, depending on overscan) when force blank is currently active causes the OAM Address Reset to occur."
				UpdateOamAddress();
			}

			_state.ForcedVblank = (value & 0x80) != 0;
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
					_console->ProcessPpuWrite(oamAddr - 1, _oamWriteBuffer, SnesMemoryType::SpriteRam);
					_oamRam[oamAddr - 1] = _oamWriteBuffer;
	
					_console->ProcessPpuWrite(oamAddr, value, SnesMemoryType::SpriteRam);
					_oamRam[oamAddr] = value;
				} else {
					_oamWriteBuffer = value;
				}
			} 

			if(!_state.ForcedVblank && _scanline < _nmiScanline) {
				//During rendering the high table is also written to when writing to OAM
				oamAddr = 0x200 | ((oamAddr & 0x1F0) >> 4);
			}
			
			if(oamAddr >= 512) {
				uint16_t address = 0x200 | (oamAddr & 0x1F);
				if((oamAddr & 0x01) == 0) {
					_oamWriteBuffer = value;
				}
				_console->ProcessPpuWrite(address, value, SnesMemoryType::SpriteRam);
				_oamRam[address] = value;
			}
			_internalOamAddress = (_internalOamAddress + 1) & 0x3FF;
			break;
		}

		case 0x2105:
			if(_state.BgMode != (value & 0x07)) {
				LogDebug("[Debug] Entering mode: " + std::to_string(value & 0x07) + " (SL: " + std::to_string(_scanline) + ")");
			}
			_state.BgMode = value & 0x07;
			_state.Mode1Bg3Priority = (value & 0x08) != 0;

			_state.Layers[0].LargeTiles = (value & 0x10) != 0;
			_state.Layers[1].LargeTiles = (value & 0x20) != 0;
			_state.Layers[2].LargeTiles = (value & 0x40) != 0;
			_state.Layers[3].LargeTiles = (value & 0x80) != 0;
			break;

		case 0x2106: {
			//MOSAIC - Screen Pixelation
			_state.MosaicSize = ((value & 0xF0) >> 4) + 1;
			_state.MosaicEnabled = value & 0x0F;
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
			if(_scanline >= _nmiScanline || _state.ForcedVblank) {
				//Only write the value if in vblank or forced blank (writes to VRAM outside vblank/forced blank are not allowed)
				_console->ProcessPpuWrite(GetVramAddress() << 1, value, SnesMemoryType::VideoRam);
				_vram[GetVramAddress()] = value | (_vram[GetVramAddress()] & 0xFF00);
			}

			//The VRAM address is incremented even outside of vblank/forced blank
			if(!_state.VramAddrIncrementOnSecondReg) {
				_state.VramAddress = (_state.VramAddress + _state.VramIncrementValue) & 0x7FFF;
			}
			break;

		case 0x2119:
			//VMDATAH - VRAM Data Write high byte
			if(_scanline >= _nmiScanline || _state.ForcedVblank) {
				//Only write the value if in vblank or forced blank (writes to VRAM outside vblank/forced blank are not allowed)
				_console->ProcessPpuWrite((GetVramAddress() << 1) + 1, value, SnesMemoryType::VideoRam);
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
				_console->ProcessPpuWrite(_state.CgramAddress >> 1, _state.CgramWriteBuffer, SnesMemoryType::CGRam);
				_console->ProcessPpuWrite((_state.CgramAddress >> 1) + 1, value & 0x7F, SnesMemoryType::CGRam);

				_cgram[_state.CgramAddress] = _state.CgramWriteBuffer | ((value & 0x7F) << 8);
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
			_state.ColorMathSubstractMode = (value & 0x80) != 0;
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

		case 0x2133:
			//SETINI - Screen Mode/Video Select
			//_externalSync = (value & 0x80) != 0;  //NOT USED
			_state.ExtBgEnabled = (value & 0x40) != 0;
			_state.HiResMode = (value & 0x08) != 0;
			_state.OverscanMode = (value & 0x04) != 0;
			_state.ObjInterlace = (value & 0x02) != 0;
			_state.ScreenInterlace = (value & 0x01) != 0;
			UpdateNmiScanline();
			break;

		default:
			LogDebug("[Debug] Unimplemented register write: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

void Ppu::Serialize(Serializer &s)
{
	uint16_t unused_oamRenderAddress = 0;
	s.Stream(
		_state.ForcedVblank, _state.ScreenBrightness, _scanline, _frameCount, _drawStartX, _drawEndX, _state.BgMode,
		_state.Mode1Bg3Priority, _state.MainScreenLayers, _state.SubScreenLayers, _state.VramAddress, _state.VramIncrementValue, _state.VramAddressRemapping,
		_state.VramAddrIncrementOnSecondReg, _state.VramReadBuffer, _state.Ppu1OpenBus, _state.Ppu2OpenBus, _state.CgramAddress, _state.MosaicSize, _state.MosaicEnabled,
		_mosaicScanlineCounter, _state.OamMode, _state.OamBaseAddress, _state.OamAddressOffset, _state.OamRamAddress, _state.EnableOamPriority,
		_internalOamAddress, _oamWriteBuffer, _timeOver, _rangeOver, _state.HiResMode, _state.ScreenInterlace, _state.ObjInterlace,
		_state.OverscanMode, _state.DirectColorMode, _state.ColorMathClipMode, _state.ColorMathPreventMode, _state.ColorMathAddSubscreen, _state.ColorMathEnabled,
		_state.ColorMathSubstractMode, _state.ColorMathHalveResult, _state.FixedColor, _hvScrollLatchValue, _hScrollLatchValue, 
		_horizontalLocation, _horizontalLocToggle, _verticalLocation, _verticalLocationToggle, _locationLatched,
		_state.MaskLogic[0], _state.MaskLogic[1], _state.MaskLogic[2], _state.MaskLogic[3], _state.MaskLogic[4], _state.MaskLogic[5],
		_state.WindowMaskMain[0], _state.WindowMaskMain[1], _state.WindowMaskMain[2], _state.WindowMaskMain[3], _state.WindowMaskMain[4],
		_state.WindowMaskSub[0], _state.WindowMaskSub[1], _state.WindowMaskSub[2], _state.WindowMaskSub[3], _state.WindowMaskSub[4],
		_state.Mode7.CenterX, _state.Mode7.CenterY, _state.ExtBgEnabled, _state.Mode7.FillWithTile0, _state.Mode7.HorizontalMirroring,
		_state.Mode7.HScroll, _state.Mode7.LargeMap, _state.Mode7.Matrix[0], _state.Mode7.Matrix[1], _state.Mode7.Matrix[2], _state.Mode7.Matrix[3],
		_state.Mode7.ValueLatch, _state.Mode7.VerticalMirroring, _state.Mode7.VScroll, unused_oamRenderAddress, _oddFrame, _vblankStartScanline,
		_state.CgramAddressLatch, _state.CgramWriteBuffer, _nmiScanline, _vblankEndScanline, _adjustedVblankEndScanline, _baseVblankEndScanline,
		_overclockEnabled
	);

	for(int i = 0; i < 4; i++) {
		s.Stream(
			_state.Layers[i].ChrAddress, _state.Layers[i].DoubleHeight, _state.Layers[i].DoubleWidth, _state.Layers[i].HScroll,
			_state.Layers[i].LargeTiles, _state.Layers[i].TilemapAddress, _state.Layers[i].VScroll
		);
	}

	for(int i = 0; i < 2; i++) {
		s.Stream(
			_state.Window[i].ActiveLayers[0], _state.Window[i].ActiveLayers[1], _state.Window[i].ActiveLayers[2], _state.Window[i].ActiveLayers[3], _state.Window[i].ActiveLayers[4], _state.Window[i].ActiveLayers[5],
			_state.Window[i].InvertedLayers[0], _state.Window[i].InvertedLayers[1], _state.Window[i].InvertedLayers[2], _state.Window[i].InvertedLayers[3], _state.Window[i].InvertedLayers[4], _state.Window[i].InvertedLayers[5],
			_state.Window[i].Left, _state.Window[i].Right
		);
	}

	s.StreamArray(_vram, Ppu::VideoRamSize >> 1);
	s.StreamArray(_oamRam, Ppu::SpriteRamSize);
	s.StreamArray(_cgram, Ppu::CgRamSize >> 1);
	
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 33; j++) {
			s.Stream(
				_layerData[i].Tiles[j].ChrData[0], _layerData[i].Tiles[j].ChrData[1], _layerData[i].Tiles[j].ChrData[2], _layerData[i].Tiles[j].ChrData[3],
				_layerData[i].Tiles[j].TilemapData, _layerData[i].Tiles[j].VScroll
			);
		}
	}
	s.Stream(_hOffset, _vOffset, _fetchBgStart, _fetchBgEnd, _fetchSpriteStart, _fetchSpriteEnd);
}

/* Everything below this point is used to select the proper arguments for templates */
template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, uint16_t basePaletteOffset, bool hiResMode, bool applyMosaic>
void Ppu::RenderTilemap()
{
	if(_state.DirectColorMode) {
		RenderTilemap<layerIndex, bpp, processHighPriority, basePaletteOffset, hiResMode, applyMosaic, true>();
	} else {
		RenderTilemap<layerIndex, bpp, processHighPriority, basePaletteOffset, hiResMode, applyMosaic, false>();
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, uint16_t basePaletteOffset, bool hiResMode>
void Ppu::RenderTilemap()
{
	bool applyMosaic = ((_state.MosaicEnabled >> layerIndex) & 0x01) != 0 && (_state.MosaicSize > 1 || _state.BgMode == 5 || _state.BgMode == 6);

	if(applyMosaic) {
		RenderTilemap<layerIndex, bpp, processHighPriority, basePaletteOffset, hiResMode, true>();
	} else {
		RenderTilemap<layerIndex, bpp, processHighPriority, basePaletteOffset, hiResMode, false>();
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, uint16_t basePaletteOffset>
void Ppu::RenderTilemap()
{
	if(!IsRenderRequired(layerIndex) || (processHighPriority && !_layerData[layerIndex].HasPriorityTiles)) {
		return;
	}

	if(_state.BgMode == 5 || _state.BgMode == 6) {
		RenderTilemap<layerIndex, bpp, processHighPriority, basePaletteOffset, true>();
	} else {
		RenderTilemap<layerIndex, bpp, processHighPriority, basePaletteOffset, false>();
	}
}

template<uint8_t layerIndex, bool processHighPriority>
void Ppu::RenderTilemapMode7()
{
	bool applyMosaic = ((_state.MosaicEnabled >> layerIndex) & 0x01) != 0;

	if(applyMosaic) {
		RenderTilemapMode7<layerIndex, processHighPriority, true>();
	} else {
		RenderTilemapMode7<layerIndex, processHighPriority, false>();
	}
}

template<uint8_t layerIndex, bool processHighPriority, bool applyMosaic>
void Ppu::RenderTilemapMode7()
{
	if(_state.DirectColorMode) {
		RenderTilemapMode7<layerIndex, processHighPriority, applyMosaic, true>();
	} else {
		RenderTilemapMode7<layerIndex, processHighPriority, applyMosaic, false>();
	}
}
