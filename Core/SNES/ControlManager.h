#pragma once

#include "stdafx.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/ISerializable.h"
#include "Shared/Interfaces/IControlManager.h"

class BaseControlDevice;
class IInputRecorder;
class IInputProvider;
class Console;
class Emulator;
class SystemActionManager;
struct ControllerData;
enum class ControllerType;
enum class ExpansionPortDevice;

class ControlManager : public ISerializable, public IControlManager
{
private:
	vector<IInputRecorder*> _inputRecorders;
	vector<IInputProvider*> _inputProviders;
	
	uint32_t _pollCounter;
	uint32_t _inputConfigVersion;

protected:
	Console* _console;
	Emulator* _emu;
	SimpleLock _deviceLock;
	vector<shared_ptr<BaseControlDevice>> _controlDevices;
	shared_ptr<SystemActionManager> _systemActionManager;

	void RegisterControlDevice(shared_ptr<BaseControlDevice> controlDevice);

	ControllerType GetControllerType(uint8_t port);

public:
	ControlManager(Console* console);
	virtual ~ControlManager();

	void UpdateControlDevices() override;
	void UpdateInputState();

	uint32_t GetPollCounter() override;
	void SetPollCounter(uint32_t value) override;

	void RegisterInputProvider(IInputProvider* provider) override;
	void UnregisterInputProvider(IInputProvider* provider) override;

	void RegisterInputRecorder(IInputRecorder* recorder) override;
	void UnregisterInputRecorder(IInputRecorder* recorder) override;

	vector<ControllerData> GetPortStates();

	SystemActionManager* GetSystemActionManager();
	shared_ptr<BaseControlDevice> GetControlDevice(uint8_t port) override;
	vector<shared_ptr<BaseControlDevice>> GetControlDevices();
	
	static shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port, Console* console);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer &s) override;
};
