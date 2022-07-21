#pragma once

#include "stdafx.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/ISerializable.h"
#include "Shared/BaseControlManager.h"
#include "Shared/SettingTypes.h"

class BaseControlDevice;
class IInputRecorder;
class IInputProvider;
class SnesConsole;
class Emulator;
class SystemActionManager;
struct ControllerData;
enum class ControllerType;

class SnesControlManager : public ISerializable, public BaseControlManager
{
private:
	SnesConfig _prevConfig = {};

protected:
	SnesConsole* _console;

public:
	SnesControlManager(SnesConsole* console);
	virtual ~SnesControlManager();

	void UpdateControlDevices() override;

	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) override;

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer &s) override;
};
