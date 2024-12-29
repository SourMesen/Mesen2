#include "pch.h"
#include "SNES/Debugger/TraceLogger/Cx4TraceLogger.h"
#include "SNES/SnesPpu.h"
#include "SNES/SnesMemoryManager.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/HexUtilities.h"

Cx4TraceLogger::Cx4TraceLogger(Debugger* debugger, IDebugger* cpuDebugger, SnesPpu* ppu, SnesMemoryManager* memoryManager) : BaseTraceLogger(debugger, cpuDebugger, CpuType::Cx4)
{
	_ppu = ppu;
	_memoryManager = memoryManager;
}

RowDataType Cx4TraceLogger::GetFormatTagType(string& tag)
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
	} else if(tag == "MAR") {
		return RowDataType::MAR;
	} else if(tag == "MDR") {
		return RowDataType::MDR;
	} else if(tag == "DPR") {
		return RowDataType::DPR;
	} else if(tag == "ML") {
		return RowDataType::ML;
	} else if(tag == "MH") {
		return RowDataType::MH;
	} else if(tag == "PB") {
		return RowDataType::PB;
	} else if(tag == "P") {
		return RowDataType::P;
	} else if(tag == "PS") {
		return RowDataType::PS;
	} else if(tag == "A") {
		return RowDataType::A;
	} else {
		return RowDataType::Text;
	}
}

void Cx4TraceLogger::GetTraceRow(string& output, Cx4State& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo)
{
	for(RowPart& rowPart : _rowParts) {
		switch(rowPart.DataType) {
			case RowDataType::PS: {
				string status = string(cpuState.Carry ? "C" : "c") + (cpuState.Zero ? "Z" : "z") + (cpuState.Overflow ? "V" : "v") + (cpuState.Negative ? "N" : "n");
				WriteStringValue(output, status, rowPart);
				break;
			}

			case RowDataType::A: WriteIntValue(output, cpuState.A, rowPart); break;

			case RowDataType::R0: WriteIntValue(output, cpuState.Regs[0], rowPart); break;
			case RowDataType::R1: WriteIntValue(output, cpuState.Regs[1], rowPart); break;
			case RowDataType::R2: WriteIntValue(output, cpuState.Regs[2], rowPart); break;
			case RowDataType::R3: WriteIntValue(output, cpuState.Regs[3], rowPart); break;
			case RowDataType::R4: WriteIntValue(output, cpuState.Regs[4], rowPart); break;
			case RowDataType::R5: WriteIntValue(output, cpuState.Regs[5], rowPart); break;
			case RowDataType::R6: WriteIntValue(output, cpuState.Regs[6], rowPart); break;
			case RowDataType::R7: WriteIntValue(output, cpuState.Regs[7], rowPart); break;
			case RowDataType::R8: WriteIntValue(output, cpuState.Regs[8], rowPart); break;
			case RowDataType::R9: WriteIntValue(output, cpuState.Regs[9], rowPart); break;
			case RowDataType::R10: WriteIntValue(output, cpuState.Regs[10], rowPart); break;
			case RowDataType::R11: WriteIntValue(output, cpuState.Regs[11], rowPart); break;
			case RowDataType::R12: WriteIntValue(output, cpuState.Regs[12], rowPart); break;
			case RowDataType::R13: WriteIntValue(output, cpuState.Regs[13], rowPart); break;
			case RowDataType::R14: WriteIntValue(output, cpuState.Regs[14], rowPart); break;
			case RowDataType::R15: WriteIntValue(output, cpuState.Regs[15], rowPart); break;

			case RowDataType::MAR: WriteIntValue(output, cpuState.MemoryAddressReg, rowPart); break;
			case RowDataType::MDR: WriteIntValue(output, cpuState.MemoryDataReg, rowPart); break;
			case RowDataType::DPR: WriteIntValue(output, cpuState.DataPointerReg, rowPart); break;
			case RowDataType::ML: WriteIntValue(output, (uint32_t)(cpuState.Mult & 0xFFFFFF), rowPart); break;
			case RowDataType::MH: WriteIntValue(output, (uint32_t)(cpuState.Mult >> 24), rowPart); break;

			case RowDataType::PB: WriteIntValue(output, cpuState.PB, rowPart); break;
			case RowDataType::P: WriteIntValue(output, cpuState.P, rowPart); break;

			default: ProcessSharedTag(rowPart, output, cpuState, ppuState, disassemblyInfo); break;
		}
	}
}

void Cx4TraceLogger::LogPpuState()
{
	_ppuState[_currentPos] = {
		_ppu->GetCycle(),
		_memoryManager->GetHClock(),
		_ppu->GetScanline(),
		_ppu->GetFrameCount()
	};
}