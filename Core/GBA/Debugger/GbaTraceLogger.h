#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "GBA/GbaTypes.h"

class DisassemblyInfo;
class Debugger;
class GbaPpu;

class GbaTraceLogger : public BaseTraceLogger<GbaTraceLogger, GbaCpuState>
{
private:
	GbaPpu* _ppu = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	GbaTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, GbaPpu* ppu);
	
	void GetTraceRow(string& output, GbaCpuState& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(GbaCpuState& state) { return state.Pipeline.Execute.Address; }
	__forceinline uint64_t GetCycleCount(GbaCpuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(GbaCpuState& state) { return (uint8_t)state.R[13]; }
};
