#pragma once
#include "../stdafx.h"
#include "../MessageManager.h"

class BaseLoader
{
protected:
	bool _checkOnly;

	void Log(string message)
	{
		if(!_checkOnly) {
			MessageManager::Log(message);
		}
	}

public:
	BaseLoader(bool checkOnly = false)
	{
		_checkOnly = checkOnly;
	}
};