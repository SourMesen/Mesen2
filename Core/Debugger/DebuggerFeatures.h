#pragma once
#include "stdafx.h"

struct DebuggerFeatures
{
	bool RunToIrq;
	bool RunToNmi;
	bool StepOver;
	bool StepOut;
	bool StepBack;
	bool ChangeProgramCounter;
	bool CallStack;
};
