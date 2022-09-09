#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "PCE/PceTypes.h"

class DisassemblyInfo;
class Debugger;
class PceVdc;

class PceTraceLogger : public BaseTraceLogger<PceTraceLogger, PceCpuState>
{
private:
	PceVdc* _vdc = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	PceTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, PceVdc* vdc);
	
	void GetTraceRow(string& output, PceCpuState& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(PceCpuState& state) { return state.PC; }
	__forceinline uint64_t GetCycleCount(PceCpuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(PceCpuState& state) { return (uint8_t)state.SP; }
};
