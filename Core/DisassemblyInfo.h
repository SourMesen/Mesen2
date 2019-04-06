#pragma once
#include "stdafx.h"
#include <unordered_map>

class Console;
class MemoryManager;

struct CpuState;
enum class AddrMode : uint8_t;
enum class CpuType : uint8_t;

class DisassemblyInfo
{
private:
	uint8_t _byteCode[4];
	uint8_t _opSize;
	uint8_t _flags;
	CpuType _cpuType;
	bool _initialized = false;

public:
	DisassemblyInfo();
	DisassemblyInfo(uint8_t *opPointer, uint8_t cpuFlags, CpuType type);

	void Initialize(uint8_t *opPointer, uint8_t cpuFlags, CpuType type);
	bool IsInitialized();
	void Reset();

	void GetDisassembly(string &out, uint32_t memoryAddr);
	
	uint8_t GetOpCode();
	uint8_t GetOperandSize();
	uint8_t GetFlags();
	uint8_t* GetByteCode();

	void GetByteCode(uint8_t copyBuffer[4]);
	void GetByteCode(string &out);

	static uint8_t GetOperandSize(uint8_t opCode, uint8_t flags, CpuType type);
	
	int32_t GetEffectiveAddress(Console *console, void *cpuState);
	uint16_t GetMemoryValue(uint32_t effectiveAddress, MemoryManager *memoryManager, uint8_t &valueSize);
};

