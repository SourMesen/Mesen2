#include "stdafx.h"
#include <regex>
#include <algorithm>
#include "Debugger/TraceLogger.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/LabelManager.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"
#include "Debugger/DebugBreakHelper.h"
#include "SNES/MemoryManager.h"
#include "SNES/Console.h"
#include "SNES/CpuTypes.h"
#include "SNES/SpcTypes.h"
#include "SNES/Coprocessors/DSP/NecDspTypes.h"
#include "SNES/PpuTypes.h"
#include "SNES/Coprocessors/CX4/Cx4Types.h"
#include "SNES/Coprocessors/GSU/GsuTypes.h"
#include "Gameboy/GbTypes.h"
#include "NES/NesTypes.h"
#include "NES/Debugger/NesDebuggerTypes.h"
#include "Utilities/HexUtilities.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

string TraceLogger::_executionTrace = "";

TraceLogger::TraceLogger(Debugger* debugger)
{
	_debugger = debugger;
	_console = debugger->GetConsole();
	_settings = debugger->GetEmulator()->GetSettings();
	_labelManager = debugger->GetLabelManager().get();
	_memoryDumper = debugger->GetMemoryDumper();
	_options = {};
	_currentPos = 0;
	_logCount = 0;
	_logToFile = false;
	_pendingLog = false;

	_snesCpuState = new CpuState[TraceLogger::ExecutionLogSize];
	_spcState = new SpcState[TraceLogger::ExecutionLogSize];
	_necDspState = new NecDspState[TraceLogger::ExecutionLogSize];
	_gsuState = new GsuState[TraceLogger::ExecutionLogSize];
	_cx4State = new Cx4State[TraceLogger::ExecutionLogSize];
	_gbCpuState = new GbCpuState[TraceLogger::ExecutionLogSize];
	_nesCpuState = new NesCpuState[TraceLogger::ExecutionLogSize];

	_disassemblyCache = new DisassemblyInfo[TraceLogger::ExecutionLogSize];
	_logCpuType = new CpuType[TraceLogger::ExecutionLogSize];

	SetOptions({});
	_logCpu[(int)CpuType::Nes] = true;
}

TraceLogger::~TraceLogger()
{
	StopLogging();

	delete[] _disassemblyCache;
	delete[] _logCpuType;

	delete[] _snesCpuState;
	delete[] _spcState;
	delete[] _necDspState;
	delete[] _gsuState;
	delete[] _cx4State;
	delete[] _gbCpuState;
	delete[] _nesCpuState;
}

template<typename T>
void TraceLogger::WriteValue(string &output, T value, RowPart& rowPart)
{
	string str = rowPart.DisplayInHex ? HexUtilities::ToHex(value) : std::to_string(value);
	output += str;
	if(rowPart.MinWidth > (int)str.size()) {
		output += std::string(rowPart.MinWidth - str.size(), ' ');
	}
}

template<>
void TraceLogger::WriteValue(string &output, string value, RowPart& rowPart)
{
	output += value;
	if(rowPart.MinWidth > (int)value.size()) {
		output += std::string(rowPart.MinWidth - value.size(), ' ');
	}
}

void TraceLogger::SetOptions(TraceLoggerOptions options)
{
	DebugBreakHelper helper(_debugger);
	_options = options;
	
	_logCpu[(int)CpuType::Cpu] = options.LogCpu;
	_logCpu[(int)CpuType::Spc] = options.LogSpc;
	_logCpu[(int)CpuType::NecDsp] = options.LogNecDsp;
	_logCpu[(int)CpuType::Sa1] = options.LogSa1;
	_logCpu[(int)CpuType::Gsu] = options.LogGsu;
	_logCpu[(int)CpuType::Cx4] = options.LogCx4;
	_logCpu[(int)CpuType::Gameboy] = options.LogGameboy;
	_logCpu[(int)CpuType::Nes] = options.LogNes;

	string condition = _options.Condition;
	string format = _options.Format;

	/*_conditionData = ExpressionData();
	if(!condition.empty()) {
		bool success = false;
		ExpressionData rpnList = _expEvaluator->GetRpnList(condition, success);
		if(success) {
			_conditionData = rpnList;
		}
	}*/

	ParseFormatString(_rowParts, format);
	ParseFormatString(_spcRowParts, "[PC,4h]   [ByteCode,11h] [Disassembly][EffectiveAddress] [MemoryValue,h][Align,48] A:[A,2h] X:[X,2h] Y:[Y,2h] S:[SP,2h] P:[P,8] H:[Cycle,3] V:[Scanline,3]");
	ParseFormatString(_dspRowParts, "[PC,4h]   [ByteCode,11h] [Disassembly] [Align,65] [A,2h] S:[SP,2h] H:[Cycle,3] V:[Scanline,3]");
	ParseFormatString(_gsuRowParts, "[PC,6h]   [ByteCode,11h] [Disassembly] [Align,50] SRC:[X,2] DST:[Y,2] R0:[A,2h] H:[Cycle,3] V:[Scanline,3]");
	ParseFormatString(_cx4RowParts, "[PC,6h]   [ByteCode,11h] [Disassembly] [Align,45] [A,2h] H:[Cycle,3] V:[Scanline,3]");
	ParseFormatString(_gbRowParts, "[PC,6h]   [ByteCode,11h] [Disassembly] [Align,45] A:[A,2h] B:[B,2h] C:[C,2h] D:[D,2h] E:[E,2h] HL:[H,2h][L,2h] F:[F,2h] SP:[SP,4h] CYC:[Cycle,3] LY:[Scanline,3]");
	ParseFormatString(_nesRowParts, "[PC,4h]  [ByteCode,11h] [Disassembly][EffectiveAddress] [MemoryValue,2h][Align,48] A:[A,2h] X:[X,2h] Y:[Y,2h] P:[P,8] SP:[SP,2h] CYC:[Cycle,3] SL:[Scanline,3] FC:[FrameCount] CPU Cycle:[CycleCount]");
}

void TraceLogger::ParseFormatString(vector<RowPart> &rowParts, string format)
{
	rowParts.clear();

	std::regex formatRegex = std::regex("(\\[\\s*([^[]*?)\\s*(,\\s*([\\d]*)\\s*(h){0,1}){0,1}\\s*\\])|([^[]*)", std::regex_constants::icase);
	std::sregex_iterator start = std::sregex_iterator(format.cbegin(), format.cend(), formatRegex);
	std::sregex_iterator end = std::sregex_iterator();

	for(std::sregex_iterator it = start; it != end; it++) {
		const std::smatch& match = *it;

		if(match.str(1) == "") {
			RowPart part = {};
			part.DataType = RowDataType::Text;
			part.Text = match.str(6);
			rowParts.push_back(part);
		} else {
			RowPart part = {};

			string dataType = match.str(2);
			if(dataType == "ByteCode") {
				part.DataType = RowDataType::ByteCode;
			} else if(dataType == "Disassembly") {
				part.DataType = RowDataType::Disassembly;
			} else if(dataType == "EffectiveAddress") {
				part.DataType = RowDataType::EffectiveAddress;
			} else if(dataType == "MemoryValue") {
				part.DataType = RowDataType::MemoryValue;
			} else if(dataType == "Align") {
				part.DataType = RowDataType::Align;
			} else if(dataType == "PC") {
				part.DataType = RowDataType::PC;
			} else if(dataType == "A") {
				part.DataType = RowDataType::A;
			} else if(dataType == "B") {
				part.DataType = RowDataType::B;
			} else if(dataType == "C") {
				part.DataType = RowDataType::C;
			} else if(dataType == "D") {
				part.DataType = RowDataType::D;
			} else if(dataType == "E") {
				part.DataType = RowDataType::E;
			} else if(dataType == "F") {
				part.DataType = RowDataType::F;
			} else if(dataType == "H") {
				part.DataType = RowDataType::H;
			} else if(dataType == "L") {
				part.DataType = RowDataType::L;
			} else if(dataType == "X") {
				part.DataType = RowDataType::X;
			} else if(dataType == "Y") {
				part.DataType = RowDataType::Y;
			} else if(dataType == "D") {
				part.DataType = RowDataType::D;
			} else if(dataType == "DB") {
				part.DataType = RowDataType::DB;
			} else if(dataType == "P") {
				part.DataType = RowDataType::PS;
			} else if(dataType == "SP") {
				part.DataType = RowDataType::SP;
			} else if(dataType == "Cycle") {
				part.DataType = RowDataType::Cycle;
			} else if(dataType == "HClock") {
				part.DataType = RowDataType::HClock;
			} else if(dataType == "Scanline") {
				part.DataType = RowDataType::Scanline;
			} else if(dataType == "FrameCount") {
				part.DataType = RowDataType::FrameCount;
			} else if(dataType == "CycleCount") {
				part.DataType = RowDataType::CycleCount;
			} else {
				part.DataType = RowDataType::Text;
				part.Text = "[Invalid tag]";
			}

			if(!match.str(4).empty()) {
				try {
					part.MinWidth = std::stoi(match.str(4));
				} catch(std::exception&) {
				}
			}
			part.DisplayInHex = match.str(5) == "h";

			rowParts.push_back(part);
		}
	}
}

void TraceLogger::StartLogging(string filename)
{
	_outputBuffer.clear();
	_outputFile.open(filename, ios::out | ios::binary);
	_logToFile = true;
}

void TraceLogger::StopLogging()
{
	if(_logToFile) {
		_logToFile = false;
		if(_outputFile) {
			if(!_outputBuffer.empty()) {
				_outputFile << _outputBuffer;
			}
			_outputFile.close();
		}
	}
}

void TraceLogger::LogExtraInfo(const char *log, uint32_t cycleCount)
{
	if(_logToFile && _options.ShowExtraInfo) {
		//Flush current buffer
		_outputFile << _outputBuffer;
		_outputBuffer.clear();
		_outputFile << "[" << log << " - Cycle: " << std::to_string(cycleCount) << "]" << (_options.UseWindowsEol ? "\r\n" : "\n");
	}
}

template<CpuType cpuType>
void TraceLogger::GetStatusFlag(string &output, uint8_t ps, RowPart& part)
{
	constexpr char cpuActiveStatusLetters[8] = { 'N', 'V', 'M', 'X', 'D', 'I', 'Z', 'C' };
	constexpr char cpuInactiveStatusLetters[8] = { 'n', 'v', 'm', 'x', 'd', 'i', 'z', 'c' };

	constexpr char spcActiveStatusLetters[8] = { 'N', 'V', 'P', 'B', 'H', 'I', 'Z', 'C' };
	constexpr char spcInactiveStatusLetters[8] = { 'n', 'v', 'p', 'b', 'h', 'i', 'z', 'c' };

	constexpr char nesActiveStatusLetters[8] = { 'N', 'V', '-', '-', 'D', 'I', 'Z', 'C' };
	constexpr char nesInactiveStatusLetters[8] = { 'n', 'v', '-', '-', 'd', 'i', 'z', 'c' };

	const char *activeStatusLetters = cpuType == CpuType::Cpu ? cpuActiveStatusLetters : (cpuType == CpuType::Spc ? spcActiveStatusLetters : nesActiveStatusLetters);
	const char *inactiveStatusLetters = cpuType == CpuType::Cpu ? cpuInactiveStatusLetters : (cpuType == CpuType::Spc ? spcInactiveStatusLetters : nesInactiveStatusLetters);

	if(part.DisplayInHex) {
		WriteValue(output, ps, part);
	} else {
		string flags;
		for(int i = 0; i < 8; i++) {
			if(ps & 0x80) {
				flags += activeStatusLetters[i];
			} else if(part.MinWidth >= 8) {
				flags += inactiveStatusLetters[i];
			}
			ps <<= 1;
		}
		WriteValue(output, flags, part);
	}
}

void TraceLogger::WriteByteCode(DisassemblyInfo &info, RowPart &rowPart, string &output)
{
	string byteCode;
	info.GetByteCode(byteCode);
	if(!rowPart.DisplayInHex) {
		//Remove $ marks if not in "hex" mode (but still display the bytes as hex)
		byteCode.erase(std::remove(byteCode.begin(), byteCode.end(), '$'), byteCode.end());
	}
	WriteValue(output, byteCode, rowPart);
}

void TraceLogger::WriteDisassembly(DisassemblyInfo &info, RowPart &rowPart, uint8_t sp, uint32_t pc, string &output)
{
	int indentLevel = 0;
	string code;

	if(_options.IndentCode) {
		indentLevel = 0xFF - (sp & 0xFF);
		code = std::string(indentLevel, ' ');
	}

	LabelManager* labelManager = _options.UseLabels ? _labelManager : nullptr;
	info.GetDisassembly(code, pc, labelManager, _settings);
	WriteValue(output, code, rowPart);
}

void TraceLogger::WriteEffectiveAddress(DisassemblyInfo &info, RowPart &rowPart, void *cpuState, string &output, SnesMemoryType cpuMemoryType, CpuType cpuType)
{
	int32_t effectiveAddress = info.GetEffectiveAddress(_debugger, cpuState, cpuType);
	if(effectiveAddress >= 0) {
		if(_options.UseLabels) {
			AddressInfo addr { effectiveAddress, cpuMemoryType };
			string label = _labelManager->GetLabel(addr);
			if(!label.empty()) {
				WriteValue(output, " [" + label + "]", rowPart);
				return;
			}
		}
		WriteValue(output, " [" + HexUtilities::ToHex24(effectiveAddress) + "]", rowPart);
	}
}

void TraceLogger::WriteMemoryValue(DisassemblyInfo &info, RowPart &rowPart, void *cpuState, string &output, SnesMemoryType memType, CpuType cpuType)
{
	int32_t address = info.GetEffectiveAddress(_debugger, cpuState, cpuType);
	if(address >= 0) {
		uint8_t valueSize;
		uint16_t value = info.GetMemoryValue(address, _memoryDumper, memType, valueSize);
		if(rowPart.DisplayInHex) {
			output += "= $";
			if(valueSize == 2) {
				WriteValue(output, (uint16_t)value, rowPart);
			} else {
				WriteValue(output, (uint8_t)value, rowPart);
			}
		} else {
			output += "= ";
		}
	}
}

void TraceLogger::WriteAlign(int originalSize, RowPart &rowPart, string &output)
{
	if((int)output.size() - originalSize < rowPart.MinWidth) {
		output += std::string(rowPart.MinWidth - (output.size() - originalSize), ' ');
	}
}

void TraceLogger::GetTraceRow(string &output, CpuState &cpuState, PpuState &ppuState, DisassemblyInfo &disassemblyInfo, SnesMemoryType memType, CpuType cpuType)
{
	int originalSize = (int)output.size();
	uint32_t pcAddress = (cpuState.K << 16) | cpuState.PC;
	for(RowPart& rowPart : _rowParts) {
		switch(rowPart.DataType) {
			case RowDataType::Text: output += rowPart.Text; break;
			case RowDataType::ByteCode: WriteByteCode(disassemblyInfo, rowPart, output); break;
			case RowDataType::Disassembly: WriteDisassembly(disassemblyInfo, rowPart, (uint8_t)cpuState.SP, pcAddress, output); break;
			case RowDataType::EffectiveAddress: WriteEffectiveAddress(disassemblyInfo, rowPart, &cpuState, output, memType, cpuType); break;
			case RowDataType::MemoryValue: WriteMemoryValue(disassemblyInfo, rowPart, &cpuState, output, memType, cpuType); break;
			case RowDataType::Align: WriteAlign(originalSize, rowPart, output); break;

			case RowDataType::PC: WriteValue(output, HexUtilities::ToHex24(pcAddress), rowPart); break;
			case RowDataType::A: WriteValue(output, cpuState.A, rowPart); break;
			case RowDataType::X: WriteValue(output, cpuState.X, rowPart); break;
			case RowDataType::Y: WriteValue(output, cpuState.Y, rowPart); break;
			case RowDataType::D: WriteValue(output, cpuState.D, rowPart); break;
			case RowDataType::DB: WriteValue(output, cpuState.DBR, rowPart); break;
			case RowDataType::SP: WriteValue(output, cpuState.SP, rowPart); break;
			case RowDataType::PS: GetStatusFlag<CpuType::Cpu>(output, cpuState.PS, rowPart); break;
			case RowDataType::Cycle: WriteValue(output, ppuState.Cycle, rowPart); break;
			case RowDataType::Scanline: WriteValue(output, ppuState.Scanline, rowPart); break;
			case RowDataType::HClock: WriteValue(output, ppuState.HClock, rowPart); break;
			case RowDataType::FrameCount: WriteValue(output, ppuState.FrameCount, rowPart); break;
			case RowDataType::CycleCount:
				WriteValue(output, (uint32_t)(cpuState.CycleCount >> 32), rowPart);
				WriteValue(output, (uint32_t)cpuState.CycleCount, rowPart); 
				break;
			default: break;
		}
	}
	output += _options.UseWindowsEol ? "\r\n" : "\n";
}

void TraceLogger::GetTraceRow(string &output, SpcState &cpuState, PpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	int originalSize = (int)output.size();
	uint32_t pcAddress = cpuState.PC;
	for(RowPart& rowPart : _spcRowParts) {
		switch(rowPart.DataType) {
			case RowDataType::Text: output += rowPart.Text; break;
			case RowDataType::ByteCode: WriteByteCode(disassemblyInfo, rowPart, output); break;
			case RowDataType::Disassembly: WriteDisassembly(disassemblyInfo, rowPart, cpuState.SP, pcAddress, output); break;
			case RowDataType::EffectiveAddress: WriteEffectiveAddress(disassemblyInfo, rowPart, &cpuState, output, SnesMemoryType::SpcMemory, CpuType::Spc); break;
			case RowDataType::MemoryValue: WriteMemoryValue(disassemblyInfo, rowPart, &cpuState, output, SnesMemoryType::SpcMemory, CpuType::Spc); break;
			case RowDataType::Align: WriteAlign(originalSize, rowPart, output); break;

			case RowDataType::PC: WriteValue(output, HexUtilities::ToHex((uint16_t)pcAddress), rowPart); break;
			case RowDataType::A: WriteValue(output, cpuState.A, rowPart); break;
			case RowDataType::X: WriteValue(output, cpuState.X, rowPart); break;
			case RowDataType::Y: WriteValue(output, cpuState.Y, rowPart); break;
			case RowDataType::SP: WriteValue(output, cpuState.SP, rowPart); break;
			case RowDataType::PS: GetStatusFlag<CpuType::Spc>(output, cpuState.PS, rowPart); break;
			case RowDataType::Cycle: WriteValue(output, ppuState.Cycle, rowPart); break;
			case RowDataType::Scanline: WriteValue(output, ppuState.Scanline, rowPart); break;
			case RowDataType::HClock: WriteValue(output, ppuState.HClock, rowPart); break;
			case RowDataType::FrameCount: WriteValue(output, ppuState.FrameCount, rowPart); break;

			default: break;
		}
	}
	output += _options.UseWindowsEol ? "\r\n" : "\n";
}

void TraceLogger::GetTraceRow(string &output, NecDspState &cpuState, PpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	int originalSize = (int)output.size();
	uint32_t pcAddress = cpuState.PC;
	for(RowPart& rowPart : _dspRowParts) {
		switch(rowPart.DataType) {
			case RowDataType::Text: output += rowPart.Text; break;
			case RowDataType::ByteCode: WriteByteCode(disassemblyInfo, rowPart, output); break;
			case RowDataType::Disassembly: WriteDisassembly(disassemblyInfo, rowPart, cpuState.SP, pcAddress, output); break;
			case RowDataType::Align: WriteAlign(originalSize, rowPart, output); break;

			case RowDataType::PC: WriteValue(output, HexUtilities::ToHex((uint16_t)pcAddress), rowPart); break;
			case RowDataType::A:
				output += "A:" + HexUtilities::ToHex(cpuState.A);
				output += " B:" + HexUtilities::ToHex(cpuState.B);
				output += " DR:" + HexUtilities::ToHex(cpuState.DR);
				output += " DP:" + HexUtilities::ToHex(cpuState.DP);
				output += " SR:" + HexUtilities::ToHex(cpuState.SR);
				output += " K:" + HexUtilities::ToHex(cpuState.K);
				output += " L:" + HexUtilities::ToHex(cpuState.L);
				output += " M:" + HexUtilities::ToHex(cpuState.M);
				output += " N:" + HexUtilities::ToHex(cpuState.N);
				output += " RP:" + HexUtilities::ToHex(cpuState.RP);
				output += " TR:" + HexUtilities::ToHex(cpuState.TR);
				output += " TRB:" + HexUtilities::ToHex(cpuState.TRB) + " ";
				//output += "FA=" + HexUtilities::ToHex(cpuState.FlagsA);
				//output += "FB=" + HexUtilities::ToHex(cpuState.FlagsB);
				WriteValue(output, cpuState.A, rowPart); 
				break;
			case RowDataType::SP: WriteValue(output, cpuState.SP, rowPart); break;
			case RowDataType::Cycle: WriteValue(output, ppuState.Cycle, rowPart); break;
			case RowDataType::Scanline: WriteValue(output, ppuState.Scanline, rowPart); break;
			case RowDataType::HClock: WriteValue(output, ppuState.HClock, rowPart); break;
			case RowDataType::FrameCount: WriteValue(output, ppuState.FrameCount, rowPart); break;
			default: break;
		}
	}
	output += _options.UseWindowsEol ? "\r\n" : "\n";
}

void TraceLogger::GetTraceRow(string &output, GsuState &gsuState, PpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	int originalSize = (int)output.size();
	uint32_t pcAddress = (gsuState.ProgramBank << 16) | gsuState.R[15];
	for(RowPart& rowPart : _gsuRowParts) {
		switch(rowPart.DataType) {
			case RowDataType::Text: output += rowPart.Text; break;
			case RowDataType::ByteCode: WriteByteCode(disassemblyInfo, rowPart, output); break;
			case RowDataType::Disassembly: WriteDisassembly(disassemblyInfo, rowPart, 0, pcAddress, output); break;
			case RowDataType::Align: WriteAlign(originalSize, rowPart, output); break;

			case RowDataType::PC: WriteValue(output, HexUtilities::ToHex24(pcAddress), rowPart); break;
			case RowDataType::A:
				WriteValue(output, gsuState.R[0], rowPart);
				for(int i = 1; i < 16; i++) {
					output += " R" + std::to_string(i) + ":" + HexUtilities::ToHex(gsuState.R[i]);
				}
				break;
			case RowDataType::X: WriteValue(output, gsuState.SrcReg, rowPart); break;
			case RowDataType::Y: WriteValue(output, gsuState.DestReg, rowPart); break;

			case RowDataType::Cycle: WriteValue(output, ppuState.Cycle, rowPart); break;
			case RowDataType::Scanline: WriteValue(output, ppuState.Scanline, rowPart); break;
			case RowDataType::HClock: WriteValue(output, ppuState.HClock, rowPart); break;
			case RowDataType::FrameCount: WriteValue(output, ppuState.FrameCount, rowPart); break;
			default: break;
		}
	}
	output += _options.UseWindowsEol ? "\r\n" : "\n";
}


void TraceLogger::GetTraceRow(string &output, Cx4State &cx4State, PpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	int originalSize = (int)output.size();
	uint32_t pcAddress = (cx4State.Cache.Address[cx4State.Cache.Page] + (cx4State.PC * 2)) & 0xFFFFFF;
	for(RowPart& rowPart : _cx4RowParts) {
		switch(rowPart.DataType) {
			case RowDataType::Text: output += rowPart.Text; break;
			case RowDataType::ByteCode: WriteByteCode(disassemblyInfo, rowPart, output); break;
			case RowDataType::Disassembly: WriteDisassembly(disassemblyInfo, rowPart, 0, pcAddress, output); break;
			case RowDataType::Align: WriteAlign(originalSize, rowPart, output); break;

			case RowDataType::PC: WriteValue(output, HexUtilities::ToHex24(pcAddress), rowPart); break;
			case RowDataType::A:
				output += " A:" + HexUtilities::ToHex24(cx4State.A);
				output += string(" ") + (cx4State.Carry ? "C" : "c") + (cx4State.Zero ? "Z" : "z") + (cx4State.Overflow ? "V" : "v") + (cx4State.Negative ? "N" : "n");

				output += " PC:" + HexUtilities::ToHex(cx4State.PC);
				output += " MAR:" + HexUtilities::ToHex24(cx4State.MemoryAddressReg);
				output += " MDR:" + HexUtilities::ToHex24(cx4State.MemoryDataReg);
				output += " DPR:" + HexUtilities::ToHex24(cx4State.DataPointerReg);
				output += " ML:" + HexUtilities::ToHex24((uint32_t)cx4State.Mult & 0xFFFFFF);
				output += " MH:" + HexUtilities::ToHex24((uint32_t)(cx4State.Mult >> 24) & 0xFFFFFF);
				for(int i = 0; i < 16; i++) {
					output += " R" + std::to_string(i) + ":" + HexUtilities::ToHex24(cx4State.Regs[i]);
				}
				break;

			case RowDataType::Cycle: WriteValue(output, ppuState.Cycle, rowPart); break;
			case RowDataType::Scanline: WriteValue(output, ppuState.Scanline, rowPart); break;
			case RowDataType::HClock: WriteValue(output, ppuState.HClock, rowPart); break;
			case RowDataType::FrameCount: WriteValue(output, ppuState.FrameCount, rowPart); break;
			default: break;
		}
	}
	output += _options.UseWindowsEol ? "\r\n" : "\n";
}

void TraceLogger::GetTraceRow(string& output, GbCpuState& cpuState, GbPpuState& ppuState, DisassemblyInfo& disassemblyInfo)
{
	int originalSize = (int)output.size();
	uint32_t pcAddress = cpuState.PC;
	for(RowPart& rowPart : _gbRowParts) {
		switch(rowPart.DataType) {
			case RowDataType::Text: output += rowPart.Text; break;
			case RowDataType::ByteCode: WriteByteCode(disassemblyInfo, rowPart, output); break;
			case RowDataType::Disassembly: WriteDisassembly(disassemblyInfo, rowPart, (uint8_t)cpuState.SP, pcAddress, output); break;
			case RowDataType::EffectiveAddress: WriteEffectiveAddress(disassemblyInfo, rowPart, &cpuState, output, SnesMemoryType::GameboyMemory, CpuType::Gameboy); break;
			case RowDataType::MemoryValue: WriteMemoryValue(disassemblyInfo, rowPart, &cpuState, output, SnesMemoryType::GameboyMemory, CpuType::Gameboy); break;
			case RowDataType::Align: WriteAlign(originalSize, rowPart, output); break;

			case RowDataType::PC: WriteValue(output, HexUtilities::ToHex((uint16_t)pcAddress), rowPart); break;
			case RowDataType::A: WriteValue(output, cpuState.A, rowPart); break;
			case RowDataType::B: WriteValue(output, cpuState.B, rowPart); break;
			case RowDataType::C: WriteValue(output, cpuState.C, rowPart); break;
			case RowDataType::D: WriteValue(output, cpuState.D, rowPart); break;
			case RowDataType::E: WriteValue(output, cpuState.E, rowPart); break;
			case RowDataType::F: WriteValue(output, cpuState.Flags, rowPart); break;
			case RowDataType::H: WriteValue(output, cpuState.H, rowPart); break;
			case RowDataType::L: WriteValue(output, cpuState.L, rowPart); break;
			case RowDataType::SP: WriteValue(output, cpuState.SP, rowPart); break;
			case RowDataType::Cycle: WriteValue(output, ppuState.Cycle, rowPart); break;
			case RowDataType::Scanline: WriteValue(output, ppuState.Scanline, rowPart); break;
			case RowDataType::FrameCount: WriteValue(output, ppuState.FrameCount, rowPart); break;
			default: break;
		}
	}
	output += _options.UseWindowsEol ? "\r\n" : "\n";
}

void TraceLogger::GetTraceRow(string &output, NesCpuState &cpuState, NesPpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	int originalSize = (int)output.size();
	for(RowPart& rowPart : _nesRowParts) {
		switch(rowPart.DataType) {
			case RowDataType::Text: output += rowPart.Text; break;
			case RowDataType::ByteCode: WriteByteCode(disassemblyInfo, rowPart, output); break;
			case RowDataType::Disassembly: WriteDisassembly(disassemblyInfo, rowPart, cpuState.SP, cpuState.PC, output); break;
			case RowDataType::EffectiveAddress: WriteEffectiveAddress(disassemblyInfo, rowPart, &cpuState, output, SnesMemoryType::NesMemory, CpuType::Nes); break;
			case RowDataType::MemoryValue: WriteMemoryValue(disassemblyInfo, rowPart, &cpuState, output, SnesMemoryType::NesMemory, CpuType::Nes); break;
			case RowDataType::Align: WriteAlign(originalSize, rowPart, output); break;

			case RowDataType::PC: WriteValue(output, HexUtilities::ToHex((uint16_t)cpuState.PC), rowPart); break;
			case RowDataType::A: WriteValue(output, cpuState.A, rowPart); break;
			case RowDataType::X: WriteValue(output, cpuState.X, rowPart); break;
			case RowDataType::Y: WriteValue(output, cpuState.Y, rowPart); break;
			case RowDataType::SP: WriteValue(output, cpuState.SP, rowPart); break;
			case RowDataType::PS: GetStatusFlag<CpuType::Nes>(output, cpuState.PS, rowPart); break;
			case RowDataType::Cycle: WriteValue(output, ppuState.Cycle, rowPart); break;
			case RowDataType::Scanline: WriteValue(output, ppuState.Scanline, rowPart); break;
			case RowDataType::FrameCount: WriteValue(output, ppuState.FrameCount, rowPart); break;
			case RowDataType::CycleCount: 
				WriteValue(output, (uint32_t)(cpuState.CycleCount >> 32), rowPart);
				WriteValue(output, (uint32_t)cpuState.CycleCount, rowPart);
				break;

			default: break;
		}
	}
	output += _options.UseWindowsEol ? "\r\n" : "\n";
}

/*
bool TraceLogger::ConditionMatches(DebugState &state, DisassemblyInfo &disassemblyInfo, OperationInfo &operationInfo)
{
	if(!_conditionData.RpnQueue.empty()) {
		EvalResultType type;
		if(!_expEvaluator->Evaluate(_conditionData, state, type, operationInfo)) {
			if(operationInfo.OperationType == MemoryOperationType::ExecOpCode) {
				//Condition did not match, keep state/disassembly info for instruction's subsequent cycles
				_lastState = state;
				_lastDisassemblyInfo = disassemblyInfo;
				_pendingLog = true;
			}
			return false;
		}
	}
	return true;
}
*/

void TraceLogger::GetTraceRow(string &output, CpuType cpuType, DisassemblyInfo &disassemblyInfo, BaseState &state)
{
	PpuState ppu = {};
	GbPpuState gbPpu = {};
	NesPpuState nesPpu = {};
	switch(cpuType) {
		case CpuType::Cpu: GetTraceRow(output, (CpuState&)state, ppu, disassemblyInfo, SnesMemoryType::CpuMemory, cpuType); break;
		case CpuType::Spc: GetTraceRow(output, (SpcState&)state, ppu, disassemblyInfo); break;
		case CpuType::NecDsp: GetTraceRow(output, (NecDspState&)state, ppu, disassemblyInfo); break;
		case CpuType::Sa1: GetTraceRow(output, (CpuState&)state, ppu, disassemblyInfo, SnesMemoryType::Sa1Memory, cpuType); break;
		case CpuType::Gsu: GetTraceRow(output, (GsuState&)state, ppu, disassemblyInfo); break;
		case CpuType::Cx4: GetTraceRow(output, (Cx4State&)state, ppu, disassemblyInfo); break;
		case CpuType::Gameboy: GetTraceRow(output, (GbCpuState&)state, gbPpu, disassemblyInfo); break;
		case CpuType::Nes: GetTraceRow(output, (NesCpuState&)state, nesPpu, disassemblyInfo); break;
		default: throw std::runtime_error("Trace logger - Unsupported CPU type");
	}
}

template<class T>
void TraceLogger::AddRow(CpuType cpuType, T& cpuState, DisassemblyInfo &disassemblyInfo)
{
	_logCpuType[_currentPos] = cpuType;
	_disassemblyCache[_currentPos] = disassemblyInfo;

	switch(cpuType) {
		case CpuType::Cpu: _snesCpuState[_currentPos] = (CpuState&)cpuState; break;
		case CpuType::Spc: _spcState[_currentPos] = (SpcState&)cpuState; break;
		case CpuType::NecDsp: _necDspState[_currentPos] = (NecDspState&)cpuState; break;
		case CpuType::Sa1: _snesCpuState[_currentPos] = (CpuState&)cpuState; break;
		case CpuType::Gsu: _gsuState[_currentPos] = (GsuState&)cpuState; break;
		case CpuType::Cx4: _cx4State[_currentPos] = (Cx4State&)cpuState; break;
		case CpuType::Gameboy: _gbCpuState[_currentPos] = (GbCpuState&)cpuState; break;
		case CpuType::Nes: _nesCpuState[_currentPos] = (NesCpuState&)cpuState; break;
		default: throw std::runtime_error("Trace logger - Unsupported CPU type");
	}

	_pendingLog = false;

	if(_logCount < ExecutionLogSize) {
		_logCount++;
	}

	if(_logToFile) {
		GetTraceRow(_outputBuffer, cpuType, _disassemblyCache[_currentPos], cpuState);
		if(_outputBuffer.size() > 32768) {
			_outputFile << _outputBuffer;
			_outputBuffer.clear();
		}
	}

	_currentPos = (_currentPos + 1) % ExecutionLogSize;
}
/*
void TraceLogger::LogNonExec(OperationInfo& operationInfo)
{
	if(_pendingLog) {
		if(ConditionMatches(_lastState, _lastDisassemblyInfo, operationInfo)) {
			AddRow(_lastDisassemblyInfo, _lastState);
		}
	}
}*/

template<class T>
void TraceLogger::Log(CpuType cpuType, T& cpuState, DisassemblyInfo &disassemblyInfo)
{
	if(_logCpu[(int)cpuType]) {
		//For the sake of performance, only log data for the CPUs we're actively displaying/logging
		//if(ConditionMatches(state, disassemblyInfo, operationInfo)) {
			AddRow(cpuType, cpuState, disassemblyInfo);
		//}
	}
}

void TraceLogger::Clear()
{
	_logCount = 0;
}

const char* TraceLogger::GetExecutionTrace(uint32_t lineCount)
{
	DebugBreakHelper helper(_debugger);
	_executionTrace.clear();

	int startPos = (_currentPos > 0 ? _currentPos : TraceLogger::ExecutionLogSize) - 1;
	
	bool enabled = false;
	for(int i = 0; i <= (int)DebugUtilities::GetLastCpuType(); i++) {
		enabled |= _logCpu[i];
	}

	if(enabled && lineCount > 0) {
		for(int i = 0; i < TraceLogger::ExecutionLogSize; i++) {
			int index = (startPos - i);
			if(index < 0) {
				index = TraceLogger::ExecutionLogSize + index;
			}

			if((i > 0 && startPos == index) || !_disassemblyCache[index].IsInitialized()) {
				//If the entire array was checked, or this element is not initialized, stop
				break;
			}

			CpuType cpuType = _logCpuType[index];
			if(!_logCpu[(int)cpuType]) {
				//This line isn't for a CPU currently being logged
				continue;
			}

			BaseState* state;
			switch(cpuType) {
				case CpuType::Cpu: {
					state = &_snesCpuState[index];
					CpuState* snesCpu = (CpuState*)state;
					_executionTrace += "\x2\x1" + HexUtilities::ToHex24((snesCpu->K << 16) | snesCpu->PC) + "\x1";
					break;
				}

				case CpuType::Spc: {
					state = &_spcState[index];
					SpcState* spc = (SpcState*)state;
					_executionTrace += "\x3\x1" + HexUtilities::ToHex(spc->PC) + "\x1";
					break;
				}

				case CpuType::NecDsp: {
					state = &_spcState[index];
					NecDspState* necDsp = (NecDspState*)state;
					_executionTrace += "\x4\x1" + HexUtilities::ToHex(necDsp->PC) + "\x1";
					break;
				}

				case CpuType::Sa1: {
					state = &_snesCpuState[index];
					CpuState* sa1 = (CpuState*)state;
					_executionTrace += "\x4\x1" + HexUtilities::ToHex24((sa1->K << 16) | sa1->PC) + "\x1";
					break;
				}

				case CpuType::Gsu: {
					state = &_gsuState[index];
					GsuState* gsu = (GsuState*)state;
					_executionTrace += "\x4\x1" + HexUtilities::ToHex24((gsu->ProgramBank << 16) | gsu->R[15]) + "\x1";
					break;
				}

				case CpuType::Cx4: {
					state = &_cx4State[index];
					Cx4State* cx4 = (Cx4State*)state;
					_executionTrace += "\x4\x1" + HexUtilities::ToHex24((cx4->Cache.Address[cx4->Cache.Page] + (cx4->PC * 2)) & 0xFFFFFF) + "\x1"; 
					break;
				}

				case CpuType::Gameboy: {
					state = &_gbCpuState[index];
					GbCpuState* gb = (GbCpuState*)state;
					_executionTrace += "\x4\x1" + HexUtilities::ToHex(gb->PC) + "\x1";
					break;
				}

				case CpuType::Nes: {
					state = &_nesCpuState[index];
					NesCpuState* nes = (NesCpuState*)state;
					_executionTrace += "\x4\x1" + HexUtilities::ToHex(nes->PC) + "\x1";
					break;
				}

				default:
					throw std::runtime_error("Trace logger - Unsupported CPU type");
			}

			string byteCode;
			_disassemblyCache[index].GetByteCode(byteCode);
			_executionTrace += byteCode + "\x1";
			GetTraceRow(_executionTrace, cpuType, _disassemblyCache[index], *state);

			lineCount--;
			if(lineCount == 0) {
				break;
			}
		}
	}
	return _executionTrace.c_str();
}

template void TraceLogger::Log(CpuType cpuType, CpuState& cpuState, DisassemblyInfo& disassemblyInfo);
template void TraceLogger::Log(CpuType cpuType, SpcState& cpuState, DisassemblyInfo& disassemblyInfo);
template void TraceLogger::Log(CpuType cpuType, GsuState& cpuState, DisassemblyInfo& disassemblyInfo);
template void TraceLogger::Log(CpuType cpuType, Cx4State& cpuState, DisassemblyInfo& disassemblyInfo);
template void TraceLogger::Log(CpuType cpuType, NecDspState& cpuState, DisassemblyInfo& disassemblyInfo);
template void TraceLogger::Log(CpuType cpuType, GbCpuState& cpuState, DisassemblyInfo& disassemblyInfo);
template void TraceLogger::Log(CpuType cpuType, NesCpuState& cpuState, DisassemblyInfo& disassemblyInfo);
