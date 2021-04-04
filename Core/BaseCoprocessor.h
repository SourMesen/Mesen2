#pragma once
#include "stdafx.h"
#include "Utilities/ISerializable.h"
#include "IMemoryHandler.h"

class BaseCoprocessor : public ISerializable, public IMemoryHandler
{
public:
	using IMemoryHandler::IMemoryHandler;

	virtual void Reset() = 0;

	virtual void Run() { }	
	virtual void ProcessEndOfFrame() { }
	virtual void LoadBattery() { }
	virtual void SaveBattery() { }
};