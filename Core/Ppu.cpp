#include "stdafx.h"
#include "Ppu.h"
#include "Console.h"
#include "Cpu.h"

Ppu::Ppu(shared_ptr<Console> console)
{
	_console = console;

	_vram = new uint8_t[0x10000];
	memset(_vram, 0, 0x10000);

	_layerConfig[0] = {};
	_layerConfig[1] = {};
	_layerConfig[2] = {};
	_layerConfig[3] = {};

	_outputBuffers[0] = new uint16_t[256 * 224];
	_outputBuffers[1] = new uint16_t[256 * 224];

	_currentBuffer = _outputBuffers[0];

	_cgramAddress = 0;

	_vramAddress = 0;
	_vramIncrementValue = 1;
	_vramAddressRemapping = 0;
	_vramAddrIncrementOnSecondReg = false;
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
		_cycle = 0;
		_scanline++;

		if(_scanline == 225) {
			_nmiFlag = true;
			SendFrame();

			if(_enableNmi) {
				_console->GetCpu()->SetNmiFlag();
			}
		}

		if(_scanline == 260) {
			_nmiFlag = false;
			_scanline = 0;
			_frameCount++;
		}
	}

	_cycle++;
}

void Ppu::SendFrame()
{
	for(int i = 0; i < 256 * 224; i++) {
		_currentBuffer[i] = i;
	}
	_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];

	//_console->GetVideoDecoder()->SendFrame(_currentBuffer);
}

uint8_t Ppu::Read(uint16_t addr)
{
	switch(addr) {
		case 0x4210:
			return _nmiFlag ? 0x80 : 0;
			break;

		case 0x4212:
			return (
				(_scanline >= 225 ? 0x80 : 0) ||
				((_cycle >= 0x121 || _cycle <= 0x15) ? 0x40 : 0)
				);
			break;
	}

	return 0;
}

void Ppu::Write(uint32_t addr, uint8_t value)
{
	switch(addr) {
		case 0x2107: case 0x2108: case 0x2109: case 0x210A:
			//BG 1-4 Tilemap Address and Size (BG1SC, BG2SC, BG3SC, BG4SC)
			_layerConfig[addr - 0x2107].TilemapAddress = (value & 0xFC) << 8;
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
			_vramAddress = (_vramAddress & 0xFF00) | value;
			break;

		case 0x2117:
			//VMADDH - VRAM Address high byte
			_vramAddress = (_vramAddress & 0x00FF) | (value << 8);
			break;

		case 0x2118:
			//VMDATAL - VRAM Data Write low byte
			_vram[_vramAddress << 1] = value;
			if(!_vramAddrIncrementOnSecondReg) {
				_vramAddress += _vramIncrementValue;
			}
			break;

		case 0x2119:
			//VMDATAH - VRAM Data Write high byte
			_vram[(_vramAddress << 1) + 1] = value;
			if(_vramAddrIncrementOnSecondReg) {
				_vramAddress += _vramIncrementValue;
			}
			break;

		case 0x2121:
			//CGRAM Address(CGADD)
			_cgramAddress = value;
			break;

		case 0x2122: 
			//CGRAM Data write (CGDATA)
			_cgram[_cgramAddress] = value;
			break;

		case 0x4200:
			_enableNmi = (value & 0x80) != 0;
			break;
	}
}
