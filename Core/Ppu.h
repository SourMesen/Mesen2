#pragma once
#include "stdafx.h"
#include "PpuTypes.h"

class Ppu
{
private:
	uint16_t _cycle = 0;
	uint16_t _scanline = 0;
	uint32_t _frameCount = 0;
	bool _nmiFlag = false;

public:
	PpuState GetState()
	{
		return {
			_cycle,
			_scanline,
			_frameCount
		};
	}

	void Exec()
	{
		if(_cycle == 340) {
			_cycle = 0;
			_scanline++;

			if(_scanline == 225) {
				_nmiFlag = true;
			}

			if(_scanline == 260) {
				_nmiFlag = false; 
				_scanline = 0;
				_frameCount++;
			}
		}

		_cycle++;
	}

	uint8_t Read(uint16_t addr)
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

	void Write(uint32_t addr, uint8_t value)
	{

	}
};