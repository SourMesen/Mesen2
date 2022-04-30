#pragma once
#include "stdafx.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/ControllerHub.h"

class Emulator;

class PceTurboTap : public ControllerHub<5>
{
private:
	uint8_t _index = 0;
	uint8_t _prevValue = 0;

public:
	PceTurboTap(Emulator* emu, uint8_t port, ControllerConfig controllers[]);

	uint8_t ReadRam(uint16_t addr) override;
	void WriteRam(uint16_t addr, uint8_t value) override;
};