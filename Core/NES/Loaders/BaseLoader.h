#pragma once
#include "pch.h"
#include "Shared/MessageManager.h"

class BaseLoader
{
protected:
	void Log(string message)
	{
		MessageManager::Log(message);
	}

public:
	BaseLoader()
	{
	}
};