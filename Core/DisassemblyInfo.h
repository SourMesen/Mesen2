#pragma once
#include "stdafx.h"
#include <unordered_map>

class MemoryManager;

struct CpuState;
enum class AddrMode : uint8_t;

class DisassemblyInfo
{
public:
	static string OpName[256];
	static AddrMode OpMode[256];
	static uint8_t OpSize[256];

private:
	uint8_t _byteCode[4];
	uint8_t _opSize;
	AddrMode _addrMode;
	uint8_t _flags;
	bool _emulationMode;
	int32_t _effectiveAddress;

public:
	DisassemblyInfo();
	DisassemblyInfo(CpuState &state, MemoryManager *memoryManager);

	void GetDisassembly(string &out, uint32_t memoryAddr);
	uint32_t GetOperandAddress(uint32_t memoryAddr);
	uint8_t GetOperandSize();
	void GetByteCode(string &out);
	
	void SetEffectiveAddress(int32_t effectiveAddress);
	void GetEffectiveAddressString(string &out);
	int32_t GetEffectiveAddress();

	/*int32_t GetMemoryValue(CpuState& cpuState, MemoryManager* memoryManager);
	uint16_t GetJumpDestination(uint16_t pc, MemoryManager* memoryManager);
	uint16_t GetIndirectJumpDestination(MemoryManager* memoryManager);
	*/
};

