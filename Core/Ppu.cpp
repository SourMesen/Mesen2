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

enum PixelFlags
{
	Filled = 0x01,
	AllowColorMath = 0x02,
};

Ppu::Ppu(shared_ptr<Console> console)
{
	_console = console;
	_regs = console->GetInternalRegisters();

	_outputBuffers[0] = new uint16_t[256 * 224];
	_outputBuffers[1] = new uint16_t[256 * 224];
	_subScreenBuffer = new uint16_t[256 * 224];

	_currentBuffer = _outputBuffers[0];

	_layerConfig[0] = {};
	_layerConfig[1] = {};
	_layerConfig[2] = {};
	_layerConfig[3] = {};

	_cgramAddress = 0;

	_vram = new uint8_t[Ppu::VideoRamSize];
	memset(_vram, 0, Ppu::VideoRamSize);

	_vramAddress = 0;
	_vramIncrementValue = 1;
	_vramAddressRemapping = 0;
	_vramAddrIncrementOnSecondReg = false;
}

Ppu::~Ppu()
{
	delete[] _vram;
	delete[] _subScreenBuffer;
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
		_frameCount
	};
}

void Ppu::Exec()
{
	if(_cycle == 340) {
		_cycle = -1;
		_scanline++;

		_rangeOver = false;
		_timeOver = false;
		if(_scanline < 224) {
			RenderScanline();
		} else if(_scanline == 225) {
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
		} else if(_scanline == 261) {
			_regs->SetNmiFlag(false);
			_scanline = 0;
			if(_mosaicEnabled) {
				_mosaicStartScanline = 0;
			}
			_console->GetDmaController()->InitHdmaChannels();
			RenderScanline();
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

	if(_cycle == 278 && _scanline < 225) {
		_console->GetDmaController()->ProcessHdmaChannels();
	}
}

template<uint8_t priority, bool forMainScreen>
void Ppu::DrawSprites()
{
	if(forMainScreen) {
		if((_mainScreenLayers & 0x10) == 0) {
			return;
		}

		for(int x = 0; x < 256; x++) {
			if(!_rowPixelFlags[x] && _spritePriority[x] == priority) {
				_currentBuffer[(_scanline << 8) | x] = _spritePixels[x];
				_rowPixelFlags[x] |= PixelFlags::Filled | (((_colorMathEnabled & 0x10) && _spritePalette[x] > 3) ? PixelFlags::AllowColorMath : 0);
			}
		}
	} else {
		if((_subScreenLayers & 0x10) == 0) {
			return;
		}

		for(int x = 0; x < 256; x++) {
			if(!_subScreenFilled[x] && _spritePriority[x] == priority) {
				_subScreenBuffer[(_scanline << 8) | x] = _spritePixels[x];
				_subScreenFilled[x] = true;
			}
		}
	}
}

void Ppu::RenderScanline()
{
	memset(_rowPixelFlags, 0, sizeof(_rowPixelFlags));
	memset(_subScreenFilled, 0, sizeof(_subScreenFilled));

	if(_forcedVblank) {
		RenderBgColor<true>();
		return;
	}

	switch(_bgMode) {
		case 0:
			DrawSprites<3, true>();
			RenderTilemap<0, 2, true, true>();
			RenderTilemap<1, 2, true, true, 64>();
			DrawSprites<2, true>();
			RenderTilemap<0, 2, false, true>();
			RenderTilemap<1, 2, false, true, 64>();
			DrawSprites<1, true>();
			RenderTilemap<2, 2, true, true, 128>();
			RenderTilemap<3, 2, true, true, 192>();
			DrawSprites<0, true>();
			RenderTilemap<2, 2, false, true, 128>();
			RenderTilemap<3, 2, false, true, 192>();
			RenderBgColor<true>();
			break;

		case 1:
			//Main screen
			if(_mode1Bg3Priority) {
				RenderTilemap<2, 2, true, true>();
			}
			DrawSprites<3, true>();
			RenderTilemap<0, 4, true, true>();
			RenderTilemap<1, 4, true, true>();
			DrawSprites<2, true>();
			RenderTilemap<0, 4, false, true>();
			RenderTilemap<1, 4, false, true>();
			DrawSprites<1, true>();
			if(!_mode1Bg3Priority) {
				RenderTilemap<2, 2, true, true>();
			}
			DrawSprites<0, true>();
			RenderTilemap<2, 2, false, true>();
			RenderBgColor<true>();

			//Subscreen
			if(_mode1Bg3Priority) {
				RenderTilemap<2, 2, true, false>();
			}
			DrawSprites<3, false>();
			RenderTilemap<0, 4, true, false>();
			RenderTilemap<1, 4, true, false>();
			DrawSprites<2, false>();
			RenderTilemap<0, 4, false, false>();
			RenderTilemap<1, 4, false, false>();
			DrawSprites<1, false>();
			if(!_mode1Bg3Priority) {
				RenderTilemap<2, 2, true, false>();
			}
			DrawSprites<0, true>();
			RenderTilemap<2, 2, false, false>();
			RenderBgColor<false>();
			break;

		case 2:
			DrawSprites<3, true>();
			RenderTilemap<0, 4, true, true>();
			DrawSprites<2, true>();
			RenderTilemap<1, 4, true, true>();
			DrawSprites<1, true>();
			RenderTilemap<0, 4, false, true>();
			DrawSprites<0, true>();
			RenderTilemap<1, 4, false, true>();
			RenderBgColor<true>();
			break;

		case 3:
			DrawSprites<3, true>();
			RenderTilemap<0, 8, true, true>();
			DrawSprites<2, true>();
			RenderTilemap<1, 4, true, true>();
			DrawSprites<1, true>();
			RenderTilemap<0, 8, false, true>();
			DrawSprites<0, true>();
			RenderTilemap<1, 4, false, true>();
			RenderBgColor<true>();
			break;

		case 5:
			RenderTilemap<1, 2, false, true>();
			RenderTilemap<0, 4, false, true>();
			RenderBgColor<true>();
			break;

		case 6:
			RenderTilemap<0, 8, false, true>();
			RenderBgColor<true>();
			break;
	}

	ApplyColorMath();
	
	//Process sprites for next scanline
	memset(_spritePriority, 0xFF, sizeof(_spritePriority));
	memset(_spritePixels, 0xFF, sizeof(_spritePixels));
	memset(_spritePalette, 0, sizeof(_spritePalette));	
	_spriteCount = 0;
	uint16_t totalWidth = 0;

	for(int i = 0; i < 512; i += 4) {
		uint8_t y = _oamRam[i + 1];

		uint8_t highTableOffset = i >> 4;
		uint8_t shift = ((i >> 2) & 0x03) << 1;
		uint8_t highTableValue = _oamRam[0x200 | highTableOffset] >> shift;
		uint8_t largeSprite = (highTableValue & 0x02) >> 1;
		uint8_t height = _oamSizes[_oamMode][largeSprite][1] << 3;

		if(y > _scanline + 1 || y + height <= _scanline + 1) {
			//Not visible on this scanline
			continue;
		}

		SpriteInfo &info = _sprites[_spriteCount];
		info.LargeSprite = largeSprite;
		uint8_t width = _oamSizes[_oamMode][info.LargeSprite][0] << 3;

		bool negativeX = (highTableValue & 0x01) != 0;
		info.X = negativeX ? -_oamRam[i] : _oamRam[i];
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
			yOffset = (height - (_scanline + 1 - y)) & 0x07;
			rowOffset = (height - (_scanline + 1 - y)) >> 3;
		} else {
			yOffset = (_scanline + 1 - y) & 0x07;
			rowOffset = (_scanline + 1 - y) >> 3;
		} 

		uint8_t row = (info.TileRow + rowOffset) & 0x0F;
		constexpr uint16_t bpp = 4;

		for(int x = info.X; x > 0 && x < info.X + width && x < 256; x++) {
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

				uint16_t color = 0;
				for(int plane = 0; plane < bpp; plane++) {
					uint8_t offset = (plane >> 1) * 16;
					uint8_t tileData = _vram[tileStart + yOffset * 2 + offset + (plane & 0x01)];
					color |= ((tileData >> (7 - xOffset)) & 0x01) << plane;
				}

				if(color != 0) {
					uint16_t paletteRamOffset = 256 + ((info.Palette * (1 << bpp) + color) * 2);
					_spritePixels[x] = _cgram[paletteRamOffset] | (_cgram[paletteRamOffset + 1] << 8);
					_spritePriority[x] = info.Priority;
					_spritePalette[x] = info.Palette;
				}
			}
		}

		totalWidth += width;
		if(totalWidth >= 34 * 8) {
			_timeOver = true;
		}

		_spriteCount++;
		if(_spriteCount == 32) {
			_rangeOver = true;
		}

		if(_timeOver || _rangeOver) {
			break;
		}
	}
}

template<bool forMainScreen>
void Ppu::RenderBgColor()
{
	uint16_t bgColor = _cgram[0] | (_cgram[1] << 8);
	for(int x = 0; x < 256; x++) {
		if(forMainScreen) {
			uint8_t pixelFlags = PixelFlags::Filled | ((_colorMathEnabled & 0x20) ? PixelFlags::AllowColorMath : 0);
			if(!_rowPixelFlags[x]) {
				_currentBuffer[(_scanline << 8) | x] = bgColor;
				_rowPixelFlags[x] = pixelFlags;
			}
		} else {
			if(!_subScreenFilled[x]) {
				_subScreenBuffer[(_scanline << 8) | x] = bgColor;
			}
		}
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset>
void Ppu::RenderTilemap()
{
	if(forMainScreen) {
		if(((_mainScreenLayers >> layerIndex) & 0x01) == 0) {
			//This screen is disabled
			return;
		}
	} else {
		if(((_subScreenLayers >> layerIndex) & 0x01) == 0) {
			//This screen is disabled
			return;
		}
	}

	bool applyMosaic = forMainScreen && ((_mosaicEnabled >> layerIndex) & 0x01) != 0;
	bool mosaicScanline = applyMosaic && (_scanline - _mosaicStartScanline) % _mosaicSize != 0;

	uint8_t pixelFlags = PixelFlags::Filled | (((_colorMathEnabled >> layerIndex) & 0x01) ? PixelFlags::AllowColorMath : 0);

	LayerConfig &config = _layerConfig[layerIndex];
	uint16_t tilemapAddr = config.TilemapAddress >> 1;
	uint16_t chrAddr = config.ChrAddress;
	
	for(int x = 0; x < 256; x++) {
		uint16_t row = (_scanline + config.VScroll) >> 3;
		uint16_t column = (x + config.HScroll) >> 3;

		uint32_t addr = tilemapAddr + ((row & 0x1F) << 5) + (column & 0x1F) + (config.VerticalMirroring ? ((row & 0x20) << (config.HorizontalMirroring ? 6 : 5)) : 0) + (config.HorizontalMirroring ? ((column & 0x20) << 5) : 0);
		addr <<= 1;

		if(forMainScreen) {
			if(_rowPixelFlags[x] || ((uint8_t)processHighPriority != ((_vram[addr + 1] & 0x20) >> 5))) {
				continue;
			}
		} else {
			if(_subScreenFilled[x] || ((uint8_t)processHighPriority != ((_vram[addr + 1] & 0x20) >> 5))) {
				continue;
			}
		}

		if(mosaicScanline || (applyMosaic && x % _mosaicSize != 0)) {
			//If this is not the top-left pixels in the mosaic pattern, override it with the top-left pixel data
			_currentBuffer[(_scanline << 8) | x] = _mosaicColor[x];
			_rowPixelFlags[x] = pixelFlags;
			continue;
		}

		uint8_t palette = (_vram[addr + 1] >> 2) & 0x07;
		uint16_t tileIndex = ((_vram[addr + 1] & 0x03) << 8) | _vram[addr];
		bool vMirror = (_vram[addr + 1] & 0x80) != 0;
		bool hMirror = (_vram[addr + 1] & 0x40) != 0;

		uint16_t tileStart = chrAddr + tileIndex * 8 * bpp;

		uint8_t yOffset = (_scanline + config.VScroll) & 0x07;
		if(vMirror) {
			yOffset = 7 - yOffset;
		}

		uint16_t color = 0;

		uint8_t xOffset = (x + config.HScroll) & 0x07;
		uint8_t shift = hMirror ? xOffset : (7 - xOffset);
		for(int plane = 0; plane < bpp; plane++) {
			uint8_t offset = (plane >> 1) * 16;
			color |= (((_vram[tileStart + yOffset * 2 + offset + (plane & 0x01)] >> shift) & 0x01) << bpp);
			color >>= 1;
		}

		if(color > 0) {
			uint16_t paletteRamOffset = basePaletteOffset + (palette * (1 << bpp) + color) * 2;
			uint16_t paletteColor = _cgram[paletteRamOffset] | (_cgram[paletteRamOffset + 1] << 8);

			if(forMainScreen) {
				_currentBuffer[(_scanline << 8) | x] = paletteColor;
				_rowPixelFlags[x] = pixelFlags;
				if(applyMosaic && x % _mosaicSize == 0) {
					//This is the source for the mosaic pattern, store it for use in the next scanlines
					for(int i = 0; i < _mosaicSize && x + i < 256; i++) {
						_mosaicColor[x+i] = paletteColor;
					}
				}
			} else {
				_subScreenBuffer[(_scanline << 8) | x] = paletteColor;
				_subScreenFilled[x] = true;
			}
		}
	}
}

void Ppu::ApplyColorMath()
{
	if(!_colorMathEnabled) {
		return;
	}

	for(int x = 0; x < 256; x++) {
		if(_rowPixelFlags[x] & PixelFlags::AllowColorMath) {
			uint16_t otherPixel;
			uint8_t halfShift = _colorMathHalveResult ? 1 : 0;
			if(_colorMathAddSubscreen) {
				if(_subScreenFilled[x]) {
					otherPixel = _subScreenBuffer[(_scanline << 8) | x];
				} else {
					//there's nothing in the subscreen at this pixel, use the fixed color and disable halve operation
					otherPixel = _fixedColor;
					halfShift = 0;
				}
			} else {
				otherPixel = _fixedColor;
			}

			uint16_t &mainPixel = _currentBuffer[(_scanline << 8) | x];

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

void Ppu::SendFrame()
{
	_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);
	_console->GetVideoDecoder()->UpdateFrame(_currentBuffer, _frameCount);
	_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
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
}

uint8_t Ppu::Read(uint16_t addr)
{
	switch(addr) {
		case 0x2134: return ((int16_t)_mode7MatrixA * ((int16_t)_mode7MatrixB >> 8)) & 0xFF;
		case 0x2135: return (((int16_t)_mode7MatrixA * ((int16_t)_mode7MatrixB >> 8)) >> 8) & 0xFF;
		case 0x2136: return (((int16_t)_mode7MatrixA * ((int16_t)_mode7MatrixB >> 8)) >> 16) & 0xFF;

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
			} else {
				value = _oamRam[0x200 | (_internalOamAddress & 0x1F)];
			}
			_internalOamAddress = (_internalOamAddress + 1) & 0x3FF;
			return value;
		}

		case 0x2139: {
			//VMDATALREAD - VRAM Data Read low byte
			uint8_t returnValue = _vramReadBuffer;
			_vramReadBuffer = _vram[_vramAddress << 1];
			if(!_vramAddrIncrementOnSecondReg) {
				_vramAddress = (_vramAddress + _vramIncrementValue) & 0x7FFF;
			}
			return returnValue;
		}

		case 0x213A: {
			//VMDATAHREAD - VRAM Data Read high byte
			uint8_t returnValue = _vramReadBuffer;
			_vramReadBuffer = _vram[(_vramAddress << 1) + 1];
			if(_vramAddrIncrementOnSecondReg) {
				_vramAddress = (_vramAddress + _vramIncrementValue) & 0x7FFF;
			}
			return returnValue;
		}

		case 0x213B: {
			//CGDATAREAD - CGRAM Data read
			uint8_t value = _cgram[_cgramAddress];
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
			//TODO open bus on bit 4

			//"The high/low selector is reset to ÅelowÅf when $213f is read"
			_horizontalLocToggle = false;
			_verticalLocationToggle = false;

			return (
				(_timeOver ? 0x80 : 0) |
				(_rangeOver ? 0x40 : 0) |
				0x01 //PPU chip version
			);

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

			//TODO Apply brightness
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
					_oamRam[_internalOamAddress - 1] = _oamWriteBuffer;
					_oamRam[_internalOamAddress] = value;
				} else {
					_oamWriteBuffer = value;
				}
			} else {
				uint16_t address = 0x200 | (_internalOamAddress & 0x1F);
				if((_internalOamAddress & 0x01) == 0) {
					_oamWriteBuffer = value;
				}
				_oamRam[address] = value;
			}
			_internalOamAddress = (_internalOamAddress + 1) & 0x3FF;
			break;
			
		case 0x2105:
			_bgMode = value & 0x07;
			_mode1Bg3Priority = (value & 0x08) != 0;

			_layerConfig[0].LargeTiles = (value & 0x10) != 0;
			_layerConfig[1].LargeTiles = (value & 0x20) != 0;
			_layerConfig[2].LargeTiles = (value & 0x30) != 0;
			_layerConfig[3].LargeTiles = (value & 0x40) != 0;
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
			//TODO Mode 7 portion of register
		case 0x210F: case 0x2111: case 0x2113:
			_layerConfig[(addr - 0x210D) >> 1].HScroll = ((value << 8) | (_hvScrollLatchValue & ~0x07) | (_hScrollLatchValue & 0x07)) & 0x3FF;
			_hvScrollLatchValue = value;
			_hScrollLatchValue = value;
			break;

		case 0x210E:
			//TODO Mode 7 portion of register
		case 0x2110: case 0x2112: case 0x2114:
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
			break;

		case 0x2117:
			//VMADDH - VRAM Address high byte
			_vramAddress = (_vramAddress & 0x00FF) | ((value & 0x7F) << 8);
			break;

		case 0x2118:
			//VMDATAL - VRAM Data Write low byte
			_vram[_vramAddress << 1] = value;
			if(!_vramAddrIncrementOnSecondReg) {
				_vramAddress = (_vramAddress + _vramIncrementValue) & 0x7FFF;
			}
			break;

		case 0x2119:
			//VMDATAH - VRAM Data Write high byte
			_vram[(_vramAddress << 1) + 1] = value;
			if(_vramAddrIncrementOnSecondReg) {
				_vramAddress = (_vramAddress + _vramIncrementValue) & 0x7FFF;
			}
			break;

		case 0x211B:
			//M7A - Mode 7 Matrix A (also used with $2134/6)
			_mode7MatrixA = (value << 8) | _mode7Latch;
			_mode7Latch = value;
			break;

		case 0x211C:
			//M7B - Mode 7 Matrix B (also used with $2134/6)
			_mode7MatrixB = (value << 8) | _mode7Latch;
			_mode7Latch = value;
			break;

		case 0x2121:
			//CGRAM Address(CGADD)
			_cgramAddress = value * 2;
			break;

		case 0x2122: 
			//CGRAM Data write (CGDATA)
			_cgram[_cgramAddress] = value;
			_cgramAddress = (_cgramAddress + 1) & (Ppu::CgRamSize - 1);
			break;

		case 0x212C:
			//TM - Main Screen Designation
			_mainScreenLayers = value & 0x1F;
			break;

		case 0x212D:
			//TS - Subscreen Designation
			_subScreenLayers = value & 0x1F;
			break;
		
		case 0x2130:
			//CGWSEL - Color Addition Select
			_colorMathClipMode = (value >> 6) & 0x03; //TODO
			_colorMathPreventMode = (value >> 4) & 0x03; //TODO
			_colorMathAddSubscreen = (value & 0x02) != 0;
			_directColorMode = (value & 0x01) != 0; //TODO
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

		default:
			MessageManager::DisplayMessage("Debug", "Unimplemented register write: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}
