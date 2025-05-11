#include "pch.h"
#include "GBA/Cart/GbaGpio.h"
#include "GBA/Cart/GbaRtc.h"
#include "Utilities/Serializer.h"

GbaGpio::GbaGpio(GbaRtc* rtc)
{
	_rtc = rtc;
}

bool GbaGpio::CanRead()
{
	return _state.ReadWrite;
}

uint8_t GbaGpio::Read(uint32_t addr)
{
	switch(addr) {
		case 0x80000C4: return _state.Data;
		case 0x80000C6: return _state.WritablePins;
		case 0x80000C8: return (uint8_t)_state.ReadWrite;
		default: return 0;
	}
}

void GbaGpio::UpdateDataPins()
{
	_state.Data = (_rtc->Read() & ~_state.WritablePins) | (_state.Data & _state.WritablePins);

	//SCK and CS are forced to 0 when the pins aren't writeable
	if((_state.WritablePins & 0x01) == 0) {
		_state.Data &= ~0x01;
	}
	if((_state.WritablePins & 0x04) == 0) {
		_state.Data &= ~0x04;
	}
}

void GbaGpio::Write(uint32_t addr, uint8_t value)
{
	switch(addr) {
		case 0x80000C4: {
			uint8_t data = (value & _state.WritablePins) | (_state.Data & ~_state.WritablePins);
			_rtc->Write(data);
			UpdateDataPins();
			break;
		}

		case 0x80000C6:
			_state.WritablePins = value & 0x0F;
			UpdateDataPins();
			break;

		case 0x80000C8: _state.ReadWrite = value & 0x01; break;
	}
}

void GbaGpio::Serialize(Serializer& s)
{
	SV(_state.Data);
	SV(_state.ReadWrite);
	SV(_state.WritablePins);
}
