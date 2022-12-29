#include "pch.h"
#include "NES/Debugger/NesTraceLogger.h"
#include "NES/NesConsole.h"
#include "NES/BaseNesPpu.h"
#include "NES/NesTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/HexUtilities.h"

NesTraceLogger::NesTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, NesConsole* console) : BaseTraceLogger(debugger, cpuDebugger, CpuType::Nes)
{
	_console = console;
}

RowDataType NesTraceLogger::GetFormatTagType(string& tag)
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

void NesTraceLogger::GetTraceRow(string &output, NesCpuState &cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	constexpr char activeStatusLetters[8] = { 'N', 'V', '-', '-', 'D', 'I', 'Z', 'C' };
	constexpr char inactiveStatusLetters[8] = { 'n', 'v', '-', '-', 'd', 'i', 'z', 'c' };

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

void NesTraceLogger::LogPpuState()
{
	BaseNesPpu* ppu = _console->GetPpu();
	_ppuState[_currentPos] = {
		ppu->GetCurrentCycle(),
		ppu->GetCurrentCycle(),
		ppu->GetCurrentScanline(),
		ppu->GetFrameCount()
	};
}