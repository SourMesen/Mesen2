#pragma once
#include "stdafx.h"
#include "DisassemblyInfo.h"
#include "DebugTypes.h"
#include "DebugUtilities.h"
#include "Utilities/SimpleLock.h"

class MemoryManager;
class Console;
class Cpu;
class Spc;
class Gsu;
class Sa1;
class Cx4;
class NecDsp;
class Gameboy;
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
	MemoryManager *_memoryManager;
	Console *_console;
	Cpu* _cpu;
	Spc* _spc;
	Gsu* _gsu;
	Sa1* _sa1;
	Cx4* _cx4;
	NecDsp* _necDsp;
	Gameboy* _gameboy;
	EmuSettings* _settings;
	Debugger *_debugger;
	shared_ptr<CodeDataLogger> _cdl;
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
	Disassembler(shared_ptr<Console> console, shared_ptr<CodeDataLogger> cdl, Debugger* debugger);

	uint32_t BuildCache(AddressInfo &addrInfo, uint8_t cpuFlags, CpuType type);
	void ResetPrgCache();
	void InvalidateCache(AddressInfo addrInfo, CpuType type);
	void Disassemble(CpuType cpuType);

	DisassemblyInfo GetDisassemblyInfo(AddressInfo &info, uint32_t cpuAddress, uint8_t cpuFlags, CpuType type);

	void RefreshDisassembly(CpuType type);
	uint32_t GetLineCount(CpuType type);
	uint32_t GetLineIndex(CpuType type, uint32_t cpuAddress);
	bool GetLineData(CpuType type, uint32_t lineIndex, CodeLineData &data);
	int32_t SearchDisassembly(CpuType type, const char* searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards);
};