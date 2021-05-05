#pragma once

#include "stdafx.h"

class IMessageManager
{
public:
	virtual void DisplayMessage(string title, string message) = 0;
};
