#pragma once
#include "pch.h"
#include "Shared/BaseControlManager.h"
#include "Shared/SettingTypes.h"
#include "PCE/PceTypes.h"
#include "Utilities/ISerializable.h"

class Emulator;

class PceControlManager final : public BaseControlManager
{
private:
	PceControlManagerState _state = {};
	PcEngineConfig _prevConfig = {};

public:
	PceControlManager(Emulator* emu);
	
	PceControlManagerState& GetState();

	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) override;

	uint8_t ReadInputPort();
	void WriteInputPort(uint8_t value);
	void UpdateControlDevices() override;

	void Serialize(Serializer& s) override;
};