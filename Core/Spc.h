#pragma once
#include "stdafx.h"
#include "PpuTypes.h"

class Console;
struct SNES_SPC;

class Spc
{
private:
	shared_ptr<Console> _console;
	uint8_t _spcBios[64];
	SNES_SPC* _spc;
	int16_t *_soundBuffer;

public:
	Spc(shared_ptr<Console> console);
	~Spc();

	uint8_t Read(uint16_t addr);
	void Write(uint32_t addr, uint8_t value);

	void ProcessEndFrame();
};