#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"

class Disassembler;
class Debugger;
class GbTraceLogger;
class Gameboy;
class CallstackManager;
class MemoryAccessCounter;
class SnesConsole;
class BreakpointManager;
class EmuSettings;
class GbEventManager;
class GbAssembler;
class Emulator;
class CodeDataLogger;
class GbCpu;
class GbPpu;
class GbPpuTools;
class DummyGbCpu;

enum class MemoryOperationType;

class GbDebugger final : public IDebugger
{
	Debugger* _debugger;
	Emulator* _emu;
	GbCpu* _cpu;
	GbPpu* _ppu;
	Disassembler* _disassembler;
	MemoryAccessCounter* _memoryAccessCounter;
	Gameboy* _gameboy;
	EmuSettings* _settings;

	unique_ptr<GbEventManager> _eventManager;
	unique_ptr<CallstackManager> _callstackManager;
	unique_ptr<CodeDataLogger> _codeDataLogger;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<GbAssembler> _assembler;
	unique_ptr<GbTraceLogger> _traceLogger;
	unique_ptr<GbPpuTools> _ppuTools;
	unique_ptr<DummyGbCpu> _dummyCpu;

	uint8_t _prevOpCode = 0xFF;
	uint16_t _prevStackPointer = 0;
	uint32_t _prevProgramCounter = 0;

	string _cdlFile;

	__forceinline void ProcessCallStackUpdates(AddressInfo& destAddr, uint16_t destPc, uint16_t sp);

public:
	GbDebugger(Debugger* debugger);
	~GbDebugger();

	void Reset() override;

	void ProcessInstruction();
	void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi) override;
	void ProcessPpuRead(uint16_t addr, uint8_t value, MemoryType memoryType);
	void ProcessPpuWrite(uint16_t addr, uint8_t value, MemoryType memoryType);
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