#pragma once
#include "stdafx.h"
#include "DisassemblyInfo.h"
#include "DebugTypes.h"
#include "../Utilities/SimpleLock.h"

class MemoryManager;
class Console;
class Spc;
class Debugger;
class LabelManager;
class CodeDataLogger;
class MemoryDumper;
struct CpuState;
enum class CpuType : uint8_t;

class Disassembler
{
private:
	MemoryManager *_memoryManager;
	Console *_console;
	Spc *_spc;
	Debugger *_debugger;
	shared_ptr<CodeDataLogger> _cdl;
	shared_ptr<LabelManager> _labelManager;
	MemoryDumper *_memoryDumper;

	vector<DisassemblyInfo> _prgCache;
	vector<DisassemblyInfo> _wramCache;
	vector<DisassemblyInfo> _sramCache;
	vector<DisassemblyInfo> _spcRamCache;
	vector<DisassemblyInfo> _spcRomCache;
	vector<DisassemblyInfo> _necDspRomCache;
	vector<DisassemblyInfo> _sa1InternalRamCache;
	
	SimpleLock _disassemblyLock;
	vector<DisassemblyResult> _disassembly;
	vector<DisassemblyResult> _spcDisassembly;
	vector<DisassemblyResult> _sa1Disassembly;

	bool _needDisassemble[(int)CpuType::Sa1+1] = { true, true, true, true };

	uint8_t *_prgRom;
	uint32_t _prgRomSize;
	uint8_t *_wram;
	uint32_t _wramSize;
	uint8_t *_sram;
	uint32_t _sramSize;
	uint8_t *_spcRam;
	uint32_t _spcRamSize;
	uint8_t *_spcRom;
	uint32_t _spcRomSize;
	
	uint8_t *_necDspProgramRom;
	uint32_t _necDspProgramRomSize;

	uint8_t *_sa1InternalRam;
	uint32_t _sa1InternalRamSize;

	void GetSource(AddressInfo &info, uint8_t **source, uint32_t &size, vector<DisassemblyInfo> **cache);
	vector<DisassemblyResult>& GetDisassemblyList(CpuType type);
	void SetDisassembleFlag(CpuType type);

public:
	Disassembler(shared_ptr<Console> console, shared_ptr<CodeDataLogger> cdl, Debugger* debugger);

	uint32_t BuildCache(AddressInfo &addrInfo, uint8_t cpuFlags, CpuType type);
	void ResetPrgCache();
	void InvalidateCache(AddressInfo addrInfo, CpuType type);
	void Disassemble(CpuType cpuType);

	DisassemblyInfo GetDisassemblyInfo(AddressInfo &info);

	void RefreshDisassembly(CpuType type);
	uint32_t GetLineCount(CpuType type);
	uint32_t GetLineIndex(CpuType type, uint32_t cpuAddress);
	bool GetLineData(CpuType type, uint32_t lineIndex, CodeLineData &data);
	int32_t SearchDisassembly(CpuType type, const char* searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards);
};