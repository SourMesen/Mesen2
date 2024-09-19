#include "pch.h"
#include "WS/Debugger/WsTraceLogger.h"
#include "WS/WsPpu.h"
#include "WS/WsTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FastString.h"

WsTraceLogger::WsTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, WsPpu* ppu) : BaseTraceLogger(debugger, cpuDebugger, CpuType::Ws)
{
	_ppu = ppu;
}

RowDataType WsTraceLogger::GetFormatTagType(string& tag)
{
	if(tag == "AX") {
		return RowDataType::AX;
	} else if(tag == "BX") {
		return RowDataType::BX;
	} else if(tag == "CX") {
		return RowDataType::CX;
	} else if(tag == "DX") {
		return RowDataType::DX;
	} else if(tag == "CS") {
		return RowDataType::CS;
	} else if(tag == "IP") {
		return RowDataType::IP;
	} else if(tag == "SS") {
		return RowDataType::SS;
	} else if(tag == "SP") {
		return RowDataType::SP;
	} else if(tag == "BP") {
		return RowDataType::BP;
	} else if(tag == "DS") {
		return RowDataType::DS;
	} else if(tag == "ES") {
		return RowDataType::ES;
	} else if(tag == "SI") {
		return RowDataType::SI;
	} else if(tag == "DI") {
		return RowDataType::DI;
	} else if(tag == "F") {
		return RowDataType::F;
	} else if(tag == "PC") {
		return RowDataType::PC;
	} else {
		return RowDataType::Text;
	}
}

void WsTraceLogger::GetTraceRow(string &output, WsCpuState &cpuState, TraceLogPpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	for(RowPart& rowPart : _rowParts) {
		switch(rowPart.DataType) {
			case RowDataType::AX: WriteIntValue(output, cpuState.AX, rowPart); break;
			case RowDataType::BX: WriteIntValue(output, cpuState.BX, rowPart); break;
			case RowDataType::CX: WriteIntValue(output, cpuState.CX, rowPart); break;
			case RowDataType::DX: WriteIntValue(output, cpuState.DX, rowPart); break;
			case RowDataType::CS: WriteIntValue(output, cpuState.CS, rowPart); break;
			case RowDataType::IP: WriteIntValue(output, cpuState.IP, rowPart); break;
			case RowDataType::SS: WriteIntValue(output, cpuState.SS, rowPart); break;
			case RowDataType::SP: WriteIntValue(output, cpuState.SP, rowPart); break;
			case RowDataType::BP: WriteIntValue(output, cpuState.BP, rowPart); break;
			case RowDataType::DS: WriteIntValue(output, cpuState.DS, rowPart); break;
			case RowDataType::ES: WriteIntValue(output, cpuState.ES, rowPart); break;
			case RowDataType::SI: WriteIntValue(output, cpuState.SI, rowPart); break;
			case RowDataType::DI: WriteIntValue(output, cpuState.DI, rowPart); break;
			case RowDataType::F:
				if(rowPart.DisplayInHex) {
					WriteIntValue(output, cpuState.Flags.Get(), rowPart);
				} else {
					FastString flags;

					bool showInactive = (rowPart.MinWidth > 0);
					flags.Write(cpuState.Flags.Carry ? "C" : (showInactive ? "c" : ""));
					flags.Write(cpuState.Flags.Parity ? "P" : (showInactive ? "p" : ""));
					flags.Write(cpuState.Flags.AuxCarry ? "A" : (showInactive ? "a" : ""));
					flags.Write(cpuState.Flags.Zero ? "Z" : (showInactive ? "z" : ""));
					flags.Write(cpuState.Flags.Sign ? "S" : (showInactive ? "s" : ""));
					flags.Write(cpuState.Flags.Trap ? "T" : (showInactive ? "t" : ""));
					flags.Write(cpuState.Flags.Irq ? "I" : (showInactive ? "i" : ""));
					flags.Write(cpuState.Flags.Direction ? "D" : (showInactive ? "d" : ""));
					flags.Write(cpuState.Flags.Overflow ? "O" : (showInactive ? "o" : ""));
					flags.Write(cpuState.Flags.Mode ? "M" : (showInactive ? "m" : ""));
					WriteStringValue(output, flags.ToString(), rowPart);
				}
				break;

			default: ProcessSharedTag(rowPart, output, cpuState, ppuState, disassemblyInfo); break;
		}
	}
}

void WsTraceLogger::LogPpuState()
{
	_ppuState[_currentPos] = {
		_ppu->GetCycle(),
		_ppu->GetCycle(),
		_ppu->GetScanline(),
		_ppu->GetFrameCount()
	};
}