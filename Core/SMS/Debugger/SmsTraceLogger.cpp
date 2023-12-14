#include "pch.h"
#include "SMS/Debugger/SmsTraceLogger.h"
#include "SMS/SmsVdp.h"
#include "SMS/SmsTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/HexUtilities.h"

SmsTraceLogger::SmsTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, SmsVdp* vdp) : BaseTraceLogger(debugger, cpuDebugger, CpuType::Sms)
{
	_vdp = vdp;
}

RowDataType SmsTraceLogger::GetFormatTagType(string& tag)
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
	} else if(tag == "A'") {
		return RowDataType::AltA;
	} else if(tag == "B'") {
		return RowDataType::AltB;
	} else if(tag == "C'") {
		return RowDataType::AltC;
	} else if(tag == "D'") {
		return RowDataType::AltD;
	} else if(tag == "E'") {
		return RowDataType::AltE;
	} else if(tag == "F'") {
		return RowDataType::AltF;
	} else if(tag == "H'") {
		return RowDataType::AltH;
	} else if(tag == "L'") {
		return RowDataType::AltL;
	} else if(tag == "I") {
		return RowDataType::I;
	} else if(tag == "R") {
		return RowDataType::R;
	} else if(tag == "IX") {
		return RowDataType::IX;
	} else if(tag == "IY") {
		return RowDataType::IY;
	} else if(tag == "PS") {
		return RowDataType::PS;
	} else if(tag == "SP") {
		return RowDataType::SP;
	} else {
		return RowDataType::Text;
	}
}

void SmsTraceLogger::GetTraceRow(string &output, SmsCpuState &cpuState, TraceLogPpuState &vdpState, DisassemblyInfo &disassemblyInfo)
{
	constexpr char activeStatusLetters[8] = { 'S', 'Z', '5', 'H', '3', 'P', 'N', 'C' };
	constexpr char inactiveStatusLetters[8] = { 's', 'z', '-', 'h', '-', 'p', 'n', 'c' };
	
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
			case RowDataType::I: WriteIntValue(output, cpuState.I, rowPart); break;
			case RowDataType::R: WriteIntValue(output, cpuState.R, rowPart); break;
			case RowDataType::AltA: WriteIntValue(output, cpuState.AltA, rowPart); break;
			case RowDataType::AltB: WriteIntValue(output, cpuState.AltB, rowPart); break;
			case RowDataType::AltC: WriteIntValue(output, cpuState.AltC, rowPart); break;
			case RowDataType::AltD: WriteIntValue(output, cpuState.AltD, rowPart); break;
			case RowDataType::AltE: WriteIntValue(output, cpuState.AltE, rowPart); break;
			case RowDataType::AltF: WriteIntValue(output, cpuState.AltFlags, rowPart); break;
			case RowDataType::AltH: WriteIntValue(output, cpuState.AltH, rowPart); break;
			case RowDataType::AltL: WriteIntValue(output, cpuState.AltL, rowPart); break;
			case RowDataType::IX: WriteIntValue(output, (uint16_t)(cpuState.IXL | (cpuState.IXH << 8)), rowPart); break;
			case RowDataType::IY: WriteIntValue(output, (uint16_t)(cpuState.IYL | (cpuState.IYH << 8)), rowPart); break;
			case RowDataType::SP: WriteIntValue(output, cpuState.SP, rowPart); break;
			case RowDataType::PS: GetStatusFlag(activeStatusLetters, inactiveStatusLetters, output, cpuState.Flags, rowPart, 8); break;
			default: ProcessSharedTag(rowPart, output, cpuState, vdpState, disassemblyInfo); break;
		}
	}
}

void SmsTraceLogger::LogPpuState()
{
	_ppuState[_currentPos] = {
		_vdp->GetCycle(),
		_vdp->GetCycle(),
		_vdp->GetScanline(),
		_vdp->GetFrameCount()
	};
}