#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "SNES/SpcTypes.h"

class DisassemblyInfo;
class Debugger;
class SnesPpu;
class SnesMemoryManager;

class SpcTraceLogger : public BaseTraceLogger<SpcTraceLogger, SpcState>
{
private:
	SnesPpu* _ppu = nullptr;
	SnesMemoryManager* _memoryManager = nullptr;

protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	SpcTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, SnesPpu* ppu, SnesMemoryManager* memoryManager);

	void GetTraceRow(string &output, SpcState &cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo);
	void LogPpuState();
	
	__forceinline uint32_t GetProgramCounter(SpcState& state) { return state.PC; }
	__forceinline uint64_t GetCycleCount(SpcState& state) { return state.Cycle; }
	__forceinline uint8_t GetStackPointer(SpcState& state) { return state.SP; }
};
