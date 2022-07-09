#pragma once

#include "stdafx.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/ISerializable.h"

class BaseControlDevice;
class IInputRecorder;
class IInputProvider;
class Emulator;
struct ControllerData;
enum class ControllerType;

class BaseControlManager
{
private:
	vector<IInputRecorder*> _inputRecorders;
	vector<IInputProvider*> _inputProviders;

protected:
	Emulator* _emu = nullptr;
	SimpleLock _deviceLock;
	vector<shared_ptr<BaseControlDevice>> _systemDevices;
	vector<shared_ptr<BaseControlDevice>> _controlDevices;
	uint32_t _pollCounter = 0;

	void RegisterControlDevice(shared_ptr<BaseControlDevice> controlDevice);

	void ClearDevices();

public:
	BaseControlManager(Emulator* emu);
	virtual ~BaseControlManager();

	void AddSystemControlDevice(shared_ptr<BaseControlDevice> device);

	virtual void UpdateControlDevices() {}
	virtual void UpdateInputState();

	bool HasControlDevice(ControllerType type);
	virtual bool IsKeyboardConnected() { return false; }

	uint32_t GetPollCounter();
	void SetPollCounter(uint32_t value);

	virtual void Reset(bool softReset) {}

	void RegisterInputProvider(IInputProvider* provider);
	void UnregisterInputProvider(IInputProvider* provider);

	void RegisterInputRecorder(IInputRecorder* recorder);
	void UnregisterInputRecorder(IInputRecorder* recorder);

	virtual shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) = 0;

	vector<ControllerData> GetPortStates();

	shared_ptr<BaseControlDevice> GetControlDevice(uint8_t port);
	shared_ptr<BaseControlDevice> GetControlDeviceByIndex(uint8_t index);
	void RefreshHubState();
	vector<shared_ptr<BaseControlDevice>> GetControlDevices();
	
	template<typename T>
	shared_ptr<T> GetControlDevice()
	{
		auto lock = _deviceLock.AcquireSafe();

		for (shared_ptr<BaseControlDevice>& device : _controlDevices) {
			shared_ptr<T> typedDevice = std::dynamic_pointer_cast<T>(device);
			if (typedDevice) {
				return typedDevice;
			}
		}
		return nullptr;
	}
};
