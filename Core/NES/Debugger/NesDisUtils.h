#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class LabelManager;
class MemoryDumper;
class EmuSettings;
struct NesCpuState;
enum class NesAddrMode;

class NesDisUtils
{
private:
	static uint32_t GetOperandAddress(DisassemblyInfo& info, uint32_t memoryAddr);
	static uint8_t GetOpSize(NesAddrMode addrMode);

public:
	static void GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static EffectiveAddressInfo GetEffectiveAddress(DisassemblyInfo& info, NesCpuState& state, MemoryDumper* memoryDumper);

	static uint8_t GetOpSize(uint8_t opCode);
	static char const* const GetOpName(uint8_t opCode);
	static NesAddrMode GetOpMode(uint8_t opCode);
	static bool IsOpUnofficial(uint8_t opCode);
	static bool IsUnconditionalJump(uint8_t opCode);
	static bool IsConditionalJump(uint8_t opCode);
	static CdlFlags::CdlFlags GetOpFlags(uint8_t opCode, uint16_t pc, uint16_t prevPc);
	static bool IsJumpToSub(uint8_t opCode);
	static bool IsReturnInstruction(uint8_t opCode);
};
