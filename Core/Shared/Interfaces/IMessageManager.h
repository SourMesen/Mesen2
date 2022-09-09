#pragma once

#include "pch.h"

class IMessageManager
{
public:
	virtual void DisplayMessage(string title, string message) = 0;
};
