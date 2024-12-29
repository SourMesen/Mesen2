#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class SnesConsole;
struct ArmV3CpuState;

class St018DisUtils
{
public:
	static EffectiveAddressInfo GetEffectiveAddress(DisassemblyInfo& info, SnesConsole* console, ArmV3CpuState& state);
};
