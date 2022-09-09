#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/ControllerHub.h"
#include "Utilities/Serializer.h"

class TwoPlayerAdapter : public ControllerHub<2>
{
public:
	TwoPlayerAdapter(Emulator* emu, ControllerType type, ControllerConfig controllers[]) : ControllerHub(emu, type, BaseControlDevice::ExpDevicePort, controllers)
	{}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		value &= 0x01;
		ControllerHub::WriteRam(addr, value);
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		StrobeProcessRead();
		uint8_t output = 0;
		uint8_t i = addr - 0x4016;
		
		if(_ports[i]) {
			output |= (ReadPort(i) & 0x01) << 1;
		}

		return output;
	}
};
