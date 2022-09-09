#pragma once
#include "pch.h"

class BaseControlDevice;

class IInputRecorder
{
public:
	virtual void RecordInput(vector<shared_ptr<BaseControlDevice>> devices) = 0;
};