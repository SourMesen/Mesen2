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

public:
	DisassemblyInfo();
	DisassemblyInfo(CpuState &state, MemoryManager *memoryManager);

	void GetDisassembly(string &out, MemoryManager* memoryManager);
	uint8_t GetOperandSize();
	void GetByteCode(string &out);
	/*
	int32_t GetEffectiveAddress(CpuState& cpuState, MemoryManager* memoryManager);
	void GetEffectiveAddressString(string &out, CpuState& cpuState, MemoryManager* memoryManager);

	/*int32_t GetMemoryValue(CpuState& cpuState, MemoryManager* memoryManager);
	uint16_t GetJumpDestination(uint16_t pc, MemoryManager* memoryManager);
	uint16_t GetIndirectJumpDestination(MemoryManager* memoryManager);
	void ToString(string &out, uint32_t memoryAddr, MemoryManager* memoryManager, bool extendZeroPage);
	void GetByteCode(string &out);
	uint32_t GetSize();
	uint16_t GetOpAddr(uint16_t memoryAddr);*/
};

