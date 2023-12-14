#pragma once
#include "pch.h"
#include "Debugger/AddressInfo.h"
#include "Shared/MemoryType.h"

class IConsole;
class MemoryDumper;
class LabelManager;
class Debugger;
class EmuSettings;

enum class MemoryType;
enum class CpuType : uint8_t;

struct EffectiveAddressInfo
{
	AddressInfo Address = { -1, MemoryType::None };
	uint8_t ValueSize = 0;
	bool ShowAddress = false;

	EffectiveAddressInfo(int32_t address = -1, uint8_t valueSize = 1, bool showAddress = true, MemoryType memType = MemoryType::None)
	{
		if(address >= 0) {
			Address.Address = address;
			Address.Type = memType;
			ValueSize = valueSize;
			ShowAddress = showAddress;
		}
	}
};

class DisassemblyInfo
{
private:
	uint8_t _byteCode[8];
	uint8_t _opSize;
	uint8_t _flags;
	CpuType _cpuType;
	bool _initialized = false;

public:
	DisassemblyInfo();
	DisassemblyInfo(uint32_t cpuAddress, uint8_t cpuFlags, CpuType cpuType, MemoryType memType, MemoryDumper* memoryDumper);

	void Initialize(uint32_t cpuAddress, uint8_t cpuFlags, CpuType cpuType, MemoryType memType, MemoryDumper* memoryDumper);
	bool IsInitialized();
	bool IsValid(uint8_t cpuFlags);
	void Reset();

	void GetDisassembly(string &out, uint32_t memoryAddr, LabelManager *labelManager, EmuSettings* settings);
	
	CpuType GetCpuType();
	uint8_t GetOpCode();
	uint8_t GetOpSize();
	uint8_t GetFlags();
	uint8_t* GetByteCode();

	void GetByteCode(uint8_t copyBuffer[8]);
	void GetByteCode(string &out);

	static uint8_t GetOpSize(uint8_t opCode, uint8_t flags, CpuType type, uint32_t cpuAddress, MemoryType memType, MemoryDumper* memoryDumper);
	bool IsJumpToSub();
	bool IsReturnInstruction();
	
	bool CanDisassembleNextOp();

	bool IsUnconditionalJump();
	bool IsJump();
	void UpdateCpuFlags(uint8_t& cpuFlags);

	EffectiveAddressInfo GetEffectiveAddress(Debugger* debugger, void *cpuState, CpuType type);

	uint16_t GetMemoryValue(EffectiveAddressInfo effectiveAddress, MemoryDumper *memoryDumper, MemoryType memType);
};

