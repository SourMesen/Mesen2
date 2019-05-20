#include "stdafx.h"
#include <regex>
#include <algorithm>
#include "TraceLogger.h"
#include "DisassemblyInfo.h"
#include "Console.h"
#include "Debugger.h"
#include "MemoryManager.h"
#include "LabelManager.h"
#include "../Utilities/HexUtilities.h"

string TraceLogger::_executionTrace = "";

TraceLogger::TraceLogger(Debugger* debugger, shared_ptr<Console> console)
{
	_console = console;
	_labelManager = debugger->GetLabelManager();
	_currentPos = 0;
	_logCount = 0;
	_logToFile = false;
	_pendingLog = false;
}

TraceLogger::~TraceLogger()
{
	StopLogging();
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
	_options = options;
	
	_logCpu[(int)CpuType::Cpu] = options.LogCpu;
	_logCpu[(int)CpuType::Spc] = options.LogSpc;

	string condition = _options.Condition;
	string format = _options.Format;

	auto lock = _lock.AcquireSafe();
	/*_conditionData = ExpressionData();
	if(!condition.empty()) {
		bool success = false;
		ExpressionData rpnList = _expEvaluator->GetRpnList(condition, success);
		if(success) {
			_conditionData = rpnList;
		}
	}*/

	ParseFormatString(_rowParts, format);
	ParseFormatString(_spcRowParts, "[PC,4h]   [ByteCode,15h] [Disassembly][EffectiveAddress] [MemoryValue,h][Align,48] A:[A,2h] X:[X,2h] Y:[Y,2h] S:[SP,2h] P:[P,8] H:[Cycle,3] V:[Scanline,3]");
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
				} catch(std::exception) {
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

	const char *activeStatusLetters = cpuType == CpuType::Cpu ? cpuActiveStatusLetters : spcActiveStatusLetters;
	const char *inactiveStatusLetters = cpuType == CpuType::Cpu ? cpuInactiveStatusLetters : spcInactiveStatusLetters;

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

	LabelManager* labelManager = _options.UseLabels ? _labelManager.get() : nullptr;
	info.GetDisassembly(code, pc, labelManager);
	WriteValue(output, code, rowPart);
}

void TraceLogger::WriteEffectiveAddress(DisassemblyInfo &info, RowPart &rowPart, void *cpuState, string &output, SnesMemoryType cpuMemoryType)
{
	int32_t effectiveAddress = info.GetEffectiveAddress(_console.get(), cpuState);
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

void TraceLogger::WriteMemoryValue(DisassemblyInfo &info, RowPart &rowPart, void *cpuState, string &output)
{
	int32_t address = info.GetEffectiveAddress(_console.get(), cpuState);
	if(address >= 0) {
		uint8_t valueSize;
		uint16_t value = info.GetMemoryValue(address, _console->GetMemoryManager().get(), valueSize);
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

void TraceLogger::GetTraceRow(string &output, CpuState &cpuState, PpuState &ppuState, DisassemblyInfo &disassemblyInfo)
{
	int originalSize = (int)output.size();
	uint32_t pcAddress = (cpuState.K << 16) | cpuState.PC;
	for(RowPart& rowPart : _rowParts) {
		switch(rowPart.DataType) {
			case RowDataType::Text: output += rowPart.Text; break;
			case RowDataType::ByteCode: WriteByteCode(disassemblyInfo, rowPart, output); break;
			case RowDataType::Disassembly: WriteDisassembly(disassemblyInfo, rowPart, (uint8_t)cpuState.SP, pcAddress, output); break;
			case RowDataType::EffectiveAddress: WriteEffectiveAddress(disassemblyInfo, rowPart, &cpuState, output, SnesMemoryType::CpuMemory); break;
			case RowDataType::MemoryValue: WriteMemoryValue(disassemblyInfo, rowPart, &cpuState, output); break;
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
			case RowDataType::FrameCount: WriteValue(output, ppuState.FrameCount, rowPart); break;
			case RowDataType::CycleCount: WriteValue(output, (uint32_t)cpuState.CycleCount, rowPart); break;
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
			case RowDataType::EffectiveAddress: WriteEffectiveAddress(disassemblyInfo, rowPart, &cpuState, output, SnesMemoryType::SpcMemory); break;
			case RowDataType::MemoryValue: WriteMemoryValue(disassemblyInfo, rowPart, &cpuState, output); break;
			case RowDataType::Align: WriteAlign(originalSize, rowPart, output); break;

			case RowDataType::PC: WriteValue(output, HexUtilities::ToHex((uint16_t)pcAddress), rowPart); break;
			case RowDataType::A: WriteValue(output, cpuState.A, rowPart); break;
			case RowDataType::X: WriteValue(output, cpuState.X, rowPart); break;
			case RowDataType::Y: WriteValue(output, cpuState.Y, rowPart); break;
			case RowDataType::SP: WriteValue(output, cpuState.SP, rowPart); break;
			case RowDataType::PS: GetStatusFlag<CpuType::Spc>(output, cpuState.PS, rowPart); break;
			case RowDataType::Cycle: WriteValue(output, ppuState.Cycle, rowPart); break;
			case RowDataType::Scanline: WriteValue(output, ppuState.Scanline, rowPart); break;
			case RowDataType::FrameCount: WriteValue(output, ppuState.FrameCount, rowPart); break;

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

void TraceLogger::GetTraceRow(string &output, DisassemblyInfo &disassemblyInfo, DebugState &state)
{
	switch(disassemblyInfo.GetCpuType()) {
		case CpuType::Cpu: GetTraceRow(output, state.Cpu, state.Ppu, disassemblyInfo); break;
		case CpuType::Spc: GetTraceRow(output, state.Spc, state.Ppu, disassemblyInfo); break;
	}
}

void TraceLogger::AddRow(DisassemblyInfo &disassemblyInfo, DebugState &state)
{
	_disassemblyCache[_currentPos] = disassemblyInfo;
	_stateCache[_currentPos] = state;
	_pendingLog = false;

	if(_logCount < ExecutionLogSize) {
		_logCount++;
	}

	if(_logToFile && _logCpu[(int)disassemblyInfo.GetCpuType()]) {
		GetTraceRow(_outputBuffer, _disassemblyCache[_currentPos], _stateCache[_currentPos]);
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
		auto lock = _lock.AcquireSafe();
		if(ConditionMatches(_lastState, _lastDisassemblyInfo, operationInfo)) {
			AddRow(_lastDisassemblyInfo, _lastState);
		}
	}
}*/

void TraceLogger::Log(DebugState &state, DisassemblyInfo &disassemblyInfo)
{
	auto lock = _lock.AcquireSafe();
	//if(ConditionMatches(state, disassemblyInfo, operationInfo)) {
		AddRow(disassemblyInfo, state);
	//}
}

void TraceLogger::Clear()
{
	_logCount = 0;
}

const char* TraceLogger::GetExecutionTrace(uint32_t lineCount)
{
	int startPos;

	_executionTrace.clear();
	{
		auto lock = _lock.AcquireSafe();
		lineCount = std::min(lineCount, _logCount);
		memcpy(_stateCacheCopy, _stateCache, sizeof(_stateCacheCopy));
		memcpy(_disassemblyCacheCopy, _disassemblyCache, sizeof(_disassemblyCacheCopy));
		startPos = (_currentPos > 0 ? _currentPos : TraceLogger::ExecutionLogSize) - 1;
	}

	bool enabled = false;
	for(int i = 0; i <= (int)CpuType::Spc; i++) {
		enabled |= _logCpu[i];
	}

	if(enabled && lineCount > 0) {
		for(int i = 0; i < TraceLogger::ExecutionLogSize; i++) {
			int index = (startPos - i);
			if(index < 0) {
				index = TraceLogger::ExecutionLogSize + index;
			}

			if((i > 0 && startPos == index) || !_disassemblyCacheCopy[index].IsInitialized()) {
				//If the entire array was checked, or this element is not initialized, stop
				break;
			}

			CpuType cpuType = _disassemblyCacheCopy[index].GetCpuType();
			if(!_logCpu[(int)cpuType]) {
				//This line isn't for a CPU currently being logged
				continue;
			}

			switch(cpuType) {
				case CpuType::Cpu: _executionTrace += "\x2\x1" + HexUtilities::ToHex24((_stateCacheCopy[index].Cpu.K << 16) | _stateCacheCopy[index].Cpu.PC) + "\x1"; break;
				case CpuType::Spc: _executionTrace += "\x3\x1" + HexUtilities::ToHex(_stateCacheCopy[index].Spc.PC) + "\x1"; break;
			}

			string byteCode;
			_disassemblyCacheCopy[index].GetByteCode(byteCode);
			_executionTrace += byteCode + "\x1";
			GetTraceRow(_executionTrace, _disassemblyCacheCopy[index], _stateCacheCopy[index]);

			lineCount--;
			if(lineCount == 0) {
				break;
			}
		}
	}
	return _executionTrace.c_str();
}