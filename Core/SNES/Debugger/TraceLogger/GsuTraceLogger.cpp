#include "pch.h"
#include "SNES/Debugger/TraceLogger/GsuTraceLogger.h"
#include "SNES/SnesPpu.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/Coprocessors/GSU/GsuTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/HexUtilities.h"

GsuTraceLogger::GsuTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, SnesPpu* ppu, SnesMemoryManager* memoryManager) : BaseTraceLogger(debugger, cpuDebugger, CpuType::Gsu)
{
	_ppu = ppu;
	_memoryManager = memoryManager;
}

RowDataType GsuTraceLogger::GetFormatTagType(string& tag)
{
	if(tag == "R0") {
		return RowDataType::R0;
	} else if(tag == "R1") {
		return RowDataType::R1;
	} else if(tag == "R2") {
		return RowDataType::R2;
	} else if(tag == "R3") {
		return RowDataType::R3;
	} else if(tag == "R4") {
		return RowDataType::R4;
	} else if(tag == "R5") {
		return RowDataType::R5;
	} else if(tag == "R6") {
		return RowDataType::R6;
	} else if(tag == "R7") {
		return RowDataType::R7;
	} else if(tag == "R8") {
		return RowDataType::R8;
	} else if(tag == "R9") {
		return RowDataType::R9;
	} else if(tag == "R10") {
		return RowDataType::R10;
	} else if(tag == "R11") {
		return RowDataType::R11;
	} else if(tag == "R12") {
		return RowDataType::R12;
	} else if(tag == "R13") {
		return RowDataType::R13;
	} else if(tag == "R14") {
		return RowDataType::R14;
	} else if(tag == "R15") {
		return RowDataType::R15;
	} else if(tag == "SRC") {
		return RowDataType::Src;
	} else if(tag == "DST") {
		return RowDataType::Dst;
	} else if(tag == "SFR") {
		return RowDataType::SFR;
	} else {
		return RowDataType::Text;
	}
}

void GsuTraceLogger::GetTraceRow(string &output, GsuState &cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	for(RowPart& rowPart : _rowParts) {
		switch(rowPart.DataType) {
			case RowDataType::R0: WriteIntValue(output, cpuState.R[0], rowPart); break;
			case RowDataType::R1: WriteIntValue(output, cpuState.R[1], rowPart); break;
			case RowDataType::R2: WriteIntValue(output, cpuState.R[2], rowPart); break;
			case RowDataType::R3: WriteIntValue(output, cpuState.R[3], rowPart); break;
			case RowDataType::R4: WriteIntValue(output, cpuState.R[4], rowPart); break;
			case RowDataType::R5: WriteIntValue(output, cpuState.R[5], rowPart); break;
			case RowDataType::R6: WriteIntValue(output, cpuState.R[6], rowPart); break;
			case RowDataType::R7: WriteIntValue(output, cpuState.R[7], rowPart); break;
			case RowDataType::R8: WriteIntValue(output, cpuState.R[8], rowPart); break;
			case RowDataType::R9: WriteIntValue(output, cpuState.R[9], rowPart); break;
			case RowDataType::R10: WriteIntValue(output, cpuState.R[10], rowPart); break;
			case RowDataType::R11: WriteIntValue(output, cpuState.R[11], rowPart); break;
			case RowDataType::R12: WriteIntValue(output, cpuState.R[12], rowPart); break;
			case RowDataType::R13: WriteIntValue(output, cpuState.R[13], rowPart); break;
			case RowDataType::R14: WriteIntValue(output, cpuState.R[14], rowPart); break;
			case RowDataType::R15: WriteIntValue(output, cpuState.R[15], rowPart); break;

			case RowDataType::Src: WriteIntValue(output, cpuState.SrcReg, rowPart); break;
			case RowDataType::Dst: WriteIntValue(output, cpuState.DestReg, rowPart); break;

			case RowDataType::SFR: {
				constexpr char activeStatusLetters[16] = { 'I', '-', '-', 'P', 'J', 'I', '2', '1', '-', 'P', 'R', 'V', 'S', 'C', 'Z', '-' };
				constexpr char inactiveStatusLetters[16] = { 'i', '-', '-', 'p', 'j', 'i', '-', '-', '-', 'p', 'r', 'V', 's', 'c', 'z', '-' };
				GetStatusFlag(activeStatusLetters, inactiveStatusLetters, output, (cpuState.SFR.GetFlagsHigh() << 8) | cpuState.SFR.GetFlagsLow(), rowPart, 16);
				break;
			}

			default: ProcessSharedTag(rowPart, output, cpuState, ppuState, disassemblyInfo); break;
		}
	}
}

void GsuTraceLogger::LogPpuState()
{
	_ppuState[_currentPos] = {
		_ppu->GetCycle(),
		_memoryManager->GetHClock(),
		_ppu->GetScanline(),
		_ppu->GetFrameCount()
	};
}