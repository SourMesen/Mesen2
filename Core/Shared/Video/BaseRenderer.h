#pragma once
#include "Core/Shared/Interfaces/IMessageManager.h"
#include "Core/Shared/SettingTypes.h"
#include "Utilities/Timer.h"

class Emulator;

class BaseRenderer
{
protected:
	shared_ptr<Emulator> _emu;

	uint32_t _screenWidth = 0;
	uint32_t _screenHeight = 0;
	
	BaseRenderer(shared_ptr<Emulator> emu, bool registerAsMessageManager);
	virtual ~BaseRenderer();
};
