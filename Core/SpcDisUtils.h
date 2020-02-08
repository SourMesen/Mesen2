#pragma once
#include "stdafx.h"

class DisassemblyInfo;
class Console;
class LabelManager;
class EmuSettings;
struct SpcState;

class SpcDisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static int32_t GetEffectiveAddress(DisassemblyInfo &info, Console *console, SpcState &state);
	static uint8_t GetOpSize(uint8_t opCode);
};
