#include "pch.h"
#include "GBA/Debugger/GbaTraceLogger.h"
#include "GBA/GbaPpu.h"
#include "GBA/GbaTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/HexUtilities.h"

GbaTraceLogger::GbaTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, GbaPpu* ppu) : BaseTraceLogger(debugger, cpuDebugger, CpuType::Gba)
{
	_ppu = ppu;
}

RowDataType GbaTraceLogger::GetFormatTagType(string& tag)
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
	} else if(tag == "CPSR") {
		return RowDataType::CPSR;
	} else if(tag == "Mode") {
		return RowDataType::Mode;
	} else {
		return RowDataType::Text;
	}
}

void GbaTraceLogger::GetTraceRow(string &output, GbaCpuState &cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo)
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

			case RowDataType::CPSR: {
				if(rowPart.DisplayInHex) {
					WriteIntValue(output, cpuState.CPSR.ToInt32(), rowPart);
				} else {
					FastString flags;
					
					bool showInactive = (rowPart.MinWidth > 0);

					flags.Write(cpuState.CPSR.Negative ? "N" : (showInactive ? "n" : ""));
					flags.Write(cpuState.CPSR.Zero ? "Z" : (showInactive ? "z" : ""));
					flags.Write(cpuState.CPSR.Carry ? "C" : (showInactive ? "c" : ""));
					flags.Write(cpuState.CPSR.Overflow ? "V" : (showInactive ? "v" : ""));
					
					flags.Write(cpuState.CPSR.Thumb ? "T" : (showInactive ? "t" : ""));
					flags.Write(cpuState.CPSR.FiqDisable ? "F" : (showInactive ? "f" : ""));
					flags.Write(cpuState.CPSR.IrqDisable ? "I" : (showInactive ? "i" : ""));

					WriteStringValue(output, flags.ToString(), rowPart);
				}
				break;
			}

			case RowDataType::Mode: {
				switch(cpuState.CPSR.Mode) {
					case GbaCpuMode::User: WriteStringValue(output, "USR", rowPart); break;
					case GbaCpuMode::Fiq: WriteStringValue(output, "FIQ", rowPart); break;
					case GbaCpuMode::Irq: WriteStringValue(output, "IRQ", rowPart); break;
					case GbaCpuMode::Supervisor: WriteStringValue(output, "SVC", rowPart); break;
					case GbaCpuMode::Abort: WriteStringValue(output, "ABT", rowPart); break;
					case GbaCpuMode::Undefined: WriteStringValue(output, "UND", rowPart); break;
					case GbaCpuMode::System: WriteStringValue(output, "SYS", rowPart); break;
				}
				break;
			}

			default: ProcessSharedTag(rowPart, output, cpuState, ppuState, disassemblyInfo); break;
		}
	}
}

void GbaTraceLogger::LogPpuState()
{
	_ppuState[_currentPos] = {
		_ppu->GetCycle(),
		_ppu->GetCycle(),
		_ppu->GetScanline(),
		_ppu->GetFrameCount()
	};
}