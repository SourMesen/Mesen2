#include "pch.h"
#include "SNES/Debugger/TraceLogger/SnesCpuTraceLogger.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/SnesPpu.h"
#include "SNES/SnesMemoryManager.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"

SnesCpuTraceLogger::SnesCpuTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, CpuType cpuType, SnesPpu* ppu, SnesMemoryManager* memoryManager) : BaseTraceLogger(debugger, cpuDebugger, cpuType)
{
	_ppu = ppu;
	_memoryManager = memoryManager;
}

RowDataType SnesCpuTraceLogger::GetFormatTagType(string& tag)
{
	if(tag == "A") {
		return RowDataType::A;
	} else if(tag == "X") {
		return RowDataType::X;
	} else if(tag == "Y") {
		return RowDataType::Y;
	} else if(tag == "D") {
		return RowDataType::D;
	} else if(tag == "DB") {
		return RowDataType::DB;
	} else if(tag == "P") {
		return RowDataType::PS;
	} else if(tag == "SP") {
		return RowDataType::SP;
	} else {
		return RowDataType::Text;
	}
}

void SnesCpuTraceLogger::GetTraceRow(string &output, SnesCpuState &cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	constexpr char activeStatusLetters[8] = { 'N', 'V', 'M', 'X', 'D', 'I', 'Z', 'C' };
	constexpr char inactiveStatusLetters[8] = { 'n', 'v', 'm', 'x', 'd', 'i', 'z', 'c' };

	for(RowPart& rowPart : _rowParts) {
		switch(rowPart.DataType) {
			case RowDataType::A: WriteIntValue(output, cpuState.A, rowPart); break;
			case RowDataType::X: WriteIntValue(output, cpuState.X, rowPart); break;
			case RowDataType::Y: WriteIntValue(output, cpuState.Y, rowPart); break;
			case RowDataType::D: WriteIntValue(output, cpuState.D, rowPart); break;
			case RowDataType::DB: WriteIntValue(output, cpuState.DBR, rowPart); break;
			case RowDataType::SP: WriteIntValue(output, cpuState.SP, rowPart); break;
			case RowDataType::PS: GetStatusFlag(activeStatusLetters, inactiveStatusLetters, output, cpuState.PS, rowPart); break;
			default: ProcessSharedTag(rowPart, output, cpuState, ppuState, disassemblyInfo); break;
		}
	}
}

void SnesCpuTraceLogger::LogPpuState()
{
	_ppuState[_currentPos] = {
		_ppu->GetCycle(),
		_memoryManager->GetHClock(),
		_ppu->GetScanline(),
		_ppu->GetFrameCount()
	};
}