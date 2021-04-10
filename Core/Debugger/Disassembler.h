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
struct CpuState;
enum class CpuType : uint8_t;

struct DisassemblerSource
{
	uint8_t *Data;
	vector<DisassemblyInfo> *Cache;
	uint32_t Size;
};

class Disassembler
{
private:
	IConsole *_console;
	EmuSettings* _settings;
	Debugger *_debugger;
	shared_ptr<LabelManager> _labelManager;
	MemoryDumper *_memoryDumper;

	DisassemblerSource _sources[(int)SnesMemoryType::Register] = {};
	vector<DisassemblyInfo> _disassemblyCache[(int)SnesMemoryType::Register];

	SimpleLock _disassemblyLock;
	vector<DisassemblyResult> _disassemblyResult[(int)DebugUtilities::GetLastCpuType()+1];
	bool _needDisassemble[(int)DebugUtilities::GetLastCpuType()+1];

	void InitSource(SnesMemoryType type);
	DisassemblerSource& GetSource(SnesMemoryType type);
	void SetDisassembleFlag(CpuType type);

public:
	Disassembler(IConsole* console, Debugger* debugger);

	uint32_t BuildCache(AddressInfo &addrInfo, uint8_t cpuFlags, CpuType type);
	void ResetPrgCache();
	void InvalidateCache(AddressInfo addrInfo, CpuType type);
	void Disassemble(CpuType cpuType);

	__forceinline DisassemblyInfo GetDisassemblyInfo(AddressInfo& info, uint32_t cpuAddress, uint8_t cpuFlags, CpuType type)
	{
		DisassemblyInfo disassemblyInfo = (*GetSource(info.Type).Cache)[info.Address];
		if(!disassemblyInfo.IsInitialized()) {
			disassemblyInfo.Initialize(cpuAddress, cpuFlags, type, _memoryDumper);
		}
		return disassemblyInfo;
	}

	void RefreshDisassembly(CpuType type);
	uint32_t GetLineCount(CpuType type);
	uint32_t GetLineIndex(CpuType type, uint32_t cpuAddress);
	bool GetLineData(CpuType type, uint32_t lineIndex, CodeLineData &data);
	int32_t SearchDisassembly(CpuType type, const char* searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards);
};