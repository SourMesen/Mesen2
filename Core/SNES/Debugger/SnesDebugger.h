#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"
#include "SNES/SnesCpuTypes.h"

class SnesConsole;
class Emulator;
class Disassembler;
class Debugger;
class SnesCpuTraceLogger;
class SnesCpu;
class CallstackManager;
class MemoryAccessCounter;
class SnesMemoryManager;
class SnesCodeDataLogger;
class EmuSettings;
class ScriptManager;
class BaseEventManager;
class MemoryMappings;
class BreakpointManager;
class Sa1;
class BaseCartridge;
class Spc;
class SnesPpu;
class SnesAssembler;
class SnesPpuTools;
class PpuTools;
class DummySnesCpu;
enum class MemoryOperationType;

class SnesDebugger final : public IDebugger
{
	Emulator* _emu;
	Debugger* _debugger;
	Disassembler* _disassembler;
	MemoryAccessCounter* _memoryAccessCounter;
	SnesMemoryManager* _memoryManager;
	EmuSettings* _settings;
	SnesConsole* _console;
	SnesCpu* _cpu;
	Sa1* _sa1;
	BaseCartridge* _cart;
	Spc* _spc;
	SnesPpu* _ppu;
	MemoryMappings* _memoryMappings;
	SnesCodeDataLogger* _cdl;

	unique_ptr<SnesCodeDataLogger> _codeDataLogger;
	unique_ptr<BaseEventManager> _eventManager;
	unique_ptr<SnesAssembler> _assembler;
	unique_ptr<CallstackManager> _callstackManager;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<StepRequest> _step;
	unique_ptr<SnesCpuTraceLogger> _traceLogger;
	unique_ptr<SnesPpuTools> _ppuTools;
	unique_ptr<DummySnesCpu> _dummyCpu;

	ITraceLogger* _spcTraceLogger = nullptr;
	ITraceLogger* _dspTraceLogger = nullptr;

	CpuType _cpuType;
	bool _enableBreakOnUninitRead = false;
	uint8_t _prevOpCode = 0xFF;
	uint32_t _prevProgramCounter = 0;

	bool _predictiveBreakpoints = false;
	bool _debuggerEnabled = false;
	bool _needCoprocessors = false;
	bool _runCoprocessors = false;
	bool _runSpc = false;

	string _cdlFile;

	SnesCpuState& GetCpuState();
	bool IsRegister(uint32_t addr);

public:
	SnesDebugger(Debugger* debugger, CpuType cpuType);
	~SnesDebugger();

	void Init() override;
	void Reset() override;

	void ProcessConfigChange() override;

	void ProcessInstruction();
	void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi) override;
	void ProcessPpuRead(uint16_t addr, uint8_t value, MemoryType memoryType);
	void ProcessPpuWrite(uint16_t addr, uint8_t value, MemoryType memoryType);
	void ProcessPpuCycle();

	void Run() override;
	void Step(int32_t stepCount, StepType type) override;

	void DrawPartialFrame() override;
	
	DebuggerFeatures GetSupportedFeatures() override;
	void SetProgramCounter(uint32_t addr) override;
	uint32_t GetProgramCounter(bool getInstPc) override;

	ITraceLogger* GetTraceLogger() override;
	BreakpointManager* GetBreakpointManager() override;
	CallstackManager* GetCallstackManager() override;
	IAssembler* GetAssembler() override;
	BaseEventManager* GetEventManager() override;
	PpuTools* GetPpuTools() override;

	BaseState& GetState() override;
	void GetPpuState(BaseState& state) override;
	void SetPpuState(BaseState& state) override;
	void SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption);
};