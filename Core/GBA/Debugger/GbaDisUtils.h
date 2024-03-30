#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class GbaConsole;
class LabelManager;
class EmuSettings;
class FastString;
struct GbaCpuState;

class GbaDisUtils
{
private:
	static void ArmDisassemble(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static void ThumbDisassemble(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);

	static void WriteRegList(FastString& str, uint16_t regMask, uint8_t size);
	static void WriteReg(FastString& str, uint8_t reg);
	static void WriteCond(FastString& str, uint32_t opCode);

	static bool IsArmBranch(uint32_t opCode);

public:
	static void GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static EffectiveAddressInfo GetEffectiveAddress(DisassemblyInfo& info, GbaConsole* console, GbaCpuState& state);
	static uint8_t GetOpSize(uint32_t opCode, uint8_t flags);
	static bool IsJumpToSub(uint32_t opCode, uint8_t flags);
	static bool IsReturnInstruction(uint32_t opCode, uint8_t flags);
	static bool IsUnconditionalJump(uint32_t opCode, uint8_t flags);
	static bool IsConditionalJump(uint32_t opCode, uint8_t flags);
	static CdlFlags::CdlFlags GetOpFlags(uint32_t opCode, uint8_t flags, uint32_t pc, uint32_t prevPc);
	static bool IsThumbMode(uint8_t flags);
};
