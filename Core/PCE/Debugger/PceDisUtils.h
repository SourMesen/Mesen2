#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class LabelManager;
class MemoryDumper;
class EmuSettings;
class PceConsole;
struct PceCpuState;
enum class PceAddrMode;

class PceDisUtils
{
private:
	static uint8_t GetOpSize(PceAddrMode addrMode);

public:
	static void GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static EffectiveAddressInfo GetEffectiveAddress(DisassemblyInfo& info, PceConsole* console, PceCpuState& state);

	static uint8_t GetOpSize(uint8_t opCode);
	static char const* const GetOpName(uint8_t opCode);
	static PceAddrMode GetOpMode(uint8_t opCode);
	static bool IsUnconditionalJump(uint8_t opCode);
	static bool IsConditionalJump(uint8_t opCode);
	static bool IsJumpToSub(uint8_t opCode);
	static bool IsReturnInstruction(uint8_t opCode);
	static CdlFlags::CdlFlags GetOpFlags(uint8_t opCode, uint16_t pc, uint16_t prevPc);
	static bool IsOpUnofficial(uint8_t opCode);
};
