#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "Gameboy/GbTypes.h"

class DisassemblyInfo;
class Debugger;
class GbPpu;

class GbTraceLogger : public BaseTraceLogger<GbTraceLogger, GbCpuState>
{
private:
	GbPpu* _ppu = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	GbTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, GbPpu* ppu);
	
	void GetTraceRow(string& output, GbCpuState& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(GbCpuState& state) { return state.PC; }
	__forceinline uint64_t GetCycleCount(GbCpuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(GbCpuState& state) { return (uint8_t)state.SP; }
};
