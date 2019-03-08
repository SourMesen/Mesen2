#pragma once
#include "stdafx.h"
#include "CpuTypes.h"
#include "PpuTypes.h"
#include "DisassemblyInfo.h"
#include "../Utilities/SimpleLock.h"

class Console;
class Debugger;
struct DebugState;

struct TraceLoggerOptions
{
	bool ShowExtraInfo;
	bool IndentCode;
	bool UseLabels;
	bool UseWindowsEol;
	bool ExtendZeroPage;

	char Condition[1000];
	char Format[1000];
};

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
	X,
	Y,
	D,
	DB,
	SP,
	PS,
	Cycle,
	Scanline,
	FrameCount,
	CycleCount
};

struct RowPart
{
	RowDataType DataType;
	string Text;
	bool DisplayInHex;
	int MinWidth;
};

class TraceLogger
{
private:
	static constexpr int ExecutionLogSize = 30000;

	//Must be static to be thread-safe when switching game
	static string _executionTrace;

	TraceLoggerOptions _options;
	string _outputFilepath;
	string _outputBuffer;
	ofstream _outputFile;
	shared_ptr<Console> _console;
	//shared_ptr<LabelManager> _labelManager;

	vector<RowPart> _rowParts;

	bool _pendingLog;
	//CpuState _lastState;
	//DisassemblyInfo _lastDisassemblyInfo;

	bool _logToFile;
	uint16_t _currentPos;
	uint32_t _logCount;
	CpuState _cpuStateCache[ExecutionLogSize] = {};
	PpuState _ppuStateCache[ExecutionLogSize] = {};
	DisassemblyInfo _disassemblyCache[ExecutionLogSize];

	CpuState _cpuStateCacheCopy[ExecutionLogSize] = {};
	PpuState _ppuStateCacheCopy[ExecutionLogSize] = {};
	DisassemblyInfo _disassemblyCacheCopy[ExecutionLogSize];

	SimpleLock _lock;

	void GetStatusFlag(string &output, uint8_t ps, RowPart& part);
	void AddRow(DisassemblyInfo &disassemblyInfo, DebugState &state);
	//bool ConditionMatches(DebugState &state, DisassemblyInfo &disassemblyInfo, OperationInfo &operationInfo);

	void GetTraceRow(string &output, CpuState &cpuState, PpuState &ppuState, DisassemblyInfo &disassemblyInfo);

	template<typename T> void WriteValue(string &output, T value, RowPart& rowPart);

public:
	TraceLogger(Debugger* debugger, shared_ptr<Console> console);
	~TraceLogger();

	void Log(DebugState &state, DisassemblyInfo &disassemblyInfo);
	//void LogNonExec(OperationInfo& operationInfo);
	void SetOptions(TraceLoggerOptions options);
	void StartLogging(string filename);
	void StopLogging();

	void LogExtraInfo(const char *log, uint32_t cycleCount);

	const char* GetExecutionTrace(uint32_t lineCount);
};
