#pragma once
#include "stdafx.h"

class DisassemblyInfo;
class LabelManager;
class EmuSettings;
struct GbCpuState;

class GameboyDisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static int32_t GetEffectiveAddress(DisassemblyInfo& info, GbCpuState& state);
	static uint8_t GetOpSize(uint8_t opCode);
	static bool IsJumpToSub(uint8_t opCode);
	static bool IsReturnInstruction(uint8_t opCode);
	static bool IsUnconditionalJump(uint8_t opCode);
	static bool IsConditionalJump(uint8_t opCode);
	static string GetOpTemplate(uint8_t op, bool prefixed);
};
