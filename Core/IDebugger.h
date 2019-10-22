#pragma once
#include "stdafx.h"

enum class StepType;

class IDebugger
{
public:
	virtual void Step(int32_t stepCount, StepType type) = 0;
};