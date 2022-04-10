#pragma once
#include "stdafx.h"

class DisassemblyInfo;
class LabelManager;
class MemoryDumper;
class EmuSettings;
struct PceCpuState;
enum class PceAddrMode;

class PceDisUtils
{
private:
	static uint8_t GetOpSize(PceAddrMode addrMode);

public:
	static void GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static int32_t GetEffectiveAddress(DisassemblyInfo& info, PceCpuState& state, MemoryDumper* memoryDumper);

	static uint8_t GetOpSize(uint8_t opCode);
	static char const* const GetOpName(uint8_t opCode);
	static PceAddrMode GetOpMode(uint8_t opCode);
	static bool IsUnconditionalJump(uint8_t opCode);
	static bool IsConditionalJump(uint8_t opCode);
	static bool IsJumpToSub(uint8_t opCode);
	static bool IsReturnInstruction(uint8_t opCode);
	static bool IsOpUnofficial(uint8_t opCode);
};
