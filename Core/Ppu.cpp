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

Ppu::Ppu(shared_ptr<Console> console)
{
	_console = console;

	_vram = new uint8_t[Ppu::VideoRamSize];
	_console->GetSettings()->InitializeRam(_vram, Ppu::VideoRamSize);
	_console->GetSettings()->InitializeRam(_cgram, Ppu::CgRamSize);
	_console->GetSettings()->InitializeRam(_oamRam, Ppu::SpriteRamSize);

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
	_allowFrameSkip = false;
	_regs = _console->GetInternalRegisters();
	_memoryManager = _console->GetMemoryManager();

	_vblankStart = _overscanMode ? 240 : 225;
	
	_currentBuffer = _outputBuffers[0];

	_layerConfig[0] = {};
	_layerConfig[1] = {};
	_layerConfig[2] = {};
	_layerConfig[3] = {};

	_cgramAddress = 0;

	memset(_vram, 0, Ppu::VideoRamSize);
	memset(_oamRam, 0, Ppu::SpriteRamSize);
	memset(_cgram, 0, Ppu::CgRamSize);

	_vramAddress = 0;
	_vramIncrementValue = 1;
	_vramAddressRemapping = 0;
	_vramAddrIncrementOnSecondReg = false;
}

void Ppu::Reset()
{
	_scanline = 0;
	_forcedVblank = true;
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
	return (hClock - ((hClock > 1292) << 1) - ((hClock > 1310) << 1)) >> 2;
}

uint16_t Ppu::GetVblankStart()
{
	return _vblankStart;
}

PpuState Ppu::GetState()
{
	PpuState state;
	state.Cycle = GetCycle();
	state.Scanline = _scanline;
	state.HClock = _memoryManager->GetHClock();
	state.FrameCount = _frameCount;
	state.OverscanMode = _overscanMode;
	state.BgMode = _bgMode;
	state.DirectColorMode = _directColorMode;
	state.Mode7 = _mode7;
	state.Layers[0] = _layerConfig[0];
	state.Layers[1] = _layerConfig[1];
	state.Layers[2] = _layerConfig[2];
	state.Layers[3] = _layerConfig[3];

	state.OamMode = _oamMode;
	state.OamBaseAddress = _oamBaseAddress;
	state.OamAddressOffset = _oamAddressOffset;
	state.EnableOamPriority = _enableOamPriority;
	state.ObjInterlace = _objInterlace;

	return state;
}

template<bool hiResMode>
void Ppu::GetTilemapData(uint8_t layerIndex, uint8_t columnIndex)
{
	/* The current layer's options */
	LayerConfig &config = _layerConfig[layerIndex];

	/* Layer's tilemap start address */
	uint16_t tilemapAddr = config.TilemapAddress >> 1;

	uint16_t vScroll = config.VScroll;
	uint16_t hScroll = hiResMode ? (config.HScroll << 1) : config.HScroll;	
	if(_hOffset || _vOffset) {
		uint16_t enableBit = layerIndex == 0 ? 0x2000 : 0x4000;
		if(_bgMode == 4) {
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

	/* Current scanline (in interlaced mode, switches between even and odd rows every frame */
	uint16_t realY = IsDoubleHeight() ? (_oddFrame ? ((_scanline << 1) + 1) : (_scanline << 1)) : _scanline;

	/* The current row of tiles (e.g scanlines 16-23 is row 2) */
	uint16_t row = (realY + vScroll) >> (config.LargeTiles ? 4 : 3);

	/* Tilemap offset based on the current row & tilemap size options */
	uint16_t addrVerticalScrollingOffset = config.DoubleHeight ? ((row & 0x20) << (config.DoubleWidth ? 6 : 5)) : 0;

	/* The start address for tiles on this row */
	uint16_t baseOffset = tilemapAddr + addrVerticalScrollingOffset + ((row & 0x1F) << 5);

	/* The current column index (in terms of 8x8 or 16x16 tiles) */
	uint16_t column = columnIndex + (hScroll >> 3);
	if(!hiResMode && config.LargeTiles) {
		//For 16x16 tiles, need to return the same tile for 2 columns 8 pixel columns in a row
		column >>= 1;
	}

	/* The tilemap address to read the tile data from */
	uint16_t addr = (baseOffset + (column & 0x1F) + (config.DoubleWidth ? ((column & 0x20) << 5) : 0)) << 1;

	uint16_t tilemapData = _vram[addr] | (_vram[addr + 1] << 8);
	_layerData[layerIndex].Tiles[columnIndex].TilemapData = tilemapData;
	_layerData[layerIndex].Tiles[columnIndex].VScroll = vScroll;
}

template<bool hiResMode, uint8_t bpp, bool secondTile>
void Ppu::GetChrData(uint8_t layerIndex, uint8_t column, uint8_t plane)
{
	LayerConfig &config = _layerConfig[layerIndex];
	TileData &tileData = _layerData[layerIndex].Tiles[column];
	uint16_t tilemapData = tileData.TilemapData;

	bool largeTileWidth = hiResMode || config.LargeTiles;

	uint16_t chrAddr = config.ChrAddress;
	bool vMirror = (tilemapData & 0x8000) != 0;
	bool hMirror = (tilemapData & 0x4000) != 0;

	uint16_t realY = IsDoubleHeight() ? (_oddFrame ? ((_scanline << 1) + 1) : (_scanline << 1)) : _scanline;

	bool useSecondTile = secondTile;
	if(!hiResMode && config.LargeTiles) {
		//For 16x16 tiles, need to return the 2nd part of the tile every other column
		useSecondTile = (((column << 3) + config.HScroll) & 0x08) == 0x08;
	}

	uint16_t tileIndex = tilemapData & 0x03FF;
	if(largeTileWidth || config.LargeTiles) {
		tileIndex = (
			tileIndex +
			(config.LargeTiles ? (((realY + tileData.VScroll) & 0x08) ? (vMirror ? 0 : 16) : (vMirror ? 16 : 0)) : 0) +
			(largeTileWidth ? (useSecondTile ? (hMirror ? 0 : 1) : (hMirror ? 1 : 0)) : 0)
		) & 0x3FF;
	}

	uint16_t tileStart = chrAddr + tileIndex * 8 * bpp;
	
	uint8_t baseYOffset = (realY + tileData.VScroll) & 0x07;

	uint8_t yOffset = vMirror ? (7 - baseYOffset) : baseYOffset;
	uint16_t pixelStart = tileStart + yOffset * 2 + (plane << 4);

	uint16_t chrData = _vram[pixelStart] | (_vram[(uint16_t)(pixelStart + 1)] << 8);
	_layerData[layerIndex].Tiles[column].ChrData[plane + (secondTile ? bpp / 2 : 0)] = chrData;
}

void Ppu::GetHorizontalOffsetByte(uint8_t columnIndex)
{
	uint16_t columnOffset = (((columnIndex << 3) + (_layerConfig[2].HScroll & ~0x07)) >> 3) & (_layerConfig[2].DoubleWidth ? 0x3F : 0x1F);
	uint16_t rowOffset = (_layerConfig[2].VScroll >> 3) & (_layerConfig[2].DoubleHeight ? 0x3F : 0x1F);

	uint16_t tileOffset = (columnOffset << 1) + (rowOffset << 6);
	uint16_t hOffsetAddr = _layerConfig[2].TilemapAddress + tileOffset;

	_hOffset = _vram[hOffsetAddr] | (_vram[hOffsetAddr + 1] << 8);
}

void Ppu::GetVerticalOffsetByte(uint8_t columnIndex)
{
	uint16_t columnOffset = (((columnIndex << 3) + (_layerConfig[2].HScroll & ~0x07)) >> 3) & (_layerConfig[2].DoubleWidth ? 0x3F : 0x1F);
	uint16_t rowOffset = (_layerConfig[2].VScroll >> 3) & (_layerConfig[2].DoubleHeight ? 0x3F : 0x1F);

	uint16_t tileOffset = (columnOffset << 1) + (rowOffset << 6);

	//The vertical offset is 0x40 bytes later - but wraps around within the tilemap based on the tilemap size (0x800 or 0x1000 bytes)
	uint16_t vOffsetAddr = _layerConfig[2].TilemapAddress + ((tileOffset + 0x40) & (_layerConfig[2].DoubleHeight ? 0xFFF : 0x7FF));

	_vOffset = _vram[vOffsetAddr] | (_vram[vOffsetAddr + 1] << 8);
}

void Ppu::FetchTileData()
{
	if(_fetchStartX == 0) {
		_hOffset = 0;
		_vOffset = 0;
	}

	if(_bgMode == 0) {
		for(int x = _fetchStartX; x <= _fetchEndX; x++) {
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
	} else if(_bgMode == 1) {
		for(int x = _fetchStartX; x <= _fetchEndX; x++) {
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
	} else if(_bgMode == 2) {
		for(int x = _fetchStartX; x <= _fetchEndX; x++) {
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
	} else if(_bgMode == 3) {
		for(int x = _fetchStartX; x <= _fetchEndX; x++) {
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
	} else if(_bgMode == 4) {
		for(int x = _fetchStartX; x <= _fetchEndX; x++) {
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
	} else if(_bgMode == 5) {
		for(int x = _fetchStartX; x <= _fetchEndX; x++) {
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
	} else if(_bgMode == 6) {
		for(int x = _fetchStartX; x <= _fetchEndX; x++) {
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
	if(hClock >= 1364 || (hClock == 1360 && _scanline == 240 && _oddFrame && !_screenInterlace)) {
		//"In non-interlace mode scanline 240 of every other frame (those with $213f.7=1) is only 1360 cycles."
		_scanline++;
		
		_drawStartX = 0;
		_drawEndX = 0;
		_fetchStartX = 0;
		_fetchEndX = 0;

		_pixelsDrawn = 0;
		_subPixelsDrawn = 0;
		memset(_rowPixelFlags, 0, sizeof(_rowPixelFlags));
		memset(_subScreenFilled, 0, sizeof(_subScreenFilled));

		if(_scanline == _vblankStart) {
			//Reset OAM address at the start of vblank?
			if(!_forcedVblank) {
				//TODO, the timing of this may be slightly off? should happen at H=10 based on anomie's docs
				_internalOamAddress = (_oamRamAddress << 1);
			}

			VideoConfig cfg = _console->GetSettings()->GetVideoConfig();
			_configVisibleLayers = (cfg.HideBgLayer0 ? 0 : 1) | (cfg.HideBgLayer1 ? 0 : 2) | (cfg.HideBgLayer2 ? 0 : 4) | (cfg.HideBgLayer3 ? 0 : 8) | (cfg.HideSprites ? 0 : 16);

			_console->ProcessEvent(EventType::EndFrame);

			_frameCount++;
			_console->GetSpc()->ProcessEndFrame();
			_console->GetControlManager()->UpdateInputState();
			_regs->ProcessAutoJoypadRead();
			_regs->SetNmiFlag(true);
			SendFrame();

			if(_regs->IsNmiEnabled()) {
				_console->GetCpu()->SetNmiFlag();
			}
		} else if(_scanline >= GetLastScanline() + 1) {
			//"Frames are 262 scanlines in non-interlace mode, while in interlace mode frames with $213f.7=0 are 263 scanlines"
			_oddFrame ^= 1;
			_regs->SetNmiFlag(false);
			_scanline = 0;
			_rangeOver = false;
			_timeOver = false;
			_console->ProcessEvent(EventType::StartFrame);

			_allowFrameSkip = !_console->GetVideoRenderer()->IsRecording() && (_console->GetSettings()->GetEmulationSpeed() == 0 || _console->GetSettings()->GetEmulationSpeed() > 150);
			if(!_allowFrameSkip || (_frameCount & 0x03) == 0) {
				//If we're not skipping this frame, reset the high resolution flag
				_useHighResOutput = false;
			}

			if(_mosaicEnabled) {
				_mosaicStartScanline = 1;
			}
		}
		return true;
	}
	return false;
}

uint16_t Ppu::GetLastScanline()
{
	if(_console->GetRegion() == ConsoleRegion::Ntsc) {
		if(!_screenInterlace || _oddFrame) {
			return 261;
		} else {
			return 262;
		}
	} else {
		if(!_screenInterlace || _oddFrame) {
			return 311;
		} else {
			return 312;
		}
	}
}

void Ppu::EvaluateNextLineSprites()
{
	if(_forcedVblank) {
		return;
	}

	memset(_spritePriority, 0xFF, sizeof(_spritePriority));
	memset(_spritePixels, 0xFF, sizeof(_spritePixels));
	memset(_spritePalette, 0, sizeof(_spritePalette));
	_spriteCount = 0;
	uint16_t screenY = _scanline;

	uint16_t baseAddr = _enableOamPriority ? (_internalOamAddress & 0x1FC) : 0;
	for(int i = 0; i < 512; i += 4) {
		uint16_t addr = (baseAddr + i) & 0x1FF;
		uint8_t y = _oamRam[addr + 1];

		uint8_t highTableOffset = addr >> 4;
		uint8_t shift = ((addr >> 2) & 0x03) << 1;
		uint8_t highTableValue = _oamRam[0x200 | highTableOffset] >> shift;
		uint8_t largeSprite = (highTableValue & 0x02) >> 1;
		uint8_t height = _oamSizes[_oamMode][largeSprite][1] << 3;
		if(_objInterlace) {
			height /= 2;
		}

		uint8_t endY = (y + height) & 0xFF;

		bool visible = (screenY >= y && screenY < endY) || (endY < y && screenY < endY);
		if(!visible) {
			//Not visible on this scanline
			continue;
		}

		SpriteInfo &info = _sprites[_spriteCount];
		info.LargeSprite = largeSprite;
		uint8_t width = _oamSizes[_oamMode][info.LargeSprite][0] << 3;
		uint16_t sign = (highTableValue & 0x01) << 8;
		info.X = (int16_t)((sign | _oamRam[addr]) << 7) >> 7;
		info.Y = y;

		if(info.X != -256 && (info.X + width <= 0 || info.X > 255)) {
			//Sprite is not visible (and must be ignored for time/range flag calculations)
			//Sprites at X=-256 are always used when considering Time/Range flag calculations, but not actually drawn.
			continue;
		}

		info.Index = i >> 2;
		info.TileRow = (_oamRam[addr + 2] & 0xF0) >> 4;
		info.TileColumn = _oamRam[addr + 2] & 0x0F;

		uint8_t flags = _oamRam[addr + 3];
		info.UseSecondTable = (flags & 0x01) != 0;
		info.Palette = (flags >> 1) & 0x07;
		info.Priority = (flags >> 4) & 0x03;
		info.HorizontalMirror = (flags & 0x40) != 0;
		info.VerticalMirror = (flags & 0x80) != 0;

		if(_spriteCount < 32) {
			_spriteCount++;
		} else {
			_rangeOver = true;
			break;
		}
	}

	uint16_t spriteTileCount = 0;
	for(int i = (int16_t)_spriteCount - 1; i >= 0; i--) {
		SpriteInfo &info = _sprites[i];
		uint8_t height = _oamSizes[_oamMode][info.LargeSprite][1] << 3;
		uint8_t width = _oamSizes[_oamMode][info.LargeSprite][0] << 3;

		uint8_t yOffset;
		int rowOffset;
		int yGap = (screenY - info.Y);
		if(_objInterlace) {
			yGap <<= 1;
			yGap |= _oddFrame;
		}
		if(info.VerticalMirror) {
			yOffset = (height - 1 - yGap) & 0x07;
			rowOffset = (height - 1 - yGap) >> 3;
		} else {
			yOffset = yGap & 0x07;
			rowOffset = yGap >> 3;
		}

		uint8_t row = (info.TileRow + rowOffset) & 0x0F;
		int prevColumnOffset = -1;

		//Keep the last address the PPU used while rendering sprites (needed for Uniracers, which writes to OAM during rendering)
		_oamRenderAddress = 0x200 + (info.Index >> 2);

		for(int x = std::max<int16_t>(info.X, 0); x < info.X + width && x < 256; x++) {
			uint8_t xOffset;
			int columnOffset;
			if(info.HorizontalMirror) {
				xOffset = (width - (x - info.X) - 1) & 0x07;
				columnOffset = (width - (x - info.X) - 1) >> 3;
			} else {
				xOffset = (x - info.X) & 0x07;
				columnOffset = (x - info.X) >> 3;
			}

			if(prevColumnOffset != columnOffset) {
				spriteTileCount++;
				if(spriteTileCount > 34) {
					_timeOver = true;
					return;
				}
				prevColumnOffset = columnOffset;
			}

			uint8_t column = (info.TileColumn + columnOffset) & 0x0F;
			uint8_t tileIndex = (row << 4) | column;
			uint16_t tileStart = ((_oamBaseAddress + (tileIndex << 4) + (info.UseSecondTable ? _oamAddressOffset : 0)) & 0x7FFF) << 1;

			uint16_t color = GetTilePixelColor<4>(tileStart + yOffset * 2, 7 - xOffset);

			if(color != 0) {
				uint16_t paletteRamOffset = 256 + (((info.Palette << 4) + color) << 1);
				_spritePixels[x] = paletteRamOffset;
				_spritePriority[x] = info.Priority;
				_spritePalette[x] = info.Palette;
			}
		}
	}
}

template<bool forMainScreen>
void Ppu::RenderMode0()
{
	RenderSprites<3, forMainScreen>();
	RenderTilemap<0, 2, true, forMainScreen>();
	RenderTilemap<1, 2, true, forMainScreen, 64>();
	RenderSprites<2, forMainScreen>();
	RenderTilemap<0, 2, false, forMainScreen>();
	RenderTilemap<1, 2, false, forMainScreen, 64>();
	RenderSprites<1, forMainScreen>();
	RenderTilemap<2, 2, true, forMainScreen, 128>();
	RenderTilemap<3, 2, true, forMainScreen, 192>();
	RenderSprites<0, forMainScreen>();
	RenderTilemap<2, 2, false, forMainScreen, 128>();
	RenderTilemap<3, 2, false, forMainScreen, 192>();
	RenderBgColor<forMainScreen>();
}

template<bool forMainScreen>
void Ppu::RenderMode1()
{
	if(_mode1Bg3Priority) {
		RenderTilemap<2, 2, true, forMainScreen>();
	}
	RenderSprites<3, forMainScreen>();
	RenderTilemap<0, 4, true, forMainScreen>();
	RenderTilemap<1, 4, true, forMainScreen>();
	RenderSprites<2, forMainScreen>();
	RenderTilemap<0, 4, false, forMainScreen>();
	RenderTilemap<1, 4, false, forMainScreen>();
	RenderSprites<1, forMainScreen>();
	if(!_mode1Bg3Priority) {
		RenderTilemap<2, 2, true, forMainScreen>();
	}
	RenderSprites<0, forMainScreen>();
	RenderTilemap<2, 2, false, forMainScreen>();
	RenderBgColor<forMainScreen>();
}

template<bool forMainScreen>
void Ppu::RenderMode2()
{
	RenderSprites<3, forMainScreen>();
	RenderTilemap<0, 4, true, forMainScreen>();
	RenderSprites<2, forMainScreen>();
	RenderTilemap<1, 4, true, forMainScreen>();
	RenderSprites<1, forMainScreen>();
	RenderTilemap<0, 4, false, forMainScreen>();
	RenderSprites<0, forMainScreen>();
	RenderTilemap<1, 4, false, forMainScreen>();
	RenderBgColor<forMainScreen>();
}

template<bool forMainScreen>
void Ppu::RenderMode3()
{
	RenderSprites<3, forMainScreen>();
	RenderTilemap<0, 8, true, forMainScreen>();
	RenderSprites<2, forMainScreen>();
	RenderTilemap<1, 4, true, forMainScreen>();
	RenderSprites<1, forMainScreen>();
	RenderTilemap<0, 8, false, forMainScreen>();
	RenderSprites<0, forMainScreen>();
	RenderTilemap<1, 4, false, forMainScreen>();
	RenderBgColor<forMainScreen>();
}

template<bool forMainScreen>
void Ppu::RenderMode4()
{
	RenderSprites<3, forMainScreen>();
	RenderTilemap<0, 8, true, forMainScreen>();
	RenderSprites<2, forMainScreen>();
	RenderTilemap<1, 2, true, forMainScreen>();
	RenderSprites<1, forMainScreen>();
	RenderTilemap<0, 8, false, forMainScreen>();
	RenderSprites<0, forMainScreen>();
	RenderTilemap<1, 2, false, forMainScreen>();
	RenderBgColor<forMainScreen>();
}

template<bool forMainScreen>
void Ppu::RenderMode5()
{
	RenderSprites<3, forMainScreen>();
	RenderTilemap<0, 4, true, forMainScreen>();
	RenderSprites<2, forMainScreen>();
	RenderTilemap<1, 2, true, forMainScreen>();
	RenderSprites<1, forMainScreen>();
	RenderTilemap<0, 4, false, forMainScreen>();
	RenderSprites<0, forMainScreen>();
	RenderTilemap<1, 2, false, forMainScreen>();
	RenderBgColor<forMainScreen>();
}

template<bool forMainScreen>
void Ppu::RenderMode6()
{
	RenderSprites<3, forMainScreen>();
	RenderTilemap<0, 4, true, forMainScreen>();
	RenderSprites<2, forMainScreen>();
	RenderSprites<1, forMainScreen>();
	RenderTilemap<0, 4, false, forMainScreen>();
	RenderSprites<0, forMainScreen>();
	RenderBgColor<forMainScreen>();
}

template<bool forMainScreen>
void Ppu::RenderMode7()
{
	RenderSprites<3, forMainScreen>();
	RenderSprites<2, forMainScreen>();
	if(_mode7.ExtBgEnabled) {
		RenderTilemapMode7<1, forMainScreen, true>();
	}
	RenderSprites<1, forMainScreen>();
	RenderTilemapMode7<0, forMainScreen, false>();
	RenderSprites<0, forMainScreen>();
	if(_mode7.ExtBgEnabled) {
		RenderTilemapMode7<1, forMainScreen, false>();
	}
	RenderBgColor<forMainScreen>();
}

void Ppu::RenderScanline()
{
	if(_drawStartX > 255 || (_allowFrameSkip && (_frameCount & 0x03) != 0)) {
		return;
	}

	_fetchEndX = std::min((_memoryManager->GetHClock() - 6*4) >> 2, 263);
	if(_fetchStartX > _fetchEndX) {
		return;
	}
	FetchTileData();
	_fetchStartX = _fetchEndX + 1;

	if(_memoryManager->GetHClock() < 22 * 4) {
		return;
	}

	_drawEndX = std::min((_memoryManager->GetHClock() - 22*4) >> 2, 255);

	uint8_t bgMode = _bgMode;
	if(_forcedVblank) {
		bgMode = 8;
	}

	switch(bgMode) {
		case 0:
			RenderMode0<true>();
			RenderMode0<false>();
			break;

		case 1:
			RenderMode1<true>();
			RenderMode1<false>();
			break;

		case 2:
			RenderMode2<true>();
			RenderMode2<false>();
			break;

		case 3:
			RenderMode3<true>();
			RenderMode3<false>();
			break;
		
		case 4:
			RenderMode4<true>();
			RenderMode4<false>();
			break;

		case 5:
			RenderMode5<true>();
			RenderMode5<false>();
			break;

		case 6:
			RenderMode6<true>();
			RenderMode6<false>();
			break;

		case 7:
			RenderMode7<true>();
			RenderMode7<false>();
			break;

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

template<bool forMainScreen>
void Ppu::RenderBgColor()
{
	if((forMainScreen && _pixelsDrawn == 256) || (!forMainScreen && _subPixelsDrawn == 256)) {
		return;
	}

	uint16_t bgColor = _cgram[0] | (_cgram[1] << 8);
	for(int x = _drawStartX; x <= _drawEndX; x++) {
		if(forMainScreen) {
			if(!_rowPixelFlags[x]) {
				uint8_t pixelFlags = PixelFlags::Filled | ((_colorMathEnabled & 0x20) ? PixelFlags::AllowColorMath : 0);
				_mainScreenBuffer[x] = bgColor;
				_rowPixelFlags[x] = pixelFlags;
			}
		} else {
			if(!_subScreenFilled[x]) {
				_subScreenBuffer[x] = bgColor;
			}
		}
	}
}

template<uint8_t priority, bool forMainScreen>
void Ppu::RenderSprites()
{
	if(!IsRenderRequired<forMainScreen>(Ppu::SpriteLayerIndex)) {
		return;
	}

	uint8_t activeWindowCount = 0;
	if(forMainScreen) {
		if(_windowMaskMain[Ppu::SpriteLayerIndex]) {
			activeWindowCount = (uint8_t)_window[0].ActiveLayers[Ppu::SpriteLayerIndex] + (uint8_t)_window[1].ActiveLayers[Ppu::SpriteLayerIndex];
		}
	} else {
		if(_windowMaskSub[Ppu::SpriteLayerIndex]) {
			activeWindowCount = (uint8_t)_window[0].ActiveLayers[Ppu::SpriteLayerIndex] + (uint8_t)_window[1].ActiveLayers[Ppu::SpriteLayerIndex];
		}
	}

	if(forMainScreen) {
		for(int x = _drawStartX; x <= _drawEndX; x++) {
			if(!_rowPixelFlags[x] && _spritePriority[x] == priority) {
				if(activeWindowCount && ProcessMaskWindow<Ppu::SpriteLayerIndex>(activeWindowCount, x)) {
					//This pixel was masked
					continue;
				}

				uint16_t paletteRamOffset = _spritePixels[x];
				_mainScreenBuffer[x] = _cgram[paletteRamOffset] | (_cgram[paletteRamOffset + 1] << 8);
				_rowPixelFlags[x] |= PixelFlags::Filled | (((_colorMathEnabled & 0x10) && _spritePalette[x] > 3) ? PixelFlags::AllowColorMath : 0);
			}
		}
	} else {
		for(int x = _drawStartX; x <= _drawEndX; x++) {
			if(!_subScreenFilled[x] && _spritePriority[x] == priority) {
				if(activeWindowCount && ProcessMaskWindow<Ppu::SpriteLayerIndex>(activeWindowCount, x)) {
					//This pixel was masked
					continue;
				}
				
				uint16_t paletteRamOffset = _spritePixels[x];
				_subScreenBuffer[x] = _cgram[paletteRamOffset] | (_cgram[paletteRamOffset + 1] << 8);
				_subScreenFilled[x] = true;
			}
		}
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset, bool hiResMode, uint8_t activeWindowCount, bool applyMosaic, bool directColorMode>
void Ppu::RenderTilemap()
{
	if(!IsRenderRequired<forMainScreen>(layerIndex)) {
		return;
	}

	/* Current scanline (in interlaced mode, switches between even and odd rows every frame */
	uint16_t realY = IsDoubleHeight() ? (_oddFrame ? ((_scanline << 1) + 1) : (_scanline << 1)) : _scanline;

	/* Keeps track of whether or not the pixel is allowed to participate in color math */
	uint8_t pixelFlags = PixelFlags::Filled | (((_colorMathEnabled >> layerIndex) & 0x01) ? PixelFlags::AllowColorMath : 0);

	/* True when the entire scanline has to be replaced by a mosaic pattern */
	bool mosaicScanline = applyMosaic && (realY - _mosaicStartScanline) % _mosaicSize != 0;
	
	if(applyMosaic && ((realY - _mosaicStartScanline) % _mosaicSize == 0)) {
		//On each "first" line of the mosaic pattern, clear the entire buffer before processing the scanline
		memset(_mosaicColor[layerIndex][processHighPriority], 0xFF, 256*2);
	}

	/* The current layer's options */
	LayerConfig &config = _layerConfig[layerIndex];

	uint16_t hScroll = hiResMode ? (config.HScroll << 1) : config.HScroll;

	/* The current pixel x position (normally 0-255, but 0-511 in hi-res mode - even on subscreen, odd on main screen) */
	uint16_t realX;
	
	for(int x = _drawStartX; x <= _drawEndX; x++) {
		if(hiResMode) {
			realX = (x << 1) + (forMainScreen ? 1 : 0);
		} else {
			realX = x;
		}

		int lookupIndex;
		int chrDataOffset;
		if(hiResMode) {
			lookupIndex = (x + (config.HScroll & 0x07)) >> 2;
			chrDataOffset = lookupIndex & 0x01;
			lookupIndex >>= 1;
		} else {
			lookupIndex = (x + (config.HScroll & 0x07)) >> 3;
			chrDataOffset = 0;
		}

		TileData &tileData =  _layerData[layerIndex].Tiles[lookupIndex];
		uint16_t tilemapData = tileData.TilemapData;

		//Skip pixels that were filled by previous layers (or that don't match the priority level currently being processed)
		if(forMainScreen) {
			if((!applyMosaic && _rowPixelFlags[x]) || ((uint8_t)processHighPriority != ((tilemapData & 0x2000) >> 13))) {
				continue;
			}
		} else {
			if((!applyMosaic && _subScreenFilled[x]) || ((uint8_t)processHighPriority != ((tilemapData & 0x2000) >> 13))) {
				continue;
			}
		}

		if(!applyMosaic && activeWindowCount && ProcessMaskWindow<layerIndex>(activeWindowCount, x)) {
			//This pixel was masked, skip it
			continue;
		}

		//The pixel is empty, not clipped and not part of a mosaic pattern, process it
		bool hMirror = (tilemapData & 0x4000) != 0;

		uint8_t xOffset = (realX + hScroll) & 0x07;
		uint8_t shift = hMirror ? xOffset : (7 - xOffset);
		
		uint8_t color = GetTilePixelColor<bpp>(tileData.ChrData + (chrDataOffset ? bpp / 2 : 0), shift);

		if(color > 0) {
			uint16_t paletteColor;
			if(bpp == 8 && directColorMode) {
				uint8_t palette = (tilemapData >> 10) & 0x07;
				paletteColor = (
					((((color & 0x07) << 1) | (palette & 0x01)) << 1) |
					(((color & 0x38) | ((palette & 0x02) << 1)) << 4) |
					(((color & 0xC0) | ((palette & 0x04) << 3)) << 7)
				);
			} else {
				/* Ignore palette bits for 256-color layers */
				uint8_t palette = bpp == 8 ? 0 : (tilemapData >> 10) & 0x07;
				uint16_t paletteRamOffset = basePaletteOffset + (palette * (1 << bpp) + color) * 2;
				paletteColor = _cgram[paletteRamOffset] | (_cgram[paletteRamOffset + 1] << 8);
			}

			if(forMainScreen) {
				if(applyMosaic) {
					bool skipDraw = activeWindowCount && ProcessMaskWindow<layerIndex>(activeWindowCount, x);
					if(!mosaicScanline && (x % _mosaicSize) == 0) {
						//If this is the top-left pixel, save its color and use it
						for(int i = 0; i < _mosaicSize && x+i < 256; i++) {
							_mosaicColor[layerIndex][processHighPriority][x+i] = paletteColor;
						}
					} else {
						//Otherwise, use the top-left pixel's color
						paletteColor = _mosaicColor[layerIndex][processHighPriority][x];
						if(paletteColor == 0xFFFF) {
							continue;
						}
					}
					
					if(!skipDraw && !_rowPixelFlags[x]) {
						//If this pixel isn't hidden by the window or already set, draw it
						DrawMainPixel(x, paletteColor, pixelFlags);
					}
				} else {
					DrawMainPixel(x, paletteColor, pixelFlags);
				}
			} else {
				DrawSubPixel(x, paletteColor);
			}
		}
	}
}

template<bool forMainScreen>
bool Ppu::IsRenderRequired(uint8_t layerIndex)
{
	if(forMainScreen) {
		if(_pixelsDrawn == 256 || (((_mainScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01) == 0) {
			//This screen is disabled, or we've drawn all pixels already
			return false;
		}
	} else {
		if(_subPixelsDrawn == 256 || (((_subScreenLayers & _configVisibleLayers) >> layerIndex) & 0x01) == 0) {
			//This screen is disabled, or we've drawn all pixels already
			return false;
		}
	}
	return true;
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

template<uint8_t bpp>
uint16_t Ppu::GetTilePixelColor(const uint16_t pixelStart, const uint8_t shift)
{
	uint16_t color;
	if(bpp == 2) {
		color = (((_vram[pixelStart + 0] >> shift) & 0x01) << 0);
		color |= (((_vram[pixelStart + 1] >> shift) & 0x01) << 1);
	} else if(bpp == 4) {
		color = (((_vram[pixelStart + 0] >> shift) & 0x01) << 0);
		color |= (((_vram[pixelStart + 1] >> shift) & 0x01) << 1);
		color |= (((_vram[pixelStart + 16] >> shift) & 0x01) << 2);
		color |= (((_vram[pixelStart + 17] >> shift) & 0x01) << 3);
	} else if(bpp == 8) {
		color = (((_vram[pixelStart + 0] >> shift) & 0x01) << 0);
		color |= (((_vram[pixelStart + 1] >> shift) & 0x01) << 1);
		color |= (((_vram[pixelStart + 16] >> shift) & 0x01) << 2);
		color |= (((_vram[pixelStart + 17] >> shift) & 0x01) << 3);
		color |= (((_vram[pixelStart + 32] >> shift) & 0x01) << 4);
		color |= (((_vram[pixelStart + 33] >> shift) & 0x01) << 5);
		color |= (((_vram[pixelStart + 48] >> shift) & 0x01) << 6);
		color |= (((_vram[pixelStart + 49] >> shift) & 0x01) << 7);
	} else {
		throw std::runtime_error("unsupported bpp");
	}
	return color;
}

template<uint8_t layerIndex, bool forMainScreen, bool processHighPriority, bool applyMosaic, bool directColorMode>
void Ppu::RenderTilemapMode7()
{
	if(!IsRenderRequired<forMainScreen>(layerIndex)) {
		return;
	}

	auto clip = [](int32_t val) { return (val & 0x2000) ? (val | ~0x3ff) : (val & 0x3ff); };

	int32_t lutX[256];
	int32_t lutY[256];

	int32_t hScroll = ((int32_t)_mode7.HScroll << 19) >> 19;
	int32_t vScroll = ((int32_t)_mode7.VScroll << 19) >> 19;
	int32_t centerX = ((int32_t)_mode7.CenterX << 19) >> 19;
	int32_t centerY = ((int32_t)_mode7.CenterY << 19) >> 19;
	uint16_t realY = _mode7.VerticalMirroring ? (255 - _scanline) : _scanline;

	lutX[0] = (
		((_mode7.Matrix[0] * clip(hScroll - centerX)) & ~63) +
		((_mode7.Matrix[1] * realY) & ~63) +
		((_mode7.Matrix[1] * clip(vScroll - centerY)) & ~63) +
		(centerX << 8)
	);

	lutY[0] = (
		((_mode7.Matrix[2] * clip(hScroll - centerX)) & ~63) +
		((_mode7.Matrix[3] * realY) & ~63) +
		((_mode7.Matrix[3] * clip(vScroll - centerY)) & ~63) +
		(centerY << 8)
	);

	for(int x = 1; x < 256; x++) {
		lutX[x] = lutX[x - 1] + _mode7.Matrix[0];
		lutY[x] = lutY[x - 1] + _mode7.Matrix[2];
	}

	uint8_t activeWindowCount = 0;
	if((forMainScreen && _windowMaskMain[layerIndex]) || (!forMainScreen && _windowMaskSub[layerIndex])) {
		activeWindowCount = (uint8_t)_window[0].ActiveLayers[layerIndex] + (uint8_t)_window[1].ActiveLayers[layerIndex];
	}

	uint8_t pixelFlags = PixelFlags::Filled | (((_colorMathEnabled >> layerIndex) & 0x01) ? PixelFlags::AllowColorMath : 0);

	for(int x = _drawStartX; x <= _drawEndX; x++) {
		uint16_t realX = _mode7.HorizontalMirroring ? (255 - x) : x;

		if(forMainScreen) {
			if(_rowPixelFlags[x]) {
				continue;
			}
		} else {
			if(_subScreenFilled[x]) {
				continue;
			}
		}

		if(activeWindowCount && ProcessMaskWindow<layerIndex>(activeWindowCount, x)) {
			//This pixel was masked, skip it
			continue;
		}

		int32_t xOffset = (lutX[realX] >> 8);
		int32_t yOffset = (lutY[realX] >> 8);

		uint8_t tileIndex;
		if(!_mode7.LargeMap) {
			yOffset &= 0x3FF;
			xOffset &= 0x3FF;
			tileIndex = _vram[(((yOffset & ~0x07) << 4) | (xOffset >> 3)) << 1];
		} else {
			if(yOffset < 0 || yOffset > 0x3FF || xOffset < 0 || xOffset > 0x3FF) {
				if(_mode7.FillWithTile0) {
					tileIndex = 0;
				} else {
					//Draw nothing for this pixel, we're outside the map
					continue;
				}
			} else {
				tileIndex = _vram[(((yOffset & ~0x07) << 4) | (xOffset >> 3)) << 1];
			}
		}

		uint16_t colorIndex;
		if(layerIndex == 1) {
			uint8_t color = _vram[(((tileIndex << 6) + ((yOffset & 0x07) << 3) + (xOffset & 0x07)) << 1) + 1];
			if(((uint8_t)processHighPriority << 7) != (color & 0x80)) {
				//Wrong priority, skip this pixel
				continue;
			}
			colorIndex = (color & 0x7F);
		} else {
			colorIndex = (_vram[(((tileIndex << 6) + ((yOffset & 0x07) << 3) + (xOffset & 0x07)) << 1) + 1]);
		}

		if(colorIndex > 0) {
			uint16_t paletteColor;
			if(directColorMode) {
				paletteColor = ((colorIndex & 0x07) << 2) | ((colorIndex & 0x38) << 4) | ((colorIndex & 0xC0) << 7);
			} else {
				paletteColor = _cgram[colorIndex << 1] | (_cgram[(colorIndex << 1) + 1] << 8);
			}
			if(forMainScreen) {
				DrawMainPixel(x, paletteColor, pixelFlags);
			} else {
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
	uint8_t activeWindowCount = (uint8_t)_window[0].ActiveLayers[Ppu::ColorWindowIndex] + (uint8_t)_window[1].ActiveLayers[Ppu::ColorWindowIndex];
	bool hiResMode = _hiResMode || _bgMode == 5 || _bgMode == 6;
	uint16_t prevMainPixel = 0;
	int prevX = _drawStartX > 0 ? _drawStartX - 1 : 0;

	for(int x = _drawStartX; x <= _drawEndX; x++) {
		bool isInsideWindow = activeWindowCount && ProcessMaskWindow<Ppu::ColorWindowIndex>(activeWindowCount, x);

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
	uint8_t halfShift = _colorMathHalveResult ? 1 : 0;

	//Set color to black as needed based on clip mode
	switch(_colorMathClipMode) {
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
	switch(_colorMathPreventMode) {
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
	if(_colorMathAddSubscreen) {
		if(_subScreenFilled[x]) {
			otherPixel = pixelB;
		} else {
			//there's nothing in the subscreen at this pixel, use the fixed color and disable halve operation
			otherPixel = _fixedColor;
			halfShift = 0;
		}
	} else {
		otherPixel = _fixedColor;
	}

	if(_colorMathSubstractMode) {
		uint16_t r = std::max((pixelA & 0x001F) - (otherPixel & 0x001F), 0) >> halfShift;
		uint16_t g = std::max(((pixelA >> 5) & 0x001F) - ((otherPixel >> 5) & 0x001F), 0) >> halfShift;
		uint16_t b = std::max(((pixelA >> 10) & 0x001F) - ((otherPixel >> 10) & 0x001F), 0) >> halfShift;

		pixelA = r | (g << 5) | (b << 10);
	} else {
		uint16_t r = std::min(((pixelA & 0x001F) + (otherPixel & 0x001F)) >> halfShift, 0x1F);
		uint16_t g = std::min((((pixelA >> 5) & 0x001F) + ((otherPixel >> 5) & 0x001F)) >> halfShift, 0x1F);
		uint16_t b = std::min((((pixelA >> 10) & 0x001F) + ((otherPixel >> 10) & 0x001F)) >> halfShift, 0x1F);

		pixelA = r | (g << 5) | (b << 10);
	}
}

template<bool forMainScreen>
void Ppu::ApplyBrightness()
{
	if(_screenBrightness != 15) {
		for(int x = _drawStartX; x <= _drawEndX; x++) {
			uint16_t &pixel = (forMainScreen ? _mainScreenBuffer : _subScreenBuffer)[x];
			uint16_t r = (pixel & 0x1F) * _screenBrightness / 15;
			uint16_t g = ((pixel >> 5) & 0x1F) * _screenBrightness / 15;
			uint16_t b = ((pixel >> 10) & 0x1F) * _screenBrightness / 15;
			pixel = r | (g << 5) | (b << 10);
		}
	}
}

void Ppu::ConvertToHiRes()
{
	uint16_t scanline = _overscanMode ? (_scanline - 1) : (_scanline + 6);

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
	uint16_t scanline = _overscanMode ? (_scanline - 1) : (_scanline + 6);

	bool useHighResOutput = _useHighResOutput || IsDoubleWidth() || IsDoubleHeight();
	if(_useHighResOutput != useHighResOutput) {
		//Convert standard res picture to high resolution when the PPU starts drawing in high res mid frame
		ConvertToHiRes();
		_useHighResOutput = useHighResOutput;
	}

	if(!_useHighResOutput) {
		memcpy(_currentBuffer + (scanline << 8) + _drawStartX, _mainScreenBuffer + _drawStartX, (_drawEndX - _drawStartX + 1) << 2);
	} else {
		uint32_t screenY = IsDoubleHeight() ? (_oddFrame ? ((scanline << 1) + 1) : (scanline << 1)) : (scanline << 1);
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

		if(!IsDoubleHeight()) {
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
	if(activeWindowCount == 1) {
		if(_window[0].ActiveLayers[layerIndex]) {
			return _window[0].PixelNeedsMasking<layerIndex>(x);
		} else {
			return _window[1].PixelNeedsMasking<layerIndex>(x);
		}
	} else {
		switch(_maskLogic[layerIndex]) {
			default:
			case WindowMaskLogic::Or: return _window[0].PixelNeedsMasking<layerIndex>(x) | _window[1].PixelNeedsMasking<layerIndex>(x);
			case WindowMaskLogic::And: return _window[0].PixelNeedsMasking<layerIndex>(x) & _window[1].PixelNeedsMasking<layerIndex>(x);
			case WindowMaskLogic::Xor: return _window[0].PixelNeedsMasking<layerIndex>(x) ^ _window[1].PixelNeedsMasking<layerIndex>(x);
			case WindowMaskLogic::Xnor: return !(_window[0].PixelNeedsMasking<layerIndex>(x) ^ _window[1].PixelNeedsMasking<layerIndex>(x));
		}
	}
}

void Ppu::ProcessWindowMaskSettings(uint8_t value, uint8_t offset)
{
	_window[0].ActiveLayers[0 + offset] = (value & 0x02) != 0;
	_window[0].ActiveLayers[1 + offset] = (value & 0x20) != 0;
	_window[0].InvertedLayers[0 + offset] = (value & 0x01) != 0;
	_window[0].InvertedLayers[1 + offset] = (value & 0x10) != 0;

	_window[1].ActiveLayers[0 + offset] = (value & 0x08) != 0;
	_window[1].ActiveLayers[1 + offset] = (value & 0x80) != 0;
	_window[1].InvertedLayers[0 + offset] = (value & 0x04) != 0;
	_window[1].InvertedLayers[1 + offset] = (value & 0x40) != 0;
}

void Ppu::SendFrame()
{
	uint16_t width = _useHighResOutput ? 512 : 256;
	uint16_t height = _useHighResOutput ? 478 : 239;

	_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);

	if(!_overscanMode) {
		//Clear the top 7 and bottom 8 rows
		int top = (_useHighResOutput ? 14 : 7);
		int bottom = (_useHighResOutput ? 16 : 8);
		memset(_currentBuffer, 0, width * top * sizeof(uint16_t));
		memset(_currentBuffer + width * (height - bottom), 0, width * bottom * sizeof(uint16_t));
	}

	bool isRewinding = _console->GetRewindManager()->IsRewinding();
#ifdef LIBRETRO
	_console->GetVideoDecoder()->UpdateFrameSync(_currentBuffer, width, height, _frameCount, isRewinding);
#else
	if(isRewinding || _screenInterlace) {
		_console->GetVideoDecoder()->UpdateFrameSync(_currentBuffer, width, height, _frameCount, isRewinding);
	} else {
		_console->GetVideoDecoder()->UpdateFrame(_currentBuffer, width, height, _frameCount);

		if(!_allowFrameSkip || (_frameCount & 0x03) == 0) {
			_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
		}
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

uint8_t* Ppu::GetVideoRam()
{
	return _vram;
}

uint8_t* Ppu::GetCgRam()
{
	return _cgram;
}

uint8_t* Ppu::GetSpriteRam()
{
	return _oamRam;
}

bool Ppu::IsDoubleHeight()
{
	return _screenInterlace && (_bgMode == 5 || _bgMode == 6);
}

bool Ppu::IsDoubleWidth()
{
	return _hiResMode || _bgMode == 5 || _bgMode == 6;
}

void Ppu::LatchLocationValues()
{
	_horizontalLocation = GetCycle();
	_verticalLocation = _scanline;
	_locationLatched = true;
}

void Ppu::UpdateVramReadBuffer()
{
	uint16_t addr = GetVramAddress();
	_vramReadBuffer = _vram[addr << 1] | (_vram[(addr << 1) + 1] << 8);
}

uint16_t Ppu::GetVramAddress()
{
	uint16_t addr = _vramAddress;
	switch(_vramAddressRemapping) {
		default:
		case 0: return addr;
		case 1: return (addr & 0xFF00) | ((addr & 0xE0) >> 5) | ((addr & 0x1F) << 3);
		case 2: return (addr & 0xFE00) | ((addr & 0x1C0) >> 6) | ((addr & 0x3F) << 3);
		case 3: return (addr & 0xFC00) | ((addr & 0x380) >> 7) | ((addr & 0x7F) << 3);
	}
}

uint8_t Ppu::Read(uint16_t addr)
{
	switch(addr) {
		case 0x2134:
			_ppu1OpenBus = ((int16_t)_mode7.Matrix[0] * ((int16_t)_mode7.Matrix[1] >> 8)) & 0xFF;
			return _ppu1OpenBus;

		case 0x2135:
			_ppu1OpenBus = (((int16_t)_mode7.Matrix[0] * ((int16_t)_mode7.Matrix[1] >> 8)) >> 8) & 0xFF;
			return _ppu1OpenBus;

		case 0x2136:
			_ppu1OpenBus = (((int16_t)_mode7.Matrix[0] * ((int16_t)_mode7.Matrix[1] >> 8)) >> 16) & 0xFF;
			return _ppu1OpenBus;

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
			uint16_t addr = (_forcedVblank || (_scanline >= _vblankStart)) ? _internalOamAddress : _oamRenderAddress;
			uint8_t value;
			if(addr < 512) {
				value = _oamRam[addr];
				_console->ProcessPpuRead(addr, value, SnesMemoryType::SpriteRam);
			} else {
				value = _oamRam[0x200 | (addr & 0x1F)];
				_console->ProcessPpuRead(0x200 | (addr & 0x1F), value, SnesMemoryType::SpriteRam);
			}
			_internalOamAddress = (_internalOamAddress + 1) & 0x3FF;
			_ppu1OpenBus = value;
			return value;
		}

		case 0x2139: {
			//VMDATALREAD - VRAM Data Read low byte
			uint8_t returnValue = (uint8_t)_vramReadBuffer;
			_console->ProcessPpuRead(GetVramAddress(), returnValue, SnesMemoryType::VideoRam);
			if(!_vramAddrIncrementOnSecondReg) {
				UpdateVramReadBuffer();
				_vramAddress = (_vramAddress + _vramIncrementValue) & 0x7FFF;
			}
			_ppu1OpenBus = returnValue;
			return returnValue;
		}

		case 0x213A: {
			//VMDATAHREAD - VRAM Data Read high byte
			uint8_t returnValue = (uint8_t)(_vramReadBuffer >> 8);
			_console->ProcessPpuRead(GetVramAddress() + 1, returnValue, SnesMemoryType::VideoRam);
			if(_vramAddrIncrementOnSecondReg) {
				UpdateVramReadBuffer();
				_vramAddress = (_vramAddress + _vramIncrementValue) & 0x7FFF;
			}
			_ppu1OpenBus = returnValue;
			return returnValue;
		}

		case 0x213B: {
			//CGDATAREAD - CGRAM Data read
			uint8_t value = _cgram[_cgramAddress];
			if(_cgramAddress & 0x01) {
				value = (value & 0x7F) | (_ppu2OpenBus & 0x80);
			}
			_console->ProcessPpuRead(_cgramAddress, value, SnesMemoryType::CGRam);
			_cgramAddress = (_cgramAddress + 1) & (Ppu::CgRamSize - 1);
			_ppu2OpenBus = value;
			return value;
		}

		case 0x213C: {
			//OPHCT - Horizontal Scanline Location
			uint8_t value;
			if(_horizontalLocToggle) {
				//"Note that the value read is only 9 bits: bits 1-7 of the high byte are PPU2 Open Bus."
				value = ((_horizontalLocation & 0x100) >> 8) | (_ppu2OpenBus & 0xFE);
			} else {
				value = _horizontalLocation & 0xFF;
			}
			_ppu2OpenBus = value;
			_horizontalLocToggle = !_horizontalLocToggle;
			return value;
		}

		case 0x213D: {
			//OPVCT - Vertical Scanline Location
			uint8_t value;
			if(_verticalLocationToggle) {
				//"Note that the value read is only 9 bits: bits 1-7 of the high byte are PPU2 Open Bus."
				value = ((_verticalLocation & 0x100) >> 8) | (_ppu2OpenBus & 0xFE);
			} else {
				value = _verticalLocation & 0xFF;
			}
			_ppu2OpenBus = value;
			_verticalLocationToggle = !_verticalLocationToggle;
			return value;
		}

		case 0x213E: {
			//STAT77 - PPU Status Flag and Version
			uint8_t value = (
				(_timeOver ? 0x80 : 0) |
				(_rangeOver ? 0x40 : 0) |
				(_ppu1OpenBus & 0x10) |
				0x01 //PPU (5c77) chip version
			);
			_ppu1OpenBus = value;
			return value;
		}

		case 0x213F: {
			//STAT78 - PPU Status Flag and Version
			uint8_t value = (
				(_oddFrame ? 0x80 : 0) |
				(_locationLatched ? 0x40 : 0) |
				(_ppu2OpenBus & 0x20) |
				(_console->GetRegion() == ConsoleRegion::Pal ? 0x10 : 0) |
				0x02 //PPU (5c78) chip version
			);

			if(_regs->GetIoPortOutput() & 0x80) {
				_locationLatched = false;

				//"The high/low selector is reset to ÅelowÅf when $213F is read" (the selector is NOT reset when the counter is latched)
				_horizontalLocToggle = false;
				_verticalLocationToggle = false;
			}
			_ppu2OpenBus = value;
			return value;
		}

		default:
			MessageManager::Log("[Debug] Unimplemented register read: " + HexUtilities::ToHex(addr));
			break;
	}
	
	uint16_t reg = addr & 0x210F;
	if((reg >= 0x2104 && reg <= 0x2106) || (reg >= 0x2108 && reg <= 0x210A)) {
		//Registers matching $21x4-6 or $21x8-A (where x is 0-2) return the last value read from any of the PPU1 registers $2134-6, $2138-A, or $213E.
		return _ppu1OpenBus;
	}
	return _console->GetMemoryManager()->GetOpenBus();
}

void Ppu::Write(uint32_t addr, uint8_t value)
{
	if(_scanline < _vblankStart && _scanline > 0 && _memoryManager->GetHClock() >= 22*4 && _memoryManager->GetHClock() <= 278*4) {
		RenderScanline();
	}

	switch(addr) {
		case 0x2100:
			if(_forcedVblank && _scanline == _vblankStart) {
				//"writing this register on the first line of V-Blank (225 or 240, depending on overscan) when force blank is currently active causes the OAM Address Reset to occur."
				_internalOamAddress = (_oamRamAddress << 1);
			}

			_forcedVblank = (value & 0x80) != 0;
			_screenBrightness = value & 0x0F;
			break;

		case 0x2101:
			_oamMode = (value & 0xE0) >> 5;
			_oamBaseAddress = (value & 0x07) << 13;
			_oamAddressOffset = (((value & 0x18) >> 3) + 1) << 12;
			break;

		case 0x2102:
			_oamRamAddress = (_oamRamAddress & 0x100) | value;
			_internalOamAddress = (_oamRamAddress << 1);
			break;

		case 0x2103:
			_oamRamAddress = (_oamRamAddress & 0xFF) | ((value & 0x01) << 8);
			_internalOamAddress = (_oamRamAddress << 1);
			_enableOamPriority = (value & 0x80) != 0;
			break;

		case 0x2104: {
			//When trying to read/write during rendering, the internal address used by the PPU's sprite rendering is used
			//This is approximated by _oamRenderAddress (but is not cycle accurate) - needed for Uniracers
			uint16_t addr = (_forcedVblank || (_scanline >= _vblankStart)) ? _internalOamAddress : _oamRenderAddress;

			if(addr < 512) {
				if(addr & 0x01) {
					_console->ProcessPpuWrite(addr - 1, _oamWriteBuffer, SnesMemoryType::SpriteRam);
					_oamRam[addr - 1] = _oamWriteBuffer;
	
					_console->ProcessPpuWrite(addr, value, SnesMemoryType::SpriteRam);
					_oamRam[addr] = value;
				} else {
					_oamWriteBuffer = value;
				}
			} else {
				uint16_t address = 0x200 | (addr & 0x1F);
				if((addr & 0x01) == 0) {
					_oamWriteBuffer = value;
				}
				_console->ProcessPpuWrite(address, value, SnesMemoryType::SpriteRam);
				_oamRam[address] = value;
			}
			_internalOamAddress = (_internalOamAddress + 1) & 0x3FF;
			break;
		}

		case 0x2105:
			if(_bgMode != (value & 0x07)) {
				MessageManager::Log("[Debug] Entering mode: " + std::to_string(value & 0x07) + " (SL: " + std::to_string(_scanline) + ")");
			}
			_bgMode = value & 0x07;
			_mode1Bg3Priority = (value & 0x08) != 0;

			_layerConfig[0].LargeTiles = (value & 0x10) != 0;
			_layerConfig[1].LargeTiles = (value & 0x20) != 0;
			_layerConfig[2].LargeTiles = (value & 0x40) != 0;
			_layerConfig[3].LargeTiles = (value & 0x80) != 0;
			break;

		case 0x2106:
			//MOSAIC - Screen Pixelation
			_mosaicSize = ((value & 0xF0) >> 4) + 1;
			_mosaicEnabled = value & 0x0F;
			if(_mosaicEnabled) {
				//"If this register is set during the frame, the Åstarting scanline is the current scanline, otherwise it is the first visible scanline of the frame."
				_mosaicStartScanline = _scanline;
			}
			break;

		case 0x2107: case 0x2108: case 0x2109: case 0x210A:
			//BG 1-4 Tilemap Address and Size (BG1SC, BG2SC, BG3SC, BG4SC)
			_layerConfig[addr - 0x2107].TilemapAddress = (value & 0xFC) << 9;
			_layerConfig[addr - 0x2107].DoubleWidth = (value & 0x01) != 0;
			_layerConfig[addr - 0x2107].DoubleHeight = (value & 0x02) != 0;
			break;

		case 0x210B: case 0x210C:
			//BG1+2 / BG3+4 Chr Address (BG12NBA / BG34NBA)
			_layerConfig[(addr - 0x210B) * 2].ChrAddress = (value & 0x0F) << 13;
			_layerConfig[(addr - 0x210B) * 2 + 1].ChrAddress = (value & 0xF0) << 9;
			break;
		
		case 0x210D:
			//M7HOFS - Mode 7 BG Horizontal Scroll
			//BG1HOFS - BG1 Horizontal Scroll
			_mode7.HScroll = ((value << 8) | (_mode7.ValueLatch)) & 0x1FFF;
			_mode7.ValueLatch = value;
			//no break, keep executing to set the matching BG1 HScroll register, too

		case 0x210F: case 0x2111: case 0x2113:
			//BGXHOFS - BG1/2/3/4 Horizontal Scroll
			_layerConfig[(addr - 0x210D) >> 1].HScroll = ((value << 8) | (_hvScrollLatchValue & ~0x07) | (_hScrollLatchValue & 0x07)) & 0x3FF;
			_hvScrollLatchValue = value;
			_hScrollLatchValue = value;
			break;

		case 0x210E:
			//M7VOFS - Mode 7 BG Vertical Scroll
			//BG1VOFS - BG1 Vertical Scroll
			_mode7.VScroll = ((value << 8) | (_mode7.ValueLatch)) & 0x1FFF;
			_mode7.ValueLatch = value;
			//no break, keep executing to set the matching BG1 HScroll register, too

		case 0x2110: case 0x2112: case 0x2114:
			//BGXVOFS - BG1/2/3/4 Vertical Scroll
			_layerConfig[(addr - 0x210E) >> 1].VScroll = ((value << 8) | _hvScrollLatchValue) & 0x3FF;
			_hvScrollLatchValue = value;
			break;

		case 0x2115:
			//VMAIN - Video Port Control
			switch(value & 0x03) {
				case 0: _vramIncrementValue = 1; break;
				case 1: _vramIncrementValue = 32; break;
				
				case 2: 
				case 3: _vramIncrementValue = 128; break;
			}

			_vramAddressRemapping = (value & 0x0C) >> 2;
			_vramAddrIncrementOnSecondReg = (value & 0x80) != 0;
			break;

		case 0x2116:
			//VMADDL - VRAM Address low byte
			_vramAddress = (_vramAddress & 0x7F00) | value;
			UpdateVramReadBuffer();
			break;

		case 0x2117:
			//VMADDH - VRAM Address high byte
			_vramAddress = (_vramAddress & 0x00FF) | ((value & 0x7F) << 8);
			UpdateVramReadBuffer();
			break;

		case 0x2118:
			//VMDATAL - VRAM Data Write low byte
			if(_scanline >= _vblankStart || _forcedVblank) {
				//Only write the value if in vblank or forced blank (writes to VRAM outside vblank/forced blank are not allowed)
				_console->ProcessPpuWrite(GetVramAddress() << 1, value, SnesMemoryType::VideoRam);
				_vram[GetVramAddress() << 1] = value;
			}

			//The VRAM address is incremented even outside of vblank/forced blank
			if(!_vramAddrIncrementOnSecondReg) {
				_vramAddress = (_vramAddress + _vramIncrementValue) & 0x7FFF;
			}
			break;

		case 0x2119:
			//VMDATAH - VRAM Data Write high byte
			if(_scanline >= _vblankStart || _forcedVblank) {
				//Only write the value if in vblank or forced blank (writes to VRAM outside vblank/forced blank are not allowed)
				_console->ProcessPpuWrite((GetVramAddress() << 1) + 1, value, SnesMemoryType::VideoRam);
				_vram[(GetVramAddress() << 1) + 1] = value;
			}
			
			//The VRAM address is incremented even outside of vblank/forced blank
			if(_vramAddrIncrementOnSecondReg) {
				_vramAddress = (_vramAddress + _vramIncrementValue) & 0x7FFF;
			}
			break;

		case 0x211A:
			//M7SEL - Mode 7 Settings
			_mode7.LargeMap = (value & 0x80) != 0;
			_mode7.FillWithTile0 = (value & 0x40) != 0;
			_mode7.HorizontalMirroring = (value & 0x01) != 0;
			_mode7.VerticalMirroring = (value & 0x02) != 0;
			break;

		case 0x211B: case 0x211C: case 0x211D: case 0x211E:
			//M7A/B/C/D - Mode 7 Matrix A/B/C/D (A/B are also used with $2134/6)
			_mode7.Matrix[addr - 0x211B] = (value << 8) | _mode7.ValueLatch;
			_mode7.ValueLatch = value;
			break;
		
		case 0x211F:
			//M7X - Mode 7 Center X
			_mode7.CenterX = ((value << 8) | _mode7.ValueLatch);
			_mode7.ValueLatch = value;
			break;

		case 0x2120:
			//M7Y - Mode 7 Center Y
			_mode7.CenterY = ((value << 8) | _mode7.ValueLatch);
			_mode7.ValueLatch = value;
			break;

		case 0x2121:
			//CGRAM Address(CGADD)
			_cgramAddress = value * 2;
			break;

		case 0x2122: 
			//CGRAM Data write (CGDATA)
			if(_cgramAddress & 0x01) {
				//MSB ignores the 7th bit (colors are 15-bit only)
				_console->ProcessPpuWrite(_cgramAddress - 1, _cgramWriteBuffer, SnesMemoryType::CGRam);
				_cgram[_cgramAddress - 1] = _cgramWriteBuffer;

				_console->ProcessPpuWrite(_cgramAddress, value & 0x7F, SnesMemoryType::CGRam);
				_cgram[_cgramAddress] = value & 0x7F;
			} else {
				_cgramWriteBuffer = value;
			}
			_cgramAddress = (_cgramAddress + 1) & (Ppu::CgRamSize - 1);
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
			_window[0].Left = value;
			break;
		
		case 0x2127:
			//WH1 - Window 1 Right Position
			_window[0].Right = value;
			break;

		case 0x2128:
			//WH2 - Window 2 Left Position
			_window[1].Left = value;
			break;

		case 0x2129:
			//WH3 - Window 2 Right Position
			_window[1].Right = value;
			break;

		case 0x212A:
			//WBGLOG - Window mask logic for BG
			_maskLogic[0] = (WindowMaskLogic)(value & 0x03);
			_maskLogic[1] = (WindowMaskLogic)((value >> 2) & 0x03);
			_maskLogic[2] = (WindowMaskLogic)((value >> 4) & 0x03);
			_maskLogic[3] = (WindowMaskLogic)((value >> 6) & 0x03);
			break;

		case 0x212B:
			//WOBJLOG - Window mask logic for OBJs and Color Window
			_maskLogic[4] = (WindowMaskLogic)((value >> 0) & 0x03);
			_maskLogic[5] = (WindowMaskLogic)((value >> 2) & 0x03);
			break;

		case 0x212C:
			//TM - Main Screen Designation
			_mainScreenLayers = value & 0x1F;
			break;

		case 0x212D:
			//TS - Subscreen Designation
			_subScreenLayers = value & 0x1F;
			break;

		case 0x212E:
			//TMW - Window Mask Designation for the Main Screen
			for(int i = 0; i < 5; i++) {
				_windowMaskMain[i] = ((value >> i) & 0x01) != 0;
			}
			break;

		case 0x212F:
			//TSW - Window Mask Designation for the Subscreen
			for(int i = 0; i < 5; i++) {
				_windowMaskSub[i] = ((value >> i) & 0x01) != 0;
			}
			break;
		
		case 0x2130:
			//CGWSEL - Color Addition Select
			_colorMathClipMode = (ColorWindowMode)((value >> 6) & 0x03);
			_colorMathPreventMode = (ColorWindowMode)((value >> 4) & 0x03);
			_colorMathAddSubscreen = (value & 0x02) != 0;
			_directColorMode = (value & 0x01) != 0;
			break;

		case 0x2131:
			//CGADSUB - Color math designation
			_colorMathEnabled = value & 0x3F;
			_colorMathSubstractMode = (value & 0x80) != 0;
			_colorMathHalveResult = (value & 0x40) != 0;
			break;

		case 0x2132: 
			//COLDATA - Fixed Color Data
			if(value & 0x80) { //B
				_fixedColor = (_fixedColor & ~0x7C00) | ((value & 0x1F) << 10);
			}
			if(value & 0x40) { //G
				_fixedColor = (_fixedColor & ~0x3E0) | ((value & 0x1F) << 5);
			}
			if(value & 0x20) { //R
				_fixedColor = (_fixedColor & ~0x1F) | (value & 0x1F);
			}
			break;

		case 0x2133:
			//SETINI - Screen Mode/Video Select
			//_externalSync = (value & 0x80) != 0;  //NOT USED
			_mode7.ExtBgEnabled = (value & 0x40) != 0;
			_hiResMode = (value & 0x08) != 0;
			_overscanMode = (value & 0x04) != 0;
			_vblankStart = _overscanMode ? 240 : 225;
			_objInterlace = (value & 0x02) != 0;
			_screenInterlace = (value & 0x01) != 0;
			break;

		default:
			MessageManager::Log("[Debug] Unimplemented register write: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

void Ppu::Serialize(Serializer &s)
{
	s.Stream(
		_forcedVblank, _screenBrightness, _scanline, _frameCount, _drawStartX, _drawEndX, _bgMode,
		_mode1Bg3Priority, _mainScreenLayers, _subScreenLayers, _vramAddress, _vramIncrementValue, _vramAddressRemapping,
		_vramAddrIncrementOnSecondReg, _vramReadBuffer, _ppu1OpenBus, _ppu2OpenBus, _cgramAddress, _mosaicSize, _mosaicEnabled,
		_mosaicStartScanline, _oamMode, _oamBaseAddress, _oamAddressOffset, _oamRamAddress, _enableOamPriority,
		_internalOamAddress, _oamWriteBuffer, _timeOver, _rangeOver, _hiResMode, _screenInterlace, _objInterlace,
		_overscanMode, _directColorMode, _colorMathClipMode, _colorMathPreventMode, _colorMathAddSubscreen, _colorMathEnabled,
		_colorMathSubstractMode, _colorMathHalveResult, _fixedColor, _hvScrollLatchValue, _hScrollLatchValue, 
		_horizontalLocation, _horizontalLocToggle, _verticalLocation, _verticalLocationToggle, _locationLatched,
		_maskLogic[0], _maskLogic[1], _maskLogic[2], _maskLogic[3], _maskLogic[4], _maskLogic[5],
		_windowMaskMain[0], _windowMaskMain[1], _windowMaskMain[2], _windowMaskMain[3], _windowMaskMain[4],
		_windowMaskSub[0], _windowMaskSub[1], _windowMaskSub[2], _windowMaskSub[3], _windowMaskSub[4],
		_mode7.CenterX, _mode7.CenterY, _mode7.ExtBgEnabled, _mode7.FillWithTile0, _mode7.HorizontalMirroring,
		_mode7.HScroll, _mode7.LargeMap, _mode7.Matrix[0], _mode7.Matrix[1], _mode7.Matrix[2], _mode7.Matrix[3],
		_mode7.ValueLatch, _mode7.VerticalMirroring, _mode7.VScroll, _oamRenderAddress, _oddFrame, _vblankStart
	);

	for(int i = 0; i < 4; i++) {
		s.Stream(
			_layerConfig[i].ChrAddress, _layerConfig[i].DoubleHeight, _layerConfig[i].DoubleWidth, _layerConfig[i].HScroll,
			_layerConfig[i].LargeTiles, _layerConfig[i].TilemapAddress, _layerConfig[i].VScroll
		);
	}

	for(int i = 0; i < 2; i++) {
		s.Stream(
			_window[i].ActiveLayers[0], _window[i].ActiveLayers[1], _window[i].ActiveLayers[2], _window[i].ActiveLayers[3], _window[i].ActiveLayers[4], _window[i].ActiveLayers[5],
			_window[i].InvertedLayers[0], _window[i].InvertedLayers[1], _window[i].InvertedLayers[2], _window[i].InvertedLayers[3], _window[i].InvertedLayers[4], _window[i].InvertedLayers[5],
			_window[i].Left, _window[i].Right
		);
	}

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 33; j++) {
			s.Stream(
				_layerData[i].Tiles[j].ChrData[0], _layerData[i].Tiles[j].ChrData[1], _layerData[i].Tiles[j].ChrData[2], _layerData[i].Tiles[j].ChrData[3],
				_layerData[i].Tiles[j].TilemapData, _layerData[i].Tiles[j].VScroll
			);
		}
	}
	s.Stream(_hOffset, _vOffset, _fetchStartX, _fetchEndX);

	s.StreamArray(_vram, Ppu::VideoRamSize);
	s.StreamArray(_oamRam, Ppu::SpriteRamSize);
	s.StreamArray(_cgram, Ppu::CgRamSize);
}

/* Everything below this point is used to select the proper arguments for templates */
template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset, bool hiResMode, uint8_t activeWindowCount, bool applyMosaic>
void Ppu::RenderTilemap()
{
	if(_directColorMode) {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, activeWindowCount, applyMosaic, true>();
	} else {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, activeWindowCount, applyMosaic, false>();
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset, bool hiResMode, uint8_t activeWindowCount>
void Ppu::RenderTilemap()
{
	bool applyMosaic = forMainScreen && ((_mosaicEnabled >> layerIndex) & 0x01) != 0 && (_mosaicSize > 1 || _bgMode == 5 || _bgMode == 6);

	if(applyMosaic) {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, activeWindowCount, true>();
	} else {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, activeWindowCount, false>();
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset, bool hiResMode>
void Ppu::RenderTilemap()
{
	uint8_t activeWindowCount = 0;
	if((forMainScreen && _windowMaskMain[layerIndex]) || (!forMainScreen && _windowMaskSub[layerIndex])) {
		activeWindowCount = (uint8_t)_window[0].ActiveLayers[layerIndex] + (uint8_t)_window[1].ActiveLayers[layerIndex];
	}

	if(activeWindowCount == 0) {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, 0>();
	} else if(activeWindowCount == 1) {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, 1>();
	} else {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, 2>();
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset>
void Ppu::RenderTilemap()
{
	if(_bgMode == 5 || _bgMode == 6) {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, true>();
	} else {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, false>();
	}
}

template<uint8_t layerIndex, bool forMainScreen, bool processHighPriority>
void Ppu::RenderTilemapMode7()
{
	bool applyMosaic = forMainScreen && ((_mosaicEnabled >> layerIndex) & 0x01) != 0;

	if(applyMosaic) {
		RenderTilemapMode7<layerIndex, forMainScreen, processHighPriority, true>();
	} else {
		RenderTilemapMode7<layerIndex, forMainScreen, processHighPriority, false>();
	}
}

template<uint8_t layerIndex, bool forMainScreen, bool processHighPriority, bool applyMosaic>
void Ppu::RenderTilemapMode7()
{
	if(_directColorMode) {
		RenderTilemapMode7<layerIndex, forMainScreen, processHighPriority, applyMosaic, true>();
	} else {
		RenderTilemapMode7<layerIndex, forMainScreen, processHighPriority, applyMosaic, false>();
	}
}
