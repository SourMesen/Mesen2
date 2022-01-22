#include "stdafx.h"
#include <cmath>
#include "Shared/Video/BaseRenderer.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"

BaseRenderer::BaseRenderer(Emulator* emu, bool registerAsMessageManager)
{
	_emu = emu;

	if(registerAsMessageManager) {
		//Only display messages on the master CPU's screen
		//MessageManager::RegisterMessageManager(this);
	}
}

BaseRenderer::~BaseRenderer()
{
	//MessageManager::UnregisterMessageManager(this);  
}
