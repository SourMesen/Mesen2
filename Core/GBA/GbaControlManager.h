#pragma once
#include "Shared/BaseControlManager.h"
#include "Shared/SettingTypes.h"
#include "GBA/GbaTypes.h"

class Emulator;
class GbaConsole;
class GbaMemoryManager;
class BaseControlDevice;

class GbaControlManager final : public BaseControlManager
{
private:
	GbaConsole* _console = nullptr;
	GbaMemoryManager* _memoryManager = nullptr;
	GbaConfig _prevConfig = {};
	GbaControlManagerState _state = {};
	
	uint8_t ReadController(uint32_t addr);
	void CheckForIrq();

public:
	GbaControlManager(Emulator* emu, GbaConsole* console);
	void Init(GbaMemoryManager* memoryManager);

	GbaControlManagerState& GetState();
	
	void UpdateInputState() override;

	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) override;
	void UpdateControlDevices() override;
	uint8_t ReadInputPort(uint32_t addr);
	__noinline bool CheckInputCondition();
	void WriteInputPort(GbaAccessModeVal mode, uint32_t addr, uint8_t value);
	void Serialize(Serializer& s) override;
};