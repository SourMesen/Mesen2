#pragma once
#include "stdafx.h"

class DisassemblyInfo;
class LabelManager;
class MemoryDumper;
class EmuSettings;
struct NesCpuState;
enum class NesAddrMode;

class NesDisUtils
{
private:
	static uint8_t OpSize[17];
	static string OpName[256];
	static NesAddrMode OpMode[256];

	static uint32_t GetOperandAddress(DisassemblyInfo& info, uint32_t memoryAddr);
	static uint8_t GetOpSize(NesAddrMode addrMode);

public:
	static void GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static uint8_t GetOpSize(uint8_t opCode);
	static int32_t NesDisUtils::GetEffectiveAddress(DisassemblyInfo& info, NesCpuState& state, MemoryDumper* memoryDumper);
};
