#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "WS/WsTypes.h"

class DisassemblyInfo;
class Debugger;
class WsPpu;

class WsTraceLogger : public BaseTraceLogger<WsTraceLogger, WsCpuState>
{
private:
	WsPpu* _ppu = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	WsTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, WsPpu* ppu);
	
	void GetTraceRow(string& output, WsCpuState& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(WsCpuState& state) { return (state.CS << 4) + state.IP; }
	__forceinline uint64_t GetCycleCount(WsCpuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(WsCpuState& state) { return (uint8_t)state.SP; }
};
