#pragma once
#include "stdafx.h"
#include "Debugger/BaseTraceLogger.h"
#include "PCE/PceTypes.h"

class DisassemblyInfo;
class Debugger;
class PcePpu;

class PceTraceLogger : public BaseTraceLogger<PceTraceLogger, PceCpuState>
{
private:
	PcePpu* _ppu = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	PceTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, PcePpu* ppu);
	
	void GetTraceRow(string& output, PceCpuState& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(PceCpuState& state) { return state.PC; }
	__forceinline uint64_t GetCycleCount(PceCpuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(PceCpuState& state) { return (uint8_t)state.SP; }
};
