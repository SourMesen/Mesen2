#pragma once
#include "pch.h"

enum class CpuType : uint8_t
{
	Snes,
	Spc,
	NecDsp,
	Sa1,
	Gsu,
	Cx4,
	Gameboy,
	Nes,
	Pce,
	Sms,
	Gba
};

class CpuTypeUtilities
{
public:
	static constexpr int GetCpuTypeCount()
	{
		return (int)CpuType::Sms + 1;
	}
};