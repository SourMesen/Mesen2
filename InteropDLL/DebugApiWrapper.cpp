#include "stdafx.h"
#include "Core/Shared/Emulator.h"
#include "Core/Debugger/Debugger.h"
#include "Core/Debugger/MemoryDumper.h"
#include "Core/Debugger/MemoryAccessCounter.h"
#include "Core/Debugger/Disassembler.h"
#include "Core/Debugger/DebugTypes.h"
#include "Core/Debugger/Breakpoint.h"
#include "Core/Debugger/BreakpointManager.h"
#include "Core/Debugger/PpuTools.h"
#include "Core/Debugger/CodeDataLogger.h"
#include "Core/Debugger/CallstackManager.h"
#include "Core/Debugger/LabelManager.h"
#include "Core/Debugger/ScriptManager.h"
#include "Core/Debugger/Profiler.h"
#include "Core/Debugger/IAssembler.h"
#include "Core/Debugger/BaseEventManager.h"
#include "Core/Debugger/ITraceLogger.h"
#include "Core/Debugger/TraceLogFileSaver.h"
#include "Core/Gameboy/GbTypes.h"

extern unique_ptr<Emulator> _emu;
static string _logString;

template<typename T>
T WrapDebuggerCall(std::function<T(Debugger* debugger)> func)
{
	Emulator::DebuggerRequest dbgRequest = _emu->GetDebugger(true);
	if(dbgRequest.GetDebugger()) {
		return func(dbgRequest.GetDebugger());
	} else {
		return {};
	}
}

template<>
void WrapDebuggerCall(std::function<void(Debugger* debugger)> func)
{
	Emulator::DebuggerRequest dbgRequest = _emu->GetDebugger(true);
	if(dbgRequest.GetDebugger()) {
		func(dbgRequest.GetDebugger());
	}
}

#define WithDebugger(t, x) WrapDebuggerCall<t>([&](Debugger* dbg) { return dbg->x; });

extern "C"
{
	//Debugger wrapper
	DllExport void __stdcall InitializeDebugger()
	{
		_emu->InitDebugger();
	}

	DllExport void __stdcall ReleaseDebugger()
	{
		_emu->StopDebugger();
	}

	DllExport bool __stdcall IsDebuggerRunning()
	{
		return _emu->GetDebugger().GetDebugger() != nullptr;
	}

	DllExport bool __stdcall IsExecutionStopped() { return WithDebugger(bool, IsExecutionStopped()); }
	DllExport void __stdcall ResumeExecution() { if(IsDebuggerRunning()) WithDebugger(void, Run()); }
	DllExport void __stdcall Step(CpuType cpuType, uint32_t count, StepType type) { WithDebugger(void, Step(cpuType, count, type)); }

	DllExport uint32_t __stdcall GetDisassemblyOutput(CpuType type, uint32_t lineIndex, CodeLineData output[], uint32_t rowCount) { return WithDebugger(uint32_t, GetDisassembler()->GetDisassemblyOutput(type, lineIndex, output, rowCount)); }
	DllExport uint32_t __stdcall GetDisassemblyRowAddress(CpuType type, uint32_t address, int32_t rowOffset) { return WithDebugger(uint32_t, GetDisassembler()->GetDisassemblyRowAddress(type, address, rowOffset)); }
	DllExport int32_t __stdcall SearchDisassembly(CpuType type, const char* searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards) { return WithDebugger(int32_t, GetDisassembler()->SearchDisassembly(type, searchString, startPosition, endPosition, searchBackwards)); }

	DllExport void __stdcall SetTraceOptions(CpuType type, TraceLoggerOptions options) { WithDebugger(void, GetTraceLogger(type)->SetOptions(options)); }
	DllExport uint32_t __stdcall GetExecutionTrace(TraceRow output[], uint32_t startOffset, uint32_t lineCount) { return WithDebugger(uint32_t, GetExecutionTrace(output, startOffset, lineCount)); }

	DllExport void __stdcall StartLogTraceToFile(const char* filename) { WithDebugger(void, GetTraceLogFileSaver()->StartLogging(filename)); }
	DllExport void __stdcall StopLogTraceToFile() { WithDebugger(void, GetTraceLogFileSaver()->StopLogging()); }

	DllExport void __stdcall SetBreakpoints(Breakpoint breakpoints[], uint32_t length) { WithDebugger(void, SetBreakpoints(breakpoints, length)); }
	DllExport int32_t __stdcall EvaluateExpression(const char* expression, CpuType cpuType, EvalResultType *resultType, bool useCache) { return WithDebugger(int32_t, EvaluateExpression(expression, cpuType, *resultType, useCache)); }
	DllExport void __stdcall GetCallstack(CpuType cpuType, StackFrameInfo *callstackArray, uint32_t &callstackSize) { WithDebugger(void, GetCallstackManager(cpuType)->GetCallstack(callstackArray, callstackSize)); }
	DllExport void __stdcall GetProfilerData(CpuType cpuType, ProfiledFunction* profilerData, uint32_t& functionCount) { WithDebugger(void, GetCallstackManager(cpuType)->GetProfiler()->GetProfilerData(profilerData, functionCount)); }
	DllExport void __stdcall ResetProfiler(CpuType cpuType) { WithDebugger(void, GetCallstackManager(cpuType)->GetProfiler()->Reset()); }

	DllExport void __stdcall GetConsoleState(BaseState& state, ConsoleType consoleType) { WithDebugger(void, GetConsoleState(state, consoleType)); }
	DllExport void __stdcall GetCpuState(BaseState& state, CpuType cpuType) { WithDebugger(void, GetCpuState(state, cpuType)); }
	DllExport void __stdcall GetPpuState(BaseState& state, CpuType cpuType) { WithDebugger(void, GetPpuState(state, cpuType)); }
	
	DllExport void __stdcall SetCpuState(BaseState& state, CpuType cpuType) { WithDebugger(void, SetCpuState(state, cpuType)); }
	DllExport void __stdcall SetPpuState(BaseState& state, CpuType cpuType) { WithDebugger(void, SetPpuState(state, cpuType)); }

	DllExport const char* __stdcall GetDebuggerLog()
	{
		_logString = WithDebugger(string, GetLog());
		return _logString.c_str();
	}

	DllExport void __stdcall SetMemoryState(MemoryType type, uint8_t *buffer, int32_t length) { WithDebugger(void, GetMemoryDumper()->SetMemoryState(type, buffer, length)); }
	DllExport uint32_t __stdcall GetMemorySize(MemoryType type) { return WithDebugger(uint32_t, GetMemoryDumper()->GetMemorySize(type)); }
	DllExport void __stdcall GetMemoryState(MemoryType type, uint8_t *buffer) { WithDebugger(void, GetMemoryDumper()->GetMemoryState(type, buffer)); }
	DllExport uint8_t __stdcall GetMemoryValue(MemoryType type, uint32_t address) { return WithDebugger(uint8_t, GetMemoryDumper()->GetMemoryValue(type, address)); }
	DllExport void __stdcall GetMemoryValues(MemoryType type, uint32_t start, uint32_t end, uint8_t* output) { return WithDebugger(void, GetMemoryDumper()->GetMemoryValues(type, start, end, output)); }
	DllExport void __stdcall SetMemoryValue(MemoryType type, uint32_t address, uint8_t value) { return WithDebugger(void, GetMemoryDumper()->SetMemoryValue(type, address, value)); }
	DllExport void __stdcall SetMemoryValues(MemoryType type, uint32_t address, uint8_t* data, int32_t length) { return WithDebugger(void, GetMemoryDumper()->SetMemoryValues(type, address, data, length)); }

	DllExport AddressInfo __stdcall GetAbsoluteAddress(AddressInfo relAddress) { return WithDebugger(AddressInfo, GetAbsoluteAddress(relAddress)); }
	DllExport AddressInfo __stdcall GetRelativeAddress(AddressInfo absAddress, CpuType cpuType) { return WithDebugger(AddressInfo, GetRelativeAddress(absAddress, cpuType)); }

	DllExport void __stdcall SetLabel(uint32_t address, MemoryType memType, char* label, char* comment) { WithDebugger(void, GetLabelManager()->SetLabel(address, memType, label, comment)); }
	DllExport void __stdcall ClearLabels() { WithDebugger(void, GetLabelManager()->ClearLabels()); }

	DllExport void __stdcall ResetMemoryAccessCounts() { WithDebugger(void, GetMemoryAccessCounter()->ResetCounts()); }
	DllExport void __stdcall GetMemoryAccessCounts(uint32_t offset, uint32_t length, MemoryType memoryType, AddressCounters* counts) { WithDebugger(void, GetMemoryAccessCounter()->GetAccessCounts(offset, length, memoryType, counts)); }
	
	DllExport void __stdcall GetCdlData(uint32_t offset, uint32_t length, MemoryType memoryType, uint8_t* cdlData) { WithDebugger(void, GetCdlData(offset, length, memoryType, cdlData)); }
	DllExport void __stdcall SetCdlData(CpuType cpuType, uint8_t* cdlData, uint32_t length) { WithDebugger(void, SetCdlData(cpuType, cdlData, length)); }
	DllExport void __stdcall MarkBytesAs(CpuType cpuType, uint32_t start, uint32_t end, uint8_t flags) { WithDebugger(void, MarkBytesAs(cpuType, start, end, flags)); }
	
	DllExport void __stdcall GetTileView(CpuType cpuType, GetTileViewOptions options, uint8_t *source, uint32_t srcSize, uint32_t *colors, uint32_t *buffer) { WithDebugger(void, GetPpuTools(cpuType)->GetTileView(options, source, srcSize, colors, buffer)); }
	
	DllExport DebugTilemapInfo __stdcall GetTilemap(CpuType cpuType, GetTilemapOptions options, BaseState& state, uint8_t *vram, uint32_t* palette, uint32_t *outputBuffer) { return WithDebugger(DebugTilemapInfo, GetPpuTools(cpuType)->GetTilemap(options, state, vram, palette, outputBuffer)); }
	DllExport FrameInfo __stdcall GetTilemapSize(CpuType cpuType, GetTilemapOptions options, BaseState& state) { return WithDebugger(FrameInfo, GetPpuTools(cpuType)->GetTilemapSize(options, state)); }
	DllExport DebugTilemapTileInfo __stdcall GetTilemapTileInfo(uint32_t x, uint32_t y, CpuType cpuType, GetTilemapOptions options, uint8_t* vram, BaseState& state) { return WithDebugger(DebugTilemapTileInfo, GetPpuTools(cpuType)->GetTilemapTileInfo(x, y, vram, options, state)); }
		
	DllExport DebugSpritePreviewInfo __stdcall GetSpritePreviewInfo(CpuType cpuType, GetSpritePreviewOptions options, BaseState& state) { return WithDebugger(DebugSpritePreviewInfo, GetPpuTools(cpuType)->GetSpritePreviewInfo(options, state)); }
	DllExport void __stdcall GetSpritePreview(CpuType cpuType, GetSpritePreviewOptions options, BaseState& state, uint8_t* vram, uint8_t *oamRam, uint32_t* palette, uint32_t *buffer) { WithDebugger(void, GetPpuTools(cpuType)->GetSpritePreview(options, state, vram, oamRam, palette, buffer)); }
	DllExport void __stdcall GetSpriteList(CpuType cpuType, GetSpritePreviewOptions options, BaseState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo sprites[]) { WithDebugger(void, GetPpuTools(cpuType)->GetSpriteList(options, state, vram, oamRam, palette, sprites)); }

	DllExport DebugPaletteInfo __stdcall GetPaletteInfo(CpuType cpuType) { return WithDebugger(DebugPaletteInfo, GetPpuTools(cpuType)->GetPaletteInfo()); }

	DllExport void __stdcall SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle, CpuType cpuType) { WithDebugger(void, GetPpuTools(cpuType)->SetViewerUpdateTiming(viewerId, scanline, cycle)); }

	DllExport void __stdcall SetEventViewerConfig(CpuType cpuType, BaseEventViewerConfig& config) { WithDebugger(void, GetEventManager(cpuType)->SetConfiguration(config)); }
	DllExport void __stdcall GetDebugEvents(CpuType cpuType, DebugEventInfo *infoArray, uint32_t &maxEventCount) { WithDebugger(void, GetEventManager(cpuType)->GetEvents(infoArray, maxEventCount)); }
	DllExport uint32_t __stdcall GetDebugEventCount(CpuType cpuType) { return WithDebugger(uint32_t, GetEventManager(cpuType)->GetEventCount()); }
	DllExport FrameInfo __stdcall GetEventViewerDisplaySize(CpuType cpuType) { return WithDebugger(FrameInfo, GetEventManager(cpuType)->GetDisplayBufferSize()); }
	DllExport void __stdcall GetEventViewerOutput(CpuType cpuType, uint32_t *buffer, uint32_t bufferSize) { WithDebugger(void, GetEventManager(cpuType)->GetDisplayBuffer(buffer, bufferSize)); }
	DllExport DebugEventInfo __stdcall GetEventViewerEvent(CpuType cpuType, uint16_t scanline, uint16_t cycle) { return WithDebugger(DebugEventInfo, GetEventManager(cpuType)->GetEvent(scanline, cycle)); }
	DllExport uint32_t __stdcall TakeEventSnapshot(CpuType cpuType) { return WithDebugger(uint32_t, GetEventManager(cpuType)->TakeEventSnapshot()); }

	DllExport int32_t __stdcall LoadScript(char* name, char* content, int32_t scriptId) { return WithDebugger(int32_t, GetScriptManager()->LoadScript(name, content, scriptId)); }
	DllExport void __stdcall RemoveScript(int32_t scriptId) { WithDebugger(void, GetScriptManager()->RemoveScript(scriptId)); }
	DllExport const char* __stdcall GetScriptLog(int32_t scriptId) { return WithDebugger(const char*, GetScriptManager()->GetScriptLog(scriptId)); }
	//DllExport void __stdcall DebugSetScriptTimeout(uint32_t timeout) { LuaScriptingContext::SetScriptTimeout(timeout); }

	DllExport uint32_t __stdcall AssembleCode(CpuType cpuType, char* code, uint32_t startAddress, int16_t* assembledOutput) { return WithDebugger(uint32_t, GetAssembler(cpuType)->AssembleCode(code, startAddress, assembledOutput)); }
	
	DllExport void __stdcall SaveRomToDisk(char* filename, bool saveIpsFile, CdlStripOption cdlStripOption) { WithDebugger(void, SaveRomToDisk(filename, saveIpsFile, cdlStripOption)); }
};