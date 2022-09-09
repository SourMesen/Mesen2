#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "SNES/Coprocessors/GSU/GsuTypes.h"

class DisassemblyInfo;
class Debugger;
class SnesPpu;
class SnesMemoryManager;

class GsuTraceLogger : public BaseTraceLogger<GsuTraceLogger, GsuState>
{
private:
	SnesPpu* _ppu = nullptr;
	SnesMemoryManager* _memoryManager = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	GsuTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, SnesPpu* ppu, SnesMemoryManager* memoryManager);
	
	void GetTraceRow(string& output, GsuState& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(GsuState& state) { return (state.ProgramBank << 16) | state.R[15]; }
	__forceinline uint64_t GetCycleCount(GsuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(GsuState& state) { return 0; }
};
