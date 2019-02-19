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
		if(_scanline < 224) {
			RenderScanline();
		} else if(_scanline == 225) {
			//Reset OAM address at the start of vblank?
			_internalOamAddress = (_oamRamAddress << 1);

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
}

struct SpriteInfo
{
	int16_t X;
	bool HorizontalMirror;
	bool VerticalMirror;
	uint8_t Priority;

	uint8_t TileColumn;
	uint8_t TileRow;
	uint8_t Palette;
	bool UseSecondTable;
	uint8_t LargeSprite;
};

SpriteInfo _sprites[32] = {};
uint8_t _spriteCount = 0;
uint8_t _spritePriority[256] = {};
uint16_t _spritePixels[256] = {};

template<uint8_t priority, bool forMainScreen>
void Ppu::DrawSprites()
{
	if(forMainScreen) {
		for(int x = 0; x < 256; x++) {
			if(!_filled[x] && _spritePriority[x] == priority) {
				_currentBuffer[(_scanline << 8) | x] = _spritePixels[x];
				_filled[x] = true;
			}
		}
	} else {
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
	memset(_filled, 0, sizeof(_filled));
	memset(_subScreenFilled, 0, sizeof(_subScreenFilled));

	switch(_bgMode) {
		case 0:
			DrawSprites<3, true>();
			RenderTilemap<0, 2, true, true>();
			RenderTilemap<1, 2, true, true>();
			DrawSprites<2, true>();
			RenderTilemap<0, 2, false, true>();
			RenderTilemap<1, 2, false, true>();
			DrawSprites<1, true>();
			RenderTilemap<2, 2, true, true>();
			RenderTilemap<3, 2, true, true>();
			DrawSprites<0, true>();
			RenderTilemap<2, 2, false, true>();
			RenderTilemap<3, 2, false, true>();
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

			ApplyColorMath();
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
	
	//Process sprites for next scanline
	memset(_spritePriority, 0xFF, sizeof(_spritePriority));
	memset(_spritePixels, 0xFFFF, sizeof(_spritePixels));
	_spriteCount = 0;
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

		for(int x = info.X; x < info.X + width; x++) {
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
				}
			}
		}

		_spriteCount++;

		if(_spriteCount == 32) {
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
			if(!_filled[x]) {
				_currentBuffer[(_scanline << 8) | x] = bgColor;
			}
		} else {
			if(!_subScreenFilled[x]) {
				_subScreenBuffer[(_scanline << 8) | x] = bgColor;
			}
		}
	}
}

template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen>
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

	LayerConfig &config = _layerConfig[layerIndex];
	uint16_t tilemapAddr = config.TilemapAddress;
	uint16_t chrAddr = config.ChrAddress;
	uint32_t addr = tilemapAddr + ((_scanline >> 3) << 6) - 2;

	for(int x = 0; x < 256; x++) {
		if((x & 0x07) == 0) {
			addr += 2;
		}
		
		if(forMainScreen) {
			if(_filled[x] || ((uint8_t)processHighPriority != ((_vram[addr + 1] & 0x20) >> 5))) {
				continue;
			}
		} else {
			if(_subScreenFilled[x] || ((uint8_t)processHighPriority != ((_vram[addr + 1] & 0x20) >> 5))) {
				continue;
			}
		}

		uint8_t palette = (_vram[addr + 1] >> 2) & 0x07;
		uint16_t tileIndex = ((_vram[addr + 1] & 0x03) << 8) | _vram[addr];
		bool vMirror = (_vram[addr + 1] & 0x80) != 0;
		bool hMirror = (_vram[addr + 1] & 0x40) != 0;

		uint16_t tileStart = chrAddr + tileIndex * 8 * bpp;
		uint8_t yOffset = vMirror ? (7 - (_scanline & 0x07)) : (_scanline & 0x07);

		uint16_t color = 0;
		uint8_t shift = hMirror ? (x & 0x07) : (7 - (x & 0x07));
		for(int plane = 0; plane < bpp; plane++) {
			uint8_t offset = (plane >> 1) * 16;
			color |= (((_vram[tileStart + yOffset * 2 + offset + (plane & 0x01)] >> shift) & 0x01) << bpp);
			color >>= 1;
		}

		if(color > 0) {
			uint16_t paletteRamOffset = (palette * (1 << bpp) + color) * 2;
			uint16_t paletteColor = _cgram[paletteRamOffset] | (_cgram[paletteRamOffset + 1] << 8);

			if(forMainScreen) {
				_currentBuffer[(_scanline << 8) | x] = paletteColor;
				_filled[x] = true;
			} else {
				_subScreenBuffer[(_scanline << 8) | x] = paletteColor;
				_subScreenFilled[x] = true;
			}
		}
	}
}

void Ppu::ApplyColorMath()
{
	bool useColorMath = _colorMathEnabled;// >> layerIndex) & 0x01) != 0;
	if(!useColorMath) {
		return;
	}

	for(int x = 0; x < 256; x++) {
		uint16_t &mainPixel = _currentBuffer[(_scanline << 8) | x];
		uint16_t &subPixel = _subScreenBuffer[(_scanline << 8) | x];
		
		uint8_t halfShift = _colorMathHalveResult ? 1 : 0;
		uint16_t r = std::min(((mainPixel & 0x001F) + (subPixel & 0x001F)) >> halfShift, 0x1F);
		uint16_t g = std::min((((mainPixel >> 5) & 0x001F) + ((subPixel >> 5) & 0x001F)) >> halfShift, 0x1F);
		uint16_t b = std::min((((mainPixel >> 10) & 0x001F) + ((subPixel >> 10) & 0x001F)) >> halfShift, 0x1F);

		mainPixel = r | (g << 5) | (b << 10);
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

uint8_t Ppu::Read(uint16_t addr)
{
	switch(addr) {
		default:
			MessageManager::DisplayMessage("Debug", "Unimplemented register read: " + HexUtilities::ToHex(addr));
			break;
	}

	return 0;
}

void Ppu::Write(uint32_t addr, uint8_t value)
{
	switch(addr) {
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
			} else if(_internalOamAddress >= 512) {
				uint16_t address = 0x200 | (_internalOamAddress & 0x1F);
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

		case 0x2107: case 0x2108: case 0x2109: case 0x210A:
			//BG 1-4 Tilemap Address and Size (BG1SC, BG2SC, BG3SC, BG4SC)
			_layerConfig[addr - 0x2107].TilemapAddress = (value & 0xFC) << 9;
			_layerConfig[addr - 0x2107].HorizontalMirrorring = (value & 0x01) != 0;
			_layerConfig[addr - 0x2107].VerticalMirrorring = (value & 0x02) != 0;
			break;

		case 0x210B: case 0x210C:
			//BG1+2 / BG3+4 Chr Address (BG12NBA / BG34NBA)
			_layerConfig[(addr - 0x210B) * 2].ChrAddress = (value & 0x0F) << 12;
			_layerConfig[(addr - 0x210B) * 2 + 1].ChrAddress = (value & 0xF0) << 8;
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
			_colorMathClipMode = (value >> 6) & 0x03;
			_colorMathPreventMode = (value >> 4) & 0x03;
			_colorMathAddSubscreen = (value & 0x02) != 0;
			_directColorMode = (value & 0x01) != 0;
			break;

		case 0x2131:
			_colorMathEnabled = value & 0x3F;
			_colorMathSubstractMode = (value & 0x80) != 0;
			_colorMathHalveResult = (value & 0x40) != 0;
			break;

		default:
			MessageManager::DisplayMessage("Debug", "Unimplemented register write: " + HexUtilities::ToHex(addr));
			break;
	}
}
