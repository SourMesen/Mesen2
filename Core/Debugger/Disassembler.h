#pragma once
#include "stdafx.h"
#include "DisassemblyInfo.h"
#include "DebugTypes.h"
#include "DebugUtilities.h"
#include "Utilities/SimpleLock.h"

class IConsole;
class Debugger;
class LabelManager;
class CodeDataLogger;
class MemoryDumper;
class EmuSettings;
struct SnesCpuState;
enum class CpuType : uint8_t;

struct DisassemblerSource
{
	uint8_t *Data;
	vector<DisassemblyInfo> Cache;
	uint32_t Size;
};

class Disassembler
{
private:
	IConsole *_console;
	EmuSettings* _settings;
	Debugger *_debugger;
	LabelManager* _labelManager;
	MemoryDumper *_memoryDumper;

	DisassemblerSource _sources[(int)SnesMemoryType::Register] = {};
	
	void InitSource(SnesMemoryType type);
	DisassemblerSource& GetSource(SnesMemoryType type);

	CodeLineData GetLineData(DisassemblyResult& result, CpuType type, SnesMemoryType memType);
	int32_t GetMatchingRow(vector<DisassemblyResult>& rows, uint32_t address);
	vector<DisassemblyResult> Disassemble(CpuType cpuType, uint16_t bank);
	uint16_t GetMaxBank(CpuType cpuType);

public:
	Disassembler(IConsole* console, Debugger* debugger);

	uint32_t BuildCache(AddressInfo &addrInfo, uint8_t cpuFlags, CpuType type);
	void ResetPrgCache();
	void InvalidateCache(AddressInfo addrInfo, CpuType type);

	__forceinline DisassemblyInfo GetDisassemblyInfo(AddressInfo& info, uint32_t cpuAddress, uint8_t cpuFlags, CpuType type)
	{
		DisassemblyInfo disassemblyInfo = GetSource(info.Type).Cache[info.Address];
		if(!disassemblyInfo.IsInitialized()) {
			disassemblyInfo.Initialize(cpuAddress, cpuFlags, type, _memoryDumper);
		}
		return disassemblyInfo;
	}

	uint32_t GetDisassemblyOutput(CpuType type, uint32_t address, CodeLineData output[], uint32_t rowCount);
	int32_t GetDisassemblyRowAddress(CpuType type, uint32_t address, int32_t rowOffset);
	int32_t SearchDisassembly(CpuType type, const char* searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards);
};