#pragma once
#include "stdafx.h"
#include <regex>
#include "Utilities/SimpleLock.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/IDebugger.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/LabelManager.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/ITraceLogger.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/TraceLogFileSaver.h"
#include "Utilities/HexUtilities.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

class IConsole;
class Debugger;
class LabelManager;
class MemoryDumper;
class EmuSettings;

enum class RowDataType
{
	Text = 0,
	ByteCode,
	Disassembly,
	EffectiveAddress,
	MemoryValue,
	Align,
	PC,
	A,
	B,
	C,
	D,
	E,
	F,
	H,
	K,
	L,
	M,
	N,
	X,
	Y,
	DB,
	SP,
	PS,
	Cycle,
	Scanline,
	HClock,
	FrameCount,
	CycleCount,

	R0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,
	Src,
	Dst,

	MAR,
	MDR,
	DPR,
	ML,
	MH,
	PB,
	P,

	RP,
	DP,
	DR,
	SR,
	TR,
	TRB,
	
	FlagsA,
	FlagsB
};

struct TraceLogPpuState
{
	uint32_t Cycle;
	uint32_t HClock;
	int32_t Scanline;
	uint32_t FrameCount;
};

struct RowPart
{
	RowDataType DataType;
	string Text;
	bool DisplayInHex;
	int MinWidth;
};

template<typename TraceLoggerType, typename CpuStateType>
class BaseTraceLogger : public ITraceLogger
{
protected:
	static constexpr int ExecutionLogSize = 30000;

	TraceLoggerOptions _options;
	IConsole* _console;
	EmuSettings* _settings;
	LabelManager* _labelManager;
	MemoryDumper* _memoryDumper;
	Debugger* _debugger;

	CpuType _cpuType = CpuType::Snes;
	MemoryType _cpuMemoryType = MemoryType::SnesMemory;

	vector<RowPart> _rowParts;

	uint32_t _currentPos = 0;

	bool _pendingLog = false;
	CpuStateType _lastState = {};
	DisassemblyInfo _lastDisassemblyInfo = {};

	CpuStateType* _cpuState = nullptr;
	DisassemblyInfo *_disassemblyCache = nullptr;
	uint64_t* _rowIds = nullptr;
	TraceLogPpuState* _ppuState = nullptr;

	unique_ptr<ExpressionEvaluator> _expEvaluator;
	ExpressionData _conditionData;

	void WriteByteCode(DisassemblyInfo& info, RowPart& rowPart, string& output)
	{
		string byteCode;
		info.GetByteCode(byteCode);
		if(!rowPart.DisplayInHex) {
			//Remove $ marks if not in "hex" mode (but still display the bytes as hex)
			byteCode.erase(std::remove(byteCode.begin(), byteCode.end(), '$'), byteCode.end());
		}
		WriteStringValue(output, byteCode, rowPart);
	}

	void WriteDisassembly(DisassemblyInfo& info, RowPart& rowPart, uint8_t sp, uint32_t pc, string& output)
	{
		int indentLevel = 0;
		string code;

		if(_options.IndentCode) {
			indentLevel = 0xFF - (sp & 0xFF);
			code = std::string(indentLevel, ' ');
		}

		LabelManager* labelManager = _options.UseLabels ? _labelManager : nullptr;
		info.GetDisassembly(code, pc, labelManager, _settings);
		WriteStringValue(output, code, rowPart);
	}
	
	void WriteEffectiveAddress(DisassemblyInfo& info, RowPart& rowPart, void* cpuState, string& output, MemoryType cpuMemoryType, CpuType cpuType)
	{
		int32_t effectiveAddress = info.GetEffectiveAddress(_debugger, cpuState, cpuType);
		if(effectiveAddress >= 0) {
			if(_options.UseLabels) {
				AddressInfo addr { effectiveAddress, cpuMemoryType };
				string label = _labelManager->GetLabel(addr);
				if(!label.empty()) {
					WriteStringValue(output, " [" + label + "]", rowPart);
					return;
				}
			}
			WriteStringValue(output, " [" + HexUtilities::ToHex24(effectiveAddress) + "]", rowPart);
		}
	}

	void WriteMemoryValue(DisassemblyInfo& info, RowPart& rowPart, void* cpuState, string& output, MemoryType memType, CpuType cpuType)
	{
		int32_t address = info.GetEffectiveAddress(_debugger, cpuState, cpuType);
		if(address >= 0) {
			uint8_t valueSize;
			uint16_t value = info.GetMemoryValue(address, _memoryDumper, memType, valueSize);
			if(rowPart.DisplayInHex) {
				output += "= $";
				if(valueSize == 2) {
					WriteIntValue(output, (uint16_t)value, rowPart);
				} else {
					WriteIntValue(output, (uint8_t)value, rowPart);
				}
			} else {
				output += "= ";
			}
		}
	}

	void GetStatusFlag(const char* activeStatusLetters, const char* inactiveStatusLetters, string& output, uint8_t ps, RowPart& part, int length = 8)
	{
		if(part.DisplayInHex) {
			WriteIntValue(output, ps, part);
		} else {
			string flags;
			for(int i = 0; i < length; i++) {
				if(ps & 0x80) {
					flags += activeStatusLetters[i];
				} else if(part.MinWidth >= length) {
					flags += inactiveStatusLetters[i];
				}
				ps <<= 1;
			}
			WriteStringValue(output, flags, part);
		}
	}

	void WriteAlign(int originalSize, RowPart& rowPart, string& output)
	{
		if((int)output.size() - originalSize < rowPart.MinWidth) {
			output += std::string(rowPart.MinWidth - (output.size() - originalSize), ' ');
		}
	}
	
	void WriteIntValue(string& output, uint32_t value, RowPart& rowPart)
	{
		string str = rowPart.DisplayInHex ? HexUtilities::ToHex(value) : std::to_string(value);
		if(rowPart.MinWidth > (int)str.size()) {
			if(rowPart.DisplayInHex) {
				str = std::string(rowPart.MinWidth - str.size(), '0') + str;
			} else {
				str += std::string(rowPart.MinWidth - str.size(), ' ');
			}
		}
		output += str;
	}
	
	void WriteIntValue(string& output, int32_t value, RowPart& rowPart)
	{
		string str = rowPart.DisplayInHex ? HexUtilities::ToHex(value) : std::to_string(value);
		output += str;
		if(rowPart.MinWidth > (int)str.size()) {
			output += std::string(rowPart.MinWidth - str.size(), ' ');
		}
	}

	void WriteIntValue(string& output, uint16_t value, RowPart& rowPart)
	{
		string str = rowPart.DisplayInHex ? HexUtilities::ToHex(value) : std::to_string(value);
		output += str;
		if(rowPart.MinWidth > (int)str.size()) {
			output += std::string(rowPart.MinWidth - str.size(), ' ');
		}
	}

	void WriteIntValue(string& output, uint8_t value, RowPart& rowPart)
	{
		string str = rowPart.DisplayInHex ? HexUtilities::ToHex(value) : std::to_string(value);
		output += str;
		if(rowPart.MinWidth > (int)str.size()) {
			output += std::string(rowPart.MinWidth - str.size(), ' ');
		}
	}

	void WriteStringValue(string& output, string value, RowPart& rowPart)
	{
		output += value;
		if(rowPart.MinWidth > (int)value.size()) {
			output += std::string(rowPart.MinWidth - value.size(), ' ');
		}
	}

	void AddRow(CpuStateType& cpuState, DisassemblyInfo& disassemblyInfo)
	{
		_disassemblyCache[_currentPos] = disassemblyInfo;
		_cpuState[_currentPos] = cpuState;
		((TraceLoggerType*)this)->LogPpuState();

		_rowIds[_currentPos] = ITraceLogger::NextRowId;
		ITraceLogger::NextRowId++;

		_pendingLog = false;

		if(_debugger->GetTraceLogFileSaver()->IsEnabled()) {
			string row;
			
			//Display PC
			RowPart rowPart = {};
			rowPart.DisplayInHex = true;
			rowPart.MinWidth = DebugUtilities::GetProgramCounterSize(_cpuType);
			WriteIntValue(row, ((TraceLoggerType*)this)->GetProgramCounter(cpuState), rowPart);
			row += "  ";

			((TraceLoggerType*)this)->GetTraceRow(row, cpuState, _ppuState[_currentPos], disassemblyInfo);
			_debugger->GetTraceLogFileSaver()->Log(row);
		}

		_currentPos = (_currentPos + 1) % ExecutionLogSize;
	}

	void ParseFormatString(string format)
	{
		_rowParts.clear();

		std::regex formatRegex = std::regex("(\\[\\s*([^[]*?)\\s*(,\\s*([\\d]*)\\s*(h){0,1}){0,1}\\s*\\])|([^[]*)", std::regex_constants::icase);
		std::sregex_iterator start = std::sregex_iterator(format.cbegin(), format.cend(), formatRegex);
		std::sregex_iterator end = std::sregex_iterator();

		for(std::sregex_iterator it = start; it != end; it++) {
			const std::smatch& match = *it;

			if(match.str(1) == "") {
				RowPart part = {};
				part.DataType = RowDataType::Text;
				part.Text = match.str(6);
				_rowParts.push_back(part);
			} else {
				RowPart part = {};

				string tag = match.str(2);
				part.DataType = InternalGetFormatTagType(tag);
				if(part.DataType == RowDataType::Text) {
					part.Text = "[Invalid tag]";
				}

				if(!match.str(4).empty()) {
					try {
						part.MinWidth = std::stoi(match.str(4));
					} catch(std::exception&) {
					}
				}
				part.DisplayInHex = match.str(5) == "h";

				_rowParts.push_back(part);
			}
		}
	}

	RowDataType InternalGetFormatTagType(string& tag)
	{
		if(tag == "ByteCode") {
			return RowDataType::ByteCode;
		} else if(tag == "Disassembly") {
			return RowDataType::Disassembly;
		} else if(tag == "EffectiveAddress") {
			return RowDataType::EffectiveAddress;
		} else if(tag == "MemoryValue") {
			return RowDataType::MemoryValue;
		} else if(tag == "Align") {
			return RowDataType::Align;
		} else if(tag == "PC") {
			return RowDataType::PC;
		} else if(tag == "Cycle") {
			return RowDataType::Cycle;
		} else if(tag == "HClock") {
			return RowDataType::HClock;
		} else if(tag == "Scanline") {
			return RowDataType::Scanline;
		} else if(tag == "FrameCount") {
			return RowDataType::FrameCount;
		} else if(tag == "CycleCount") {
			return RowDataType::CycleCount;
		}

		return GetFormatTagType(tag);
	}

	virtual RowDataType GetFormatTagType(string& tag) = 0;

	void ProcessSharedTag(RowPart& rowPart, string& output, CpuStateType& cpuState, TraceLogPpuState& ppuState, DisassemblyInfo& disassemblyInfo)
	{
		switch(rowPart.DataType) {
			case RowDataType::Text: output += rowPart.Text; break;
			case RowDataType::ByteCode: WriteByteCode(disassemblyInfo, rowPart, output); break;
			case RowDataType::Disassembly: WriteDisassembly(disassemblyInfo, rowPart, ((TraceLoggerType*)this)->GetStackPointer(cpuState), ((TraceLoggerType*)this)->GetProgramCounter(cpuState), output); break;
			case RowDataType::EffectiveAddress: WriteEffectiveAddress(disassemblyInfo, rowPart, &cpuState, output, _cpuMemoryType, _cpuType); break;
			case RowDataType::MemoryValue: WriteMemoryValue(disassemblyInfo, rowPart, &cpuState, output, _cpuMemoryType, _cpuType); break;
			case RowDataType::Align: WriteAlign(0, rowPart, output); break;

			case RowDataType::Cycle: WriteIntValue(output, ppuState.Cycle, rowPart); break;
			case RowDataType::Scanline: WriteIntValue(output, ppuState.Scanline, rowPart); break;
			case RowDataType::HClock: WriteIntValue(output, ppuState.HClock, rowPart); break;
			case RowDataType::FrameCount: WriteIntValue(output, ppuState.FrameCount, rowPart); break;
			case RowDataType::CycleCount:
				WriteIntValue(output, (uint32_t)(((TraceLoggerType*)this)->GetCycleCount(cpuState) >> 32), rowPart);
				WriteIntValue(output, (uint32_t)((TraceLoggerType*)this)->GetCycleCount(cpuState), rowPart);
				break;

			case RowDataType::PC: WriteStringValue(output, HexUtilities::ToHex(((TraceLoggerType*)this)->GetProgramCounter(cpuState)), rowPart); break;
		}
	}

public:
	BaseTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, CpuType cpuType)
	{
		_debugger = debugger;
		_console = debugger->GetConsole();
		_settings = debugger->GetEmulator()->GetSettings();
		_labelManager = debugger->GetLabelManager();
		_memoryDumper = debugger->GetMemoryDumper();
		_options = {};
		_currentPos = 0;
		_pendingLog = false;

		_disassemblyCache = new DisassemblyInfo[BaseTraceLogger::ExecutionLogSize];
		_rowIds = new uint64_t[BaseTraceLogger::ExecutionLogSize];
		memset(_disassemblyCache, 0, sizeof(DisassemblyInfo) * BaseTraceLogger::ExecutionLogSize);
		memset(_rowIds, 0, sizeof(uint64_t) * BaseTraceLogger::ExecutionLogSize);

		_ppuState = new TraceLogPpuState[BaseTraceLogger::ExecutionLogSize];
		memset(_ppuState, 0, sizeof(TraceLogPpuState) * BaseTraceLogger::ExecutionLogSize);

		_cpuState = new CpuStateType[BaseTraceLogger::ExecutionLogSize];
		memset(_cpuState, 0, sizeof(CpuStateType) * BaseTraceLogger::ExecutionLogSize);

		_cpuType = cpuType;
		_cpuMemoryType = DebugUtilities::GetCpuMemoryType(cpuType);

		_expEvaluator.reset(new ExpressionEvaluator(debugger, cpuDebugger, cpuType));
	}

	virtual ~BaseTraceLogger()
	{
		delete[] _disassemblyCache;
		delete[] _rowIds;
		delete[] _ppuState;
		delete[] _cpuState;
	}

	void Clear()
	{
	}

	void LogNonExec(MemoryOperationInfo& operation)
	{
		if(_pendingLog) {
			int pos = _currentPos - 1;
			if(pos < 0) {
				pos = BaseTraceLogger::ExecutionLogSize - 1;
			}

			if(ConditionMatches(_lastDisassemblyInfo, operation)) {
				AddRow(_lastState, _lastDisassemblyInfo);
				_pendingLog = false;
			}
		}
	}

	void Log(CpuStateType& cpuState, DisassemblyInfo& disassemblyInfo, MemoryOperationInfo& operation)
	{
		if(_enabled) {
			//For the sake of performance, only log data for the CPUs we're actively displaying/logging
			if(ConditionMatches(disassemblyInfo, operation)) {
				AddRow(cpuState, disassemblyInfo);
			} else {
				_pendingLog = true;
				_lastState = cpuState;
				_lastDisassemblyInfo = disassemblyInfo;
			}
		}
	}

	void SetOptions(TraceLoggerOptions options) override
	{
		DebugBreakHelper helper(_debugger);
		_options = options;

		_enabled = options.Enabled;

		string condition = _options.Condition;
		string format = _options.Format;

		_conditionData = ExpressionData();
		if(!condition.empty()) {
			bool success = false;
			ExpressionData rpnList = _expEvaluator->GetRpnList(condition, success);
			if(success) {
				_conditionData = rpnList;
			}
		}

		ParseFormatString(format);
	}

	int64_t GetRowId(uint32_t offset) override
	{
		int32_t pos = ((int32_t)_currentPos - (int32_t)offset);
		int32_t i = (pos > 0 ? pos : BaseTraceLogger::ExecutionLogSize + pos) - 1;
		if(!_disassemblyCache[i].IsInitialized()) {
			return -1;
		}
		return _rowIds[i];
	}

	bool ConditionMatches(DisassemblyInfo &disassemblyInfo, MemoryOperationInfo &operationInfo)
	{
		if(!_conditionData.RpnQueue.empty()) {
			EvalResultType type;
			if(!_expEvaluator->Evaluate(_conditionData, type, operationInfo)) {
				return false;
			}
		}
		return true;
	}

	void GetExecutionTrace(TraceRow& row, uint32_t offset) override
	{
		int pos = ((int)_currentPos - offset);
		int index = (pos > 0 ? pos : BaseTraceLogger::ExecutionLogSize + pos) - 1;

		CpuStateType& state = _cpuState[index];
		string logOutput;
		((TraceLoggerType*)this)->GetTraceRow(logOutput, state, _ppuState[index], _disassemblyCache[index]);

		row.Type = _cpuType;
		_disassemblyCache[index].GetByteCode(row.ByteCode);
		row.ByteCodeSize = _disassemblyCache[index].GetOpSize();
		row.ProgramCounter = ((TraceLoggerType*)this)->GetProgramCounter(state);
		memcpy(row.LogOutput, logOutput.c_str(), logOutput.size());
	}
};
