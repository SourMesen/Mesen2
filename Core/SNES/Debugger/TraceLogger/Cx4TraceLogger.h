#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "SNES/Coprocessors/CX4/Cx4Types.h"

class DisassemblyInfo;
class Debugger;
class SnesPpu;
class SnesMemoryManager;

class Cx4TraceLogger : public BaseTraceLogger<Cx4TraceLogger, Cx4State>
{
private:
	SnesPpu* _ppu = nullptr;
	SnesMemoryManager* _memoryManager = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	Cx4TraceLogger(Debugger* debugger, IDebugger* cpuDebugger, SnesPpu* ppu, SnesMemoryManager* memoryManager);
	
	void GetTraceRow(string& output, Cx4State& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(Cx4State& state) { return (state.Cache.Address[state.Cache.Page] + (state.PC * 2)) & 0xFFFFFF; }
	__forceinline uint64_t GetCycleCount(Cx4State& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(Cx4State& state) { return state.SP; }
};
