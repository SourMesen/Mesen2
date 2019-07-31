#pragma once
#include "stdafx.h"
#include "../Utilities/ISerializable.h"
#include "IMemoryHandler.h"

class BaseCoprocessor : public ISerializable, public IMemoryHandler
{
public:
	virtual void Reset() = 0;
	
	virtual void ProcessEndOfFrame() { }
	virtual void LoadBattery(string filePath) { }
	virtual void SaveBattery(string filePath) { }
};