#include "pch.h"
#include "GBA/Cart/GbaGpio.h"
#include "GBA/Cart/GbaRtc.h"
#include "Utilities/Serializer.h"

GbaGpio::GbaGpio(GbaRtc* rtc)
{
	_rtc = rtc;
}

uint8_t GbaGpio::Read(uint32_t addr)
{
	if(_state.ReadWrite) {
		switch(addr) {
			case 0x80000C4:
				_state.Data = (_rtc->Read() & ~_state.WritablePins) | (_state.Data & _state.WritablePins);
				return _state.Data;

			case 0x80000C6: return _state.WritablePins;
			case 0x80000C8: return (uint8_t)_state.ReadWrite;
			default: return 0;
		}
	}
	return 0;
}

void GbaGpio::Write(uint32_t addr, uint8_t value)
{
	switch(addr) {
		case 0x80000C4:
			_state.Data = (value & _state.WritablePins) | (_state.Data & ~_state.WritablePins);
			_rtc->Write(_state.Data);
			break;
		case 0x80000C6: _state.WritablePins = value & 0x0F; break;
		case 0x80000C8: _state.ReadWrite = value & 0x01; break;
	}
}

void GbaGpio::Serialize(Serializer& s)
{
	SV(_state.Data);
	SV(_state.ReadWrite);
	SV(_state.WritablePins);
}
