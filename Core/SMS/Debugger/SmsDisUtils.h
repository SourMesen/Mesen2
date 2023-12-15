#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class SmsConsole;
class LabelManager;
class EmuSettings;
class MemoryDumper;
struct SmsCpuState;

enum class HlRegType
{
	HL,
	IX,
	IY
};

struct SmsOpInfo
{
	HlRegType HlType = HlRegType::HL;
	const char* Op = nullptr;
	int16_t IndexOffset = -1;
	uint8_t* ByteCode = nullptr;
	bool IsEdPrefix = false;
	bool IsCbPrefix = false;
};

class SmsDisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static SmsOpInfo GetSmsOpInfo(DisassemblyInfo& info);
	static EffectiveAddressInfo GetEffectiveAddress(DisassemblyInfo& info, SmsConsole* console, SmsCpuState& state);
	
	static uint8_t GetOpSize(uint8_t opCode, uint32_t cpuAddress, MemoryType memType, MemoryDumper* memoryDumper);

	static bool IsJumpToSub(uint8_t opCode);
	static bool IsReturnInstruction(uint16_t opCode);
	static bool IsUnconditionalJump(uint8_t opCode);
	static bool IsConditionalJump(uint8_t opCode);
	static CdlFlags::CdlFlags GetOpFlags(uint8_t opCode, uint16_t pc, uint16_t prevPc);
	static string GetOpTemplate(uint8_t op, bool cbPrefix, bool edPrefix);
};
