#pragma once

#include "pch.h"
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

class SnesControlManager final : public BaseControlManager
{
private:
	SnesConfig _prevConfig = {};

protected:
	SnesConsole* _console = nullptr;
	uint8_t _lastWriteValue = 0;
	bool _autoReadStrobe = 0;

public:
	SnesControlManager(SnesConsole* console);
	virtual ~SnesControlManager();

	void Reset(bool softReset) override;
	void UpdateControlDevices() override;
	
	uint8_t GetLastWriteValue() { return _lastWriteValue; }

	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) override;

	uint8_t Read(uint16_t addr, bool forAutoRead = false);
	
	void Write(uint16_t addr, uint8_t value);
	void SetAutoReadStrobe(bool strobe);

	void Serialize(Serializer &s) override;
};
