#include "pch.h"
#include "PceTurboTap.h"

PceTurboTap::PceTurboTap(Emulator* emu, uint8_t port, ControllerConfig controllers[]) : ControllerHub(emu, ControllerType::PceTurboTap, port, controllers)
{
}

uint8_t PceTurboTap::ReadRam(uint16_t addr)
{
	return _ports[_index] ? ReadPort(_index) : 0x0F;
}

void PceTurboTap::WriteRam(uint16_t addr, uint8_t value)
{
	bool sel = (value & 0x01) != 0;
	bool prevSel = (_prevValue & 0x01) != 0;
	bool clr = (value & 0x02) != 0;
	bool prevClr = (_prevValue & 0x02) != 0;

	if(!clr && !prevSel && sel) {
		_index = (_index + 1) % 5;
	}

	if(sel && !prevClr && clr) {
		_index = 0;
	}

	_prevValue = value;

	WritePort(_index, value);
}
