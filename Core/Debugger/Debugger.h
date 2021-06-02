#pragma once
#include "stdafx.h"
#include "Utilities/SimpleLock.h"
#include "Debugger/DebugUtilities.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"

class IConsole;
class Emulator;
class Cpu;
class Ppu;
class Spc;
class BaseCartridge;
class MemoryManager;
class InternalRegisters;
class DmaController;
class EmuSettings;

class TraceLogger;
class ExpressionEvaluator;
class MemoryDumper;
class MemoryAccessCounter;
class Disassembler;
class BreakpointManager;
class PpuTools;
class CodeDataLogger;
class EventManager;
class CallstackManager;
class LabelManager;
class ScriptManager;
class Breakpoint;
class IEventManager;
class IAssembler;
class IDebugger;

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

	EmuSettings* _settings;

	CpuInfo _debuggers[(int)DebugUtilities::GetLastCpuType() + 1];

	CpuType _mainCpuType = CpuType::Cpu;
	ConsoleType _consoleType = ConsoleType::Snes;

	shared_ptr<ScriptManager> _scriptManager;
	shared_ptr<TraceLogger> _traceLogger;
	shared_ptr<MemoryDumper> _memoryDumper;
	shared_ptr<MemoryAccessCounter> _memoryAccessCounter;
	shared_ptr<CodeDataLogger> _codeDataLogger;
	shared_ptr<Disassembler> _disassembler;
	shared_ptr<PpuTools> _ppuTools;
	shared_ptr<LabelManager> _labelManager;

	SimpleLock _logLock;
	std::list<string> _debuggerLog;

	atomic<bool> _executionStopped;
	atomic<uint32_t> _breakRequestCount;
	atomic<uint32_t> _suspendRequestCount;

	bool _waitForBreakResume = false;
	
	void Reset();

public:
	Debugger(Emulator* emu, IConsole* console);
	~Debugger();
	void Release();

	template<CpuType type> void ProcessMemoryRead(uint32_t addr, uint8_t value, MemoryOperationType opType);
	template<CpuType type> void ProcessMemoryWrite(uint32_t addr, uint8_t value, MemoryOperationType opType);
	template<CpuType type> void ProcessPpuRead(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
	template<CpuType type> void ProcessPpuWrite(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
	template<CpuType type> void ProcessPpuCycle();
	template<CpuType type> void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi);

	void ProcessEvent(EventType type);

	int32_t EvaluateExpression(string expression, CpuType cpuType, EvalResultType &resultType, bool useCache);

	void Run();
	void Step(CpuType cpuType, int32_t stepCount, StepType type);
	bool IsExecutionStopped();

	bool HasBreakRequest();
	void BreakRequest(bool release);
	void SuspendDebugger(bool release);

	void BreakImmediately(BreakSource source);

	void ProcessBreakConditions(bool needBreak, BreakpointManager *bpManager, MemoryOperationInfo &operation, AddressInfo &addressInfo, BreakSource source = BreakSource::Unspecified);
	void SleepUntilResume(BreakSource source, MemoryOperationInfo* operation = nullptr, int breakpointId = -1);

	void GetState(BaseState &state, CpuType cpuType);
	BaseState& GetStateRef(CpuType cpuType);

	AddressInfo GetAbsoluteAddress(AddressInfo relAddress);
	AddressInfo GetRelativeAddress(AddressInfo absAddress, CpuType cpuType);

	void GetCdlData(uint32_t offset, uint32_t length, SnesMemoryType memoryType, uint8_t* cdlData);
	void SetCdlData(CpuType cpuType, uint8_t * cdlData, uint32_t length);
	void MarkBytesAs(CpuType cpuType, uint32_t start, uint32_t end, uint8_t flags);
	
	void RefreshCodeCache();
	void RebuildPrgCache(CpuType cpuType);

	void SetBreakpoints(Breakpoint breakpoints[], uint32_t length);
	
	void Log(string message);
	string GetLog();

	void SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption);

	shared_ptr<TraceLogger> GetTraceLogger();
	MemoryDumper* GetMemoryDumper();
	shared_ptr<MemoryAccessCounter> GetMemoryAccessCounter();
	shared_ptr<CodeDataLogger> GetCodeDataLogger(CpuType cpuType);
	shared_ptr<Disassembler> GetDisassembler();
	shared_ptr<PpuTools> GetPpuTools();
	shared_ptr<IEventManager> GetEventManager(CpuType cpuType);
	shared_ptr<LabelManager> GetLabelManager();
	shared_ptr<ScriptManager> GetScriptManager();
	shared_ptr<CallstackManager> GetCallstackManager(CpuType cpuType);
	IConsole* GetConsole();
	Emulator* GetEmulator();
	shared_ptr<IAssembler> GetAssembler(CpuType cpuType);
};
