#include "stdafx.h"
#include "Ppu.h"
#include "Console.h"
#include "MemoryManager.h"
#include "Cpu.h"
#include "Spc.h"
#include "VideoDecoder.h"
#include "NotificationManager.h"

Ppu::Ppu(shared_ptr<Console> console)
{
	_console = console;

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
			_console->GetSpc()->ProcessEndFrame();
			_nmiFlag = true;
			SendFrame();

			if(_enableNmi) {
				_console->GetCpu()->SetNmiFlag();
			}
		} else if(_scanline == 261) {
			_nmiFlag = false;
			_scanline = 0;
			_frameCount++;
		}

		if(_enableVerticalIrq && !_enableHorizontalIrq && _cycle == _verticalTimer) {
			//An IRQ will occur sometime just after the V Counter reaches the value set in $4209/$420A.
			_console->GetCpu()->SetIrqSource(IrqSource::Ppu);
		}
	}

	if(_enableHorizontalIrq && _cycle == _horizontalTimer && (!_enableVerticalIrq || _scanline == _verticalTimer)) {
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
	return _spriteRam;
}

uint8_t Ppu::Read(uint16_t addr)
{
	switch(addr) {
		case 0x4210: {
			//open bus implementation here is needed to pass CPUPHL test
			uint8_t value = (_nmiFlag ? 0x80 : 0) | ((addr >> 8) & 0x70);
			_nmiFlag = false;
			return value;
		}

		case 0x4211: {
			uint8_t value = (_irqFlag ? 0x80 : 0) | ((addr >> 8) & 0x7F);
			_irqFlag = false;
			_console->GetCpu()->ClearIrqSource(IrqSource::Ppu);
			return value;
		}

		case 0x4212:
			return (
				(_scanline >= 225 ? 0x80 : 0) |
				((_cycle >= 0x121 || _cycle <= 0x15) ? 0x40 : 0)
			);
	}

	return 0;
}

void Ppu::Write(uint32_t addr, uint8_t value)
{
	switch(addr) {
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

		case 0x4200:
			_enableNmi = (value & 0x80) != 0;
			_enableVerticalIrq = (value & 0x20) != 0;
			_enableHorizontalIrq = (value & 0x10) != 0;

			//TODO
			//_autoJoypadRead = (value & 0x01) != 0;
			break;

		case 0x4207: _horizontalTimer = (_horizontalTimer & 0x100) | value; break;
		case 0x4208: _horizontalTimer = (_horizontalTimer & 0xFF) | ((value & 0x01) << 8); break;

		case 0x4209: _verticalTimer = (_verticalTimer & 0x100) | value; break;
		case 0x420A: _verticalTimer = (_verticalTimer & 0xFF) | ((value & 0x01) << 8); break;
	}
}
