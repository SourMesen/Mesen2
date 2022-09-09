#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "SNES/SnesCpuTypes.h"

class DisassemblyInfo;
class Debugger;
class SnesPpu;
class SnesMemoryManager;

class SnesCpuTraceLogger : public BaseTraceLogger<SnesCpuTraceLogger, SnesCpuState>
{
private:
	SnesPpu* _ppu = nullptr;
	SnesMemoryManager* _memoryManager = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	SnesCpuTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, CpuType cpuType, SnesPpu* ppu, SnesMemoryManager* memoryManager);
	
	void GetTraceRow(string &output, SnesCpuState &cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(SnesCpuState& state) { return (state.K << 16) | state.PC; }
	__forceinline uint64_t GetCycleCount(SnesCpuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(SnesCpuState& state) { return (uint8_t)state.SP; }
};
