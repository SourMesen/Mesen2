#pragma once
#include "pch.h"
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
		_connected = false;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
	}

	void ResetState()
	{
		//Only reset these flags once the reset/power cycle is done
		//Called by emu class after reloading/resetting the game
		//This is needed to avoid a reset/power cycle command from
		//being queue while a reset/power cycle is in progress (which can
		//break things when the debugger is opened, etc.)
		_needPowerCycle = false;
		_needReset = false;

		//TODOv2 review this - prevents NES from power cycling 2x in a row
		ClearBit(SystemActionManager::Buttons::ResetButton);
		ClearBit(SystemActionManager::Buttons::PowerButton);
	}

	void OnAfterSetState() override
	{
		if(_needReset) {
			SetBit(SystemActionManager::Buttons::ResetButton);
		}
		if(_needPowerCycle) {
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

	bool IsResetPending()
	{
		return _needReset || _needPowerCycle;
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