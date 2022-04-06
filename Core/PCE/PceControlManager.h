#pragma once
#include "stdafx.h"
#include "Shared/BaseControlManager.h"
#include "Shared/SettingTypes.h"
#include "PCE/PceTypes.h"

class Emulator;

class PceControlManager : public BaseControlManager
{
private:
	PceControlManagerState _state = {};
	GameboyConfig _prevConfig = {};

public:
	PceControlManager(Emulator* emu);
	
	PceControlManagerState& GetState();

	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) override;

	uint8_t ReadInputPort();
	void WriteInputPort(uint8_t value);
	void UpdateControlDevices() override;
};