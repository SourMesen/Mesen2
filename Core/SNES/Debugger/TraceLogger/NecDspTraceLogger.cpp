#include "pch.h"
#include "SNES/Debugger/TraceLogger/NecDspTraceLogger.h"
#include "SNES/SnesPpu.h"
#include "SNES/SnesMemoryManager.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/HexUtilities.h"

NecDspTraceLogger::NecDspTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, SnesPpu* ppu, SnesMemoryManager* memoryManager) : BaseTraceLogger(debugger, cpuDebugger, CpuType::NecDsp)
{
	_ppu = ppu;
	_memoryManager = memoryManager;
}

RowDataType NecDspTraceLogger::GetFormatTagType(string& tag)
{
	if(tag == "A") {
		return RowDataType::A;
	} else if(tag == "B") {
		return RowDataType::B;
	}if(tag == "FlagsA") {
		return RowDataType::FlagsA;
	} else if(tag == "FlagsB") {
		return RowDataType::FlagsB;
	} else if(tag == "K") {
		return RowDataType::K;
	} else if(tag == "L") {
		return RowDataType::L;
	} else if(tag == "M") {
		return RowDataType::M;
	} else if(tag == "N") {
		return RowDataType::N;
	} else if(tag == "RP") {
		return RowDataType::RP;
	} else if(tag == "DP") {
		return RowDataType::DP;
	} else if(tag == "DR") {
		return RowDataType::DR;
	} else if(tag == "SR") {
		return RowDataType::SR;
	} else if(tag == "TR") {
		return RowDataType::TR;
	} else if(tag == "TRB") {
		return RowDataType::TRB;
	} else {
		return RowDataType::Text;
	}
}

void NecDspTraceLogger::WriteAccFlagsValue(string& output, NecDspAccFlags flags, RowPart& rowPart)
{
	string status = string(flags.Carry ? "C" : "c") + (flags.Zero ? "Z" : "z") + (flags.Overflow0 ? "V" : "v") + (flags.Overflow1 ? "V" : "v") + (flags.Sign0 ? "N" : "n") + (flags.Sign1 ? "N" : "n");
	WriteStringValue(output, status, rowPart);
}

void NecDspTraceLogger::GetTraceRow(string& output, NecDspState& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo)
{
	for(RowPart& rowPart : _rowParts) {
		switch(rowPart.DataType) {
			case RowDataType::A: WriteIntValue(output, cpuState.A, rowPart); break;
			case RowDataType::FlagsA: WriteAccFlagsValue(output, cpuState.FlagsA, rowPart); break;
			case RowDataType::B: WriteIntValue(output, cpuState.B, rowPart); break;
			case RowDataType::FlagsB: WriteAccFlagsValue(output, cpuState.FlagsB, rowPart); break;
			case RowDataType::K: WriteIntValue(output, cpuState.K, rowPart); break;
			case RowDataType::L: WriteIntValue(output, cpuState.L, rowPart); break;
			case RowDataType::M: WriteIntValue(output, cpuState.M, rowPart); break;
			case RowDataType::N: WriteIntValue(output, cpuState.N, rowPart); break;
			case RowDataType::RP: WriteIntValue(output, cpuState.RP, rowPart); break;
			case RowDataType::DP: WriteIntValue(output, cpuState.DP, rowPart); break;
			case RowDataType::DR: WriteIntValue(output, cpuState.DR, rowPart); break;
			case RowDataType::SR: WriteIntValue(output, cpuState.SR, rowPart); break;
			case RowDataType::TR: WriteIntValue(output, cpuState.TR, rowPart); break;
			case RowDataType::TRB: WriteIntValue(output, cpuState.TRB, rowPart); break;

			default: ProcessSharedTag(rowPart, output, cpuState, ppuState, disassemblyInfo); break;
		}
	}
}

void NecDspTraceLogger::LogPpuState()
{
	_ppuState[_currentPos] = {
		_ppu->GetCycle(),
		_memoryManager->GetHClock(),
		_ppu->GetScanline(),
		_ppu->GetFrameCount()
	};
}