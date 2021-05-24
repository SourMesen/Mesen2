#pragma once
#include "stdafx.h"
#include "Shared/BaseControlManager.h"

class Emulator;
class BaseControlDevice;

class GbControlManager final : public BaseControlManager
{
private:
	Emulator* _emu;

public:
	GbControlManager(Emulator* emu);

	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) override;
	void UpdateControlDevices() override;
};