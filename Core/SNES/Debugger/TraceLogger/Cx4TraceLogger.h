#pragma once
#include "stdafx.h"
#include "Debugger/BaseTraceLogger.h"
#include "SNES/Coprocessors/CX4/Cx4Types.h"

class DisassemblyInfo;
class Debugger;
class Ppu;
class MemoryManager;

class Cx4TraceLogger : public BaseTraceLogger<Cx4TraceLogger, Cx4State>
{
private:
	Ppu* _ppu = nullptr;
	MemoryManager* _memoryManager = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	Cx4TraceLogger(Debugger* debugger, Ppu* ppu, MemoryManager* memoryManager);
	
	void GetTraceRow(string& output, Cx4State& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(Cx4State& state) { return (state.Cache.Address[state.Cache.Page] + (state.PC * 2)) & 0xFFFFFF; }
	__forceinline uint64_t GetCycleCount(Cx4State& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(Cx4State& state) { return state.SP; }
};
