#include "pch.h"
#include "Gameboy/Debugger/GbTraceLogger.h"
#include "Gameboy/GbPpu.h"
#include "Gameboy/GbTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/HexUtilities.h"

GbTraceLogger::GbTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, GbPpu* ppu) : BaseTraceLogger(debugger, cpuDebugger, CpuType::Gameboy)
{
	_ppu = ppu;
}

RowDataType GbTraceLogger::GetFormatTagType(string& tag)
{
	if(tag == "A") {
		return RowDataType::A;
	} else if(tag == "B") {
		return RowDataType::B;
	} else if(tag == "C") {
		return RowDataType::C;
	} else if(tag == "D") {
		return RowDataType::D;
	} else if(tag == "E") {
		return RowDataType::E;
	} else if(tag == "F") {
		return RowDataType::F;
	} else if(tag == "H") {
		return RowDataType::H;
	} else if(tag == "L") {
		return RowDataType::L;
	} else if(tag == "PS") {
		return RowDataType::PS;
	} else if(tag == "SP") {
		return RowDataType::SP;
	} else {
		return RowDataType::Text;
	}
}

void GbTraceLogger::GetTraceRow(string &output, GbCpuState &cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	constexpr char activeStatusLetters[4] = { 'Z', 'N', 'H', 'C' };
	constexpr char inactiveStatusLetters[4] = { 'z', 'n', 'h', 'c' };

	for(RowPart& rowPart : _rowParts) {
		switch(rowPart.DataType) {
			case RowDataType::A: WriteIntValue(output, cpuState.A, rowPart); break;
			case RowDataType::B: WriteIntValue(output, cpuState.B, rowPart); break;
			case RowDataType::C: WriteIntValue(output, cpuState.C, rowPart); break;
			case RowDataType::D: WriteIntValue(output, cpuState.D, rowPart); break;
			case RowDataType::E: WriteIntValue(output, cpuState.E, rowPart); break;
			case RowDataType::F: WriteIntValue(output, cpuState.Flags, rowPart); break;
			case RowDataType::H: WriteIntValue(output, cpuState.H, rowPart); break;
			case RowDataType::L: WriteIntValue(output, cpuState.L, rowPart); break;
			case RowDataType::SP: WriteIntValue(output, cpuState.SP, rowPart); break;
			case RowDataType::PS: GetStatusFlag(activeStatusLetters, inactiveStatusLetters, output, cpuState.Flags >> 4, rowPart, 4); break;
			default: ProcessSharedTag(rowPart, output, cpuState, ppuState, disassemblyInfo); break;
		}
	}
}

void GbTraceLogger::LogPpuState()
{
	_ppuState[_currentPos] = {
		_ppu->GetCycle(),
		_ppu->GetCycle(),
		_ppu->GetScanline(),
		_ppu->GetFrameCount()
	};
}