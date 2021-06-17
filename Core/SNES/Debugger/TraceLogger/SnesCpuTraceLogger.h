#pragma once
#include "stdafx.h"
#include "Debugger/BaseTraceLogger.h"
#include "SNES/CpuTypes.h"

class DisassemblyInfo;
class Debugger;
class Ppu;
class MemoryManager;

class SnesCpuTraceLogger : public BaseTraceLogger<SnesCpuTraceLogger, CpuState>
{
private:
	Ppu* _ppu = nullptr;
	MemoryManager* _memoryManager = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	SnesCpuTraceLogger(Debugger* debugger, CpuType cpuType, Ppu* ppu, MemoryManager* memoryManager);
	
	void GetTraceRow(string &output, CpuState &cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(CpuState& state) { return (state.K << 16) | state.PC; }
	__forceinline uint64_t GetCycleCount(CpuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(CpuState& state) { return (uint8_t)state.SP; }
};
