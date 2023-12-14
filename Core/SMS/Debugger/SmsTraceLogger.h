#pragma once
#include "pch.h"
#include "Debugger/BaseTraceLogger.h"
#include "SMS/SmsTypes.h"

class DisassemblyInfo;
class Debugger;
class SmsVdp;

class SmsTraceLogger : public BaseTraceLogger<SmsTraceLogger, SmsCpuState>
{
private:
	SmsVdp* _vdp = nullptr;
	
protected:
	RowDataType GetFormatTagType(string& tag) override;

public:
	SmsTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, SmsVdp* vdp);
	
	void GetTraceRow(string& output, SmsCpuState& cpuState, TraceLogPpuState& vdpState, DisassemblyInfo& disassemblyInfo);
	void LogPpuState();

	__forceinline uint32_t GetProgramCounter(SmsCpuState& state) { return state.PC; }
	__forceinline uint64_t GetCycleCount(SmsCpuState& state) { return state.CycleCount; }
	__forceinline uint8_t GetStackPointer(SmsCpuState& state) { return (uint8_t)state.SP; }
};
