#include "stdafx.h"
#include "GbTimer.h"
#include "GbTypes.h"
#include "GbMemoryManager.h"
#include "GbApu.h"

GbTimer::GbTimer(GbMemoryManager* memoryManager, GbApu* apu)
{
	_apu = apu;
	_memoryManager = memoryManager;
}

GbTimer::~GbTimer()
{
}

void GbTimer::Exec()
{
	uint16_t newValue = _divider + 4;
	if(_timerEnabled && !(newValue & _timerDivider) && (_divider & _timerDivider)) {
		_counter++;
		if(_counter == 0) {
			_counter = _modulo;
			_memoryManager->RequestIrq(GbIrqSource::Timer);
		}
	}

	if(!(newValue & 0x1000) && (_divider & 0x1000)) {
		_apu->ClockFrameSequencer();
	}

	_divider = newValue;
}

uint8_t GbTimer::Read(uint16_t addr)
{
	switch(addr) {
		case 0xFF04: return _divider >> 8;
		case 0xFF05: return _counter; //FF05 - TIMA - Timer counter (R/W)
		case 0xFF06: return _modulo; //FF06 - TMA - Timer Modulo (R/W)
		case 0xFF07: return _control; //FF07 - TAC - Timer Control (R/W)
	}
	return 0;
}

void GbTimer::Write(uint16_t addr, uint8_t value)
{
	//TODO properly detect edges when setting new values to registers or disabling timer, etc.
	switch(addr) {
		case 0xFF04:
			_divider = 0;
			break;

		case 0xFF05:
			//FF05 - TIMA - Timer counter (R/W)
			_counter = value;
			break;

		case 0xFF06:
			//FF06 - TMA - Timer Modulo (R/W)
			_modulo = value;
			break;

		case 0xFF07:
			//FF07 - TAC - Timer Control (R/W)
			_control = value;
			_timerEnabled = (value & 0x04) != 0;
			switch(value & 0x03) {
				case 0: _timerDivider = 1 << 9; break;
				case 1: _timerDivider = 1 << 3; break;
				case 2: _timerDivider = 1 << 5; break;
				case 3: _timerDivider = 1 << 7; break;
			}
			break;
	}
}

void GbTimer::Serialize(Serializer& s)
{
	s.Stream(_divider, _counter, _modulo, _control, _timerEnabled, _timerDivider);
}
