#pragma once
#include "stdafx.h"
#include "DisassemblyInfo.h"
#include "DebugTypes.h"
#include "../Utilities/SimpleLock.h"

class MemoryManager;
class Console;
class CodeDataLogger;
struct CpuState;

class Disassembler
{
private:
	MemoryManager *_memoryManager;
	Console *_console;
	shared_ptr<CodeDataLogger> _cdl;

	vector<DisassemblyInfo> _prgCache;
	vector<DisassemblyInfo> _wramCache;
	vector<DisassemblyInfo> _sramCache;
	
	SimpleLock _disassemblyLock;
	vector<DisassemblyResult> _disassembly;

	bool _needDisassemble = true;

	uint8_t *_prgRom;
	uint32_t _prgRomSize;
	uint8_t *_wram;
	uint32_t _wramSize;
	uint8_t *_sram;
	uint32_t _sramSize;

	void GetSource(AddressInfo &info, uint8_t **source, uint32_t &size, vector<DisassemblyInfo> **cache);

public:
	Disassembler(shared_ptr<Console> console, shared_ptr<CodeDataLogger> cdl);

	uint32_t BuildCache(AddressInfo &addrInfo, uint8_t cpuFlags);
	void InvalidateCache(AddressInfo addrInfo);
	void Disassemble();

	DisassemblyInfo GetDisassemblyInfo(AddressInfo &info);

	uint32_t GetLineCount();
	uint32_t GetLineIndex(uint32_t cpuAddress);
	bool GetLineData(uint32_t lineIndex, CodeLineData &data);
	int32_t SearchDisassembly(const char* searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards);
};