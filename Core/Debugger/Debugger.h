#pragma once
#include "pch.h"
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
class DisassemblySearch;
class BreakpointManager;
class PpuTools;
class CodeDataLogger;
class CallstackManager;
class LabelManager;
class CdlManager;
class ScriptManager;
class Breakpoint;
class BaseEventManager;
class IAssembler;
class IDebugger;
class ITraceLogger;
class TraceLogFileSaver;
class FrozenAddressManager;

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

	CpuInfo _debuggers[(int)DebugUtilities::GetLastCpuType() + 1];

	CpuType _mainCpuType = CpuType::Snes;
	unordered_set<CpuType> _cpuTypes;
	ConsoleType _consoleType = ConsoleType::Snes;

	unique_ptr<ScriptManager> _scriptManager;
	unique_ptr<MemoryDumper> _memoryDumper;
	unique_ptr<MemoryAccessCounter> _memoryAccessCounter;
	unique_ptr<CodeDataLogger> _codeDataLogger;
	unique_ptr<Disassembler> _disassembler;
	unique_ptr<DisassemblySearch> _disassemblySearch;
	unique_ptr<LabelManager> _labelManager;
	unique_ptr<CdlManager> _cdlManager;

	unique_ptr<TraceLogFileSaver> _traceLogSaver;

	SimpleLock _logLock;
	std::list<string> _debuggerLog;

	atomic<bool> _executionStopped;
	atomic<uint32_t> _breakRequestCount;
	atomic<uint32_t> _suspendRequestCount;

	DebugControllerState _inputOverrides[8] = {};

	bool _waitForBreakResume = false;
	
	void Reset();

	__noinline bool ProcessStepBack(IDebugger* debugger);

	template<CpuType type, typename DebuggerType> DebuggerType* GetDebugger();
	template<CpuType type> uint64_t GetCpuCycleCount();
	template<CpuType type, typename T> void ProcessScripts(uint32_t addr, T& value, MemoryOperationType opType);
	template<CpuType type, typename T> void ProcessScripts(uint32_t addr, T& value, MemoryType memType, MemoryOperationType opType);
	
	bool IsDebugWindowOpened(CpuType cpuType);
	bool IsBreakOptionEnabled(BreakSource src);
	template<CpuType type> void SleepOnBreakRequest();

	void ClearPendingBreakExceptions();

	bool IsBreakpointForbidden(BreakSource source, CpuType sourceCpu, MemoryOperationInfo* operation);

public:
	Debugger(Emulator* emu, IConsole* console);
	~Debugger();
	void Release();

	template<CpuType type> void ProcessInstruction();
	template<CpuType type, uint8_t accessWidth = 1, MemoryAccessFlags flags = MemoryAccessFlags::None, typename T> void ProcessMemoryRead(uint32_t addr, T& value, MemoryOperationType opType);
	template<CpuType type, uint8_t accessWidth = 1, MemoryAccessFlags flags = MemoryAccessFlags::None, typename T> bool ProcessMemoryWrite(uint32_t addr, T& value, MemoryOperationType opType);

	template<CpuType cpuType, MemoryType memType, MemoryOperationType opType, typename T> void ProcessMemoryAccess(uint32_t addr, T& value);

	template<CpuType type> void ProcessIdleCycle();
	template<CpuType type> void ProcessHaltedCpu();
	template<CpuType type, typename T> void ProcessPpuRead(uint16_t addr, T& value, MemoryType memoryType, MemoryOperationType opType);
	template<CpuType type, typename T> void ProcessPpuWrite(uint16_t addr, T& value, MemoryType memoryType);
	template<CpuType type> void ProcessPpuCycle();
	template<CpuType type> void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi);

	void InternalProcessInterrupt(CpuType cpuType, IDebugger& dbg, StepRequest& stepRequest, AddressInfo& src, uint32_t srcAddr, AddressInfo& dest, uint32_t destAddr, AddressInfo& ret, uint32_t retAddr, uint32_t retSp, bool forNmi);

	void ProcessEvent(EventType type, std::optional<CpuType> cpuType);

	void ProcessConfigChange();

	void GetTokenList(CpuType cpuType, char* tokenList);
	int64_t EvaluateExpression(string expression, CpuType cpuType, EvalResultType &resultType, bool useCache);

	void Run();
	void PauseOnNextFrame();
	void Step(CpuType cpuType, int32_t stepCount, StepType type, BreakSource source = BreakSource::Unspecified);
	bool IsPaused();
	bool IsExecutionStopped();

	bool HasBreakRequest();
	void BreakRequest(bool release);
	void ResetSuspendCounter();
	void SuspendDebugger(bool release);

	__noinline void BreakImmediately(CpuType sourceCpu, BreakSource source);

	template<uint8_t accessWidth = 1> void ProcessPredictiveBreakpoint(CpuType sourceCpu, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo);
	template<uint8_t accessWidth = 1> void ProcessBreakConditions(CpuType sourceCpu, StepRequest& step, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo);

	void SleepUntilResume(CpuType sourceCpu, BreakSource source, MemoryOperationInfo* operation = nullptr, int breakpointId = -1);

	void GetCpuState(BaseState& dstState, CpuType cpuType);
	void SetCpuState(BaseState& srcState, CpuType cpuType);
	BaseState& GetCpuStateRef(CpuType cpuType);

	void GetPpuState(BaseState& state, CpuType cpuType);
	void SetPpuState(BaseState& srcState, CpuType cpuType);

	void GetConsoleState(BaseState& state, ConsoleType consoleType);

	DebuggerFeatures GetDebuggerFeatures(CpuType cpuType);
	uint32_t GetProgramCounter(CpuType cpuType, bool forInstStart);
	uint8_t GetCpuFlags(CpuType cpuType);
	CpuInstructionProgress GetInstructionProgress(CpuType cpuType);
	void SetProgramCounter(CpuType cpuType, uint32_t addr);

	AddressInfo GetAbsoluteAddress(AddressInfo relAddress);
	AddressInfo GetRelativeAddress(AddressInfo absAddress, CpuType cpuType);

	bool HasCpuType(CpuType cpuType);

	void SetBreakpoints(Breakpoint breakpoints[], uint32_t length);

	void SetInputOverrides(uint32_t index, DebugControllerState state);
	void GetAvailableInputOverrides(uint8_t* availableIndexes);
	
	void Log(string message);
	string GetLog();

	bool SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption);

	void ClearExecutionTrace();
	uint32_t GetExecutionTrace(TraceRow output[], uint32_t startOffset, uint32_t maxLineCount);
	
	CpuType GetMainCpuType() { return _mainCpuType; }
	IDebugger* GetMainDebugger();

	TraceLogFileSaver* GetTraceLogFileSaver() { return _traceLogSaver.get(); }
	MemoryDumper* GetMemoryDumper() { return _memoryDumper.get(); }
	MemoryAccessCounter* GetMemoryAccessCounter() { return _memoryAccessCounter.get(); }
	Disassembler* GetDisassembler() { return _disassembler.get(); }
	DisassemblySearch* GetDisassemblySearch() { return _disassemblySearch.get(); }
	LabelManager* GetLabelManager() { return _labelManager.get(); }
	CdlManager* GetCdlManager() { return _cdlManager.get(); }
	ScriptManager* GetScriptManager() { return _scriptManager.get(); }
	IConsole* GetConsole() { return _console; }
	Emulator* GetEmulator() { return _emu; }

	FrozenAddressManager* GetFrozenAddressManager(CpuType cpuType);
	ITraceLogger* GetTraceLogger(CpuType cpuType);
	PpuTools* GetPpuTools(CpuType cpuType);
	BaseEventManager* GetEventManager(CpuType cpuType);
	CallstackManager* GetCallstackManager(CpuType cpuType);
	IAssembler* GetAssembler(CpuType cpuType);
};
