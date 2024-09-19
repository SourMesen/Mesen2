#pragma once
#include "pch.h"
#include "Shared/SettingTypes.h"
#include "Shared/BaseControlManager.h"
#include "WS/WsTypes.h"

class WsConsole;

class WsControlManager final : public BaseControlManager
{
private:
	WsControlManagerState _state = {};
	WsConfig _prevConfig = {};

	uint8_t _prevInput = 0;
	bool _soundButtonPressed = false;

	WsConsole* _console = nullptr;

public:
	WsControlManager(Emulator* emu, WsConsole* console);

	WsControlManagerState& GetState() { return _state; }
	
	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) override;

	void UpdateControlDevices() override;
	void UpdateInputState() override;

	uint8_t Read();
	void Write(uint8_t value);

	bool IsSoundPressed();

	void Serialize(Serializer& s) override;
};
