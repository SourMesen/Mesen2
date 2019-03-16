#pragma once
#include "stdafx.h"
#include "BaseControlDevice.h"
#include "Console.h"

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

	SystemActionManager(shared_ptr<Console> console) : BaseControlDevice(console, BaseControlDevice::ConsoleInputPort)
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
			SetBit(SystemActionManager::Buttons::ResetButton);
			_needReset = false;
		}
		if(_needPowerCycle) {
			SetBit(SystemActionManager::Buttons::PowerButton);
			_needPowerCycle = false;
		}
	}

	bool Reset()
	{
		if(!_needReset) {
			_needReset = true;
			return true;
		}
		return false;
	}

	bool PowerCycle()
	{
		if(!_needPowerCycle) {
			_needPowerCycle = true;
			return true;
		}
		return false;
	}

	void ProcessSystemActions()
	{
		shared_ptr<Console> console = _console;
		if(console) {
			if(IsPressed(SystemActionManager::Buttons::ResetButton)) {
				console->Reset();
				console->GetControlManager()->UpdateInputState();
			}
			if(IsPressed(SystemActionManager::Buttons::PowerButton)) {
				console->PowerCycle();
				//Calling PowerCycle() causes this object to be deleted - no code must be written below this line
			}
		}
	}
};
