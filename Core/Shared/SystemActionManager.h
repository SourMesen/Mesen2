#pragma once
#include "stdafx.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"

class SystemActionManager : public BaseControlDevice
{
private:
	bool _needReset = false;
	bool _needPowerCycle = false;

protected:
	string GetKeyNames() override
	{
		return "RP";
	}
	
public:
	enum Buttons { ResetButton = 0, PowerButton = 1 };

	SystemActionManager(Emulator* emu) : BaseControlDevice(emu, ControllerType::None, BaseControlDevice::ConsoleInputPort)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
	}

	void OnAfterSetState() override
	{
		if(_needReset) {
			_needReset = false;
			SetBit(SystemActionManager::Buttons::ResetButton);
		}
		if(_needPowerCycle) {
			_needPowerCycle = false;
			SetBit(SystemActionManager::Buttons::PowerButton);
		}
	}

	bool Reset()
	{
		if(!_needReset) {
			_needReset = true;
			_emu->SuspendDebugger(false);
			return true;
		}
		return false;
	}

	bool PowerCycle()
	{
		if(!_needPowerCycle) {
			_needPowerCycle = true;
			_emu->SuspendDebugger(false);
			return true;
		}
		return false;
	}

	bool IsResetPressed()
	{
		return IsPressed(SystemActionManager::Buttons::ResetButton);
	}

	bool IsPowerCyclePressed()
	{
		return IsPressed(SystemActionManager::Buttons::PowerButton);
	}
};