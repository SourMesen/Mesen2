#include "pch.h"
#include "PCE/Debugger/PceTraceLogger.h"
#include "PCE/PceConsole.h"
#include "PCE/PceTypes.h"
#include "PCE/PceVdc.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/HexUtilities.h"

PceTraceLogger::PceTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, PceVdc* vdc) : BaseTraceLogger(debugger, cpuDebugger, CpuType::Pce)
{
	_vdc = vdc;
}

RowDataType PceTraceLogger::GetFormatTagType(string& tag)
{
	if(tag == "A") {
		return RowDataType::A;
	} else if(tag == "X") {
		return RowDataType::X;
	} else if(tag == "Y") {
		return RowDataType::Y;
	} else if(tag == "P") {
		return RowDataType::PS;
	} else if(tag == "SP") {
		return RowDataType::SP;
	} else {
		return RowDataType::Text;
	}
}

void PceTraceLogger::GetTraceRow(string &output, PceCpuState &cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	constexpr char activeStatusLetters[8] = { 'N', 'V', '-', 'T', 'D', 'I', 'Z', 'C' };
	constexpr char inactiveStatusLetters[8] = { 'n', 'v', '-', 't', 'd', 'i', 'z', 'c' };

	for(RowPart& rowPart : _rowParts) {
		switch(rowPart.DataType) {
			case RowDataType::A: WriteIntValue(output, cpuState.A, rowPart); break;
			case RowDataType::X: WriteIntValue(output, cpuState.X, rowPart); break;
			case RowDataType::Y: WriteIntValue(output, cpuState.Y, rowPart); break;
			case RowDataType::SP: WriteIntValue(output, cpuState.SP, rowPart); break;
			case RowDataType::PS: GetStatusFlag(activeStatusLetters, inactiveStatusLetters, output, cpuState.PS, rowPart); break;
			default: ProcessSharedTag(rowPart, output, cpuState, ppuState, disassemblyInfo); break;
		}
	}
}

void PceTraceLogger::LogPpuState()
{
	_ppuState[_currentPos] = {};
	
	_ppuState[_currentPos] = {
		_vdc->GetHClock(),
		_vdc->GetHClock(),
		_vdc->GetScanline(),
		_vdc->GetFrameCount()
	};
}