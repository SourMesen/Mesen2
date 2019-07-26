#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

class DebugUtilities
{
public:
	static SnesMemoryType GetCpuMemoryType(CpuType type)
	{
		switch(type) {
			case CpuType::Cpu: return SnesMemoryType::CpuMemory;
			case CpuType::Spc: return SnesMemoryType::SpcMemory;
			case CpuType::Sa1: return SnesMemoryType::Sa1Memory;
			case CpuType::NecDsp: break;
		}

		throw std::runtime_error("Invalid CPU type");
	}
};