#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"

class Disassembler;
class Debugger;
class SmsTraceLogger;
class SmsConsole;
class SmsMemoryManager;
class CallstackManager;
class MemoryAccessCounter;
class BreakpointManager;
class EmuSettings;
class SmsAssembler;
class Emulator;
class CodeDataLogger;
class SmsCpu;
class SmsVdp;
class SmsVdpTools;
class SmsEventManager;
class DummySmsCpu;

enum class MemoryOperationType;

class SmsDebugger final : public IDebugger
{
	Debugger* _debugger;
	Emulator* _emu;
	SmsCpu* _cpu;
	SmsVdp* _vdp;
	SmsMemoryManager* _memoryManager;
	Disassembler* _disassembler;
	MemoryAccessCounter* _memoryAccessCounter;
	SmsConsole* _console;
	EmuSettings* _settings;

	unique_ptr<SmsEventManager> _eventManager;
	unique_ptr<SmsVdpTools> _ppuTools;
	unique_ptr<CallstackManager> _callstackManager;
	unique_ptr<CodeDataLogger> _codeDataLogger;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<SmsAssembler> _assembler;
	unique_ptr<SmsTraceLogger> _traceLogger;
	unique_ptr<DummySmsCpu> _dummyCpu;

	uint16_t _prevOpCode = 0xFF;
	uint16_t _prevStackPointer = 0;
	uint32_t _prevProgramCounter = 0;

	string _cdlFile;

	__forceinline uint8_t GetPrevOpCodeSize();
	__forceinline void ProcessCallStackUpdates(AddressInfo& destAddr, uint16_t destPc, uint16_t sp);

public:
	SmsDebugger(Debugger* debugger);
	~SmsDebugger();

	void Reset() override;

	void ProcessInstruction();
	void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type);

	template<MemoryOperationType opType>
	void ProcessMemoryAccess(uint32_t addr, uint8_t value, MemoryType memType);

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
