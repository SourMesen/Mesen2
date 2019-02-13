#pragma once
#include "stdafx.h"

class BaseControlDevice;

class IInputRecorder
{
public:
	virtual void RecordInput(vector<shared_ptr<BaseControlDevice>> devices) = 0;
};