#pragma once

class BaseControlDevice;

class IInputProvider
{
public:
	virtual bool SetInput(BaseControlDevice* device) = 0;
};