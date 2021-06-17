#pragma once
#include "stdafx.h"
#include "Debugger/BaseTraceLogger.h"
#include "NES/NesTypes.h"

class DisassemblyInfo;
class Debugger;
class BaseNesPpu;

class NesTraceLogger : public BaseTraceLogger<NesTraceLogger, NesCpuState>
{
private:
	BaseNesPpu* _ppu = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	NesTraceLogger(Debugger* debugger, BaseNesPpu* ppu);
	
	void GetTraceRow(string& output, NesCpuState& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(NesCpuState& state) { return state.PC; }
	__forceinline uint64_t GetCycleCount(NesCpuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(NesCpuState& state) { return (uint8_t)state.SP; }
};
