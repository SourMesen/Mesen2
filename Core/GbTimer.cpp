#include "stdafx.h"
#include "GbTimer.h"
#include "GbTypes.h"
#include "GbMemoryManager.h"
#include "GbApu.h"

GbTimer::GbTimer(GbMemoryManager* memoryManager, GbApu* apu)
{
	_apu = apu;
	_memoryManager = memoryManager;
	
	//Passes boot_div-dmgABCmgb
	//But that test depends on LCD power on timings, so may be wrong.
	_divider = 0x06; 
}

GbTimer::~GbTimer()
{
}

void GbTimer::Exec()
{
	_reloaded = false;
	if(_needReload) {
		ReloadCounter();
	}
	SetDivider(_divider + 4);	
}

void GbTimer::ReloadCounter()
{
	_counter = _modulo;
	_memoryManager->RequestIrq(GbIrqSource::Timer);
	_needReload = false;
	_reloaded = true;
}

void GbTimer::SetDivider(uint16_t newValue)
{
	if(_timerEnabled && !(newValue & _timerDivider) && (_divider & _timerDivider)) {
		_counter++;
		if(_counter == 0) {
			_needReload = true;
		}
	}

	uint16_t frameSeqBit = _memoryManager->IsHighSpeed() ? 0x2000 : 0x1000;
	if(!(newValue & frameSeqBit) && (_divider & frameSeqBit)) {
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
		case 0xFF07: return _control | 0xF8; //FF07 - TAC - Timer Control (R/W)
	}
	return 0;
}

void GbTimer::Write(uint16_t addr, uint8_t value)
{
	//TODO properly detect edges when setting new values to registers or disabling timer, etc.
	switch(addr) {
		case 0xFF04:
			SetDivider(0);
			break;

		case 0xFF05:
			//FF05 - TIMA - Timer counter (R/W)
			if(_needReload) {
				//Writing to TIMA when a reload is pending will cancel the reload and IRQ request
				_needReload = false;
			}

			if(!_reloaded) {
				//Writes to TIMA on the cycle TIMA was reloaded with TMA are ignored
				_counter = value;
			}
			break;

		case 0xFF06:
			//FF06 - TMA - Timer Modulo (R/W)
			_modulo = value;
			if(_reloaded) {
				//Writing to TMA on the same cycle it was reloaded into TIMA will also update TIMA
				_counter = value;
			}
			break;

		case 0xFF07: {
			//FF07 - TAC - Timer Control (R/W)
			_control = value;
			bool enabled = (value & 0x04) != 0;
			uint16_t newDivider = 0;
			switch(value & 0x03) {
				case 0: newDivider = 1 << 9; break;
				case 1: newDivider = 1 << 3; break;
				case 2: newDivider = 1 << 5; break;
				case 3: newDivider = 1 << 7; break;
			}

			if(_timerEnabled) {
				//When changing the value of TAC, the TIMA register can get incremented due to a glitch
				bool incrementCounter;
				if(enabled) {
					incrementCounter = (_divider & _timerDivider) != 0 && (_divider & newDivider) == 0;
				} else {
					incrementCounter = (_divider & _timerDivider) != 0;
				}

				if(incrementCounter) {
					_counter++;
					if(_counter == 0) {
						ReloadCounter();
					}
				}
			}

			_timerEnabled = enabled;
			_timerDivider = newDivider;
			break;
		}
	}
}

void GbTimer::Serialize(Serializer& s)
{
	s.Stream(_divider, _counter, _modulo, _control, _timerEnabled, _timerDivider, _needReload, _reloaded);
}
