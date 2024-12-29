#include "pch.h"
#include "SNES/Debugger/TraceLogger/St018TraceLogger.h"
#include "SNES/SnesPpu.h"
#include "SNES/Coprocessors/ST018/ArmV3Types.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/HexUtilities.h"

St018TraceLogger::St018TraceLogger(Debugger* debugger, IDebugger* cpuDebugger, SnesPpu* ppu) : BaseTraceLogger(debugger, cpuDebugger, CpuType::St018)
{
	_ppu = ppu;
}

RowDataType St018TraceLogger::GetFormatTagType(string& tag)
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

void St018TraceLogger::GetTraceRow(string &output, ArmV3CpuState& cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo)
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
					
					flags.Write(cpuState.CPSR.FiqDisable ? "F" : (showInactive ? "f" : ""));
					flags.Write(cpuState.CPSR.IrqDisable ? "I" : (showInactive ? "i" : ""));

					WriteStringValue(output, flags.ToString(), rowPart);
				}
				break;
			}

			case RowDataType::Mode: {
				switch(cpuState.CPSR.Mode) {
					case ArmV3CpuMode::User: WriteStringValue(output, "USR", rowPart); break;
					case ArmV3CpuMode::Fiq: WriteStringValue(output, "FIQ", rowPart); break;
					case ArmV3CpuMode::Irq: WriteStringValue(output, "IRQ", rowPart); break;
					case ArmV3CpuMode::Supervisor: WriteStringValue(output, "SVC", rowPart); break;
					case ArmV3CpuMode::Abort: WriteStringValue(output, "ABT", rowPart); break;
					case ArmV3CpuMode::Undefined: WriteStringValue(output, "UND", rowPart); break;
					case ArmV3CpuMode::System: WriteStringValue(output, "SYS", rowPart); break;
				}
				break;
			}

			default: ProcessSharedTag(rowPart, output, cpuState, ppuState, disassemblyInfo); break;
		}
	}
}

void St018TraceLogger::LogPpuState()
{
	_ppuState[_currentPos] = {
		_ppu->GetCycle(),
		_ppu->GetCycle(),
		_ppu->GetScanline(),
		_ppu->GetFrameCount()
	};
}