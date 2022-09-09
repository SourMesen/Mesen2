#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class LabelManager;
class EmuSettings;
class SnesConsole;
struct GsuState;

class GsuDisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static EffectiveAddressInfo GetEffectiveAddress(DisassemblyInfo& info, SnesConsole* console, GsuState& state);
	static bool IsUnconditionalJump(uint8_t opCode);
	static bool IsConditionalJump(uint8_t opCode);
	static uint8_t GetOpSize(uint8_t opCode);
	static bool CanDisassembleNextOp(uint8_t opCode);

	static void UpdateCpuFlags(uint8_t opCode, uint8_t& cpuFlags);
};
