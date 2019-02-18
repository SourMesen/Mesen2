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

		if(_scanline == 225) {
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

void Ppu::RenderTilemap(LayerConfig &config, uint8_t bpp)
{
	uint16_t tilemapAddr = config.TilemapAddress;
	uint16_t chrAddr = config.ChrAddress;
	
	uint32_t addr = tilemapAddr;
	for(int y = 0; y < 28; y++) {
		for(int x = 0; x < 32; x++) {
			uint8_t palette = (_vram[addr + 1] >> 2) & 0x07;
			uint16_t tileIndex = ((_vram[addr + 1] & 0x03) << 8) | _vram[addr];

			uint16_t tileStart = chrAddr + tileIndex * 8 * bpp;
			for(int i = 0; i < 8; i++) {
				for(int j = 0; j < 8; j++) {
					uint16_t color = 0;
					for(int plane = 0; plane < bpp; plane++) {
						uint8_t offset = (plane >> 1) * 16;
						color |= (((_vram[tileStart + i * 2 + offset + (plane & 0x01)] >> (7 - j)) & 0x01) << bpp);
						color >>= 1;
					}

					uint16_t paletteRamOffset = color == 0 ? 0 : ((palette * (1 << bpp) + color) * 2);

					uint16_t paletteColor = _cgram[paletteRamOffset] | (_cgram[paletteRamOffset + 1] << 8);
					_currentBuffer[(y * 8 + i) * 256 + x * 8 + j] = paletteColor;
				}
			}
			addr+=2;
		}
	}
}

void Ppu::SendFrame()
{
	switch(_bgMode) {
		case 0:
			RenderTilemap(_layerConfig[3], 2);
			RenderTilemap(_layerConfig[2], 2);
			RenderTilemap(_layerConfig[1], 2);
			RenderTilemap(_layerConfig[0], 2);
			break;

		case 1:
			RenderTilemap(_layerConfig[2], 2);
			RenderTilemap(_layerConfig[1], 4);
			RenderTilemap(_layerConfig[0], 4);
			break;

		case 2:
			RenderTilemap(_layerConfig[1], 4);
			RenderTilemap(_layerConfig[0], 4);
			break;

		case 3:
			RenderTilemap(_layerConfig[1], 4);
			RenderTilemap(_layerConfig[0], 8);
			break;

		case 5:
			RenderTilemap(_layerConfig[1], 2);
			RenderTilemap(_layerConfig[0], 4);
			break;
		
		case 6:
			RenderTilemap(_layerConfig[0], 8);
			break;
	}

	//Draw sprites
	for(int i = 0; i < 512; i+=4) {
		uint8_t y = _oamRam[i + 1];
		if(y >= 225) {
			continue;
		}

		int16_t x = _oamRam[i];
		uint8_t tileRow = (_oamRam[i + 2] & 0xF0) >> 4;
		uint8_t tileColumn = _oamRam[i + 2] & 0x0F;
		uint8_t flags = _oamRam[i + 3];

		uint8_t palette = (flags >> 1) & 0x07;
		bool useSecondTable = (flags & 0x01) != 0;

		//TODO: vertical, horizontal flip, priority

		uint8_t highTableOffset = i >> 4;
		uint8_t shift = ((i >> 2) & 0x03) << 1;

		uint8_t highTableValue = _oamRam[0x200 | highTableOffset] >> shift;
		bool negativeX = (highTableValue & 0x01) != 0;
		uint8_t size = (highTableValue & 0x02) >> 1;

		if(negativeX) {
			x = -x;
		}

		for(int rowOffset = 0; rowOffset < _oamSizes[_oamMode][size][1]; rowOffset++) {
			for(int columnOffset = 0; columnOffset < _oamSizes[_oamMode][size][0]; columnOffset++) {
				uint8_t column = (tileColumn + columnOffset) & 0x0F;
				uint8_t row = (tileRow + rowOffset) & 0x0F;
				uint8_t tileIndex = (row << 4) | column;

				uint16_t tileStart = ((_oamBaseAddress + (tileIndex << 4) + (useSecondTable ? _oamAddressOffset : 0)) & 0x7FFF) << 1;
				uint16_t bpp = 4;
				for(int i = 0; i < 8; i++) {
					for(int j = 0; j < 8; j++) {
						uint16_t color = 0;
						for(int plane = 0; plane < bpp; plane++) {
							uint8_t offset = (plane >> 1) * 16;
							color |= (((_vram[tileStart + i * 2 + offset + (plane & 0x01)] >> (7 - j)) & 0x01) << bpp);
							color >>= 1;
						}

						if(color != 0) {
							uint16_t paletteRamOffset = 256 + ((palette * (1 << bpp) + color) * 2);

							uint16_t paletteColor = _cgram[paletteRamOffset] | (_cgram[paletteRamOffset + 1] << 8);
							_currentBuffer[(y + i + rowOffset * 8) * 256 + x + j + columnOffset * 8] = paletteColor;
						}
					}
				}
			}
		}
	}
	
	_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);
	_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
	_console->GetVideoDecoder()->UpdateFrame(_currentBuffer, _frameCount);
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
			
			//TODO
			//_mode1Bg3Priority = (value & 0x08) != 0;

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
			_layerConfig[addr - 0x210B].ChrAddress = (value & 0x0F) << 12;
			_layerConfig[addr - 0x210B + 1].ChrAddress = (value & 0xF0) << 8;
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

		default:
			MessageManager::DisplayMessage("Debug", "Unimplemented register write: " + HexUtilities::ToHex(addr));
			break;
	}
}
