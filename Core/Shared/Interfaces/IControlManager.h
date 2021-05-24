#pragma once

#include "IInputRecorder.h"
#include "IInputProvider.h"

enum class ControllerType;

class IControlManager
{
public:
	virtual void RegisterInputProvider(IInputProvider* provider) = 0;
	virtual void UnregisterInputProvider(IInputProvider* provider) = 0;

	virtual void RegisterInputRecorder(IInputRecorder* provider) = 0;
	virtual void UnregisterInputRecorder(IInputRecorder* provider) = 0;

	virtual void AddSystemControlDevice(shared_ptr<BaseControlDevice> device) = 0;
	virtual shared_ptr<BaseControlDevice> GetControlDevice(uint8_t port) = 0;

	virtual shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) = 0;

	virtual bool HasControlDevice(ControllerType type) = 0;

	virtual void SetPollCounter(uint32_t pollCounter) = 0;
	virtual uint32_t GetPollCounter() = 0;
	virtual void UpdateControlDevices() = 0;
	virtual void UpdateInputState() = 0;
};