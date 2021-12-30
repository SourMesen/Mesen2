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

extern shared_ptr<Emulator> _emu;
static string _logString;

shared_ptr<Debugger> GetDebugger()
{
	return _emu->GetDebugger();
}

extern "C"
{
	//Debugger wrapper
	DllExport void __stdcall InitializeDebugger()
	{
		GetDebugger();
	}

	DllExport void __stdcall ReleaseDebugger()
	{
		_emu->StopDebugger();
	}

	DllExport bool __stdcall IsDebuggerRunning()
	{
		return _emu->GetDebugger(false).get() != nullptr;
	}

	DllExport bool __stdcall IsExecutionStopped() { return GetDebugger()->IsExecutionStopped(); }
	DllExport void __stdcall ResumeExecution() { if(IsDebuggerRunning()) GetDebugger()->Run(); }
	DllExport void __stdcall Step(CpuType cpuType, uint32_t count, StepType type) { GetDebugger()->Step(cpuType, count, type); }

	DllExport uint32_t __stdcall GetDisassemblyOutput(CpuType type, uint32_t lineIndex, CodeLineData output[], uint32_t rowCount) { return GetDebugger()->GetDisassembler()->GetDisassemblyOutput(type, lineIndex, output, rowCount); }
	DllExport int32_t __stdcall SearchDisassembly(CpuType type, const char* searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards) { return GetDebugger()->GetDisassembler()->SearchDisassembly(type, searchString, startPosition, endPosition, searchBackwards); }

	DllExport void __stdcall SetTraceOptions(CpuType type, TraceLoggerOptions options) { GetDebugger()->GetTraceLogger(type)->SetOptions(options); }
	DllExport uint32_t __stdcall GetExecutionTrace(TraceRow output[], uint32_t startOffset, uint32_t lineCount) { return GetDebugger()->GetExecutionTrace(output, startOffset, lineCount); }

	DllExport void __stdcall StartLogTraceToFile(const char* filename) { GetDebugger()->GetTraceLogFileSaver()->StartLogging(filename); }
	DllExport void __stdcall StopLogTraceToFile() { GetDebugger()->GetTraceLogFileSaver()->StopLogging(); }

	DllExport void __stdcall SetBreakpoints(Breakpoint breakpoints[], uint32_t length) { GetDebugger()->SetBreakpoints(breakpoints, length); }
	DllExport int32_t __stdcall EvaluateExpression(const char* expression, CpuType cpuType, EvalResultType *resultType, bool useCache) { return GetDebugger()->EvaluateExpression(expression, cpuType, *resultType, useCache); }
	DllExport void __stdcall GetCallstack(CpuType cpuType, StackFrameInfo *callstackArray, uint32_t &callstackSize) { GetDebugger()->GetCallstackManager(cpuType)->GetCallstack(callstackArray, callstackSize); }
	DllExport void __stdcall GetProfilerData(CpuType cpuType, ProfiledFunction* profilerData, uint32_t& functionCount) { GetDebugger()->GetCallstackManager(cpuType)->GetProfiler()->GetProfilerData(profilerData, functionCount); }
	DllExport void __stdcall ResetProfiler(CpuType cpuType) { GetDebugger()->GetCallstackManager(cpuType)->GetProfiler()->Reset(); }

	DllExport void __stdcall GetConsoleState(BaseState& state, ConsoleType consoleType) { GetDebugger()->GetConsoleState(state, consoleType); }
	DllExport void __stdcall GetCpuState(BaseState& state, CpuType cpuType) { GetDebugger()->GetCpuState(state, cpuType); }
	DllExport void __stdcall GetPpuState(BaseState& state, CpuType cpuType) { GetDebugger()->GetPpuState(state, cpuType); }
	
	DllExport const char* __stdcall GetDebuggerLog()
	{
		_logString = GetDebugger()->GetLog();
		return _logString.c_str();
	}

	DllExport void __stdcall SetMemoryState(SnesMemoryType type, uint8_t *buffer, int32_t length) { GetDebugger()->GetMemoryDumper()->SetMemoryState(type, buffer, length); }
	DllExport uint32_t __stdcall GetMemorySize(SnesMemoryType type) { return GetDebugger()->GetMemoryDumper()->GetMemorySize(type); }
	DllExport void __stdcall GetMemoryState(SnesMemoryType type, uint8_t *buffer) { GetDebugger()->GetMemoryDumper()->GetMemoryState(type, buffer); }
	DllExport uint8_t __stdcall GetMemoryValue(SnesMemoryType type, uint32_t address) { return GetDebugger()->GetMemoryDumper()->GetMemoryValue(type, address); }
	DllExport void __stdcall GetMemoryValues(SnesMemoryType type, uint32_t start, uint32_t end, uint8_t* output) { return GetDebugger()->GetMemoryDumper()->GetMemoryValues(type, start, end, output); }
	DllExport void __stdcall SetMemoryValue(SnesMemoryType type, uint32_t address, uint8_t value) { return GetDebugger()->GetMemoryDumper()->SetMemoryValue(type, address, value); }
	DllExport void __stdcall SetMemoryValues(SnesMemoryType type, uint32_t address, uint8_t* data, int32_t length) { return GetDebugger()->GetMemoryDumper()->SetMemoryValues(type, address, data, length); }

	DllExport AddressInfo __stdcall GetAbsoluteAddress(AddressInfo relAddress) { return GetDebugger()->GetAbsoluteAddress(relAddress); }
	DllExport AddressInfo __stdcall GetRelativeAddress(AddressInfo absAddress, CpuType cpuType) { return GetDebugger()->GetRelativeAddress(absAddress, cpuType); }

	DllExport void __stdcall SetLabel(uint32_t address, SnesMemoryType memType, char* label, char* comment) { GetDebugger()->GetLabelManager()->SetLabel(address, memType, label, comment); }
	DllExport void __stdcall ClearLabels() { GetDebugger()->GetLabelManager()->ClearLabels(); }

	DllExport void __stdcall ResetMemoryAccessCounts() { GetDebugger()->GetMemoryAccessCounter()->ResetCounts(); }
	DllExport void __stdcall GetMemoryAccessCounts(uint32_t offset, uint32_t length, SnesMemoryType memoryType, AddressCounters* counts) { GetDebugger()->GetMemoryAccessCounter()->GetAccessCounts(offset, length, memoryType, counts); }
	
	DllExport void __stdcall GetCdlData(uint32_t offset, uint32_t length, SnesMemoryType memoryType, uint8_t* cdlData) { GetDebugger()->GetCdlData(offset, length, memoryType, cdlData); }
	DllExport void __stdcall SetCdlData(CpuType cpuType, uint8_t* cdlData, uint32_t length) { GetDebugger()->SetCdlData(cpuType, cdlData, length); }
	DllExport void __stdcall MarkBytesAs(CpuType cpuType, uint32_t start, uint32_t end, uint8_t flags) { GetDebugger()->MarkBytesAs(cpuType, start, end, flags); }
	
	DllExport void __stdcall GetTileView(CpuType cpuType, GetTileViewOptions options, uint8_t *source, uint32_t srcSize, uint32_t *colors, uint32_t *buffer) { GetDebugger()->GetPpuTools(cpuType)->GetTileView(options, source, srcSize, colors, buffer); }
	
	DllExport DebugTilemapInfo __stdcall GetTilemap(CpuType cpuType, GetTilemapOptions options, BaseState& state, uint8_t *vram, uint32_t* palette, uint32_t *outputBuffer) { return GetDebugger()->GetPpuTools(cpuType)->GetTilemap(options, state, vram, palette, outputBuffer); }
	DllExport FrameInfo __stdcall GetTilemapSize(CpuType cpuType, GetTilemapOptions options, BaseState& state) { return GetDebugger()->GetPpuTools(cpuType)->GetTilemapSize(options, state); }
	DllExport DebugTilemapTileInfo __stdcall GetTilemapTileInfo(uint32_t x, uint32_t y, CpuType cpuType, GetTilemapOptions options, uint8_t* vram, BaseState& state) { return GetDebugger()->GetPpuTools(cpuType)->GetTilemapTileInfo(x, y, vram, options, state); }
		
	DllExport DebugSpritePreviewInfo __stdcall GetSpritePreviewInfo(CpuType cpuType, GetSpritePreviewOptions options, BaseState& state) { return GetDebugger()->GetPpuTools(cpuType)->GetSpritePreviewInfo(options, state); }
	DllExport void __stdcall GetSpritePreview(CpuType cpuType, GetSpritePreviewOptions options, BaseState& state, uint8_t* vram, uint8_t *oamRam, uint32_t* palette, uint32_t *buffer) { GetDebugger()->GetPpuTools(cpuType)->GetSpritePreview(options, state, vram, oamRam, palette, buffer); }
	DllExport void __stdcall GetSpriteList(CpuType cpuType, GetSpritePreviewOptions options, BaseState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo sprites[]) { GetDebugger()->GetPpuTools(cpuType)->GetSpriteList(options, state, vram, oamRam, palette, sprites); }

	DllExport void __stdcall SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle, CpuType cpuType) { GetDebugger()->GetPpuTools(cpuType)->SetViewerUpdateTiming(viewerId, scanline, cycle); }

	DllExport void __stdcall SetEventViewerConfig(CpuType cpuType, BaseEventViewerConfig& config) { GetDebugger()->GetEventManager(cpuType)->SetConfiguration(config); }
	DllExport void __stdcall GetDebugEvents(CpuType cpuType, DebugEventInfo *infoArray, uint32_t &maxEventCount) { GetDebugger()->GetEventManager(cpuType)->GetEvents(infoArray, maxEventCount); }
	DllExport uint32_t __stdcall GetDebugEventCount(CpuType cpuType) { return GetDebugger()->GetEventManager(cpuType)->GetEventCount(); }
	DllExport FrameInfo __stdcall GetEventViewerDisplaySize(CpuType cpuType) { return GetDebugger()->GetEventManager(cpuType)->GetDisplayBufferSize(); }
	DllExport void __stdcall GetEventViewerOutput(CpuType cpuType, uint32_t *buffer, uint32_t bufferSize) { GetDebugger()->GetEventManager(cpuType)->GetDisplayBuffer(buffer, bufferSize); }
	DllExport void __stdcall GetEventViewerEvent(CpuType cpuType, DebugEventInfo *evtInfo, uint16_t scanline, uint16_t cycle) { *evtInfo = GetDebugger()->GetEventManager(cpuType)->GetEvent(scanline, cycle); }
	DllExport uint32_t __stdcall TakeEventSnapshot(CpuType cpuType) { return GetDebugger()->GetEventManager(cpuType)->TakeEventSnapshot(); }

	DllExport int32_t __stdcall LoadScript(char* name, char* content, int32_t scriptId) { return GetDebugger()->GetScriptManager()->LoadScript(name, content, scriptId); }
	DllExport void __stdcall RemoveScript(int32_t scriptId) { GetDebugger()->GetScriptManager()->RemoveScript(scriptId); }
	DllExport const char* __stdcall GetScriptLog(int32_t scriptId) { return GetDebugger()->GetScriptManager()->GetScriptLog(scriptId); }
	//DllExport void __stdcall DebugSetScriptTimeout(uint32_t timeout) { LuaScriptingContext::SetScriptTimeout(timeout); }

	DllExport uint32_t __stdcall AssembleCode(CpuType cpuType, char* code, uint32_t startAddress, int16_t* assembledOutput) { return GetDebugger()->GetAssembler(cpuType)->AssembleCode(code, startAddress, assembledOutput); }
	
	DllExport void __stdcall SaveRomToDisk(char* filename, bool saveIpsFile, CdlStripOption cdlStripOption) { GetDebugger()->SaveRomToDisk(filename, saveIpsFile, cdlStripOption); }
};