#pragma once

#include "stdafx.h"
#include "NES/INesMemoryHandler.h"
#include "Shared/Interfaces/IControlManager.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/ISerializable.h"

class BaseControlDevice;
class SystemActionManager;
class IInputRecorder;
class IInputProvider;
class Emulator;
class NesConsole;
struct ControlDeviceState;
enum class ControllerType;
enum class ExpansionPortDevice;

class NesControlManager : public ISerializable, public INesMemoryHandler, public IControlManager
{
private:
	vector<IInputRecorder*> _inputRecorders;
	vector<IInputProvider*> _inputProviders;
	
	//Static so that power cycle does not reset its value
	uint32_t _pollCounter;

	shared_ptr<BaseControlDevice> _mapperControlDevice;

	uint32_t _lagCounter = 0;
	bool _isLagging = false;

protected:
	Emulator* _emu = nullptr;
	shared_ptr<NesConsole> _console;
	SimpleLock _deviceLock;
	vector<shared_ptr<BaseControlDevice>> _controlDevices;
	shared_ptr<BaseControlDevice> _systemActionManager;

	void RegisterControlDevice(shared_ptr<BaseControlDevice> controlDevice);

	virtual void Serialize(Serializer& s) override;
	virtual ControllerType GetControllerType(uint8_t port);
	virtual void RemapControllerButtons();
	virtual uint8_t GetOpenBusMask(uint8_t port);

public:
	NesControlManager(shared_ptr<NesConsole> console, shared_ptr<BaseControlDevice> systemActionManager, shared_ptr<BaseControlDevice> mapperControlDevice);
	virtual ~NesControlManager();

	virtual void UpdateControlDevices();
	void UpdateInputState();

	uint32_t GetLagCounter();
	void ResetLagCounter();

	uint32_t GetPollCounter();
	void SetPollCounter(uint32_t value);

	virtual void Reset(bool softReset);

	void RegisterInputProvider(IInputProvider* provider);
	void UnregisterInputProvider(IInputProvider* provider);

	void RegisterInputRecorder(IInputRecorder* recorder);
	void UnregisterInputRecorder(IInputRecorder* recorder);

	vector<ControlDeviceState> GetPortStates();

	shared_ptr<BaseControlDevice> GetControlDevice(uint8_t port);
	vector<shared_ptr<BaseControlDevice>> GetControlDevices();
	bool HasKeyboard();
	
	static shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port, shared_ptr<NesConsole> console);
	static shared_ptr<BaseControlDevice> CreateExpansionDevice(ExpansionPortDevice type, shared_ptr<NesConsole> console);

	virtual void GetMemoryRanges(MemoryRanges &ranges) override
	{
		ranges.AddHandler(MemoryOperation::Read, 0x4016, 0x4017);
		ranges.AddHandler(MemoryOperation::Write, 0x4016);
	}

	virtual uint8_t ReadRam(uint16_t addr) override;
	virtual void WriteRam(uint16_t addr, uint8_t value) override;
};
