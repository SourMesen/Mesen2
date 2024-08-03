#include "pch.h"
#include "PceTurboTap.h"

PceTurboTap::PceTurboTap(Emulator* emu, uint8_t port, ControllerConfig controllers[]) : ControllerHub(emu, ControllerType::PceTurboTap, port, controllers)
{
	//Start with an out-of-range index that selects none of the ports
	_selectedPort = PceTurboTap::MaxPort;
}

uint8_t PceTurboTap::ReadRam(uint16_t addr)
{
	return _selectedPort < PceTurboTap::MaxPort && _ports[_selectedPort] ? ReadPort(_selectedPort) : 0x0F;
}

void PceTurboTap::WriteRam(uint16_t addr, uint8_t value)
{
	bool sel = (value & 0x01) != 0;
	bool prevSel = (_prevValue & 0x01) != 0;
	bool clr = (value & 0x02) != 0;
	bool prevClr = (_prevValue & 0x02) != 0;

	if(!clr && !prevSel && sel && _selectedPort < PceTurboTap::MaxPort) {
		_selectedPort++;
	}

	if(sel && !prevClr && clr) {
		_selectedPort = 0;
	}

	_prevValue = value;

	for(int i = 0; i < 5; i++) {
		WritePort(i, i == _selectedPort ? value : 0x03);
	}
}

void PceTurboTap::Serialize(Serializer& s)
{
	BaseControlDevice::Serialize(s);
	SV(_selectedPort);
	SV(_prevValue);
}
