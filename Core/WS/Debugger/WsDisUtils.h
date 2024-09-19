#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class WsConsole;
class WsMemoryManager;
class LabelManager;
class EmuSettings;
class MemoryDumper;
class FastString;
struct WsCpuState;
enum class WsSegment : uint8_t;

class WsDisUtils
{
private:
	static void GetModRegParam(FastString& str, uint8_t* byteCode, bool word);
	static void GetModSegRegParam(FastString& str, uint8_t* byteCode);
	static int GetModRmParam(FastString& str, uint8_t* byteCode, WsSegment segment, bool word, bool forLeaLdsLes = false);
	static int GetModRmSize(uint8_t modRm);
	static bool IsPrefix(uint8_t opCode);
	static void GetJmpDestination(FastString& str, uint8_t* byteCode, uint8_t size);
	static void GetSegment(FastString& str, WsSegment segment, const char* defaultSegment);

public:
	static void GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static EffectiveAddressInfo GetEffectiveAddress(DisassemblyInfo& info, WsConsole* console, WsCpuState& state);
	
	static uint8_t GetOpSize(uint32_t cpuAddress, MemoryType memType, MemoryDumper* memoryDumper);

	static uint16_t GetFullOpCode(uint16_t cs, uint16_t ip, WsMemoryManager* memoryManager);
	static uint16_t GetFullOpCode(DisassemblyInfo& disInfo);

	static bool IsJumpToSub(uint16_t opCode);
	static bool IsReturnInstruction(uint16_t opCode);
	static bool IsUnconditionalJump(uint16_t opCode);
	static bool IsConditionalJump(uint16_t opCode);

	static bool IsUndefinedOpCode(uint16_t opCode);
	static bool IsPushPopInstruction(uint16_t opCode);

	static CdlFlags::CdlFlags GetOpFlags(uint16_t opCode, uint32_t pc, uint32_t prevPc, uint8_t opSize);
	static void UpdateAddressCsIp(uint32_t addr, WsCpuState& state);
};
