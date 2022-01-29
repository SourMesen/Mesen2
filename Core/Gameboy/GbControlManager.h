#pragma once
#include "stdafx.h"
#include "Shared/BaseControlManager.h"
#include "Shared/SettingTypes.h"

class Emulator;
class Gameboy;
class BaseControlDevice;

class GbControlManager final : public BaseControlManager, public ISerializable
{
private:
	Emulator* _emu = nullptr;
	Gameboy* _console = nullptr;
	GameboyConfig _prevConfig = {};
	GbControlManagerState _state = {};

public:
	GbControlManager(Emulator* emu, Gameboy* console);

	GbControlManagerState GetState();

	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) override;
	void UpdateControlDevices() override;
	uint8_t ReadInputPort();
	void WriteInputPort(uint8_t value);

	void UpdateInputState() override;

	void Serialize(Serializer& s) override;
};