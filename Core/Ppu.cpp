#include "stdafx.h"
#include "Ppu.h"
#include "Console.h"
#include "MemoryManager.h"
#include "Cpu.h"
#include "VideoDecoder.h"

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
	uint16_t tilemapAddr = _layerConfig[0].TilemapAddress;
	uint16_t chrAddr =  _layerConfig[0].ChrAddress;
	
	for(int y = 0; y < 28; y++) {
		for(int x = 0; x < 32; x++) {
			uint8_t byte1 = _vram[tilemapAddr + (y * 32 + x) * 2];
			uint8_t byte2 = _vram[tilemapAddr + (y * 32 + x) * 2 + 1];
			
			uint8_t palette = (byte2 >> 2) & 0x07;
			uint16_t tileIndex = ((byte2 & 0x03) << 8) | byte1;

			uint16_t tileStart = chrAddr + tileIndex * 8 * 2;
			for(int i = 0; i < 8; i++) {
				for(int j = 0; j < 8; j++) {
					uint8_t color = 0;
					for(int plane = 0; plane < 2; plane++) {
						color |= (((_vram[tileStart + i * 2 + plane] >> (7 - j)) & 0x01) << 2);
						color >>= 1;
					}

					uint16_t paletteColor = _cgram[(palette * 4 + color) * 2] | (_cgram[(palette * 4 + color) * 2 + 1] << 8);
					_currentBuffer[(y * 8 + i) * 256 + x * 8 + j] = paletteColor;
				}
			}
		}
	}

	_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
	_console->GetVideoDecoder()->UpdateFrame(_currentBuffer, _frameCount);
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
			_cgramAddress = value * 2;
			break;

		case 0x2122: 
			//CGRAM Data write (CGDATA)
			_cgram[_cgramAddress] = value;
			_cgramAddress++;
			break;

		case 0x4200:
			_enableNmi = (value & 0x80) != 0;
			break;

		case 0x420B:
			//MDMAEN - DMA Enable
			if(value & 0x01) {
				for(int i = 0; i < _dmaSize; i++) {
					uint8_t valToWrite = _console->GetMemoryManager()->Read(_dmaSource, MemoryOperationType::Read);
					_console->GetMemoryManager()->Write(0x2100 | _dmaDest, valToWrite, MemoryOperationType::Write);
					_dmaSource++;
				}
			}
			break;

		case 0x4300:
			//DMAPx - DMA Control for Channel x
			break;

		case 0x4301:
			//BBADx - DMA Destination Register for Channel x
			_dmaDest = value;
			break;

		case 0x4305:
			//DASxL - DMA Size / HDMA Indirect Address low byte(x = 0 - 7)
			_dmaSize = (_dmaSize & 0xFF00) | value;
			break;

		case 0x4306:
			//DASxL - DMA Size / HDMA Indirect Address low byte(x = 0 - 7)
			_dmaSize = (_dmaSize & 0xFF) | (value << 8);
			break;

		case 0x4302:
			_dmaSource = (_dmaSource & 0xFFFF00) | value;
			break;

		case 0x4303:
			_dmaSource = (_dmaSource & 0xFF00FF) | (value << 8);
			break;

		case 0x4304:
			_dmaSource = (_dmaSource & 0x00FFFF) | (value << 16);
			break;
	}
}
