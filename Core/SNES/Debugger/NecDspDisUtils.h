#pragma once
#include "pch.h"

class DisassemblyInfo;
class LabelManager;
class EmuSettings;

class NecDspDisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	
	static bool IsUnconditionalJump(uint32_t opCode);
	static bool IsConditionalJump(uint32_t opCode);
	static bool IsJumpToSub(uint32_t opCode);
	static bool IsReturnInstruction(uint32_t opCode);

	static uint8_t GetOpSize() { return 3; }
};
