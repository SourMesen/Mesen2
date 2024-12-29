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
	St018,
	Gameboy,
	Nes,
	Pce,
	Sms,
	Gba,
	Ws
};

class CpuTypeUtilities
{
public:
	static constexpr int GetCpuTypeCount()
	{
		return (int)CpuType::Ws + 1;
	}
};