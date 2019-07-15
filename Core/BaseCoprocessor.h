#pragma once
#include "stdafx.h"
#include "../Utilities/ISerializable.h"
#include "IMemoryHandler.h"

class BaseCoprocessor : public ISerializable, public IMemoryHandler
{
public:
	virtual void Run() = 0;
	virtual void Reset() = 0;
};