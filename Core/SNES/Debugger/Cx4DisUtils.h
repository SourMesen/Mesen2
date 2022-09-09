#pragma once
#include "pch.h"
#include "Debugger/DisassemblyInfo.h"

class LabelManager;
class EmuSettings;
class MemoryDumper;
struct Cx4State;

class Cx4DisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static EffectiveAddressInfo GetEffectiveAddress(DisassemblyInfo& info, Cx4State& state, MemoryDumper* memoryDumper);
	static bool IsConditionalJump(uint8_t opCode, uint8_t param);
	static bool IsUnconditionalJump(uint8_t opCode);
	static bool IsJumpToSub(uint8_t opCode);
	static bool IsReturnInstruction(uint8_t opCode);
	static uint8_t GetOpSize() { return 2; }

	static bool CanDisassembleNextOp(uint8_t opCode);
};
