#pragma once
#include "stdafx.h"
#include "Shared/Interfaces/IControlManager.h"

class Emulator;
class BaseControlDevice;
class IInputRecorder;
class IInputProvider;

class GbControlManager : public IControlManager
{
private:
	Emulator* _emu;
	shared_ptr<BaseControlDevice> _device;

public:
	GbControlManager(Emulator* emu);

	void UpdateInputState();

	// Inherited via IControlManager
	virtual void RegisterInputProvider(IInputProvider* provider) override;
	virtual void UnregisterInputProvider(IInputProvider* provider) override;
	virtual void RegisterInputRecorder(IInputRecorder* provider) override;
	virtual void UnregisterInputRecorder(IInputRecorder* provider) override;
	virtual shared_ptr<BaseControlDevice> GetControlDevice(uint8_t port) override;
	virtual void SetPollCounter(uint32_t pollCounter) override;
	virtual uint32_t GetPollCounter() override;
	virtual void UpdateControlDevices() override;
};