#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"

class MemoryDumper;
class Disassembler;
class Debugger;
class WsTraceLogger;
class WsConsole;
class WsMemoryManager;
class CallstackManager;
class MemoryAccessCounter;
class BreakpointManager;
class EmuSettings;
class WsAssembler;
class Emulator;
class CodeDataLogger;
class DummyWsCpu;
class WsCpu;
class WsPpu;
class WsPpuTools;
class WsEventManager;
class DummyWsCpu;

enum class MemoryOperationType;

class WsDebugger final : public IDebugger
{
	Debugger* _debugger = nullptr;
	MemoryDumper* _memoryDumper = nullptr;
	Emulator* _emu = nullptr;
	WsCpu* _cpu = nullptr;
	WsPpu* _ppu = nullptr;
	WsMemoryManager* _memoryManager = nullptr;
	Disassembler* _disassembler = nullptr;
	MemoryAccessCounter* _memoryAccessCounter = nullptr;
	WsConsole* _console = nullptr;
	EmuSettings* _settings = nullptr;

	unique_ptr<WsEventManager> _eventManager;
	unique_ptr<WsPpuTools> _ppuTools;
	unique_ptr<CallstackManager> _callstackManager;
	unique_ptr<CodeDataLogger> _codeDataLogger;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<WsAssembler> _assembler;
	unique_ptr<WsTraceLogger> _traceLogger;
	unique_ptr<DummyWsCpu> _dummyCpu;

	uint16_t _prevOpCode = 0;
	uint16_t _prevStackPointer = 0;
	uint32_t _prevProgramCounter = 0;

	string _cdlFile;

	__forceinline uint8_t GetPrevOpCodeSize();
	__forceinline void ProcessCallStackUpdates(AddressInfo& destAddr, uint32_t destPc, uint16_t sp);

public:
	WsDebugger(Debugger* debugger);
	~WsDebugger();

	void Reset() override;

	void ProcessInstruction();
	
	template<uint8_t accessWidth> void ProcessRead(uint32_t addr, uint16_t value, MemoryOperationType type);
	template<uint8_t accessWidth> void ProcessWrite(uint32_t addr, uint16_t value, MemoryOperationType type);

	template<MemoryOperationType opType, typename T>
	void ProcessMemoryAccess(uint32_t addr, T value, MemoryType memType);

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
	uint64_t GetCpuCycleCount(bool forProfiler) override;
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
