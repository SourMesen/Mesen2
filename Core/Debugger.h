#pragma once
#include "stdafx.h"
#include "CpuTypes.h"
#include "PpuTypes.h"
#include "DebugTypes.h"
#include "DebugUtilities.h"
#include "../Utilities/SimpleLock.h"

class Console;
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
class SpcDebugger;
class CpuDebugger;
class GsuDebugger;
class NecDspDebugger;
class Cx4Debugger;
class GbDebugger;
class Breakpoint;
class IEventManager;
class IAssembler;
class Gameboy;

enum class EventType;
enum class EvalResultType : int32_t;

class Debugger
{
private:
	shared_ptr<Console> _console;
	shared_ptr<Cpu> _cpu;
	shared_ptr<Ppu> _ppu;
	shared_ptr<Spc> _spc;
	shared_ptr<MemoryManager> _memoryManager;
	shared_ptr<BaseCartridge> _cart;
	shared_ptr<InternalRegisters> _internalRegs;
	shared_ptr<DmaController> _dmaController;

	Gameboy* _gameboy = nullptr;

	shared_ptr<EmuSettings> _settings;
	unique_ptr<SpcDebugger> _spcDebugger;
	unique_ptr<CpuDebugger> _cpuDebugger;
	unique_ptr<CpuDebugger> _sa1Debugger;
	unique_ptr<GsuDebugger> _gsuDebugger;
	unique_ptr<NecDspDebugger> _necDspDebugger;
	unique_ptr<Cx4Debugger> _cx4Debugger;
	unique_ptr<GbDebugger> _gbDebugger;

	shared_ptr<ScriptManager> _scriptManager;
	shared_ptr<TraceLogger> _traceLogger;
	shared_ptr<MemoryDumper> _memoryDumper;
	shared_ptr<MemoryAccessCounter> _memoryAccessCounter;
	shared_ptr<CodeDataLogger> _codeDataLogger;
	shared_ptr<Disassembler> _disassembler;
	shared_ptr<PpuTools> _ppuTools;
	shared_ptr<LabelManager> _labelManager;

	unique_ptr<ExpressionEvaluator> _watchExpEval[(int)DebugUtilities::GetLastCpuType() + 1];
	
	SimpleLock _logLock;
	std::list<string> _debuggerLog;

	atomic<bool> _executionStopped;
	atomic<uint32_t> _breakRequestCount;
	atomic<uint32_t> _suspendRequestCount;

	unique_ptr<StepRequest> _step;
	
	bool _waitForBreakResume = false;
	
	void Reset();

public:
	Debugger(shared_ptr<Console> console);
	~Debugger();
	void Release();

	template<CpuType type>
	void ProcessMemoryRead(uint32_t addr, uint8_t value, MemoryOperationType opType);
	
	template<CpuType type>
	void ProcessMemoryWrite(uint32_t addr, uint8_t value, MemoryOperationType opType);

	void ProcessWorkRamRead(uint32_t addr, uint8_t value);
	void ProcessWorkRamWrite(uint32_t addr, uint8_t value);

	void ProcessPpuRead(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
	void ProcessPpuWrite(uint16_t addr, uint8_t value, SnesMemoryType memoryType);

	template<CpuType cpuType>
	void ProcessPpuCycle();

	template<CpuType type>
	void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi);

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

	void GetState(DebugState &state, bool partialPpuState);

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
	shared_ptr<MemoryDumper> GetMemoryDumper();
	shared_ptr<MemoryAccessCounter> GetMemoryAccessCounter();
	shared_ptr<CodeDataLogger> GetCodeDataLogger(CpuType cpuType);
	shared_ptr<Disassembler> GetDisassembler();
	shared_ptr<PpuTools> GetPpuTools();
	shared_ptr<IEventManager> GetEventManager(CpuType cpuType);
	shared_ptr<LabelManager> GetLabelManager();
	shared_ptr<ScriptManager> GetScriptManager();
	shared_ptr<CallstackManager> GetCallstackManager(CpuType cpuType);
	shared_ptr<Console> GetConsole();
	shared_ptr<IAssembler> GetAssembler(CpuType cpuType);
};
