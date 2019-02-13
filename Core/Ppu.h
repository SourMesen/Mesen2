#pragma once
#include "stdafx.h"
#include "PpuTypes.h"

class Console;

class Ppu
{
private:
	shared_ptr<Console> _console;

	uint16_t _cycle = 0;
	uint16_t _scanline = 0;
	uint32_t _frameCount = 0;
	bool _nmiFlag = false;
	bool _enableNmi = false;

	uint8_t *_vram;
	uint16_t _vramAddress;
	uint8_t _vramIncrementValue;
	uint8_t _vramAddressRemapping;
	bool _vramAddrIncrementOnSecondReg;
	
	uint16_t _cgramAddress;
	uint16_t _cgram[256];

	uint16_t *_outputBuffers[2];
	uint16_t *_currentBuffer;

	LayerConfig _layerConfig[4];

public:
	Ppu(shared_ptr<Console> console);

	PpuState GetState();

	void Exec();

	void SendFrame();

	uint8_t Read(uint16_t addr);
	void Write(uint32_t addr, uint8_t value);
};