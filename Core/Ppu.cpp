#include "stdafx.h"
#include "Ppu.h"
#include "Console.h"
#include "MemoryManager.h"
#include "Cpu.h"
#include "Spc.h"
#include "InternalRegisters.h"
#include "ControlManager.h"
#include "VideoDecoder.h"
#include "NotificationManager.h"
#include "DmaController.h"
#include "MessageManager.h"
#include "../Utilities/HexUtilities.h"

Ppu::Ppu(shared_ptr<Console> console)
{
	_console = console;
	_regs = console->GetInternalRegisters();

	_outputBuffers[0] = new uint16_t[512 * 478];
	_outputBuffers[1] = new uint16_t[512 * 478];
	memset(_outputBuffers[0], 0, 512 * 478);
	memset(_outputBuffers[1], 0, 512 * 478);

	_currentBuffer = _outputBuffers[0];

	_layerConfig[0] = {};
	_layerConfig[1] = {};
	_layerConfig[2] = {};
	_layerConfig[3] = {};

	_cgramAddress = 0;

	_vram = new uint8_t[Ppu::VideoRamSize];
	memset(_vram, 0, Ppu::VideoRamSize);
	memset(_oamRam, 0, Ppu::SpriteRamSize);
	memset(_cgram, 0, Ppu::CgRamSize);

	_vramAddress = 0;
	_vramIncrementValue = 1;
	_vramAddressRemapping = 0;
	_vramAddrIncrementOnSecondReg = false;
}

Ppu::~Ppu()
{
	delete[] _vram;
	delete[] _outputBuffers[0];
	delete[] _outputBuffers[1];
}

uint32_t Ppu::GetFrameCount()
{
	return _frameCount;
}

PpuState Ppu::GetState()
{
	return {
		_cycle,
		_scanline,
		_frameCount,
		_overscanMode
	};
}

void Ppu::Exec()
{
	if(_cycle == 340) {
		_cycle = -1;
		_scanline++;

		if(_scanline == (_overscanMode ? 240 : 225)) {
			//Reset OAM address at the start of vblank?
			if(!_forcedVblank) {
				_internalOamAddress = (_oamRamAddress << 1);
			}

			_frameCount++;
			_console->GetSpc()->ProcessEndFrame();
			_console->GetControlManager()->UpdateInputState();
			_regs->ProcessAutoJoypadRead();
			_regs->SetNmiFlag(true);
			SendFrame();

			if(_regs->IsNmiEnabled()) {
				_console->GetCpu()->SetNmiFlag();
			}
		} else if(_scanline == 240 && _cycle == 0 && _frameCount & 0x01) {
			//Skip 1 tick every other frame
			_cycle++;
		} else if(_scanline == 261) {
			_regs->SetNmiFlag(false);
			_scanline = 0;
			_rangeOver = false;
			_timeOver = false;

			if(_mosaicEnabled) {
				_mosaicStartScanline = 0;
			}
			_console->GetDmaController()->InitHdmaChannels();
		}

		if(_regs->IsVerticalIrqEnabled() && !_regs->IsHorizontalIrqEnabled() && _scanline == _regs->GetVerticalTimer()) {
			//An IRQ will occur sometime just after the V Counter reaches the value set in $4209/$420A.
			_console->GetCpu()->SetIrqSource(IrqSource::Ppu);
		}
	}

	if(_regs->IsHorizontalIrqEnabled() && _cycle == _regs->GetHorizontalTimer() && (!_regs->IsVerticalIrqEnabled() || _scanline == _regs->GetVerticalTimer())) {
		//An IRQ will occur sometime just after the H Counter reaches the value set in $4207/$4208.
		_console->GetCpu()->SetIrqSource(IrqSource::Ppu);
	}

	_cycle++;

	if(_cycle == 278 && _scanline <= (_overscanMode ? 239 : 224)) {
		if(_scanline > 0) {
			RenderScanline();
		}
		_console->GetDmaController()->ProcessHdmaChannels();
	} else if(_cycle == 134) {
		//TODO Approximation
		_console->GetMemoryManager()->IncrementMasterClockValue<40>();
	}
}

void Ppu::EvaluateNextLineSprites()
{
	memset(_spritePriority, 0xFF, sizeof(_spritePriority));
	memset(_spritePixels, 0xFF, sizeof(_spritePixels));
	memset(_spritePalette, 0, sizeof(_spritePalette));
	_spriteCount = 0;
	uint16_t totalWidth = 0;
	uint16_t screenY = _objInterlace ? ((_frameCount & 0x01) ? ((_scanline << 1) + 1) : (_scanline << 1)) : _scanline;

	for(int i = 0; i < 512; i += 4) {
		uint8_t y = _oamRam[i + 1];

		uint8_t highTableOffset = i >> 4;
		uint8_t shift = ((i >> 2) & 0x03) << 1;
		uint8_t highTableValue = _oamRam[0x200 | highTableOffset] >> shift;
		uint8_t largeSprite = (highTableValue & 0x02) >> 1;
		uint8_t height = _oamSizes[_oamMode][largeSprite][1] << 3;

		if(y > screenY || y + height <= screenY) {
			//Not visible on this scanline
			continue;
		}

		SpriteInfo &info = _sprites[_spriteCount];
		info.LargeSprite = largeSprite;
		uint8_t width = _oamSizes[_oamMode][info.LargeSprite][0] << 3;
		uint16_t sign = (highTableValue & 0x01) << 8;
		info.X = (int16_t)((sign | _oamRam[i]) << 7) >> 7;
		
		if(info.X != -256 && info.X + width <= 0 || info.X > 255) {
			//Sprite is not visible (and must be ignored for time/range flag calculations)
			//Sprites at X=-256 are always used when considering Time/Range flag calculations, but not actually drawn.
			continue;
		}

		info.TileRow = (_oamRam[i + 2] & 0xF0) >> 4;
		info.TileColumn = _oamRam[i + 2] & 0x0F;

		uint8_t flags = _oamRam[i + 3];
		info.UseSecondTable = (flags & 0x01) != 0;
		info.Palette = (flags >> 1) & 0x07;
		info.Priority = (flags >> 4) & 0x03;
		info.HorizontalMirror = (flags & 0x40) != 0;
		info.VerticalMirror = (flags & 0x80) != 0;

		uint8_t yOffset;
		int rowOffset;
		if(info.VerticalMirror) {
			yOffset = (height - 1 - (screenY - y)) & 0x07;
			rowOffset = (height - 1 - (screenY - y)) >> 3;
		} else {
			yOffset = (screenY - y) & 0x07;
			rowOffset = (screenY - y) >> 3;
		}

		uint8_t row = (info.TileRow + rowOffset) & 0x0F;
		for(int x = std::max<int16_t>(info.X, 0); x < info.X + width && x < 256; x++) {
			if(_spritePixels[x] == 0xFFFF) {
				uint8_t xOffset;
				int columnOffset;
				if(info.HorizontalMirror) {
					xOffset = (width - (x - info.X) - 1) & 0x07;
					columnOffset = (width - (x - info.X) - 1) >> 3;
				} else {
					xOffset = (x - info.X) & 0x07;
					columnOffset = (x - info.X) >> 3;
				}

				uint8_t column = (info.TileColumn + columnOffset) & 0x0F;
				uint8_t tileIndex = (row << 4) | column;
				uint16_t tileStart = ((_oamBaseAddress + (tileIndex << 4) + (info.UseSecondTable ? _oamAddressOffset : 0)) & 0x7FFF) << 1;

				uint16_t color = GetTilePixelColor<4>(tileStart + yOffset * 2, 7 - xOffset);

				if(color != 0) {
					uint16_t paletteRamOffset = 256 + (((info.Palette << 4) + color) << 1);
					_spritePixels[x] = _cgram[paletteRamOffset] | (_cgram[paletteRamOffset + 1] << 8);
					_spritePriority[x] = info.Priority;
					_spritePalette[x] = info.Palette;
				}
			}
		}

		totalWidth += width;
		if(totalWidth > 34 * 8) {
			_timeOver = true;
		}

		_spriteCount++;
		if(_spriteCount > 32) {
			_rangeOver = true;
		}

		if(_timeOver || _rangeOver) {
			break;
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
	RenderSprites<0, true>();
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
	RenderTilemap<1, 4, true, forMainScreen>();
	RenderSprites<1, forMainScreen>();
	RenderTilemap<0, 8, false, forMainScreen>();
	RenderSprites<0, forMainScreen>();
	RenderTilemap<1, 4, false, forMainScreen>();
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
	_pixelsDrawn = 0;
	_subPixelsDrawn = 0;
	memset(_rowPixelFlags, 0, sizeof(_rowPixelFlags));
	memset(_subScreenFilled, 0, sizeof(_subScreenFilled));

	if(_forcedVblank) {
		RenderBgColor<true>();
		RenderBgColor<false>();
		ApplyHiResMode();
		return;
	}

	switch(_bgMode) {
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
	}

	ApplyColorMath();
	ApplyBrightness<true>();
	ApplyHiResMode();
	
	//Process sprites for next scanline
	EvaluateNextLineSprites();	
}

template<bool forMainScreen>
void Ppu::RenderBgColor()
{
	if((forMainScreen && _pixelsDrawn == 256) || (!forMainScreen && _subPixelsDrawn == 256)) {
		return;
	}

	uint16_t bgColor = _cgram[0] | (_cgram[1] << 8);
	for(int x = 0; x < 256; x++) {
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
		for(int x = 0; x < 256; x++) {
			if(!_rowPixelFlags[x] && _spritePriority[x] == priority) {
				if(activeWindowCount && ProcessMaskWindow<Ppu::SpriteLayerIndex>(activeWindowCount, x)) {
					//This pixel was masked
					continue;
				}

				_mainScreenBuffer[x] = _spritePixels[x];
				_rowPixelFlags[x] |= PixelFlags::Filled | (((_colorMathEnabled & 0x10) && _spritePalette[x] > 3) ? PixelFlags::AllowColorMath : 0);
			}
		}
	} else {
		for(int x = 0; x < 256; x++) {
			if(!_subScreenFilled[x] && _spritePriority[x] == priority) {
				if(activeWindowCount && ProcessMaskWindow<Ppu::SpriteLayerIndex>(activeWindowCount, x)) {
					//This pixel was masked
					continue;
				}

				_subScreenBuffer[x] = _spritePixels[x];
				_subScreenFilled[x] = true;
			}
		}
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset, bool hiResMode, bool largeTileWidth, bool largeTileHeight, uint8_t activeWindowCount, bool applyMosaic, bool directColorMode>
void Ppu::RenderTilemap()
{
	if(!IsRenderRequired<forMainScreen>(layerIndex)) {
		return;
	}

	/* Current scanline (in interlaced mode, switches between even and odd rows every frame */
	uint16_t realY = _screenInterlace ? ((_frameCount & 0x01) ? ((_scanline << 1) + 1) : (_scanline << 1)) : _scanline;

	/* True when the entire scanline has to be replaced by a mosaic pattern */
	bool mosaicScanline = applyMosaic && (realY - _mosaicStartScanline) % _mosaicSize != 0;

	/* Keeps track of whether or not the pixel is allowed to participate in color math */
	uint8_t pixelFlags = PixelFlags::Filled | (((_colorMathEnabled >> layerIndex) & 0x01) ? PixelFlags::AllowColorMath : 0);

	/* The current layer's options */
	LayerConfig &config = _layerConfig[layerIndex];

	/* Layer's tilemap start address */
	uint16_t tilemapAddr = config.TilemapAddress >> 1;

	/* Layer's CHR data start address */
	uint16_t chrAddr = config.ChrAddress;

	/* The current row of tiles (e.g scanlines 16-23 is row 2) */
	uint16_t row = (realY + config.VScroll) >> (largeTileHeight ? 4 : 3);

	/* The vertical offset to read in the tile we're processing */
	uint8_t baseYOffset = (realY + config.VScroll) & 0x07;

	/* Tilemap offset based on the current row & tilemap size options */
	uint16_t addrVerticalScrollingOffset = config.VerticalMirroring ? ((row & 0x20) << (config.HorizontalMirroring ? 6 : 5)) : 0;

	/* The start address for tiles on this row */
	uint16_t baseOffset = tilemapAddr + addrVerticalScrollingOffset + ((row & 0x1F) << 5);

	uint16_t vScroll = config.VScroll;
	uint16_t hScroll = config.HScroll;

	//"Offset per tile" mode (modes 2, 4 and 6 support this)
	bool offsetPerTileMode = (_bgMode & 0x03) == 2;

	/* The current pixel x position (normally 0-255, but 0-511 in hi-res mode - even on subscreen, odd on main screen) */
	uint16_t realX;

	/* The current column index (in terms of 8x8 or 16x16 tiles) */
	uint16_t column;

	/* The tilemap address to read the tile data from */
	uint16_t addr;

	for(int x = 0; x < 256; x++) {
		if(hiResMode) {
			realX = (x << 1) + (forMainScreen ? 1 : 0);
		} else {
			realX = x;
		}

		if(offsetPerTileMode) {
			ProcessOffsetMode<layerIndex, largeTileWidth, largeTileHeight>(x, realX, realY, hScroll, vScroll, addr);
			
			//Need to recalculate this because vScroll may change from one tile to another
			baseYOffset = (realY + vScroll) & 0x07;
		} else {
			column = (realX + hScroll) >> (largeTileWidth ? 4 : 3);
			addr = (baseOffset + (column & 0x1F) + (config.HorizontalMirroring ? ((column & 0x20) << 5) : 0)) << 1;
		}

		//Skip pixels that were filled by previous layers (or that don't match the priority level currently being processed)
		if(forMainScreen) {
			if(_rowPixelFlags[x] || ((uint8_t)processHighPriority != ((_vram[addr + 1] & 0x20) >> 5))) {
				continue;
			}
		} else {
			if(_subScreenFilled[x] || ((uint8_t)processHighPriority != ((_vram[addr + 1] & 0x20) >> 5))) {
				continue;
			}
		}

		if(activeWindowCount && ProcessMaskWindow<layerIndex>(activeWindowCount, x)) {
			//This pixel was masked, skip it
			continue;
		}

		if(applyMosaic && (mosaicScanline || x % _mosaicSize != 0)) {
			//If this is not the top-left pixels in the mosaic pattern, override it with the top-left pixel data
			_mainScreenBuffer[x] = _mosaicColor[x];
			_rowPixelFlags[x] = pixelFlags;
			if(forMainScreen) {
				_pixelsDrawn++;
			} else {
				_subPixelsDrawn++;
			}
			continue;
		}

		//The pixel is empty, not clipped and not part of a mosaic pattern, process it
		bool vMirror = (_vram[addr + 1] & 0x80) != 0;
		bool hMirror = (_vram[addr + 1] & 0x40) != 0;

		uint16_t tileIndex = ((_vram[addr + 1] & 0x03) << 8) | _vram[addr];
		if(largeTileWidth || largeTileHeight) {
			tileIndex = (
				tileIndex +
				(largeTileHeight ? (((realY + vScroll) & 0x08) ? (vMirror ? 0 : 16) : (vMirror ? 16 : 0)) : 0) +
				(largeTileWidth ? (((realX + hScroll) & 0x08) ? (hMirror ? 0 : 1) : (hMirror ? 1 : 0)) : 0)
			) & 0x3FF;
		}

		uint16_t tileStart = chrAddr + tileIndex * 8 * bpp;

		uint8_t yOffset = vMirror ? (7 - baseYOffset) : baseYOffset;
		uint16_t pixelStart = tileStart + yOffset * 2;
		
		uint8_t xOffset = (realX + hScroll) & 0x07;
		uint8_t shift = hMirror ? xOffset : (7 - xOffset);
		
		uint16_t color = GetTilePixelColor<bpp>(pixelStart, shift);

		if(color > 0) {
			/* Ignore palette bits for 256-color layers */
			uint16_t paletteColor;

			if(bpp == 8 && directColorMode) {
				uint8_t palette = (_vram[addr + 1] >> 2) & 0x07;
				paletteColor = (
					(((color & 0x07) | (palette & 0x01)) << 1) |
					(((color & 0x38) | ((palette & 0x02) << 1)) << 3) |
					(((color & 0xC0) | ((palette & 0x04) << 3)) << 7)
				);
			} else {
				uint8_t palette = bpp == 8 ? 0 : (_vram[addr + 1] >> 2) & 0x07;
				uint16_t paletteRamOffset = basePaletteOffset + (palette * (1 << bpp) + color) * 2;
				paletteColor = _cgram[paletteRamOffset] | (_cgram[paletteRamOffset + 1] << 8);
			}

			if(forMainScreen) {
				DrawMainPixel<applyMosaic>(x, paletteColor, pixelFlags);
			} else {
				DrawSubPixel(x, paletteColor);
			}
		}
	}
}

template<uint8_t layerIndex, bool largeTileWidth, bool largeTileHeight>
void Ppu::ProcessOffsetMode(uint8_t x, uint16_t realX, uint16_t realY, uint16_t &hScroll, uint16_t &vScroll, uint16_t &addr)
{
	constexpr uint16_t enableBit = layerIndex == 0 ? 0x2000 : 0x4000;
	LayerConfig &config = _layerConfig[layerIndex];
	
	hScroll = config.HScroll;
	vScroll = config.VScroll;

	//TODO: Check+fix behavior with 16x16 tiles
	//TODO: Test mode 4/6 behavior

	if((realX + hScroll) & ~0x07) {
		//For all tiles after the first tile on the row, check if an active offset exists and use it
		uint16_t columnOffset = (((x - 8) & ~0x07) + (_layerConfig[2].HScroll & ~0x07)) >> 3;
		uint16_t rowOffset = (_layerConfig[2].VScroll >> 3);

		uint16_t hOffsetAddr = _layerConfig[2].TilemapAddress + (columnOffset << 1) + (rowOffset << 6);

		if(_bgMode == 4) {
			int16_t offsetValue = _vram[hOffsetAddr] | (_vram[hOffsetAddr + 1] << 8);

			if((offsetValue & 0x8000) == 0 && (offsetValue & enableBit)) {
				hScroll = (hScroll & 0x07) | ((x & ~0x07) + (offsetValue & 0x3F8));
			}
			if((offsetValue & 0x8000) != 0 && (offsetValue & enableBit)) {
				vScroll = (offsetValue & 0x3FF);
			}
		} else {
			uint16_t vOffsetAddr = hOffsetAddr + 0x40;

			int16_t hOffsetValue = _vram[hOffsetAddr] | (_vram[hOffsetAddr + 1] << 8);
			int16_t vOffsetValue = _vram[vOffsetAddr] | (_vram[vOffsetAddr + 1] << 8);

			if(hOffsetValue & enableBit) {
				hScroll = (hScroll & 0x07) | ((x & ~0x07) + (hOffsetValue & 0x3F8));
			}
			if(vOffsetValue & enableBit) {
				vScroll = (vOffsetValue & 0x3FF);
			}
		}
	}

	//Recalculate the tile's address based on the new scroll offsets
	uint16_t tilemapAddr = config.TilemapAddress >> 1;
	uint16_t offsetModeRow = (realY + vScroll) >> (largeTileHeight ? 4 : 3);
	uint16_t offsetModeColumn = (realX + hScroll) >> (largeTileWidth ? 4 : 3);

	uint16_t addrVerticalScrollingOffset = config.VerticalMirroring ? ((offsetModeRow & 0x20) << (config.HorizontalMirroring ? 6 : 5)) : 0;
	uint16_t offsetModeBaseAddress = tilemapAddr + addrVerticalScrollingOffset + ((offsetModeRow & 0x1F) << 5);
	addr = (offsetModeBaseAddress + (offsetModeColumn & 0x1F) + (config.HorizontalMirroring ? ((offsetModeColumn & 0x20) << 5) : 0)) << 1;
}

template<bool forMainScreen>
bool Ppu::IsRenderRequired(uint8_t layerIndex)
{
	if(forMainScreen) {
		if(_pixelsDrawn == 256 || ((_mainScreenLayers >> layerIndex) & 0x01) == 0) {
			//This screen is disabled, or we've drawn all pixels already
			return false;
		}
	} else {
		if(_subPixelsDrawn == 256 || ((_subScreenLayers >> layerIndex) & 0x01) == 0) {
			//This screen is disabled, or we've drawn all pixels already
			return false;
		}
	}
	return true;
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

	constexpr auto clip = [](int32_t val) { return (val & 0x2000) ? (val | ~0x3ff) : (val & 0x3ff); };

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

	uint8_t pixelFlags = PixelFlags::Filled | (((_colorMathEnabled >> layerIndex) & 0x01) ? PixelFlags::AllowColorMath : 0);

	for(int x = 0; x < 256; x++) {
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
				DrawMainPixel<applyMosaic>(x, paletteColor, pixelFlags);
			} else {
				DrawSubPixel(x, paletteColor);
			}
		}
	}
}

template<bool applyMosaic>
void Ppu::DrawMainPixel(uint8_t x, uint16_t color, uint8_t flags)
{
	_mainScreenBuffer[x] = color;
	_rowPixelFlags[x] = flags;

	if(applyMosaic && x % _mosaicSize == 0) {
		//This is the source for the mosaic pattern, store it for use in the next scanlines
		for(int i = 0; i < _mosaicSize && x + i < 256; i++) {
			_mosaicColor[x + i] = color;
		}
	}

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
	if(!_colorMathEnabled) {
		return;
	}

	uint8_t activeWindowCount = (uint8_t)_window[0].ActiveLayers[Ppu::ColorWindowIndex] + (uint8_t)_window[1].ActiveLayers[Ppu::ColorWindowIndex];

	for(int x = 0; x < 256; x++) {
		if(_rowPixelFlags[x] & PixelFlags::AllowColorMath) {
			uint8_t halfShift = _colorMathHalveResult ? 1 : 0;
			uint16_t &mainPixel = _mainScreenBuffer[x];

			bool isInsideWindow = activeWindowCount && ProcessMaskWindow<Ppu::ColorWindowIndex>(activeWindowCount, x);
			//Set color to black as needed based on clip mode
			switch(_colorMathClipMode) {
				default:
				case ColorWindowMode::Never: break;

				case ColorWindowMode::OutsideWindow: 
					if(!isInsideWindow) {
						mainPixel = 0;
						halfShift = 0;
					}
					break;

				case ColorWindowMode::InsideWindow:
					if(isInsideWindow) {
						mainPixel = 0;
						halfShift = 0;
					}
					break;

				case ColorWindowMode::Always: mainPixel = 0; break;
			}

			//Prevent color math as needed based on mode
			switch(_colorMathPreventMode) {
				default:
				case ColorWindowMode::Never: break;

				case ColorWindowMode::OutsideWindow:
					if(!isInsideWindow) {
						continue;
					}
					break;

				case ColorWindowMode::InsideWindow:
					if(isInsideWindow) {
						continue;
					}
					break;

				case ColorWindowMode::Always: continue;
			}

			uint16_t otherPixel;
			if(_colorMathAddSubscreen) {
				if(_subScreenFilled[x]) {
					otherPixel = _subScreenBuffer[x];
				} else {
					//there's nothing in the subscreen at this pixel, use the fixed color and disable halve operation
					otherPixel = _fixedColor;
					halfShift = 0;
				}
			} else {
				otherPixel = _fixedColor;
			}

			if(_colorMathSubstractMode) {
				uint16_t r = std::max((mainPixel & 0x001F) - (otherPixel & 0x001F), 0) >> halfShift;
				uint16_t g = std::max(((mainPixel >> 5) & 0x001F) - ((otherPixel >> 5) & 0x001F), 0) >> halfShift;
				uint16_t b = std::max(((mainPixel >> 10) & 0x001F) - ((otherPixel >> 10) & 0x001F), 0) >> halfShift;

				mainPixel = r | (g << 5) | (b << 10);
			} else {
				uint16_t r = std::min(((mainPixel & 0x001F) + (otherPixel & 0x001F)) >> halfShift, 0x1F);
				uint16_t g = std::min((((mainPixel >> 5) & 0x001F) + ((otherPixel >> 5) & 0x001F)) >> halfShift, 0x1F);
				uint16_t b = std::min((((mainPixel >> 10) & 0x001F) + ((otherPixel >> 10) & 0x001F)) >> halfShift, 0x1F);

				mainPixel = r | (g << 5) | (b << 10);
			}
		}
	}
}

template<bool forMainScreen>
void Ppu::ApplyBrightness()
{
	if(_screenBrightness != 15) {
		for(int x = 0; x < 256; x++) {
			uint16_t &pixel = (forMainScreen ? _mainScreenBuffer : _subScreenBuffer)[x];
			uint16_t r = (pixel & 0x1F) * _screenBrightness / 15;
			uint16_t g = ((pixel >> 5) & 0x1F) * _screenBrightness / 15;
			uint16_t b = ((pixel >> 10) & 0x1F) * _screenBrightness / 15;
			pixel = r | (g << 5) | (b << 10);
		}
	}
}

void Ppu::ApplyHiResMode()
{
	uint16_t scanline = _scanline - 1;
	uint32_t screenY = _screenInterlace ? ((_frameCount & 0x01) ? ((scanline << 1) + 1) : (scanline << 1)) : scanline;

	if(_hiResMode || _bgMode == 5 || _bgMode == 6) {
		ApplyBrightness<false>();
		for(int i = 0; i < 512; i += 2) {
			_currentBuffer[(screenY << 9) + i] = _subScreenBuffer[i >> 1];
			_currentBuffer[(screenY << 9) + i + 1] = _mainScreenBuffer[i >> 1];
		}
	} else {
		memcpy(_currentBuffer + (screenY << 8), _mainScreenBuffer, sizeof(_mainScreenBuffer));
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
	_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);

	uint16_t width;
	uint16_t height;
	if(_hiResMode || _bgMode == 5 || _bgMode == 6) {
		width = 512;
	} else {
		width = 256;
	}

	if(_screenInterlace && (_bgMode == 5 || _bgMode == 6)) {
		height = _overscanMode ? 478 : 448;
	} else {
		height = _overscanMode ? 239 : 224;
	}

	if(_screenInterlace) {
		_console->GetVideoDecoder()->UpdateFrameSync(_currentBuffer, width, height, _frameCount);
	} else {
		_console->GetVideoDecoder()->UpdateFrame(_currentBuffer, width, height, _frameCount);
		_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
	}
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

void Ppu::LatchLocationValues()
{
	_horizontalLocation = _cycle;
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
		case 0x2134: return ((int16_t)_mode7.Matrix[0] * ((int16_t)_mode7.Matrix[1] >> 8)) & 0xFF;
		case 0x2135: return (((int16_t)_mode7.Matrix[0] * ((int16_t)_mode7.Matrix[1] >> 8)) >> 8) & 0xFF;
		case 0x2136: return (((int16_t)_mode7.Matrix[0] * ((int16_t)_mode7.Matrix[1] >> 8)) >> 16) & 0xFF;

		case 0x2137:
			//SLHV - Software Latch for H/V Counter
			//Latch values on read, and return open bus
			LatchLocationValues();
			break;
			
		case 0x2138: {
			//OAMDATAREAD - Data for OAM read
			uint8_t value;
			if(_internalOamAddress < 512) {
				value = _oamRam[_internalOamAddress];
				_console->ProcessPpuRead(_internalOamAddress, value, SnesMemoryType::SpriteRam);
			} else {
				value = _oamRam[0x200 | (_internalOamAddress & 0x1F)];
				_console->ProcessPpuRead(0x200 | (_internalOamAddress & 0x1F), value, SnesMemoryType::SpriteRam);
			}
			_internalOamAddress = (_internalOamAddress + 1) & 0x3FF;
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
			return returnValue;
		}

		case 0x213B: {
			//CGDATAREAD - CGRAM Data read
			uint8_t value = _cgram[_cgramAddress];
			_console->ProcessPpuRead(_cgramAddress, value, SnesMemoryType::CGRam);
			_cgramAddress = (_cgramAddress + 1) & (Ppu::CgRamSize - 1);
			return value;
		}

		case 0x213C: {
			//OPHCT - Horizontal Scanline Location
			uint8_t value;
			if(_horizontalLocToggle) {
				//"Note that the value read is only 9 bits: bits 1-7 of the high byte are PPU2 Open Bus."
				value = ((_horizontalLocation & 0x100) >> 8) | ((addr >> 8) & 0xFE);
			} else {
				value = _horizontalLocation & 0xFF;
			}
			_horizontalLocToggle = !_horizontalLocToggle;
			return value;
		}

		case 0x213D: {
			//OPVCT - Vertical Scanline Location
			uint8_t value;
			if(_verticalLocationToggle) {
				//"Note that the value read is only 9 bits: bits 1-7 of the high byte are PPU2 Open Bus."
				value = ((_verticalLocation & 0x100) >> 8) | ((addr >> 8) & 0xFE);
			} else {
				value = _verticalLocation & 0xFF;
			}
			_verticalLocationToggle = !_verticalLocationToggle;
			return value;
		}

		case 0x213E:
			//STAT77 - PPU Status Flag and Version
			//TODO open bus on bit 4

			return (
				(_timeOver ? 0x80 : 0) |
				(_rangeOver ? 0x40 : 0) |
				0x01 //PPU (5c77) chip version
			);

		case 0x213F: {
			//STAT78 - PPU Status Flag and Version
			//TODO open bus on bit 5
			uint8_t value = (
				((_frameCount & 0x01) ? 0x80 : 0) |
				(_locationLatched ? 0x40 : 0) |
				//TODO (_isPal ? 0x10 : 0)
				0x02 //PPU (5c78) chip version
			);

			if(_regs->GetIoPortOutput() & 0x80) {
				_locationLatched = false;

				//"The high/low selector is reset to ÅelowÅf when $213F is read" (the selector is NOT reset when the counter is latched)
				_horizontalLocToggle = false;
				_verticalLocationToggle = false;
			}

			return value;
		}

		default:
			MessageManager::DisplayMessage("Debug", "Unimplemented register read: " + HexUtilities::ToHex(addr));
			break;
	}

	return addr >> 8;
}

void Ppu::Write(uint32_t addr, uint8_t value)
{
	switch(addr) {
		case 0x2100:
			_forcedVblank = (value & 0x80) != 0;
			_screenBrightness = value & 0x0F;

			//TODO : Also, writing this register on the first line of V-Blank (225 or 240, depending on overscan) when force blank is currently active causes the OAM Address Reset to occur. 
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

		case 0x2104:
			if(_internalOamAddress < 512) {
				if(_internalOamAddress & 0x01) {
					_console->ProcessPpuWrite(_internalOamAddress - 1, _oamWriteBuffer, SnesMemoryType::SpriteRam);
					_oamRam[_internalOamAddress - 1] = _oamWriteBuffer;
	
					_console->ProcessPpuWrite(_internalOamAddress, value, SnesMemoryType::SpriteRam);
					_oamRam[_internalOamAddress] = value;
				} else {
					_oamWriteBuffer = value;
				}
			} else {
				uint16_t address = 0x200 | (_internalOamAddress & 0x1F);
				if((_internalOamAddress & 0x01) == 0) {
					_oamWriteBuffer = value;
				}
				_console->ProcessPpuWrite(address, value, SnesMemoryType::SpriteRam);
				_oamRam[address] = value;
			}
			_internalOamAddress = (_internalOamAddress + 1) & 0x3FF;
			break;
			
		case 0x2105:
			if(_bgMode != (value & 0x07)) {
				MessageManager::DisplayMessage("Debug", "Entering mode: " + std::to_string(value & 0x07));
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
				//"If this register is set during the frame, the Åestarting scanlineÅf is the current scanline, otherwise it is the first visible scanline of the frame."
				_mosaicStartScanline = _scanline;
			}
			break;

		case 0x2107: case 0x2108: case 0x2109: case 0x210A:
			//BG 1-4 Tilemap Address and Size (BG1SC, BG2SC, BG3SC, BG4SC)
			_layerConfig[addr - 0x2107].TilemapAddress = (value & 0xFC) << 9;
			_layerConfig[addr - 0x2107].HorizontalMirroring = (value & 0x01) != 0;
			_layerConfig[addr - 0x2107].VerticalMirroring = (value & 0x02) != 0;
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

			//TODO : Remapping is not implemented yet
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
			_console->ProcessPpuWrite(GetVramAddress() << 1, value, SnesMemoryType::VideoRam);

			_vram[GetVramAddress() << 1] = value;
			if(!_vramAddrIncrementOnSecondReg) {
				_vramAddress = (_vramAddress + _vramIncrementValue) & 0x7FFF;
			}
			break;

		case 0x2119:
			//VMDATAH - VRAM Data Write high byte
			_console->ProcessPpuWrite((GetVramAddress() << 1) + 1, value, SnesMemoryType::VideoRam);

			_vram[(GetVramAddress() << 1) + 1] = value;
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
			_console->ProcessPpuWrite(_cgramAddress, value, SnesMemoryType::CGRam);

			_cgram[_cgramAddress] = value;
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
			_objInterlace = (value & 0x02) != 0;
			_screenInterlace = (value & 0x01) != 0;
			break;

		default:
			MessageManager::DisplayMessage("Debug", "Unimplemented register write: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

/* Everything below this point is used to select the proper arguments for templates */
template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset, bool hiResMode, bool largeTileWidth, bool largeTileHeight, uint8_t activeWindowCount, bool applyMosaic>
void Ppu::RenderTilemap()
{
	if(_directColorMode) {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, largeTileWidth, largeTileHeight, activeWindowCount, applyMosaic, true>();
	} else {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, largeTileWidth, largeTileHeight, activeWindowCount, applyMosaic, false>();
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset, bool hiResMode, bool largeTileWidth, bool largeTileHeight, uint8_t activeWindowCount>
void Ppu::RenderTilemap()
{
	bool applyMosaic = forMainScreen && ((_mosaicEnabled >> layerIndex) & 0x01) != 0;

	if(applyMosaic) {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, largeTileWidth, largeTileHeight, activeWindowCount, true>();
	} else {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, largeTileWidth, largeTileHeight, activeWindowCount, false>();
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset, bool hiResMode, bool largeTileWidth, bool largeTileHeight>
void Ppu::RenderTilemap()
{
	uint8_t activeWindowCount = 0;
	if((forMainScreen && _windowMaskMain[layerIndex]) || (!forMainScreen && _windowMaskSub[layerIndex])) {
		activeWindowCount = (uint8_t)_window[0].ActiveLayers[layerIndex] + (uint8_t)_window[1].ActiveLayers[layerIndex];
	}

	if(activeWindowCount == 0) {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, largeTileWidth, largeTileHeight, 0>();
	} else if(activeWindowCount == 1) {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, largeTileWidth, largeTileHeight, 1>();
	} else {
		RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, largeTileWidth, largeTileHeight, 2>();
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset, bool hiResMode>
void Ppu::RenderTilemap()
{
	bool largeTileWidth = _layerConfig[layerIndex].LargeTiles | hiResMode;
	bool largeTileHeight = _layerConfig[layerIndex].LargeTiles;

	if(largeTileWidth) {
		if(largeTileHeight) {
			RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, true, true>();
		} else {
			RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, true, false>();
		}
	} else {
		if(largeTileHeight) {
			RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, false, true>();
		} else {
			RenderTilemap<layerIndex, bpp, processHighPriority, forMainScreen, basePaletteOffset, hiResMode, false, false>();
		}
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset>
void Ppu::RenderTilemap()
{
	if(_hiResMode || _bgMode == 5 || _bgMode == 6) {
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
