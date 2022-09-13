#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class Gameboy;
class LabelManager;
class EmuSettings;
struct GbCpuState;

class GameboyDisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static EffectiveAddressInfo GetEffectiveAddress(DisassemblyInfo& info, Gameboy* console, GbCpuState& state);
	static uint8_t GetOpSize(uint8_t opCode);
	static bool IsJumpToSub(uint8_t opCode);
	static bool IsReturnInstruction(uint8_t opCode);
	static bool IsUnconditionalJump(uint8_t opCode);
	static bool IsConditionalJump(uint8_t opCode);
	static CdlFlags::CdlFlags GetOpFlags(uint8_t opCode, uint16_t pc, uint16_t prevPc);
	static string GetOpTemplate(uint8_t op, bool prefixed);
};
