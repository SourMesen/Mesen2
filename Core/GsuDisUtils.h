#pragma once
#include "stdafx.h"

class DisassemblyInfo;
class LabelManager;
class EmuSettings;
class Console;
struct GsuState;

class GsuDisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static int32_t GetEffectiveAddress(DisassemblyInfo& info, Console* console, GsuState& state);
};
