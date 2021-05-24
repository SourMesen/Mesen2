#pragma once

#include "stdafx.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/ISerializable.h"
#include "Shared/Interfaces/IControlManager.h"
#include "Shared/BaseControlManager.h"

class BaseControlDevice;
class IInputRecorder;
class IInputProvider;
class Console;
class Emulator;
class SystemActionManager;
struct ControllerData;
enum class ControllerType;
enum class ExpansionPortDevice;

class ControlManager : public ISerializable, public BaseControlManager
{
private:
	uint32_t _inputConfigVersion;

protected:
	Console* _console;

	ControllerType GetControllerType(uint8_t port);

public:
	ControlManager(Console* console);
	virtual ~ControlManager();

	void UpdateControlDevices() override;

	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) override;

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer &s) override;
};
