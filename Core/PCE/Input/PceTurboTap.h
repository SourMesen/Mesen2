#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/ControllerHub.h"

class Emulator;

class PceTurboTap : public ControllerHub<5>
{
private:
	static constexpr uint8_t MaxPort = 5;
	uint8_t _selectedPort = 0;
	uint8_t _prevValue = 0;

public:
	PceTurboTap(Emulator* emu, uint8_t port, ControllerConfig controllers[]);

	uint8_t ReadRam(uint16_t addr) override;
	void WriteRam(uint16_t addr, uint8_t value) override;

	void Serialize(Serializer& s) override;
};