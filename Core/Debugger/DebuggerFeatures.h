#pragma once
#include "pch.h"

enum class VectorType
{
	Indirect,
	Direct,
};

struct CpuVectorDefinition
{
	char Name[15] = {};
	uint32_t Address = 0;
	VectorType Type = VectorType::Indirect;
};

struct DebuggerFeatures
{
	bool RunToIrq;
	bool RunToNmi;
	bool StepOver;
	bool StepOut;
	bool StepBack;
	bool ChangeProgramCounter;
	bool CallStack;
	bool CpuCycleStep;
	
	uint8_t CpuVectorCount;
	CpuVectorDefinition CpuVectors[10];
};
