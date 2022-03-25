#pragma once
#include "stdafx.h"

class DisassemblyInfo;
class LabelManager;
class EmuSettings;
class SnesConsole;
struct GsuState;

class GsuDisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static int32_t GetEffectiveAddress(DisassemblyInfo& info, SnesConsole* console, GsuState& state);
	static bool IsUnconditionalJump(uint8_t opCode);
	static bool IsConditionalJump(uint8_t opCode);
	static uint8_t GetOpSize(uint8_t opCode);
};
