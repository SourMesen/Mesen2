#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "SNES/Coprocessors/DSP/NecDspTypes.h"

class DisassemblyInfo;
class Debugger;
class SnesPpu;
class SnesMemoryManager;

class NecDspTraceLogger : public BaseTraceLogger<NecDspTraceLogger, NecDspState>
{
private:
	SnesPpu* _ppu = nullptr;
	SnesMemoryManager* _memoryManager = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

	void WriteAccFlagsValue(string& output, NecDspAccFlags flags, RowPart& rowPart);

public:
	NecDspTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, SnesPpu* ppu, SnesMemoryManager* memoryManager);
	
	void GetTraceRow(string& output, NecDspState& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(NecDspState& state) { return state.PC; }
	__forceinline uint64_t GetCycleCount(NecDspState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(NecDspState& state) { return state.SP; }
};
