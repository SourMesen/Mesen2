#pragma once

#include "pch.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/ISerializable.h"

class BaseControlDevice;
class IInputRecorder;
class IInputProvider;
class Emulator;
enum class CpuType : uint8_t;
struct ControllerData;
enum class ControllerType;

class BaseControlManager : public ISerializable
{
private:
	vector<IInputRecorder*> _inputRecorders;
	vector<IInputProvider*> _inputProviders;

protected:
	Emulator* _emu = nullptr;
	CpuType _cpuType = {};
	SimpleLock _deviceLock;
	vector<shared_ptr<BaseControlDevice>> _systemDevices;
	vector<shared_ptr<BaseControlDevice>> _controlDevices;
	uint32_t _pollCounter = 0;
	uint32_t _lagCounter = 0;
	bool _wasInputRead = false;

	void RegisterControlDevice(shared_ptr<BaseControlDevice> controlDevice);

	void ClearDevices();

public:
	BaseControlManager(Emulator* emu, CpuType cpuType);
	virtual ~BaseControlManager();
	
	void Serialize(Serializer& s) override;

	void AddSystemControlDevice(shared_ptr<BaseControlDevice> device);

	virtual void UpdateControlDevices() {}
	virtual void UpdateInputState();

	void ProcessEndOfFrame();

	void SetInputReadFlag();
	uint32_t GetLagCounter();
	void ResetLagCounter();

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

	shared_ptr<BaseControlDevice> GetControlDevice(uint8_t port, uint8_t subPort = 0);
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
