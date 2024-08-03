#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"

class Disassembler;
class Debugger;
class GbaTraceLogger;
class GbaConsole;
class CallstackManager;
class MemoryAccessCounter;
class BreakpointManager;
class EmuSettings;
class GbaEventManager;
class GbaAssembler;
class Emulator;
class CodeDataLogger;
class GbaCpu;
class GbaPpu;
class GbaMemoryManager;
class GbaPpuTools;
class DummyGbaCpu;

enum class MemoryOperationType;

class GbaDebugger final : public IDebugger
{
	Debugger* _debugger;
	Emulator* _emu;
	GbaCpu* _cpu;
	GbaPpu* _ppu;
	GbaMemoryManager* _memoryManager;
	Disassembler* _disassembler;
	MemoryAccessCounter* _memoryAccessCounter;
	GbaConsole* _console;
	EmuSettings* _settings;

	unique_ptr<GbaEventManager> _eventManager;
	unique_ptr<CallstackManager> _callstackManager;
	unique_ptr<CodeDataLogger> _codeDataLogger;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<GbaAssembler> _assembler;
	unique_ptr<GbaTraceLogger> _traceLogger;
	unique_ptr<GbaPpuTools> _ppuTools;
	unique_ptr<DummyGbaCpu> _dummyCpu;

	uint32_t _prevOpCode = 0;
	uint32_t _prevProgramCounter = 0;
	uint8_t _prevFlags = 0;

	string _cdlFile;

	__forceinline void ProcessCallStackUpdates(AddressInfo& destAddr, uint32_t destPc);
	template<uint8_t accessWidth> void ProcessInstruction();

public:
	GbaDebugger(Debugger* debugger);
	~GbaDebugger();

	void Reset() override;

	void ProcessInstruction();
	template<uint8_t accessWidth> void ProcessRead(uint32_t addr, uint32_t value, MemoryOperationType type);
	template<uint8_t accessWidth> void ProcessWrite(uint32_t addr, uint32_t value, MemoryOperationType type);

	void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi) override;
	void ProcessPpuCycle();

	void Run() override;
	void Step(int32_t stepCount, StepType type) override;
	StepBackConfig GetStepBackConfig() override;

	void DrawPartialFrame() override;

	bool SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption);

	void ProcessInputOverrides(DebugControllerState inputOverrides[8]) override;

	void SetProgramCounter(uint32_t addr, bool updateDebuggerOnly = false) override;
	uint32_t GetProgramCounter(bool getInstPc) override;
	uint64_t GetCpuCycleCount(bool forProfiler = false) override;
	void ResetPrevOpCode() override;

	uint8_t GetCpuFlags() override;

	DebuggerFeatures GetSupportedFeatures() override;

	BaseEventManager* GetEventManager() override;
	IAssembler* GetAssembler() override;
	CallstackManager* GetCallstackManager() override;
	BreakpointManager* GetBreakpointManager() override;
	ITraceLogger* GetTraceLogger() override;
	PpuTools* GetPpuTools() override;

	BaseState& GetState() override;
	void GetPpuState(BaseState& state) override;
	void SetPpuState(BaseState& state) override;
};