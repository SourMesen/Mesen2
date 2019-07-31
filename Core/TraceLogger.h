#pragma once
#include "stdafx.h"
#include "CpuTypes.h"
#include "PpuTypes.h"
#include "SpcTypes.h"
#include "DebugTypes.h"
#include "DisassemblyInfo.h"
#include "../Utilities/SimpleLock.h"

class Console;
class Debugger;
class LabelManager;
class MemoryDumper;
struct DebugState;

struct TraceLoggerOptions
{
	bool LogCpu;
	bool LogSpc;
	bool LogNecDsp;
	bool LogSa1;
	bool LogGsu;

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
	HClock,
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
	Console* _console;
	LabelManager* _labelManager;
	MemoryDumper* _memoryDumper;

	vector<RowPart> _rowParts;
	vector<RowPart> _spcRowParts;
	vector<RowPart> _dspRowParts;
	vector<RowPart> _gsuRowParts;

	bool _logCpu[(int)CpuType::Gsu + 1] = {};

	bool _pendingLog;
	//CpuState _lastState;
	//DisassemblyInfo _lastDisassemblyInfo;

	bool _logToFile;
	uint32_t _currentPos;
	uint32_t _logCount;
	DebugState *_stateCache = nullptr;
	DisassemblyInfo *_disassemblyCache = nullptr;
	CpuType* _logCpuType = nullptr;

	DebugState *_stateCacheCopy = nullptr;
	DisassemblyInfo *_disassemblyCacheCopy = nullptr;
	CpuType* _logCpuTypeCopy = nullptr;

	SimpleLock _lock;

	template<CpuType cpuType> void GetStatusFlag(string &output, uint8_t ps, RowPart& part);

	void WriteByteCode(DisassemblyInfo &info, RowPart &rowPart, string &output);
	void WriteDisassembly(DisassemblyInfo &info, RowPart &rowPart, uint8_t sp, uint32_t pc, string &output);
	void WriteEffectiveAddress(DisassemblyInfo &info, RowPart &rowPart, void *cpuState, string &output, SnesMemoryType cpuMemoryType);
	void WriteMemoryValue(DisassemblyInfo &info, RowPart &rowPart, void *cpuState, string &output, SnesMemoryType memType);
	void WriteAlign(int originalSize, RowPart &rowPart, string &output);
	void AddRow(CpuType cpuType, DisassemblyInfo &disassemblyInfo, DebugState &state);
	//bool ConditionMatches(DebugState &state, DisassemblyInfo &disassemblyInfo, OperationInfo &operationInfo);
	
	void ParseFormatString(vector<RowPart> &rowParts, string format);

	void GetTraceRow(string &output, CpuType cpuType, DisassemblyInfo &disassemblyInfo, DebugState &state);
	void GetTraceRow(string &output, CpuState &cpuState, PpuState &ppuState, DisassemblyInfo &disassemblyInfo, SnesMemoryType memType);
	void GetTraceRow(string &output, SpcState &cpuState, PpuState &ppuState, DisassemblyInfo &disassemblyInfo);
	void GetTraceRow(string &output, NecDspState &cpuState, PpuState &ppuState, DisassemblyInfo &disassemblyInfo);
	void GetTraceRow(string &output, GsuState &gsuState, PpuState &ppuState, DisassemblyInfo &disassemblyInfo);

	template<typename T> void WriteValue(string &output, T value, RowPart& rowPart);

public:
	TraceLogger(Debugger* debugger, shared_ptr<Console> console);
	~TraceLogger();

	__forceinline bool IsCpuLogged(CpuType type) { return _logCpu[(int)type]; }

	void Log(CpuType cpuType, DebugState &state, DisassemblyInfo &disassemblyInfo);
	void Clear();
	//void LogNonExec(OperationInfo& operationInfo);
	void SetOptions(TraceLoggerOptions options);
	void StartLogging(string filename);
	void StopLogging();

	void LogExtraInfo(const char *log, uint32_t cycleCount);

	const char* GetExecutionTrace(uint32_t lineCount);
};
