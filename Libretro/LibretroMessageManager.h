#pragma once
#include "libretro.h"
#include "../Core/IMessageManager.h"
#include "../Core/MessageManager.h"

class LibretroMessageManager : public IMessageManager
{
private:
	retro_log_printf_t _log = nullptr;
	retro_environment_t _retroEnv = nullptr;

public:
	LibretroMessageManager(retro_log_printf_t logCallback, retro_environment_t retroEnv)
	{
		_log = logCallback;
		_retroEnv = retroEnv;
		MessageManager::RegisterMessageManager(this);
	}

	virtual ~LibretroMessageManager()
	{
		MessageManager::RegisterMessageManager(nullptr);
	}

	// Inherited via IMessageManager
	virtual void DisplayMessage(string title, string message) override
	{
		if(title.empty()) {
			if(_log) {
				_log(RETRO_LOG_INFO, message.c_str());
			}
		} else {
			string osdMessage = "[" + title + "] " + message;
			retro_message msg = { osdMessage.c_str(), 180 };
			_retroEnv(RETRO_ENVIRONMENT_SET_MESSAGE, &msg);
		}
	}
};