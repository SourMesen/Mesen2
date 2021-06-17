#pragma once
#include "stdafx.h"
#include "Debugger/BaseTraceLogger.h"
#include "SNES/SpcTypes.h"

class DisassemblyInfo;
class Debugger;
class Ppu;
class MemoryManager;

class SpcTraceLogger : public BaseTraceLogger<SpcTraceLogger, SpcState>
{
private:
	Ppu* _ppu = nullptr;
	MemoryManager* _memoryManager = nullptr;

protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	SpcTraceLogger(Debugger* debugger, Ppu* ppu, MemoryManager* memoryManager);

	void GetTraceRow(string &output, SpcState &cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo);
	void LogPpuState();
	
	__forceinline uint32_t GetProgramCounter(SpcState& state) { return state.PC; }
	__forceinline uint64_t GetCycleCount(SpcState& state) { return state.Cycle; }
	__forceinline uint8_t GetStackPointer(SpcState& state) { return state.SP; }
};
