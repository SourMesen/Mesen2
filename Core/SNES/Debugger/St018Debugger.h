#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"

class Disassembler;
class Debugger;
class St018;
class ArmV3Cpu;
class St018TraceLogger;
class SnesConsole;
class CallstackManager;
class BreakpointManager;
class MemoryAccessCounter;
class EmuSettings;
class Emulator;
class DummyArmV3Cpu;

enum class MemoryOperationType;

class St018Debugger final : public IDebugger
{
	Debugger* _debugger;
	Emulator* _emu;
	St018* _st018;
	ArmV3Cpu* _cpu;
	Disassembler* _disassembler;
	MemoryAccessCounter* _memoryAccessCounter;
	SnesConsole* _console;
	EmuSettings* _settings;

	unique_ptr<CallstackManager> _callstackManager;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<St018TraceLogger> _traceLogger;
	unique_ptr<DummyArmV3Cpu> _dummyCpu;

	uint32_t _prevOpCode = 0;
	uint32_t _prevProgramCounter = 0;
	uint8_t _prevFlags = 0;

	__forceinline void ProcessCallStackUpdates(AddressInfo& destAddr, uint32_t destPc);
	template<uint8_t accessWidth> void ProcessInstruction();

public:
	St018Debugger(Debugger* debugger);
	~St018Debugger();

	void Reset() override;

	void ProcessInstruction();
	template<uint8_t accessWidth> void ProcessRead(uint32_t addr, uint32_t value, MemoryOperationType type);
	template<uint8_t accessWidth> void ProcessWrite(uint32_t addr, uint32_t value, MemoryOperationType type);

	void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi) override;

	void Run() override;
	void Step(int32_t stepCount, StepType type) override;

	void SetProgramCounter(uint32_t addr, bool updateDebuggerOnly = false) override;
	uint32_t GetProgramCounter(bool getInstPc) override;
	uint64_t GetCpuCycleCount(bool forProfiler = false) override;
	void ResetPrevOpCode() override;

	DebuggerFeatures GetSupportedFeatures() override;

	CallstackManager* GetCallstackManager() override;
	BreakpointManager* GetBreakpointManager() override;
	ITraceLogger* GetTraceLogger() override;
	IAssembler* GetAssembler() override;
	BaseEventManager* GetEventManager() override;

	BaseState& GetState() override;
};