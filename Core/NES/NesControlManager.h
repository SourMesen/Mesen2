#pragma once

#include "stdafx.h"
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
enum class ExpansionPortDevice;

class NesControlManager final : public ISerializable, public INesMemoryHandler, public BaseControlManager
{
private:
	shared_ptr<BaseControlDevice> _mapperControlDevice;

	NesConfig _prevConfig;
	uint32_t _lagCounter = 0;
	bool _isLagging = false;

protected:
	shared_ptr<NesConsole> _console;

	virtual void Serialize(Serializer& s) override;
	virtual ControllerType GetControllerType(uint8_t port);
	virtual void RemapControllerButtons();
	virtual uint8_t GetOpenBusMask(uint8_t port);

public:
	NesControlManager(shared_ptr<NesConsole> console, shared_ptr<BaseControlDevice> mapperControlDevice);
	virtual ~NesControlManager();

	void UpdateControlDevices() override;
	void UpdateInputState() override;

	uint32_t GetLagCounter();
	void ResetLagCounter();

	virtual void Reset(bool softReset);

	bool HasKeyboard();
	
	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port);
	shared_ptr<BaseControlDevice> CreateExpansionDevice(ExpansionPortDevice type);

	virtual void GetMemoryRanges(MemoryRanges &ranges) override
	{
		ranges.AddHandler(MemoryOperation::Read, 0x4016, 0x4017);
		ranges.AddHandler(MemoryOperation::Write, 0x4016);
	}

	virtual uint8_t ReadRam(uint16_t addr) override;
	virtual void WriteRam(uint16_t addr, uint8_t value) override;
};
