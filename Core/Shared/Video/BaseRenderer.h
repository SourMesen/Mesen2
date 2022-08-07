#pragma once
#include "Core/Shared/Interfaces/IMessageManager.h"
#include "Core/Shared/SettingTypes.h"
#include "Utilities/Timer.h"

class Emulator;

//TODO remove?
class BaseRenderer
{
protected:
	Emulator* _emu;

	uint32_t _screenWidth = 0;
	uint32_t _screenHeight = 0;
	
	BaseRenderer(Emulator* emu);
	virtual ~BaseRenderer();
};
