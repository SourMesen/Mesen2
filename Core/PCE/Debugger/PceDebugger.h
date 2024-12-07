#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"

class PceConsole;
class Disassembler;
class Debugger;
class CallstackManager;
class MemoryAccessCounter;
class CodeDataLogger;
class EmuSettings;
class ScriptManager;
class BreakpointManager;
class IAssembler;
class BaseEventManager;
class PceTraceLogger;
class PceVdcTools;

class Emulator;
class PceCpu;
class PceVdc;
class PceVce;
class PceVpc;
class PceMemoryManager;
class DummyPceCpu;

enum class MemoryOperationType;

class PceDebugger final : public IDebugger
{
	Debugger* _debugger;
	Emulator* _emu;
	Disassembler* _disassembler;
	MemoryAccessCounter* _memoryAccessCounter;
	EmuSettings* _settings;

	PceConsole* _console;
	PceCpu* _cpu;
	PceVdc* _vdc;
	PceVce* _vce;
	PceVpc* _vpc;
	PceMemoryManager* _memoryManager;

	unique_ptr<CodeDataLogger> _codeDataLogger;
	unique_ptr<BaseEventManager> _eventManager;
	unique_ptr<IAssembler> _assembler;
	unique_ptr<CallstackManager> _callstackManager;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<PceTraceLogger> _traceLogger;
	unique_ptr<PceVdcTools> _ppuTools;

	uint8_t _prevOpCode = 0x01;
	uint32_t _prevProgramCounter = 0;
	uint8_t _prevStackPointer = 0;

	unique_ptr<DummyPceCpu> _dummyCpu;

	string _cdlFile;

	bool IsRegister(MemoryOperationInfo& op);
	__forceinline void ProcessCallStackUpdates(AddressInfo& destAddr, uint16_t destPc, uint8_t sp);

public:
	PceDebugger(Debugger* debugger);
	~PceDebugger();

	void Reset() override;

	uint64_t GetCpuCycleCount(bool forProfiler) override;
	void ResetPrevOpCode() override;

	void ProcessInstruction();
	void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi) override;
	void ProcessPpuRead(uint16_t addr, uint16_t value, MemoryType memoryType);
	void ProcessPpuWrite(uint16_t addr, uint16_t value, MemoryType memoryType);
	void ProcessPpuCycle();

	void Run() override;
	void Step(int32_t stepCount, StepType type) override;
	StepBackConfig GetStepBackConfig() override;

	void DrawPartialFrame() override;

	DebuggerFeatures GetSupportedFeatures() override;
	void SetProgramCounter(uint32_t addr, bool updateDebuggerOnly = false) override;
	uint32_t GetProgramCounter(bool getInstPc) override;

	BreakpointManager* GetBreakpointManager() override;
	ITraceLogger* GetTraceLogger() override;
	PpuTools* GetPpuTools() override;
	bool SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption);
	void ProcessInputOverrides(DebugControllerState inputOverrides[8]) override;
	CallstackManager* GetCallstackManager() override;
	IAssembler* GetAssembler() override;
	BaseEventManager* GetEventManager() override;

	PceConsole* GetConsole() { return _console; }

	BaseState& GetState() override;
	void GetPpuState(BaseState& state) override;
	void SetPpuState(BaseState& state) override;
};