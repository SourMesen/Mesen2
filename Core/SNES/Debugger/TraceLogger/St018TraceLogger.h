#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "SNES/Coprocessors/ST018/ArmV3Types.h"

class DisassemblyInfo;
class Debugger;
class SnesPpu;

class St018TraceLogger : public BaseTraceLogger<St018TraceLogger, ArmV3CpuState>
{
private:
	SnesPpu* _ppu = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	St018TraceLogger(Debugger* debugger, IDebugger* cpuDebugger, SnesPpu* ppu);
	
	void GetTraceRow(string& output, ArmV3CpuState& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(ArmV3CpuState& state) { return state.Pipeline.Execute.Address; }
	__forceinline uint64_t GetCycleCount(ArmV3CpuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(ArmV3CpuState& state) { return (uint8_t)state.R[13]; }
};
