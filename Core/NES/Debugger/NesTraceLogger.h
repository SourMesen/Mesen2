#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "NES/NesTypes.h"

class DisassemblyInfo;
class Debugger;
class NesConsole;

class NesTraceLogger : public BaseTraceLogger<NesTraceLogger, NesCpuState>
{
private:
	NesConsole* _console = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	NesTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, NesConsole* console);
	
	void GetTraceRow(string& output, NesCpuState& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(NesCpuState& state) { return state.PC; }
	__forceinline uint64_t GetCycleCount(NesCpuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(NesCpuState& state) { return (uint8_t)state.SP; }
};
