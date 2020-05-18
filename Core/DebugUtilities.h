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
			case CpuType::NecDsp: return SnesMemoryType::NecDspMemory;
			case CpuType::Sa1: return SnesMemoryType::Sa1Memory;
			case CpuType::Gsu: return SnesMemoryType::GsuMemory;
			case CpuType::Cx4:  return SnesMemoryType::Cx4Memory;
			case CpuType::Gameboy:  return SnesMemoryType::GameboyMemory;
		}

		throw std::runtime_error("Invalid CPU type");
	}

	static CpuType ToCpuType(SnesMemoryType type)
	{
		switch(type) {
			case SnesMemoryType::SpcMemory:
			case SnesMemoryType::SpcRam:
			case SnesMemoryType::SpcRom:
				return CpuType::Spc;

			case SnesMemoryType::GsuMemory:
			case SnesMemoryType::GsuWorkRam:
				return CpuType::Gsu;

			case SnesMemoryType::Sa1InternalRam:
			case SnesMemoryType::Sa1Memory:
				return CpuType::Sa1;

			case SnesMemoryType::DspDataRam:
			case SnesMemoryType::DspDataRom:
			case SnesMemoryType::DspProgramRom:
				return CpuType::NecDsp;

			case SnesMemoryType::Cx4DataRam:
			case SnesMemoryType::Cx4Memory:
				return CpuType::Cx4;
				
			case SnesMemoryType::GbPrgRom:
			case SnesMemoryType::GbWorkRam:
			case SnesMemoryType::GbCartRam:
			case SnesMemoryType::GbVideoRam:
			case SnesMemoryType::GbHighRam:
				return CpuType::Gameboy;

			default:
				return CpuType::Cpu;
		}

		throw std::runtime_error("Invalid CPU type");
	}

	static constexpr SnesMemoryType GetLastCpuMemoryType()
	{
		return SnesMemoryType::GameboyMemory;
	}

	static bool IsPpuMemory(SnesMemoryType memType)
	{
		switch(memType) {
			case SnesMemoryType::VideoRam:
			case SnesMemoryType::SpriteRam:
			case SnesMemoryType::CGRam:
			case SnesMemoryType::GbVideoRam:
				return true;

			default: 
				return false;
		}
	}

	static constexpr CpuType GetLastCpuType()
	{
		return CpuType::Gameboy;
	}
};