#pragma once 

#include "pch.h"

class BaseControlDevice;

class IControllerHub
{
public:
	static constexpr int MaxSubPorts = 5;

	virtual void RefreshHubState() = 0;
	virtual int GetHubPortCount() = 0;
	virtual shared_ptr<BaseControlDevice> GetController(int index) = 0;
};