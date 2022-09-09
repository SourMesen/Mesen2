#pragma once

#include "pch.h"
#include "NES/INesMemoryHandler.h"
#include "Shared/BaseControlManager.h"
#include "Shared/SettingTypes.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/ISerializable.h"

class BaseControlDevice;
class SystemActionManager;
class IInputRecorder;
class IInputProvider;
class Emulator;
class NesConsole;
enum class ControllerType;

class NesControlManager : public ISerializable, public INesMemoryHandler, public BaseControlManager
{
private:
	NesConfig _prevConfig = {};

protected:
	NesConsole* _console;

	virtual void Serialize(Serializer& s) override;
	virtual void RemapControllerButtons();
	virtual uint8_t GetOpenBusMask(uint8_t port);

public:
	NesControlManager(NesConsole* console);
	virtual ~NesControlManager();

	void UpdateControlDevices() override;
	void UpdateInputState() override;

	void SaveBattery();

	void Reset(bool softReset) override;

	bool IsKeyboardConnected() override;
	
	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) override;

	void GetMemoryRanges(MemoryRanges &ranges) override
	{
		ranges.AddHandler(MemoryOperation::Read, 0x4016, 0x4017);
		ranges.AddHandler(MemoryOperation::Write, 0x4016);
	}

	uint8_t ReadRam(uint16_t addr) override;
	void WriteRam(uint16_t addr, uint8_t value) override;
};
