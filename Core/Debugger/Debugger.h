#pragma once
#include "stdafx.h"
#include "Utilities/SimpleLock.h"
#include "Debugger/DebugUtilities.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebuggerFeatures.h"
#include "Shared/SettingTypes.h"

class IConsole;
class Emulator;
class SnesCpu;
class SnesPpu;
class Spc;
class BaseCartridge;
class SnesMemoryManager;
class InternalRegisters;
class SnesDmaController;
class EmuSettings;

class ExpressionEvaluator;
class MemoryDumper;
class MemoryAccessCounter;
class Disassembler;
class BreakpointManager;
class PpuTools;
class CodeDataLogger;
class CallstackManager;
class LabelManager;
class ScriptManager;
class Breakpoint;
class BaseEventManager;
class IAssembler;
class IDebugger;
class ITraceLogger;
class TraceLogFileSaver;

struct TraceRow;
struct BaseState;

enum class EventType;
enum class MemoryOperationType;
enum class EvalResultType : int32_t;

struct CpuInfo
{
	unique_ptr<IDebugger> Debugger;
	unique_ptr<ExpressionEvaluator> Evaluator;
};

class Debugger
{
private:
	Emulator* _emu = nullptr;
	IConsole* _console = nullptr;

	EmuSettings* _settings = nullptr;

	CpuInfo _debuggers[(int)DebugUtilities::GetLastCpuType() + 1] = {};
	CodeDataLogger* _codeDataLoggers[(int)MemoryType::Register + 1] = {};

	CpuType _mainCpuType = CpuType::Snes;
	unordered_set<CpuType> _cpuTypes;
	ConsoleType _consoleType = ConsoleType::Snes;

	unique_ptr<ScriptManager> _scriptManager;
	unique_ptr<MemoryDumper> _memoryDumper;
	unique_ptr<MemoryAccessCounter> _memoryAccessCounter;
	unique_ptr<CodeDataLogger> _codeDataLogger;
	unique_ptr<Disassembler> _disassembler;
	unique_ptr<LabelManager> _labelManager;

	unique_ptr<TraceLogFileSaver> _traceLogSaver;

	SimpleLock _logLock;
	std::list<string> _debuggerLog;

	atomic<bool> _executionStopped;
	atomic<uint32_t> _breakRequestCount;
	atomic<uint32_t> _suspendRequestCount;

	bool _waitForBreakResume = false;
	
	void Reset();

	template<CpuType type, typename DebuggerType> DebuggerType* GetDebugger();

public:
	Debugger(Emulator* emu, IConsole* console);
	~Debugger();
	void Release();

	template<CpuType type> void ProcessInstruction();
	template<CpuType type> void ProcessMemoryRead(uint32_t addr, uint8_t value, MemoryOperationType opType);
	template<CpuType type> void ProcessMemoryWrite(uint32_t addr, uint8_t value, MemoryOperationType opType);
	template<CpuType type> void ProcessPpuRead(uint16_t addr, uint8_t value, MemoryType memoryType, MemoryOperationType opType);
	template<CpuType type> void ProcessPpuWrite(uint16_t addr, uint8_t value, MemoryType memoryType);
	template<CpuType type> void ProcessPpuCycle();
	template<CpuType type> void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi);

	void InternalProcessInterrupt(CpuType cpuType, IDebugger& dbg, StepRequest& stepRequest, AddressInfo& src, uint32_t srcAddr, AddressInfo& dest, uint32_t destAddr, AddressInfo& ret, uint32_t retAddr, bool forNmi);

	void ProcessEvent(EventType type);

	void ProcessConfigChange();

	void GetTokenList(CpuType cpuType, char* tokenList);
	int32_t EvaluateExpression(string expression, CpuType cpuType, EvalResultType &resultType, bool useCache);

	void Run();
	void Step(CpuType cpuType, int32_t stepCount, StepType type);
	bool IsPaused();
	bool IsExecutionStopped();

	bool HasBreakRequest();
	void BreakRequest(bool release);
	void ResetSuspendCounter();
	void SuspendDebugger(bool release);

	void BreakImmediately(CpuType sourceCpu, BreakSource source);

	void ProcessPredictiveBreakpoint(CpuType sourceCpu, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo);
	void ProcessBreakConditions(CpuType sourceCpu, StepRequest& step, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo);
	void SleepUntilResume(CpuType sourceCpu, BreakSource source, MemoryOperationInfo* operation = nullptr, int breakpointId = -1);

	void GetCpuState(BaseState& dstState, CpuType cpuType);
	void SetCpuState(BaseState& srcState, CpuType cpuType);
	BaseState& GetCpuStateRef(CpuType cpuType);

	void GetPpuState(BaseState& state, CpuType cpuType);
	void SetPpuState(BaseState& srcState, CpuType cpuType);

	void GetConsoleState(BaseState& state, ConsoleType consoleType);

	DebuggerFeatures GetDebuggerFeatures(CpuType cpuType);
	uint32_t GetProgramCounter(CpuType cpuType, bool forInstStart);
	void SetProgramCounter(CpuType cpuType, uint32_t addr);

	AddressInfo GetAbsoluteAddress(AddressInfo relAddress);
	AddressInfo GetRelativeAddress(AddressInfo absAddress, CpuType cpuType);

	bool HasCpuType(CpuType cpuType);

	void GetCdlData(uint32_t offset, uint32_t length, MemoryType memoryType, uint8_t* cdlData);
	int16_t GetCdlFlags(MemoryType memType, uint32_t addr);
	void SetCdlData(MemoryType memType, uint8_t * cdlData, uint32_t length);
	void MarkBytesAs(MemoryType memType, uint32_t start, uint32_t end, uint8_t flags);
	CdlStatistics GetCdlStatistics(MemoryType memType);
	void ResetCdl(MemoryType memType);
	void LoadCdlFile(MemoryType memType, char* cdlFile);
	void SaveCdlFile(MemoryType memType, char* cdlFile);
	void RegisterCdl(MemoryType memType, CodeDataLogger* cdl);

	void RefreshCodeCache();

	void SetBreakpoints(Breakpoint breakpoints[], uint32_t length);
	
	void Log(string message);
	string GetLog();

	void SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption);

	void ClearExecutionTrace();
	uint32_t GetExecutionTrace(TraceRow output[], uint32_t startOffset, uint32_t maxLineCount);

	TraceLogFileSaver* GetTraceLogFileSaver();
	ITraceLogger* GetTraceLogger(CpuType cpuType);
	MemoryDumper* GetMemoryDumper();
	MemoryAccessCounter* GetMemoryAccessCounter();
	CodeDataLogger* GetCodeDataLogger(MemoryType memType);
	Disassembler* GetDisassembler();
	PpuTools* GetPpuTools(CpuType cpuType);
	BaseEventManager* GetEventManager(CpuType cpuType);
	LabelManager* GetLabelManager();
	ScriptManager* GetScriptManager();
	CallstackManager* GetCallstackManager(CpuType cpuType);
	IConsole* GetConsole();
	Emulator* GetEmulator();
	IAssembler* GetAssembler(CpuType cpuType);
};
