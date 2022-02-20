#pragma once
#include "stdafx.h"

class DisassemblyInfo;
class LabelManager;
class EmuSettings;
class MemoryDumper;
struct Cx4State;

class Cx4DisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static int32_t GetEffectiveAddress(DisassemblyInfo& info, Cx4State& state, MemoryDumper* memoryDumper);
};
